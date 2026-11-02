// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// Update datasets applied to a model: batch calculation and in-place update.
//
// Input: one "update" document, verbatim. The grid it updates is fixed
// (base_grid.h) rather than fuzzed — the input grids are already covered by
// the single-calculation harnesses, and keeping it fixed means every byte of
// the input belongs to the update document. Every input runs, in order:
//   1. a batch power flow with the document as the per-scenario update, then
//   2. PGM_update_model with the same document.
// The library rejects whichever of the two does not fit the document's shape
// (batch vs. single), and that rejection is part of what gets fuzzed.

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "base_grid.h"
#include "fuzz_common.h"
#include "fuzz_model.h"
#include "power_grid_model_c.h"

/* Upper bound on scenarios, so the output dataset stays proportional to the
 * input rather than to a batch size the document merely claims. */
#define MAX_SCENARIOS 512

static calc_case const batch_case = {PGM_power_flow, PGM_newton_raphson, PGM_symmetric,
                                     PGM_tap_changing_strategy_disabled, PGM_short_circuit_voltage_scaling_maximum};

int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size) {
    PGM_Handle* handle = PGM_create_handle();
    if (handle == NULL) {
        return 0;
    }

    fuzz_budget budget;
    budget_reset(&budget);

    parsed_document grid;
    parsed_document update;
    memset(&update, 0, sizeof(update));

    /* model == NULL cannot happen unless base_grid.h is broken. document_parse
     * zeroes `update` before anything else, so the unconditional cleanup below
     * stays valid however early this short-circuits. */
    PGM_PowerGridModel* model = model_from_input(handle, &grid, &budget, base_grid_json, strlen(base_grid_json));
    if (model != NULL && document_parse(handle, &update, &budget, (char const*)data, size, "update") &&
        update.batch_size > 0 && update.batch_size <= MAX_SCENARIOS) {
        /* Batch first: it leaves the model as it found it, so step 2 starts
         * from the same grid on every input. */
        run_calculation(handle, model, &grid, &batch_case, update.const_dataset, update.batch_size);

        PGM_update_model(handle, model, update.const_dataset);
        drain_error(handle);
    }

    if (model != NULL) {
        PGM_destroy_model(model);
    }
    document_free(&update);
    document_free(&grid);
    PGM_destroy_handle(handle);
    return 0;
}
