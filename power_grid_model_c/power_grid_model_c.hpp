// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

// internal header file for C API

#pragma once
#ifndef POWER_GRID_MODEL_C_HPP
#define POWER_GRID_MODEL_C_HPP

// include headers
#include "power_grid_model/main_model.hpp"
#include "power_grid_model/power_grid_model.hpp"

// C linkage
extern "C" {

// the main model as alias
using PGM_PowerGridModel = power_grid_model::MainModel;
// forward declare
struct PGM_Handle;
struct PGM_Options;

}  // C linkage

// include the public header
#define PGM_DLL_EXPORTS
#include "power_grid_model_c.h"

// assert index type
static_assert(std::is_same_v<PGM_Idx, power_grid_model::Idx>);

#endif  // POWER_GRID_MODEL_C_HPP