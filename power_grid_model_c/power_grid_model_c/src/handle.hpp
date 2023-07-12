// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_C_HANDLE_HPP
#define POWER_GRID_MODEL_C_HANDLE_HPP

#ifndef PGM_DLL_EXPORTS
#define PGM_DLL_EXPORTS
#endif

#include "power_grid_model_c/handle.h"

#include <power_grid_model/power_grid_model.hpp>

// context handle
struct PGM_Handle {
    power_grid_model::Idx err_code;
    std::string err_msg;
    power_grid_model::IdxVector failed_scenarios;
    std::vector<std::string> batch_errs;
    mutable std::vector<char const*> batch_errs_c_str;
    [[no_unique_address]] power_grid_model::BatchParameter batch_parameter;
};

#endif