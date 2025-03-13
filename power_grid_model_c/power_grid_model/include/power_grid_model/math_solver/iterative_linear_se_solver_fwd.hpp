// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "y_bus.hpp"

#include "../common/calculation_info.hpp"
#include "../common/common.hpp"

#include <memory>

namespace power_grid_model::math_solver {
namespace iterative_linear_se {
template <symmetry_tag sym> class IterativeLinearSESolver;

template <symmetry_tag sym>
std::shared_ptr<IterativeLinearSESolver<sym>>
create_iterative_linear_se_solver(YBus<sym> const& y_bus, std::shared_ptr<MathModelTopology const> const& topo_ptr);

template <symmetry_tag sym>
SolverOutput<sym>
run_state_estimation_iterative_linear(std::shared_ptr<IterativeLinearSESolver<sym>> iterative_linear_se_solver,
                                      StateEstimationInput<sym> const& input, double err_tol, Idx max_iter,
                                      CalculationInfo& calculation_info, YBus<sym> const& y_bus);
} // namespace iterative_linear_se

using iterative_linear_se::IterativeLinearSESolver;
} // namespace power_grid_model::math_solver
