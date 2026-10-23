// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// Short circuit on a fuzzed input grid.
//
// Input: one "input" document, verbatim. Every input runs every case below.
// The grid needs at least one fault for the calculation to do any work.

#include <stddef.h>
#include <stdint.h>

#include "fuzz_common.h"
#include "fuzz_model.h"
#include "power_grid_model_c.h"

#define SC(symmetry, scaling)                                                                                          \
    {PGM_short_circuit, PGM_iec60909, (symmetry), PGM_tap_changing_strategy_disabled, (scaling)}

static calc_case const cases[] = {
    SC(PGM_symmetric, PGM_short_circuit_voltage_scaling_minimum),
    SC(PGM_symmetric, PGM_short_circuit_voltage_scaling_maximum),
    SC(PGM_asymmetric, PGM_short_circuit_voltage_scaling_minimum),
    SC(PGM_asymmetric, PGM_short_circuit_voltage_scaling_maximum),
};

int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size) {
    calculate_all(data, size, cases, FUZZ_ARRAY_LEN(cases));
    return 0;
}
