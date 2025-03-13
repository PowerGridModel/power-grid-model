// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "y_bus.hpp"

#include "../common/calculation_info.hpp"
#include "../common/common.hpp"

#include <memory>

namespace power_grid_model::math_solver {
namespace short_circuit {
template <symmetry_tag sym> class ShortCircuitSolver;

template <symmetry_tag sym>
std::shared_ptr<ShortCircuitSolver<sym>>
create_short_circuit_solver(YBus<sym> const& y_bus, std::shared_ptr<MathModelTopology const> const& topo_ptr);

template <symmetry_tag sym>
ShortCircuitSolverOutput<sym> run_short_circuit(std::shared_ptr<ShortCircuitSolver<sym>> short_circuit_solver,
                                                ShortCircuitInput const& input, CalculationInfo& calculation_info,
                                                YBus<sym> const& y_bus);
} // namespace short_circuit

using short_circuit::ShortCircuitSolver;
} // namespace power_grid_model::math_solver
