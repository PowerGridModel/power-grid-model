// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_C_OPTIONS_HPP
#define POWER_GRID_MODEL_C_OPTIONS_HPP

#ifndef PGM_DLL_EXPORTS
#define PGM_DLL_EXPORTS
#endif

#include "power_grid_model_c/options.h"

#include <power_grid_model/power_grid_model.hpp>

// options
struct PGM_Options {
    using Idx = power_grid_model::Idx;
    Idx calculation_type{PGM_power_flow};
    Idx calculation_method{PGM_default_method};
    Idx symmetric{1};
    double err_tol{1e-8};
    Idx max_iter{20};
    Idx threading{-1};
};

#endif
