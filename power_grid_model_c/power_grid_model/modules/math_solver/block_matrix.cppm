// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <Eigen/Core>
#include <Eigen/Dense>
#include <type_traits>

import power_grid_model.common.common;
import power_grid_model.common.three_phase_tensor;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/math_solver/block_matrix.hpp>

export module power_grid_model.math_solver.block_matrix;

export namespace power_grid_model::math_solver {
using power_grid_model::math_solver::Block;
} // namespace power_grid_model::math_solver
