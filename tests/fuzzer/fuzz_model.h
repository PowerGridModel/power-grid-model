// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// Model and calculation scaffolding shared by the harnesses that go past the
// parser: turning a document into a const dataset, building a model from it,
// and running one calculation with a fixed set of options.

#ifndef PGM_FUZZ_MODEL_H
#define PGM_FUZZ_MODEL_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "fuzz_common.h"
#include "power_grid_model_c.h"

#define FUZZ_ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

/* --------------------------------------------------------------------------
 * Parsed documents
 *
 * A document is parsed into caller-owned row buffers that have to outlive the
 * const dataset built on top of it, so ownership is tracked explicitly.
 * -------------------------------------------------------------------------- */
typedef struct {
    PGM_Deserializer* deserializer;
    PGM_WritableDataset* writable;
    PGM_DatasetInfo const* info;
    PGM_ConstDataset* const_dataset;
    void* buffers[PGM_FUZZ_MAX_COMPONENTS];
    PGM_Idx* indptrs[PGM_FUZZ_MAX_COMPONENTS];
    PGM_Idx n_components;
    PGM_Idx batch_size;
} parsed_document;

static inline void document_free(parsed_document* doc) {
    /* The const dataset views the writable dataset, which holds the buffer and
     * indptr pointers; both must go before the memory they reference. */
    if (doc->const_dataset != NULL) {
        PGM_destroy_dataset_const(doc->const_dataset);
    }
    if (doc->deserializer != NULL) {
        PGM_destroy_deserializer(doc->deserializer);
    }
    for (PGM_Idx i = 0; i < PGM_FUZZ_MAX_COMPONENTS; i++) {
        free(doc->buffers[i]);
        free(doc->indptrs[i]);
    }
    memset(doc, 0, sizeof(*doc));
}

/* Deserialize one document, allocate row buffers for every component, parse
 * into them, and freeze the result into a const dataset. Returns 0 on any
 * failure; the caller cleans up unconditionally with document_free(). */
static inline int document_parse(PGM_Handle* handle, parsed_document* doc, fuzz_budget* budget, char const* bytes,
                                 size_t length, char const* expected_dataset) {
    memset(doc, 0, sizeof(*doc));
    if (length == 0) {
        return 0;
    }

    doc->deserializer = PGM_create_deserializer_from_binary_buffer(handle, bytes, (PGM_Idx)length, PGM_json);
    if (doc->deserializer == NULL) {
        drain_error(handle);
        return 0;
    }

    doc->writable = PGM_deserializer_get_dataset(handle, doc->deserializer);
    if (doc->writable == NULL) {
        drain_error(handle);
        return 0;
    }

    doc->info = PGM_dataset_writable_get_info(handle, doc->writable);
    if (doc->info == NULL) {
        return 0;
    }

    char const* name = PGM_dataset_info_name(handle, doc->info);
    if (name == NULL || strcmp(name, expected_dataset) != 0) {
        return 0;
    }

    doc->batch_size = PGM_dataset_info_batch_size(handle, doc->info);
    doc->n_components = PGM_dataset_info_n_components(handle, doc->info);
    if (doc->batch_size < 0 || doc->n_components <= 0 || doc->n_components > PGM_FUZZ_MAX_COMPONENTS) {
        return 0;
    }

    for (PGM_Idx i = 0; i < doc->n_components; i++) {
        char const* component = PGM_dataset_info_component_name(handle, doc->info, i);
        PGM_Idx const total = PGM_dataset_info_total_elements(handle, doc->info, i);
        PGM_Idx const per_scenario = PGM_dataset_info_elements_per_scenario(handle, doc->info, i);
        if (component == NULL || total <= 0) {
            return 0;
        }

        /* Ragged batch component: supply the per-scenario offset array. Update
         * datasets are where non-uniform batches actually occur in practice —
         * scenario 1 changes two loads, scenario 2 changes five. */
        PGM_Idx* indptr = NULL;
        if (per_scenario < 0) {
            size_t const indptr_bytes = safe_mul((size_t)doc->batch_size + 1, sizeof(PGM_Idx));
            if (indptr_bytes == 0) {
                return 0;
            }
            indptr = alloc_plain(budget, indptr_bytes);
            if (indptr == NULL) {
                return 0;
            }
            memset(indptr, 0, indptr_bytes);
            doc->indptrs[i] = indptr;
        }

        PGM_MetaComponent const* meta = PGM_meta_get_component_by_name(handle, expected_dataset, component);
        if (meta == NULL) {
            drain_error(handle);
            return 0;
        }

        size_t const bytes_needed = safe_mul((size_t)total, PGM_meta_component_size(handle, meta));
        if (bytes_needed == 0) {
            return 0;
        }

        doc->buffers[i] = alloc_aligned(budget, PGM_meta_component_alignment(handle, meta), bytes_needed);
        if (doc->buffers[i] == NULL) {
            return 0;
        }
        /* NaN means "leave this attribute alone" in an update dataset, so
         * pre-filling is semantically required there, not just hygiene. */
        PGM_buffer_set_nan(handle, meta, doc->buffers[i], 0, total);
        drain_error(handle);

        PGM_dataset_writable_set_buffer(handle, doc->writable, component, indptr, doc->buffers[i]);
        if (PGM_error_code(handle) != PGM_no_error) {
            drain_error(handle);
            return 0;
        }
    }

    PGM_deserializer_parse_to_buffer(handle, doc->deserializer);
    if (PGM_error_code(handle) != PGM_no_error) {
        drain_error(handle);
        return 0;
    }

    doc->const_dataset = PGM_create_dataset_const_from_writable(handle, doc->writable);
    if (doc->const_dataset == NULL) {
        drain_error(handle);
        return 0;
    }
    return 1;
}

/* Parse an "input" document and build a model from it. Returns NULL if either
 * step fails; `doc` must be released with document_free() after the model. */
static inline PGM_PowerGridModel* model_from_input(PGM_Handle* handle, parsed_document* doc, fuzz_budget* budget,
                                                   char const* bytes, size_t length) {
    if (!document_parse(handle, doc, budget, bytes, length, "input")) {
        return NULL;
    }
    if (doc->batch_size != 1) {
        return NULL; /* a model is built from a single dataset */
    }

    PGM_PowerGridModel* model = PGM_create_model(handle, 50.0, doc->const_dataset);
    if (model == NULL) {
        drain_error(handle);
    }
    return model;
}

/* --------------------------------------------------------------------------
 * Calculations
 *
 * Each compute harness lists the option combinations it runs in a static
 * table and runs every entry on every input. Nothing about the options comes
 * from the input, so a new edge always means the document reached new code.
 * The remaining options are fixed: a bounded iteration count so a divergent
 * solve cannot dominate the run budget, and sequential execution so batch
 * dispatch is deterministic under coverage feedback.
 * -------------------------------------------------------------------------- */
typedef struct {
    PGM_Idx calculation_type;
    PGM_Idx calculation_method;
    PGM_Idx symmetric;
    PGM_Idx tap_changing_strategy;
    PGM_Idx short_circuit_voltage_scaling;
} calc_case;

#define FUZZ_ERR_TOL 1e-8
#define FUZZ_MAX_ITER 20
#define FUZZ_THREADING (-1) /* < 0: sequential */

/* Short circuit always writes sc_output; power flow and state estimation split
 * on symmetry. */
static inline char const* output_dataset_for(calc_case const* c) {
    if (c->calculation_type == PGM_short_circuit) {
        return "sc_output";
    }
    return c->symmetric == PGM_symmetric ? "sym_output" : "asym_output";
}

/* Allocate an output buffer for every input component that has a counterpart
 * in the output dataset, `scenarios` copies each, laid out scenario-major.
 * Writing results for every component rather than just nodes keeps the
 * branch/appliance/sensor output writers warm. */
static inline PGM_Idx add_output_buffers(PGM_Handle* handle, PGM_MutableDataset* output,
                                         parsed_document const* input, fuzz_budget* budget,
                                         char const* output_dataset, PGM_Idx scenarios, void** out_buffers) {
    PGM_Idx n_added = 0;

    for (PGM_Idx i = 0; i < input->n_components; i++) {
        char const* component = PGM_dataset_info_component_name(handle, input->info, i);
        if (component == NULL) {
            continue;
        }

        PGM_Idx const per_scenario = PGM_dataset_info_elements_per_scenario(handle, input->info, i);
        if (per_scenario <= 0) {
            continue;
        }

        /* Not every input component has an output counterpart (sensors in a
         * short-circuit run, for instance). A missing one is not an error. */
        PGM_MetaComponent const* meta = PGM_meta_get_component_by_name(handle, output_dataset, component);
        if (meta == NULL) {
            drain_error(handle);
            continue;
        }

        /* Compare in the unsigned domain: casting SIZE_MAX to the signed
         * PGM_Idx yields -1, which would make this guard reject everything. */
        if (scenarios <= 0 || (size_t)per_scenario > SIZE_MAX / (size_t)scenarios) {
            continue;
        }
        PGM_Idx const total = per_scenario * scenarios;
        size_t const bytes = safe_mul((size_t)total, PGM_meta_component_size(handle, meta));
        if (bytes == 0) {
            continue;
        }

        void* buffer = alloc_aligned(budget, PGM_meta_component_alignment(handle, meta), bytes);
        if (buffer == NULL) {
            break; /* out of budget: run with whatever was added so far */
        }
        /* The solver writes results here; pre-NaN so a component the solver
         * skips reads back as "not computed" rather than as stale heap. */
        PGM_buffer_set_nan(handle, meta, buffer, 0, total);
        drain_error(handle);

        PGM_dataset_mutable_add_buffer(handle, output, component, per_scenario, total, NULL, buffer);
        if (PGM_error_code(handle) != PGM_no_error) {
            drain_error(handle);
            free(buffer);
            continue;
        }
        out_buffers[n_added++] = buffer;
    }
    return n_added;
}

/* Run one calculation on `model` and discard the results.
 *
 * With `batch` == NULL this is a single calculation; otherwise `batch` is the
 * update dataset applied per scenario and `scenarios` its batch size.
 * Divergence, singular matrices, and invalid inputs all report through the
 * handle and are expected, not findings. */
static inline void run_calculation(PGM_Handle* handle, PGM_PowerGridModel* model, parsed_document const* input,
                                   calc_case const* c, PGM_ConstDataset const* batch, PGM_Idx scenarios) {
    fuzz_budget budget;
    budget_reset(&budget);

    void* output_buffers[PGM_FUZZ_MAX_COMPONENTS];
    PGM_Idx n_output_buffers = 0;
    PGM_Options* options = NULL;

    char const* output_dataset = output_dataset_for(c);
    PGM_MutableDataset* output = PGM_create_dataset_mutable(handle, output_dataset, batch != NULL, scenarios);
    if (output == NULL) {
        drain_error(handle);
        return;
    }

    n_output_buffers = add_output_buffers(handle, output, input, &budget, output_dataset, scenarios, output_buffers);
    if (n_output_buffers == 0) {
        goto cleanup;
    }

    options = PGM_create_options(handle);
    if (options == NULL) {
        drain_error(handle);
        goto cleanup;
    }
    PGM_set_calculation_type(handle, options, c->calculation_type);
    PGM_set_calculation_method(handle, options, c->calculation_method);
    PGM_set_symmetric(handle, options, c->symmetric);
    PGM_set_tap_changing_strategy(handle, options, c->tap_changing_strategy);
    PGM_set_short_circuit_voltage_scaling(handle, options, c->short_circuit_voltage_scaling);
    PGM_set_err_tol(handle, options, FUZZ_ERR_TOL);
    PGM_set_max_iter(handle, options, FUZZ_MAX_ITER);
    PGM_set_threading(handle, options, FUZZ_THREADING);
    drain_error(handle);

    PGM_calculate(handle, model, options, output, batch);
    /* Per-scenario failures arrive out-of-band; reading them is the only way
     * the batch error accessors get exercised. */
    drain_batch_error(handle);

cleanup:
    if (options != NULL) {
        PGM_destroy_options(options);
    }
    PGM_destroy_dataset_mutable(output);
    for (PGM_Idx i = 0; i < n_output_buffers; i++) {
        free(output_buffers[i]);
    }
}

/* Build a model from the input document and run every entry of `cases` on it.
 * This is the whole body of the single-calculation harnesses. */
static inline void calculate_all(uint8_t const* data, size_t size, calc_case const* cases, size_t n_cases) {
    PGM_Handle* handle = PGM_create_handle();
    if (handle == NULL) {
        return;
    }

    fuzz_budget budget;
    budget_reset(&budget);

    parsed_document input;
    PGM_PowerGridModel* model = model_from_input(handle, &input, &budget, (char const*)data, size);
    if (model != NULL) {
        for (size_t i = 0; i < n_cases; i++) {
            run_calculation(handle, model, &input, &cases[i], NULL, 1);
        }
        PGM_destroy_model(model);
    }

    document_free(&input);
    PGM_destroy_handle(handle);
}

#endif /* PGM_FUZZ_MODEL_H */
