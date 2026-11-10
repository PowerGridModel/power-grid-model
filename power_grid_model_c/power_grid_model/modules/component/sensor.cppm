// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

import power_grid_model.component.base;
import power_grid_model.auxiliary.input;
import power_grid_model.common.common;
import power_grid_model.common.enums;
import power_grid_model.auxiliary.output;
import power_grid_model.auxiliary.update;
import power_grid_model.common.three_phase_tensor;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/component/sensor.hpp>

export module power_grid_model.component.sensor;

export namespace power_grid_model {
using power_grid_model::Sensor;
} // namespace power_grid_model
