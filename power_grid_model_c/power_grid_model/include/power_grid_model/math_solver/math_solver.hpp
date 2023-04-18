// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MATH_SOLVER_MATH_SOLVER_HPP
#define POWER_GRID_MODEL_MATH_SOLVER_MATH_SOLVER_HPP

#include <optional>

#include "../calculation_parameters.hpp"
#include "../exception.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"
#include "../timer.hpp"
#include "linear_pf_solver.hpp"
// clang-format off
#include "newton_raphson_pf_solver.hpp"
#include "iterative_linear_se_solver.hpp"
#include "iterative_current_pf_solver.hpp"
// clang-format on
#include "y_bus.hpp"

namespace power_grid_model {

template <bool sym>
class MathSolver {
   public:
    MathSolver(std::shared_ptr<MathModelTopology const> const& topo_ptr,
               std::shared_ptr<MathModelParam<sym> const> const& param,
               std::shared_ptr<YBusStructure const> const& y_bus_struct = {})
        : topo_ptr_{topo_ptr},
          y_bus_{topo_ptr, param, y_bus_struct},
          all_const_y_{std::all_of(topo_ptr->load_gen_type.cbegin(), topo_ptr->load_gen_type.cend(), [](LoadGenType x) {
              return x == LoadGenType::const_y;
          })} {
    }

    MathOutput<sym> run_power_flow(PowerFlowInput<sym> const& input, double err_tol, Idx max_iter,
                                   CalculationInfo& calculation_info, CalculationMethod calculation_method) {
        // set method to always linear if there are all load_gen with const_y
        calculation_method = all_const_y_ ? CalculationMethod::linear : calculation_method;
        if (calculation_method == CalculationMethod::newton_raphson) {
            if (!newton_pf_solver_.has_value()) {
                Timer timer(calculation_info, 2210, "Create math solver");
                newton_pf_solver_.emplace(y_bus_, topo_ptr_);
            }
            return newton_pf_solver_.value().run_power_flow(y_bus_, input, err_tol, max_iter, calculation_info);
        }
        else if (calculation_method == CalculationMethod::linear) {
            if (!linear_pf_solver_.has_value()) {
                Timer timer(calculation_info, 2210, "Create math solver");
                linear_pf_solver_.emplace(y_bus_, topo_ptr_);
            }
            return linear_pf_solver_.value().run_power_flow(y_bus_, input, calculation_info);
        }

        else if (calculation_method == CalculationMethod::iterative_current ||
                 calculation_method == CalculationMethod::linear_current) {
            if (!iterative_current_pf_solver_.has_value()) {
                Timer timer(calculation_info, 2210, "Create math solver");
                iterative_current_pf_solver_.emplace(y_bus_, topo_ptr_);
            }
            if (calculation_method == CalculationMethod::linear_current) {
                err_tol = 1000;
                max_iter = 2;
            }
            return iterative_current_pf_solver_.value().run_power_flow(y_bus_, input, err_tol, max_iter,
                                                                       calculation_info);
        }

        else {
            throw InvalidCalculationMethod{};
        }
    }

    MathOutput<sym> run_state_estimation(StateEstimationInput<sym> const& input, double err_tol, Idx max_iter,
                                         CalculationInfo& calculation_info, CalculationMethod calculation_method) {
        if (calculation_method != CalculationMethod::iterative_linear) {
            throw InvalidCalculationMethod{};
        }

        // construct model if needed
        if (!iterative_linear_se_solver_.has_value()) {
            Timer timer(calculation_info, 2210, "Create math solver");
            iterative_linear_se_solver_.emplace(y_bus_, topo_ptr_);
        }

        // call calculation
        return iterative_linear_se_solver_.value().run_state_estimation(y_bus_, input, err_tol, max_iter,
                                                                        calculation_info);
    }

    void clear_solver() {
        newton_pf_solver_.reset();
        linear_pf_solver_.reset();
        iterative_current_pf_solver_.reset();
        iterative_linear_se_solver_.reset();
    }

    void update_value(std::shared_ptr<MathModelParam<sym> const> const& math_model_param) {
        y_bus_.update_admittance(math_model_param);
    }

    std::shared_ptr<YBusStructure const> shared_y_bus_struct() const {
        return y_bus_.shared_y_bus_struct();
    }

   private:
    std::shared_ptr<MathModelTopology const> topo_ptr_;
    YBus<sym> y_bus_;
    bool all_const_y_;  // if all the load_gen is const element_admittance (impedance) type
    std::optional<NewtonRaphsonPFSolver<sym>> newton_pf_solver_;
    std::optional<LinearPFSolver<sym>> linear_pf_solver_;
    std::optional<IterativeLinearSESolver<sym>> iterative_linear_se_solver_;
    std::optional<IterativeCurrentPFSolver<sym>> iterative_current_pf_solver_;
};

template class MathSolver<true>;
template class MathSolver<false>;

}  // namespace power_grid_model

#endif