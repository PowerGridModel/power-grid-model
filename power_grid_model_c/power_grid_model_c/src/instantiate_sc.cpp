// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/math_solver/short_circuit_solver.hpp>
#include <power_grid_model/math_solver/short_circuit_solver_fwd.hpp>

namespace power_grid_model::math_solver::short_circuit {
template class ShortCircuitSolver<symmetric_t>;
template class ShortCircuitSolver<asymmetric_t>;

template <symmetry_tag sym>
std::shared_ptr<ShortCircuitSolver<sym>>
create_short_circuit_solver(YBus<sym> const& y_bus, std::shared_ptr<MathModelTopology const> const& topo_ptr) {
    return std::make_shared<ShortCircuitSolver<sym>>(y_bus, topo_ptr);
}
template std::shared_ptr<ShortCircuitSolver<symmetric_t>>
create_short_circuit_solver(YBus<symmetric_t> const& y_bus, std::shared_ptr<MathModelTopology const> const& topo_ptr);
template std::shared_ptr<ShortCircuitSolver<asymmetric_t>>
create_short_circuit_solver(YBus<asymmetric_t> const& y_bus, std::shared_ptr<MathModelTopology const> const& topo_ptr);

template <symmetry_tag sym>
ShortCircuitSolverOutput<sym> run_short_circuit(std::shared_ptr<ShortCircuitSolver<sym>> short_circuit_solver,
                                                ShortCircuitInput const& input, CalculationInfo& calculation_info,
                                                YBus<sym> const& y_bus) {
    assert(short_circuit_solver != nullptr);
    return short_circuit_solver->run_short_circuit(y_bus, input);
}
template ShortCircuitSolverOutput<symmetric_t>
run_short_circuit(std::shared_ptr<ShortCircuitSolver<symmetric_t>> short_circuit_solver, ShortCircuitInput const& input,
                  CalculationInfo& calculation_info, YBus<symmetric_t> const& y_bus);
template ShortCircuitSolverOutput<asymmetric_t>
run_short_circuit(std::shared_ptr<ShortCircuitSolver<asymmetric_t>> short_circuit_solver,
                  ShortCircuitInput const& input, CalculationInfo& calculation_info, YBus<asymmetric_t> const& y_bus);
} // namespace power_grid_model::math_solver::short_circuit
