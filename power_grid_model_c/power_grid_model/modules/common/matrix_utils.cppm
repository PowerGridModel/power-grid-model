// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <Eigen/Core>
#include <Eigen/Dense>
#include <complex>

import power_grid_model.common.common;
import power_grid_model.common.three_phase_tensor;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/common/matrix_utils.hpp>

export module power_grid_model.common.matrix_utils;

export namespace power_grid_model {
using power_grid_model::average_of_diagonal_of_matrix;
using power_grid_model::average_of_off_diagonal_of_matrix;
} // namespace power_grid_model
