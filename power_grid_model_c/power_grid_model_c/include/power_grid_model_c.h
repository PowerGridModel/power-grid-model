// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

/**
 * @mainpage Power Grid Model C API Documentation
 * This gives an overview of C API.
 * All the exported functions are included in power_grid_model_c.h, except the dataset definitions
 *
 * Users can also include individual headers for a subset of the functionality
 *     - power_grid_model_c/basics.h: type and enum definition
 *     - power_grid_model_c/buffer.h: functions with buffer creation, release, set and get value
 *     - power_grid_model_c/handle.h: functions with error handling
 *     - power_grid_model_c/meta_data.h: functions with meta data
 *     - power_grid_model_c/model.h: functions with create, release, run calculation of model
 *     - power_grid_model_c/options.h: functions with setting the calculation options
 *     - power_grid_model_c/serialization.h: functions with serialization functions
 *     - power_grid_model_c/dataset.h: functions with dataset handling functions
 *     - power_grid_model_c/dataset_definitions.h: external pointer variables to all datasets, components, and
 *       attributes. You have to include this header separately. It is not included by power_grid_model_c.h.
 */

#pragma once
#ifndef POWER_GRID_MODEL_C_H
#define POWER_GRID_MODEL_C_H

#include "power_grid_model_c/basics.h"
#include "power_grid_model_c/buffer.h"
#include "power_grid_model_c/dataset.h"
#include "power_grid_model_c/handle.h"
#include "power_grid_model_c/meta_data.h"
#include "power_grid_model_c/model.h"
#include "power_grid_model_c/options.h"
#include "power_grid_model_c/serialization.h"

#endif // POWER_GRID_MODEL_C_H
