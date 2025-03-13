// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/math_solver/newton_raphson_se_solver.hpp>
#include <power_grid_model/math_solver/newton_raphson_se_solver_fwd.hpp>

namespace power_grid_model::math_solver::newton_raphson_se {
template class NewtonRaphsonSESolver<symmetric_t>;
template class NewtonRaphsonSESolver<asymmetric_t>;

template <symmetry_tag sym>
std::shared_ptr<NewtonRaphsonSESolver<sym>>
create_newton_raphson_se_solver(YBus<sym> const& y_bus, std::shared_ptr<MathModelTopology const> const& topo_ptr) {
    return std::make_shared<NewtonRaphsonSESolver<sym>>(y_bus, topo_ptr);
}
template std::shared_ptr<NewtonRaphsonSESolver<symmetric_t>>
create_newton_raphson_se_solver(YBus<symmetric_t> const& y_bus,
                                std::shared_ptr<MathModelTopology const> const& topo_ptr);
template std::shared_ptr<NewtonRaphsonSESolver<asymmetric_t>>
create_newton_raphson_se_solver(YBus<asymmetric_t> const& y_bus,
                                std::shared_ptr<MathModelTopology const> const& topo_ptr);

template <symmetry_tag sym>
SolverOutput<sym>
run_state_estimation_newton_raphson(std::shared_ptr<NewtonRaphsonSESolver<sym>> newton_raphson_se_solver,
                                    StateEstimationInput<sym> const& input, double err_tol, Idx max_iter,
                                    CalculationInfo& calculation_info, YBus<sym> const& y_bus) {
    assert(newton_raphson_se_solver != nullptr);
    return newton_raphson_se_solver->run_state_estimation(y_bus, input, err_tol, max_iter, calculation_info);
}
template SolverOutput<symmetric_t>
run_state_estimation_newton_raphson(std::shared_ptr<NewtonRaphsonSESolver<symmetric_t>> newton_raphson_se_solver,
                                    StateEstimationInput<symmetric_t> const& input, double err_tol, Idx max_iter,
                                    CalculationInfo& calculation_info, YBus<symmetric_t> const& y_bus);
template SolverOutput<asymmetric_t>
run_state_estimation_newton_raphson(std::shared_ptr<NewtonRaphsonSESolver<asymmetric_t>> newton_raphson_se_solver,
                                    StateEstimationInput<asymmetric_t> const& input, double err_tol, Idx max_iter,
                                    CalculationInfo& calculation_info, YBus<asymmetric_t> const& y_bus);
} // namespace power_grid_model::math_solver::newton_raphson_se
