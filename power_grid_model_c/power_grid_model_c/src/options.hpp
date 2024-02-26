// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef PGM_DLL_EXPORTS
#define PGM_DLL_EXPORTS
#endif

#include "forward_declarations.hpp"

#include "power_grid_model_c/options.h"

#include <power_grid_model/common/common.hpp>

// options
struct PGM_Options {
    using Idx = power_grid_model::Idx;
    Idx calculation_type{PGM_power_flow};
    Idx calculation_method{PGM_default_method};
    Idx symmetric{1};
    double err_tol{1e-8};
    Idx max_iter{20};
    Idx threading{-1};
    Idx short_circuit_voltage_scaling{PGM_short_circuit_voltage_scaling_maximum};
    Idx experimental_features{PGM_experimental_features_disabled};
};
