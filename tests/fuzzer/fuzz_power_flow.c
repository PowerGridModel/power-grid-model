// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// Power flow on a fuzzed input grid.
//
// Input: one "input" document, verbatim. Every input runs every case below.

#include <stddef.h>
#include <stdint.h>

#include "fuzz_common.h"
#include "fuzz_model.h"
#include "power_grid_model_c.h"

#define PF(method, symmetry, tap)                                                                                      \
    {PGM_power_flow, (method), (symmetry), (tap), PGM_short_circuit_voltage_scaling_maximum}

static calc_case const cases[] = {
    /* Every power flow solver, both symmetries. */
    PF(PGM_linear, PGM_symmetric, PGM_tap_changing_strategy_disabled),
    PF(PGM_linear, PGM_asymmetric, PGM_tap_changing_strategy_disabled),
    PF(PGM_newton_raphson, PGM_symmetric, PGM_tap_changing_strategy_disabled),
    PF(PGM_newton_raphson, PGM_asymmetric, PGM_tap_changing_strategy_disabled),
    PF(PGM_iterative_current, PGM_symmetric, PGM_tap_changing_strategy_disabled),
    PF(PGM_iterative_current, PGM_asymmetric, PGM_tap_changing_strategy_disabled),
    PF(PGM_linear_current, PGM_symmetric, PGM_tap_changing_strategy_disabled),
    PF(PGM_linear_current, PGM_asymmetric, PGM_tap_changing_strategy_disabled),
    /* Every automatic tap-changing strategy. These only get past option
     * validation when the grid has a transformer_tap_regulator. */
    PF(PGM_newton_raphson, PGM_symmetric, PGM_tap_changing_strategy_any_valid_tap),
    PF(PGM_newton_raphson, PGM_symmetric, PGM_tap_changing_strategy_min_voltage_tap),
    PF(PGM_newton_raphson, PGM_symmetric, PGM_tap_changing_strategy_max_voltage_tap),
    PF(PGM_newton_raphson, PGM_symmetric, PGM_tap_changing_strategy_fast_any_tap),
};

int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size) {
    calculate_all(data, size, cases, FUZZ_ARRAY_LEN(cases));
    return 0;
}
