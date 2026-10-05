// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// Parsing into caller-owned buffers.
//
// Input: one document of any dataset type, verbatim. Every input is parsed
// twice, once into row buffers and once into columnar (per-attribute) buffers,
// each with a fresh deserializer.

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "fuzz_common.h"
#include "power_grid_model_c.h"

/* Everything one component allocated, so cleanup is a single unconditional
 * sweep regardless of which layout a component ended up using. */
typedef struct {
    void* row_buffer;
    PGM_Idx* indptr;
    void* attribute_buffers[64];
    PGM_Idx n_attribute_buffers;
} component_buffers;

/* A non-uniform (ragged) batch component needs an indptr of batch_size + 1
 * entries describing where each scenario starts. The deserializer fills it in;
 * we only have to provide correctly sized storage. */
static PGM_Idx* make_indptr(fuzz_budget* budget, PGM_Idx batch_size) {
    if (batch_size < 0 || batch_size > (PGM_Idx)(SIZE_MAX / sizeof(PGM_Idx)) - 1) {
        return NULL;
    }
    size_t const bytes = safe_mul((size_t)batch_size + 1, sizeof(PGM_Idx));
    if (bytes == 0) {
        return NULL;
    }
    PGM_Idx* indptr = alloc_plain(budget, bytes);
    if (indptr != NULL) {
        memset(indptr, 0, bytes);
    }
    return indptr;
}

/* Row layout: a single packed buffer of `total` component structs. */
static int register_row_buffer(PGM_Handle* handle, PGM_WritableDataset* writable, fuzz_budget* budget,
                               char const* dataset, char const* component, PGM_Idx total, PGM_Idx* indptr,
                               component_buffers* out) {
    PGM_MetaComponent const* meta = PGM_meta_get_component_by_name(handle, dataset, component);
    if (meta == NULL) {
        drain_error(handle);
        return 0;
    }

    size_t const elem_size = PGM_meta_component_size(handle, meta);
    size_t const align = PGM_meta_component_alignment(handle, meta);
    size_t const bytes = safe_mul((size_t)(total > 0 ? total : 0), elem_size);
    if (total > 0 && bytes == 0) {
        return 0; /* overflow */
    }

    out->row_buffer = alloc_aligned(budget, align, bytes);
    if (out->row_buffer == NULL) {
        return 0;
    }
    /* Pre-poison with NaN so that any field the document does not set is
     * well-defined rather than uninitialized — otherwise MSan-style reads of
     * untouched fields downstream would be noise, not findings. */
    PGM_buffer_set_nan(handle, meta, out->row_buffer, 0, total > 0 ? total : 0);
    drain_error(handle);

    PGM_dataset_writable_set_buffer(handle, writable, component, indptr, out->row_buffer);
    if (PGM_error_code(handle) != PGM_no_error) {
        drain_error(handle);
        return 0;
    }
    return 1;
}

/* Columnar layout: signal "no row buffer" by passing NULL data, then attach one
 * array per attribute the document says it carries. Sizing each column requires
 * the attribute's ctype width, which is why PGM_meta_attribute_ctype matters
 * here and nowhere else. */
static int register_columnar_buffers(PGM_Handle* handle, PGM_WritableDataset* writable, PGM_DatasetInfo const* info,
                                     fuzz_budget* budget, char const* dataset, char const* component,
                                     PGM_Idx component_idx, PGM_Idx total, PGM_Idx* indptr, component_buffers* out) {
    if (PGM_dataset_info_has_attribute_indications(handle, info, component_idx) == 0) {
        /* Nothing to attach; a columnar registration with no columns is itself
         * a case worth handing to the library. */
        PGM_dataset_writable_set_buffer(handle, writable, component, indptr, NULL);
        drain_error(handle);
        return 1;
    }

    PGM_dataset_writable_set_buffer(handle, writable, component, indptr, NULL);
    if (PGM_error_code(handle) != PGM_no_error) {
        drain_error(handle);
        return 0;
    }

    PGM_Idx n_attrs = PGM_dataset_info_n_attribute_indications(handle, info, component_idx);
    if (n_attrs > (PGM_Idx)(sizeof(out->attribute_buffers) / sizeof(out->attribute_buffers[0]))) {
        n_attrs = (PGM_Idx)(sizeof(out->attribute_buffers) / sizeof(out->attribute_buffers[0]));
    }

    for (PGM_Idx j = 0; j < n_attrs; j++) {
        char const* attribute = PGM_dataset_info_attribute_name(handle, info, component_idx, j);
        if (attribute == NULL) {
            continue;
        }

        PGM_MetaAttribute const* meta_attr = PGM_meta_get_attribute_by_name(handle, dataset, component, attribute);
        if (meta_attr == NULL) {
            drain_error(handle);
            continue;
        }

        size_t const width = ctype_size(PGM_meta_attribute_ctype(handle, meta_attr));
        drain_error(handle);
        if (width == 0) {
            continue;
        }

        size_t const bytes = safe_mul((size_t)(total > 0 ? total : 0), width);
        if (total > 0 && bytes == 0) {
            continue; /* overflow */
        }

        /* Align to the scalar, not the attribute: double3 is 24 bytes wide but
         * only needs double alignment, and aligned_alloc requires a power of
         * two. */
        void* column = alloc_aligned(budget, width == 24 ? sizeof(double) : width, bytes);
        if (column == NULL) {
            return 0;
        }
        /* A zero-element component still gets a column; alloc_aligned bumps
         * that to one alignment unit, which for double3 is narrower than the
         * attribute, so only clear what was actually requested. */
        if (bytes > 0) {
            memset(column, 0, bytes);
        }
        out->attribute_buffers[out->n_attribute_buffers++] = column;

        PGM_dataset_writable_set_attribute_buffer(handle, writable, component, attribute, column);
        if (PGM_error_code(handle) != PGM_no_error) {
            drain_error(handle);
            return 0;
        }
    }
    return 1;
}

/* Deserialize `document`, register a buffer of the requested layout for every
 * component, and parse into them. */
static void parse_with_layout(char const* document, size_t document_len, int columnar) {
    fuzz_budget budget;
    budget_reset(&budget);

    PGM_Handle* handle = PGM_create_handle();
    if (handle == NULL) {
        return;
    }

    component_buffers buffers[PGM_FUZZ_MAX_COMPONENTS];
    memset(buffers, 0, sizeof(buffers));

    PGM_Deserializer* deserializer =
        PGM_create_deserializer_from_binary_buffer(handle, document, (PGM_Idx)document_len, PGM_json);
    if (deserializer == NULL) {
        drain_error(handle);
        PGM_destroy_handle(handle);
        return;
    }

    PGM_WritableDataset* writable = PGM_deserializer_get_dataset(handle, deserializer);
    if (writable == NULL) {
        goto cleanup;
    }

    PGM_DatasetInfo const* info = PGM_dataset_writable_get_info(handle, writable);
    if (info == NULL) {
        goto cleanup;
    }

    char const* dataset = PGM_dataset_info_name(handle, info);
    if (dataset == NULL) {
        goto cleanup;
    }

    PGM_Idx const batch_size = PGM_dataset_info_batch_size(handle, info);
    PGM_Idx const n_components = PGM_dataset_info_n_components(handle, info);
    /* No filter on dataset name and no filter on batch size — both are the
     * point of this target. Only the component count is bounded, because it
     * indexes a fixed-size array. */
    if (n_components <= 0 || n_components > PGM_FUZZ_MAX_COMPONENTS || batch_size < 0) {
        goto cleanup;
    }

    for (PGM_Idx i = 0; i < n_components; i++) {
        char const* component = PGM_dataset_info_component_name(handle, info, i);
        if (component == NULL) {
            goto cleanup;
        }

        PGM_Idx const total = PGM_dataset_info_total_elements(handle, info, i);
        PGM_Idx const per_scenario = PGM_dataset_info_elements_per_scenario(handle, info, i);
        if (total < 0) {
            goto cleanup;
        }

        /* Ragged batch component: the library needs somewhere to write the
         * per-scenario offsets. Uniform components pass NULL. */
        PGM_Idx* indptr = NULL;
        if (per_scenario < 0) {
            indptr = make_indptr(&budget, batch_size);
            if (indptr == NULL) {
                goto cleanup;
            }
            buffers[i].indptr = indptr;
        }

        int const ok = columnar ? register_columnar_buffers(handle, writable, info, &budget, dataset, component, i,
                                                            total, indptr, &buffers[i])
                                : register_row_buffer(handle, writable, &budget, dataset, component, total, indptr,
                                                      &buffers[i]);
        if (!ok) {
            goto cleanup;
        }
    }

    /* The actual subject of this target. */
    PGM_deserializer_parse_to_buffer(handle, deserializer);
    drain_error(handle);

cleanup:
    /* Order matters: the writable dataset inside the deserializer holds the
     * buffer pointers registered above, so the deserializer goes first. */
    PGM_destroy_deserializer(deserializer);
    for (PGM_Idx i = 0; i < PGM_FUZZ_MAX_COMPONENTS; i++) {
        free(buffers[i].row_buffer);
        free(buffers[i].indptr);
        for (PGM_Idx j = 0; j < buffers[i].n_attribute_buffers; j++) {
            free(buffers[i].attribute_buffers[j]);
        }
    }
    PGM_destroy_handle(handle);
}

int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size) {
    if (size == 0) {
        return 0;
    }
    parse_with_layout((char const*)data, size, 0);
    parse_with_layout((char const*)data, size, 1);
    return 0;
}
