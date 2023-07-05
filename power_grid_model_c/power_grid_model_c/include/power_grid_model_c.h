// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

/**
 * @mainpage Power Grid Model C API Documentation
 * This gives an overview of C API.
 * All the exported functions are included in power_grid_model_c.h
 * 
 * Users can also include individual headers for individual functions
 *     - basics.h: type and enum definition
 *     - buffer.h: functions with buffer creation, release, set and get value
 *     - handle.h: functions with error handling
 *     - meta_data.h: functions with meta data
 *     - model.h: functions with create, release, run calculation of model
 *     - options.h: functions with setting the calculation options
 */

#pragma once
#ifndef POWER_GRID_MODEL_C_H
#define POWER_GRID_MODEL_C_H

#include "power_grid_model_c/basics.h"
#include "power_grid_model_c/buffer.h"
#include "power_grid_model_c/handle.h"
#include "power_grid_model_c/meta_data.h"
#include "power_grid_model_c/model.h"
#include "power_grid_model_c/options.h"

#endif  // POWER_GRID_MODEL_C_H
