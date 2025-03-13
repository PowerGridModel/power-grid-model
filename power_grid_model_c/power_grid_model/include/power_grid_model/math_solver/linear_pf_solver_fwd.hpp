// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "y_bus.hpp"

#include "../common/calculation_info.hpp"
#include "../common/common.hpp"

#include <memory>

namespace power_grid_model::math_solver {
namespace linear_pf {
template <symmetry_tag sym> class LinearPFSolver;

template <symmetry_tag sym>
std::shared_ptr<LinearPFSolver<sym>> create_linear_pf_solver(YBus<sym> const& y_bus,
                                                             std::shared_ptr<MathModelTopology const> const& topo_ptr);

template <symmetry_tag sym>
SolverOutput<sym> run_power_flow_linear(std::shared_ptr<LinearPFSolver<sym>> linear_pf_solver,
                                        PowerFlowInput<sym> const& input, double err_tol, Idx max_iter,
                                        CalculationInfo& calculation_info, YBus<sym> const& y_bus);
} // namespace linear_pf

using linear_pf::LinearPFSolver;
} // namespace power_grid_model::math_solver
