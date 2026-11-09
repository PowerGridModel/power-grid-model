// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <Eigen/Core>
#include <Eigen/Dense>
#include <complex>

import power_grid_model.common.three_phase_tensor;
import power_grid_model.common.common;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/component/line_utils.hpp>

export module power_grid_model.component.line_utils;

export namespace power_grid_model {
using power_grid_model::kron_reduction;
} // namespace power_grid_model
