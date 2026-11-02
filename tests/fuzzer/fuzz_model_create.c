// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// Model construction from a fuzzed input grid, without running a solver.
//
// Input: one "input" document, verbatim. Builds the model, deep-copies it, and
// resolves component IDs through the indexer. Cheap compared to the solver
// harnesses, so topology and index construction get many more executions.

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "fuzz_common.h"
#include "fuzz_model.h"
#include "power_grid_model_c.h"

/* Resolve IDs 0..total-1, XORed with `perturb`, for every component. On a
 * real grid that is a mix of hits and misses, and a miss takes a different
 * branch through the sparse index mapping than a hit. */
static void exercise_indexer(PGM_Handle* handle, PGM_PowerGridModel const* model, parsed_document const* input,
                             fuzz_budget* budget, PGM_ID perturb) {
    for (PGM_Idx i = 0; i < input->n_components; i++) {
        char const* component = PGM_dataset_info_component_name(handle, input->info, i);
        PGM_Idx const total = PGM_dataset_info_total_elements(handle, input->info, i);
        if (component == NULL || total <= 0 || total > 4096) {
            continue;
        }

        PGM_ID* ids = alloc_plain(budget, safe_mul((size_t)total, sizeof(PGM_ID)));
        PGM_Idx* indexer = alloc_plain(budget, safe_mul((size_t)total, sizeof(PGM_Idx)));
        if (ids != NULL && indexer != NULL) {
            for (PGM_Idx j = 0; j < total; j++) {
                ids[j] = (PGM_ID)j ^ perturb;
            }
            PGM_get_indexer(handle, model, component, total, ids, indexer);
            drain_error(handle);
        }
        free(ids);
        free(indexer);
    }
}

int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size) {
    PGM_Handle* handle = PGM_create_handle();
    if (handle == NULL) {
        return 0;
    }

    fuzz_budget budget;
    budget_reset(&budget);

    parsed_document input;
    PGM_PowerGridModel* model = model_from_input(handle, &input, &budget, (char const*)data, size);
    if (model != NULL) {
        /* Deep copy: a separate constructor that has to reproduce the whole
         * internal topology, and a plausible place for an ownership bug. */
        PGM_PowerGridModel* copy = PGM_copy_model(handle, model);
        if (copy != NULL) {
            PGM_destroy_model(copy);
        } else {
            drain_error(handle);
        }

        exercise_indexer(handle, model, &input, &budget, 0x00);
        exercise_indexer(handle, model, &input, &budget, 0xA5);
        PGM_destroy_model(model);
    }

    document_free(&input);
    PGM_destroy_handle(handle);
    return 0;
}
