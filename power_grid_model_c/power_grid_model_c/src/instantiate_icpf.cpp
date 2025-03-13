// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/math_solver/iterative_current_pf_solver.hpp>
#include <power_grid_model/math_solver/iterative_current_pf_solver_fwd.hpp>

namespace power_grid_model::math_solver::iterative_current_pf {
template class IterativeCurrentPFSolver<symmetric_t>;
template class IterativeCurrentPFSolver<asymmetric_t>;

template <symmetry_tag sym>
std::shared_ptr<IterativeCurrentPFSolver<sym>>
create_iterative_current_pf_solver(YBus<sym> const& y_bus, std::shared_ptr<MathModelTopology const> const& topo_ptr) {
    return std::make_shared<IterativeCurrentPFSolver<sym>>(y_bus, topo_ptr);
}
template std::shared_ptr<IterativeCurrentPFSolver<symmetric_t>>
create_iterative_current_pf_solver(YBus<symmetric_t> const& y_bus,
                                   std::shared_ptr<MathModelTopology const> const& topo_ptr);
template std::shared_ptr<IterativeCurrentPFSolver<asymmetric_t>>
create_iterative_current_pf_solver(YBus<asymmetric_t> const& y_bus,
                                   std::shared_ptr<MathModelTopology const> const& topo_ptr);

template <symmetry_tag sym>
SolverOutput<sym>
run_power_flow_iterative_current(std::shared_ptr<IterativeCurrentPFSolver<sym>> iterative_current_pf_solver,
                                 PowerFlowInput<sym> const& input, double err_tol, Idx max_iter,
                                 CalculationInfo& calculation_info, YBus<sym> const& y_bus) {
    assert(iterative_current_pf_solver != nullptr);
    return iterative_current_pf_solver->run_power_flow(y_bus, input, err_tol, max_iter, calculation_info);
}
template SolverOutput<symmetric_t>
run_power_flow_iterative_current(std::shared_ptr<IterativeCurrentPFSolver<symmetric_t>> iterative_current_pf_solver,
                                 PowerFlowInput<symmetric_t> const& input, double err_tol, Idx max_iter,
                                 CalculationInfo& calculation_info, YBus<symmetric_t> const& y_bus);
template SolverOutput<asymmetric_t>
run_power_flow_iterative_current(std::shared_ptr<IterativeCurrentPFSolver<asymmetric_t>> iterative_current_pf_solver,
                                 PowerFlowInput<asymmetric_t> const& input, double err_tol, Idx max_iter,
                                 CalculationInfo& calculation_info, YBus<asymmetric_t> const& y_bus);
} // namespace power_grid_model::math_solver::iterative_current_pf
