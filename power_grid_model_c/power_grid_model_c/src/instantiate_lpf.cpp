// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/math_solver/linear_pf_solver.hpp>
#include <power_grid_model/math_solver/linear_pf_solver_fwd.hpp>

namespace power_grid_model::math_solver::linear_pf {
template class LinearPFSolver<symmetric_t>;
template class LinearPFSolver<asymmetric_t>;

template <symmetry_tag sym>
std::shared_ptr<LinearPFSolver<sym>> create_linear_pf_solver(YBus<sym> const& y_bus,
                                                             std::shared_ptr<MathModelTopology const> const& topo_ptr) {
    return std::make_shared<LinearPFSolver<sym>>(y_bus, topo_ptr);
}
template std::shared_ptr<LinearPFSolver<symmetric_t>>
create_linear_pf_solver(YBus<symmetric_t> const& y_bus, std::shared_ptr<MathModelTopology const> const& topo_ptr);
template std::shared_ptr<LinearPFSolver<asymmetric_t>>
create_linear_pf_solver(YBus<asymmetric_t> const& y_bus, std::shared_ptr<MathModelTopology const> const& topo_ptr);

template <symmetry_tag sym>
SolverOutput<sym> run_power_flow_linear(std::shared_ptr<LinearPFSolver<sym>> linear_pf_solver,
                                        PowerFlowInput<sym> const& input, double /*err_tol*/, Idx /*max_iter*/,
                                        CalculationInfo& calculation_info, YBus<sym> const& y_bus) {
    assert(linear_pf_solver != nullptr);
    return linear_pf_solver->run_power_flow(y_bus, input, calculation_info);
}
template SolverOutput<symmetric_t> run_power_flow_linear(std::shared_ptr<LinearPFSolver<symmetric_t>> linear_pf_solver,
                                                         PowerFlowInput<symmetric_t> const& input, double err_tol,
                                                         Idx max_iter, CalculationInfo& calculation_info,
                                                         YBus<symmetric_t> const& y_bus);
template SolverOutput<asymmetric_t>
run_power_flow_linear(std::shared_ptr<LinearPFSolver<asymmetric_t>> linear_pf_solver,
                      PowerFlowInput<asymmetric_t> const& input, double err_tol, Idx max_iter,
                      CalculationInfo& calculation_info, YBus<asymmetric_t> const& y_bus);
} // namespace power_grid_model::math_solver::linear_pf
