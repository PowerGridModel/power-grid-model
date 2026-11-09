// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <Eigen/Core>
#include <Eigen/Dense>
#include <algorithm>
#include <boost/container/small_vector.hpp>
#include <cassert>
#include <complex>
#include <concepts>
#include <functional>
#include <map>
#include <numeric>
#include <ranges>
#include <type_traits>
#include <vector>

import power_grid_model.math_solver.measured_values;
import power_grid_model.math_solver.y_bus;
import power_grid_model.calculation_parameters;
import power_grid_model.common.common;
import power_grid_model.common.counting_iterator;
import power_grid_model.common.enums;
import power_grid_model.common.exception;
import power_grid_model.common.grouped_index_vector;
import power_grid_model.common.three_phase_tensor;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/math_solver/common_solver_functions.hpp>

export module power_grid_model.math_solver.common_solver_functions;

export namespace power_grid_model::math_solver::detail {
using power_grid_model::math_solver::detail::calculate_pf_result;
using power_grid_model::math_solver::detail::calculate_se_result;
using power_grid_model::math_solver::detail::copy_y_bus;
using power_grid_model::math_solver::detail::prepare_linear_matrix_and_rhs;
} // namespace power_grid_model::math_solver::detail
