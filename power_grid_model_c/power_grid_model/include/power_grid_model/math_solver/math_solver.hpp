// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "iterative_current_pf_solver.hpp"
#include "iterative_linear_se_solver.hpp"
#include "linear_pf_solver.hpp"
#include "newton_raphson_pf_solver.hpp"
#include "newton_raphson_se_solver.hpp"
#include "short_circuit_solver.hpp"
#include "y_bus.hpp"

#include "../calculation_parameters.hpp"
#include "../common/common.hpp"
#include "../common/exception.hpp"
#include "../common/three_phase_tensor.hpp"
#include "../common/timer.hpp"

#include <optional>

namespace power_grid_model {

namespace math_solver {

template <symmetry_tag sym> class MathSolver {
  public:
    explicit MathSolver(std::shared_ptr<MathModelTopology const> const& topo_ptr)
        : topo_ptr_{topo_ptr},
          all_const_y_{std::all_of(topo_ptr->load_gen_type.cbegin(), topo_ptr->load_gen_type.cend(),
                                   [](LoadGenType x) { return x == LoadGenType::const_y; })} {}

    SolverOutput<sym> run_power_flow(PowerFlowInput<sym> const& input, double err_tol, Idx max_iter,
                                     CalculationInfo& calculation_info, CalculationMethod calculation_method,
                                     YBus<sym> const& y_bus) {
        using enum CalculationMethod;

        // set method to always linear if all load_gens have const_y
        calculation_method = all_const_y_ ? linear : calculation_method;

        switch (calculation_method) {
        case default_method:
            [[fallthrough]]; // use Newton-Raphson by default
        case newton_raphson:
            return run_power_flow_newton_raphson(input, err_tol, max_iter, calculation_info, y_bus);
        case linear:
            return run_power_flow_linear(input, err_tol, max_iter, calculation_info, y_bus);
        case linear_current:
            return run_power_flow_linear_current(input, err_tol, max_iter, calculation_info, y_bus);
        case iterative_current:
            return run_power_flow_iterative_current(input, err_tol, max_iter, calculation_info, y_bus);
        default:
            throw InvalidCalculationMethod{};
        }
    }

    SolverOutput<sym> run_state_estimation(StateEstimationInput<sym> const& input, double err_tol, Idx max_iter,
                                           CalculationInfo& calculation_info, CalculationMethod calculation_method,
                                           YBus<sym> const& y_bus) {
        using enum CalculationMethod;

        switch (calculation_method) {
        case default_method:
            [[fallthrough]]; // use iterative linear by default
        case iterative_linear:
            return run_state_estimation_iterative_linear(input, err_tol, max_iter, calculation_info, y_bus);
        case newton_raphson:
            return run_state_estimation_newton_raphson(input, err_tol, max_iter, calculation_info, y_bus);
        default:
            throw InvalidCalculationMethod{};
        }
    }

    ShortCircuitSolverOutput<sym> run_short_circuit(ShortCircuitInput const& input, CalculationInfo& calculation_info,
                                                    CalculationMethod calculation_method, YBus<sym> const& y_bus) {
        if (calculation_method != CalculationMethod::default_method &&
            calculation_method != CalculationMethod::iec60909) {
            throw InvalidCalculationMethod{};
        }

        // construct model if needed
        if (!iec60909_sc_solver_.has_value()) {
            Timer const timer(calculation_info, 2210, "Create math solver");
            iec60909_sc_solver_.emplace(y_bus, topo_ptr_);
        }

        // call calculation
        return iec60909_sc_solver_.value().run_short_circuit(y_bus, input);
    }

    void clear_solver() {
        newton_raphson_pf_solver_.reset();
        linear_pf_solver_.reset();
        iterative_current_pf_solver_.reset();
        iterative_linear_se_solver_.reset();
    }

    void parameters_changed(bool changed) {
        if (iterative_current_pf_solver_.has_value()) {
            iterative_current_pf_solver_->parameters_changed(changed);
        }
    }

  private:
    std::shared_ptr<MathModelTopology const> topo_ptr_;
    bool all_const_y_; // if all the load_gen is const element_admittance (impedance) type
    std::optional<NewtonRaphsonPFSolver<sym>> newton_raphson_pf_solver_;
    std::optional<LinearPFSolver<sym>> linear_pf_solver_;
    std::optional<IterativeCurrentPFSolver<sym>> iterative_current_pf_solver_;
    std::optional<IterativeLinearSESolver<sym>> iterative_linear_se_solver_;
    std::optional<NewtonRaphsonSESolver<sym>> newton_raphson_se_solver_;
    std::optional<ShortCircuitSolver<sym>> iec60909_sc_solver_;

    SolverOutput<sym> run_power_flow_newton_raphson(PowerFlowInput<sym> const& input, double err_tol, Idx max_iter,
                                                    CalculationInfo& calculation_info, YBus<sym> const& y_bus) {
        if (!newton_raphson_pf_solver_.has_value()) {
            Timer const timer(calculation_info, 2210, "Create math solver");
            newton_raphson_pf_solver_.emplace(y_bus, topo_ptr_);
        }
        return newton_raphson_pf_solver_.value().run_power_flow(y_bus, input, err_tol, max_iter, calculation_info);
    }

    SolverOutput<sym> run_power_flow_linear(PowerFlowInput<sym> const& input, double /* err_tol */, Idx /* max_iter */,
                                            CalculationInfo& calculation_info, YBus<sym> const& y_bus) {
        if (!linear_pf_solver_.has_value()) {
            Timer const timer(calculation_info, 2210, "Create math solver");
            linear_pf_solver_.emplace(y_bus, topo_ptr_);
        }
        return linear_pf_solver_.value().run_power_flow(y_bus, input, calculation_info);
    }

    SolverOutput<sym> run_power_flow_iterative_current(PowerFlowInput<sym> const& input, double err_tol, Idx max_iter,
                                                       CalculationInfo& calculation_info, YBus<sym> const& y_bus) {
        if (!iterative_current_pf_solver_.has_value()) {
            Timer const timer(calculation_info, 2210, "Create math solver");
            iterative_current_pf_solver_.emplace(y_bus, topo_ptr_);
        }
        return iterative_current_pf_solver_.value().run_power_flow(y_bus, input, err_tol, max_iter, calculation_info);
    }

    SolverOutput<sym> run_power_flow_linear_current(PowerFlowInput<sym> const& input, double /* err_tol */,
                                                    Idx /* max_iter */, CalculationInfo& calculation_info,
                                                    YBus<sym> const& y_bus) {
        return run_power_flow_iterative_current(input, std::numeric_limits<double>::infinity(), 1, calculation_info,
                                                y_bus);
    }

    SolverOutput<sym> run_state_estimation_iterative_linear(StateEstimationInput<sym> const& input, double err_tol,
                                                            Idx max_iter, CalculationInfo& calculation_info,
                                                            YBus<sym> const& y_bus) {
        // construct model if needed
        if (!iterative_linear_se_solver_.has_value()) {
            Timer const timer(calculation_info, 2210, "Create math solver");
            iterative_linear_se_solver_.emplace(y_bus, topo_ptr_);
        }

        // call calculation
        return iterative_linear_se_solver_.value().run_state_estimation(y_bus, input, err_tol, max_iter,
                                                                        calculation_info);
    }

    SolverOutput<sym> run_state_estimation_newton_raphson(StateEstimationInput<sym> const& input, double err_tol,
                                                          Idx max_iter, CalculationInfo& calculation_info,
                                                          YBus<sym> const& y_bus) {
        // construct model if needed
        if (!newton_raphson_se_solver_.has_value()) {
            Timer const timer(calculation_info, 2210, "Create math solver");
            newton_raphson_se_solver_.emplace(y_bus, topo_ptr_);
        }

        // call calculation
        return newton_raphson_se_solver_.value().run_state_estimation(y_bus, input, err_tol, max_iter,
                                                                      calculation_info);
    }
};

template class MathSolver<symmetric_t>;
template class MathSolver<asymmetric_t>;
} // namespace math_solver

using math_solver::MathSolver;

} // namespace power_grid_model
