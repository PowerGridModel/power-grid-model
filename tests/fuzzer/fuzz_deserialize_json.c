// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "fuzz_common.h"
#include "power_grid_model_c.h"

/* Walk every dataset-info accessor the C API exposes.
 *
 * The attribute-indication accessors matter: for columnar ("attributes": {...})
 * documents they report which attributes the document actually carries, and
 * they are the API that a caller must use to size per-attribute buffers. Note
 * PGM_dataset_info_attribute_name is documented as UB when the component has no
 * indications, so it is only reached through the has/n guard. */
static void walk_dataset_info(PGM_Handle* handle, PGM_DatasetInfo const* info) {
    char const* name = PGM_dataset_info_name(handle, info);
    if (name != NULL) {
        (void)strlen(name);
    }

    (void)PGM_dataset_info_is_batch(handle, info);
    (void)PGM_dataset_info_batch_size(handle, info);

    PGM_Idx const n_components = PGM_dataset_info_n_components(handle, info);
    for (PGM_Idx i = 0; i < n_components; i++) {
        char const* component = PGM_dataset_info_component_name(handle, info, i);
        if (component != NULL) {
            (void)strlen(component);
        }

        (void)PGM_dataset_info_elements_per_scenario(handle, info, i);
        (void)PGM_dataset_info_total_elements(handle, info, i);

        if (PGM_dataset_info_has_attribute_indications(handle, info, i) != 0) {
            PGM_Idx const n_attrs = PGM_dataset_info_n_attribute_indications(handle, info, i);
            for (PGM_Idx j = 0; j < n_attrs; j++) {
                char const* attribute = PGM_dataset_info_attribute_name(handle, info, i, j);
                if (attribute != NULL) {
                    (void)strlen(attribute);
                }
            }
        }
    }

    drain_error(handle);
}

/* One deserializer lifecycle: create, inspect, destroy. */
static void run_deserializer(PGM_Handle* handle, PGM_Deserializer* deserializer) {
    if (deserializer == NULL) {
        drain_error(handle);
        return;
    }

    PGM_WritableDataset* writable = PGM_deserializer_get_dataset(handle, deserializer);
    if (writable != NULL) {
        PGM_DatasetInfo const* info = PGM_dataset_writable_get_info(handle, writable);
        if (info != NULL) {
            walk_dataset_info(handle, info);
        }
    }
    drain_error(handle);

    PGM_destroy_deserializer(deserializer);
}

int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size) {
    if (size == 0) {
        return 0;
    }

    PGM_Handle* handle = PGM_create_handle();
    if (handle == NULL) {
        return 0;
    }

    run_deserializer(handle,
                     PGM_create_deserializer_from_binary_buffer(handle, (char const*)data, (PGM_Idx)size, PGM_json));

    PGM_destroy_handle(handle);
    return 0;
}
