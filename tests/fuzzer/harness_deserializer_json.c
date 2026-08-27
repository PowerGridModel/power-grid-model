// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// Coverage-guided fuzz harness for the Power Grid Model C API JSON
// deserialization path. It runs under AFL++ by default; libFuzzer works too.
//
// It drives PGM_create_deserializer_from_binary_buffer(..., PGM_json) and the
// downstream model/solver code:
//   deserialize -> inspect dataset -> allocate + register component buffers ->
//   parse into buffers -> buffer get/set round-trips -> create model ->
//   symmetric / asymmetric power flow + short-circuit calculation.
//
// The harness exposes the standard libFuzzer entry points
// (LLVMFuzzerTestOneInput / LLVMFuzzerInitialize); AFL++ drives them via its
// aflpp_driver (libAFLDriver.a), so the same source runs under either engine.
// Build it locally with `cmake ... -DPGM_ENABLE_FUZZER=ON` (AFL++ toolchain by
// default, or -DPGM_FUZZER_ENGINE=libfuzzer with Clang); OSS-Fuzz compiles this
// same source against $LIB_FUZZING_ENGINE.

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>

#include "power_grid_model_c.h"

/* Safety caps — prevent OOM from adversarial element counts in the input */
#define MAX_COMPONENTS 64
#define MAX_ELEMENTS   1000

/* --------------------------------------------------------------------------
 * Helper: aligned_alloc wrapper that guarantees size is a multiple of
 * alignment (required on macOS) and handles zero-element cases.
 * -------------------------------------------------------------------------- */
static void *alloc_aligned(size_t alignment, size_t size) {
    if (alignment == 0) alignment = 1;
    if (size == 0)      size = alignment;
    size = ((size + alignment - 1) / alignment) * alignment;
    return aligned_alloc(alignment, size);
}

/* --------------------------------------------------------------------------
 * Helper: retrieve the error message (exercises handle.cpp message path)
 * then clear.  Call this instead of bare PGM_clear_error() everywhere so
 * PGM_error_message() always appears in the coverage trace.
 * -------------------------------------------------------------------------- */
static void drain_error(PGM_Handle *handle) {
    if (PGM_error_code(handle) != PGM_no_error) {
        (void)PGM_error_message(handle);
        PGM_clear_error(handle);
    }
}

/* --------------------------------------------------------------------------
 * Required lifecycle stubs for libAFLDriver.a / cov_driver.c
 * -------------------------------------------------------------------------- */
int LLVMFuzzerInitialize(int *argc, char ***argv) {
    (void)argc; (void)argv;
    return 0;
}

void LLVMFuzzerCleanup(void) {}

/* --------------------------------------------------------------------------
 * Fuzzer entry point
 * -------------------------------------------------------------------------- */
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < 2)
        return 0;

    const char   *payload     = (const char *)(data);
    const size_t  payload_len = size;

    /* All resources declared upfront so cleanup is unconditional.
     * opt/out_ds/out_buf are now block-scoped in each calculation step. */
    PGM_Handle         *handle      = PGM_create_handle();
    PGM_Deserializer   *deserializer = NULL;
    void               *comp_bufs[MAX_COMPONENTS];
    void               *node_buf    = NULL;  /* points into comp_bufs for the node component */
    PGM_ConstDataset   *const_ds    = NULL;
    PGM_PowerGridModel *model       = NULL;

    memset(comp_bufs, 0, sizeof(comp_bufs));

    /* ── 1. Deserialize ────────────────────────────────────────────────── */

    /* Parse a JSON byte buffer (no null terminator required).
        * Same nlohmann_json path, different entry point. */
    deserializer = PGM_create_deserializer_from_binary_buffer(
        handle, payload, (PGM_Idx)payload_len, PGM_json);

    if (!deserializer) goto cleanup;

    /* ── 2. Inspect dataset ────────────────────────────────────────────── */

    /* Retrieve the writable dataset embedded in the deserializer. */
    PGM_WritableDataset  *writable = PGM_deserializer_get_dataset(handle, deserializer);
    if (!writable) goto cleanup;

    PGM_DatasetInfo const *info = PGM_dataset_writable_get_info(handle, writable);
    if (!info) goto cleanup;

    const char *ds_name = PGM_dataset_info_name(handle, info);
    /* Only "input" datasets can be used with PGM_create_model. */
    if (!ds_name || strcmp(ds_name, "input") != 0) goto cleanup;

    PGM_Idx batch_size   = PGM_dataset_info_batch_size(handle, info);
    PGM_Idx n_components = PGM_dataset_info_n_components(handle, info);

    if (batch_size != 1 || n_components <= 0 || n_components > MAX_COMPONENTS)
        goto cleanup;

    /* ── 3. Allocate and register per-component buffers ───────────────── */
    PGM_Idx node_count = 0;

    for (PGM_Idx i = 0; i < n_components; i++) {
        const char *comp_name = PGM_dataset_info_component_name(handle, info, i);
        PGM_Idx total    = PGM_dataset_info_total_elements(handle, info, i);
        PGM_Idx per_scen = PGM_dataset_info_elements_per_scenario(handle, info, i);

        if (per_scen < 0 || total <= 0 || total > MAX_ELEMENTS)
            goto cleanup;

        if (comp_name && strcmp(comp_name, "node") == 0)
            node_count = total;

        PGM_MetaComponent const *meta = PGM_meta_get_component_by_name(
            handle, "input", comp_name);
        if (!meta) goto cleanup;

        size_t comp_size  = PGM_meta_component_size(handle, meta);
        size_t comp_align = PGM_meta_component_alignment(handle, meta);

        comp_bufs[i] = alloc_aligned(comp_align, comp_size * (size_t)total);
        if (!comp_bufs[i]) goto cleanup;

        /* Track the node buffer — needed later for PGM_get_indexer. */
        if (comp_name && strcmp(comp_name, "node") == 0)
            node_buf = comp_bufs[i];

        PGM_dataset_writable_set_buffer(handle, writable, comp_name, NULL, comp_bufs[i]);
        if (PGM_error_code(handle) != PGM_no_error) {
            drain_error(handle);
            goto cleanup;
        }
    }

    /* ── 4. Parse serialized data into the allocated buffers ──────────── */

    /* Walk the parse tree and write each field value into the component
     * buffers registered in step 3. */
    PGM_deserializer_parse_to_buffer(handle, deserializer);
    if (PGM_error_code(handle) != PGM_no_error) {
        drain_error(handle);
        goto cleanup;
    }

    /* ── 4b. Exercise buffer.cpp: create / nan / get / set / destroy ─────
     * Re-walk the parsed component buffers using the C API's own allocation
     * and attribute I/O so that PGM_create_buffer, PGM_buffer_set_nan,
     * PGM_buffer_get_value, PGM_buffer_set_value, and PGM_destroy_buffer
     * all land in the coverage trace.
     * ──────────────────────────────────────────────────────────────────── */
    for (PGM_Idx i = 0; i < n_components; i++) {
        if (!comp_bufs[i]) continue;

        const char *cname  = PGM_dataset_info_component_name(handle, info, i);
        PGM_Idx     nelems = PGM_dataset_info_total_elements(handle, info, i);
        if (nelems <= 0) continue;

        PGM_MetaComponent const *meta2 =
            PGM_meta_get_component_by_name(handle, "input", cname);
        if (!meta2) continue;

        /* API-managed aligned allocation — exercises PGM_create_buffer. */
        void *api_buf = PGM_create_buffer(handle, meta2, nelems);
        if (!api_buf) { drain_error(handle); continue; }

        /* Fill every attribute slot with NaN — exercises PGM_buffer_set_nan. */
        PGM_buffer_set_nan(handle, meta2, api_buf, 0, nelems);
        if (PGM_error_code(handle) != PGM_no_error) {
            drain_error(handle);
            PGM_destroy_buffer(api_buf);
            continue;
        }

        /* Scratch array sized for the widest possible attribute × nelems.
         * comp_sz (the full struct size) is always >= any single attribute. */
        size_t comp_sz = PGM_meta_component_size(handle, meta2);
        void  *tmp     = malloc(comp_sz * (size_t)nelems);
        if (!tmp) { PGM_destroy_buffer(api_buf); continue; }

        /* Round-trip every attribute: get from parsed buf → set into api_buf
         * → get back. Covers both directions of buffer_get_set_value<is_get>.
         * stride=-1 exercises the negative-stride branch (use attr's own size). */
        PGM_Idx n_attrs = PGM_meta_n_attributes(handle, meta2);
        for (PGM_Idx j = 0; j < n_attrs; j++) {
            PGM_MetaAttribute const *attr =
                PGM_meta_get_attribute_by_idx(handle, meta2, j);
            if (!attr) continue;

            PGM_buffer_get_value(handle, attr, comp_bufs[i], tmp, 0, nelems, -1);
            if (PGM_error_code(handle) != PGM_no_error) { drain_error(handle); continue; }

            PGM_buffer_set_value(handle, attr, api_buf, tmp, 0, nelems, -1);
            if (PGM_error_code(handle) != PGM_no_error) { drain_error(handle); continue; }

            /* Second get covers the path where we read a buffer we filled. */
            PGM_buffer_get_value(handle, attr, api_buf, tmp, 0, nelems, -1);
            if (PGM_error_code(handle) != PGM_no_error) { drain_error(handle); continue; }
        }

        free(tmp);
        /* API-managed aligned free — exercises PGM_destroy_buffer. */
        PGM_destroy_buffer(api_buf);
    }

    /* ── 5. Create model ───────────────────────────────────────────────── */

    const_ds = PGM_create_dataset_const_from_writable(handle, writable);
    if (!const_ds) { drain_error(handle); goto cleanup; }

    model = PGM_create_model(handle, 50.0, const_ds);
    if (!model) { drain_error(handle); goto cleanup; }

    /* ── 5b. Copy model + get_indexer ─────────────────────────────────── */
    do {
        /* PGM_copy_model: exercises the deep-copy constructor path in model.cpp. */
        PGM_PowerGridModel *model_copy = PGM_copy_model(handle, model);
        if (model_copy) {
            PGM_destroy_model(model_copy);
        } else {
            drain_error(handle);
        }

        /* PGM_get_indexer: look up sequence positions for the parsed node IDs.
         * Reads the id field from the node buffer then asks the model to
         * resolve each ID to its internal sequence number. */
        if (node_count <= 0 || !node_buf) break;

        PGM_MetaAttribute const *id_attr =
            PGM_meta_get_attribute_by_name(handle, "input", "node", "id");
        if (!id_attr) break;

        PGM_ID  *node_ids = malloc(sizeof(PGM_ID)  * (size_t)node_count);
        PGM_Idx *node_seq = malloc(sizeof(PGM_Idx) * (size_t)node_count);
        if (node_ids && node_seq) {
            PGM_buffer_get_value(handle, id_attr, node_buf, node_ids, 0, node_count, -1);
            if (PGM_error_code(handle) == PGM_no_error) {
                PGM_get_indexer(handle, model, "node", node_count, node_ids, node_seq);
                drain_error(handle);   /* bad IDs may trigger an error — not a crash */
            } else {
                drain_error(handle);
            }
        }
        free(node_ids);
        free(node_seq);
    } while (0);

    if (node_count <= 0) goto cleanup;

    /* ── 6a. Symmetric power flow — all option setters called ─────────── *
     * Every PGM_set_* function is called here so that options.cpp reaches
     * full line coverage in a single pass.
     * ──────────────────────────────────────────────────────────────────── */
    do {
        PGM_MetaComponent const *meta =
            PGM_meta_get_component_by_name(handle, "sym_output", "node");
        if (!meta) break;

        void *out_buf = alloc_aligned(
            PGM_meta_component_alignment(handle, meta),
            PGM_meta_component_size(handle, meta) * (size_t)node_count);
        if (!out_buf) break;

        PGM_MutableDataset *out_ds = PGM_create_dataset_mutable(handle, "sym_output", 0, 1);
        if (!out_ds) { free(out_buf); break; }

        PGM_dataset_mutable_add_buffer(handle, out_ds, "node", node_count, node_count, NULL, out_buf);

        PGM_Options *opt = PGM_create_options(handle);
        if (opt) {
            PGM_set_calculation_type(handle, opt, PGM_power_flow);
            PGM_set_calculation_method(handle, opt, PGM_newton_raphson);
            PGM_set_symmetric(handle, opt, PGM_symmetric);
            PGM_set_err_tol(handle, opt, 1e-8);
            PGM_set_max_iter(handle, opt, 5);
            PGM_set_threading(handle, opt, -1);
            PGM_set_tap_changing_strategy(handle, opt, PGM_tap_changing_strategy_disabled);
            PGM_set_short_circuit_voltage_scaling(handle, opt,
                PGM_short_circuit_voltage_scaling_maximum);
            PGM_set_experimental_features(handle, opt, PGM_experimental_features_disabled);

            PGM_calculate(handle, model, opt, out_ds, NULL);
            drain_error(handle);   /* divergence, topology issues — not a bug */
            PGM_destroy_options(opt);
        }
        PGM_destroy_dataset_mutable(out_ds);
        free(out_buf);
    } while (0);

    /* ── 6b. Asymmetric power flow ─────────────────────────────────────── *
     * Uses asym_output (double[3] fields) and PGM_asymmetric symmetry flag,
     * exercising the asymmetric dispatch path in job_dispatch and model.cpp.
     * ──────────────────────────────────────────────────────────────────── */
    do {
        PGM_MetaComponent const *meta =
            PGM_meta_get_component_by_name(handle, "asym_output", "node");
        if (!meta) break;

        void *out_buf = alloc_aligned(
            PGM_meta_component_alignment(handle, meta),
            PGM_meta_component_size(handle, meta) * (size_t)node_count);
        if (!out_buf) break;

        PGM_MutableDataset *out_ds = PGM_create_dataset_mutable(handle, "asym_output", 0, 1);
        if (!out_ds) { free(out_buf); break; }

        PGM_dataset_mutable_add_buffer(handle, out_ds, "node", node_count, node_count, NULL, out_buf);

        PGM_Options *opt = PGM_create_options(handle);
        if (opt) {
            PGM_set_calculation_type(handle, opt, PGM_power_flow);
            PGM_set_calculation_method(handle, opt, PGM_linear);
            PGM_set_symmetric(handle, opt, PGM_asymmetric);
            PGM_set_max_iter(handle, opt, 5);

            PGM_calculate(handle, model, opt, out_ds, NULL);
            drain_error(handle);
            PGM_destroy_options(opt);
        }
        PGM_destroy_dataset_mutable(out_ds);
        free(out_buf);
    } while (0);

    /* ── 6c. Symmetric short circuit ──────────────────────────────────── *
     * Drives PGM_short_circuit + PGM_iec60909 through the dispatch layer.
     * Most random inputs will fail validation inside the solver — that is
     * expected and handled via drain_error.
     * ──────────────────────────────────────────────────────────────────── */
    do {
        PGM_MetaComponent const *meta =
            PGM_meta_get_component_by_name(handle, "sc_output", "node");
        if (!meta) break;

        void *out_buf = alloc_aligned(
            PGM_meta_component_alignment(handle, meta),
            PGM_meta_component_size(handle, meta) * (size_t)node_count);
        if (!out_buf) break;

        PGM_MutableDataset *out_ds = PGM_create_dataset_mutable(handle, "sc_output", 0, 1);
        if (!out_ds) { free(out_buf); break; }

        PGM_dataset_mutable_add_buffer(handle, out_ds, "node", node_count, node_count, NULL, out_buf);

        PGM_Options *opt = PGM_create_options(handle);
        if (opt) {
            PGM_set_calculation_type(handle, opt, PGM_short_circuit);
            PGM_set_calculation_method(handle, opt, PGM_iec60909);
            PGM_set_symmetric(handle, opt, PGM_symmetric);
            PGM_set_max_iter(handle, opt, 5);
            PGM_set_short_circuit_voltage_scaling(handle, opt,
                PGM_short_circuit_voltage_scaling_minimum);

            PGM_calculate(handle, model, opt, out_ds, NULL);
            drain_error(handle);
            PGM_destroy_options(opt);
        }
        PGM_destroy_dataset_mutable(out_ds);
        free(out_buf);
    } while (0);

    /* ── Cleanup ───────────────────────────────────────────────────────── */
cleanup:
    if (model)          PGM_destroy_model(model);
    if (const_ds)       PGM_destroy_dataset_const(const_ds);
    for (PGM_Idx i = 0; i < MAX_COMPONENTS; i++)
        free(comp_bufs[i]);   /* free(NULL) is always safe */
    if (deserializer)   PGM_destroy_deserializer(deserializer);
    PGM_destroy_handle(handle);
    return 0;
}
