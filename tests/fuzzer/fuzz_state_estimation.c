// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// State estimation on a fuzzed input grid.
//
// Input: one "input" document, verbatim. Every input runs every case below.
// The grid needs voltage/power/current sensors for the estimator to get past
// its observability check.

#include <stddef.h>
#include <stdint.h>

#include "fuzz_common.h"
#include "fuzz_model.h"
#include "power_grid_model_c.h"

#define SE(method, symmetry)                                                                                           \
    {PGM_state_estimation, (method), (symmetry), PGM_tap_changing_strategy_disabled,                                  \
     PGM_short_circuit_voltage_scaling_maximum}

static calc_case const cases[] = {
    SE(PGM_iterative_linear, PGM_symmetric),
    SE(PGM_iterative_linear, PGM_asymmetric),
    SE(PGM_newton_raphson, PGM_symmetric),
    SE(PGM_newton_raphson, PGM_asymmetric),
};

int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size) {
    calculate_all(data, size, cases, FUZZ_ARRAY_LEN(cases));
    return 0;
}
