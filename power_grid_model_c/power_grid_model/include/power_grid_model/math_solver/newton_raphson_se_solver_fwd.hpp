// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "y_bus.hpp"

#include "../common/calculation_info.hpp"
#include "../common/common.hpp"

#include <memory>

namespace power_grid_model::math_solver {
namespace newton_raphson_se {
template <symmetry_tag sym> class NewtonRaphsonSESolver;

template <symmetry_tag sym>
std::shared_ptr<NewtonRaphsonSESolver<sym>>
create_newton_raphson_se_solver(YBus<sym> const& y_bus, std::shared_ptr<MathModelTopology const> const& topo_ptr);

template <symmetry_tag sym>
SolverOutput<sym>
run_state_estimation_newton_raphson(std::shared_ptr<NewtonRaphsonSESolver<sym>> newton_raphson_se_solver,
                                    StateEstimationInput<sym> const& input, double err_tol, Idx max_iter,
                                    CalculationInfo& calculation_info, YBus<sym> const& y_bus);
} // namespace newton_raphson_se

using newton_raphson_se::NewtonRaphsonSESolver;
} // namespace power_grid_model::math_solver
