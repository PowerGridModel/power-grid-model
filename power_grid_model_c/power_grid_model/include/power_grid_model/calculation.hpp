// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "main_model_fwd.hpp"

#include "common/common.hpp"
#include "common/enum.hpp"

#include "main_core/calculation_input_preparation.hpp"
#include "main_core/main_model_type.hpp"

namespace power_grid_model {

struct calculation_type_t {};
struct power_flow_t : calculation_type_t {};
struct state_estimation_t : calculation_type_t {};
struct short_circuit_t : calculation_type_t {};

template <typename T>
concept calculation_type_tag = std::derived_from<T, calculation_type_t>;

template <class Functor, class... Args>
decltype(auto) calculation_symmetry_func_selector(CalculationSymmetry calculation_symmetry, Functor&& f,
                                                  Args&&... args) {
    using enum CalculationSymmetry;

    switch (calculation_symmetry) {
    case symmetric:
        return std::forward<Functor>(f).template operator()<symmetric_t>(std::forward<Args>(args)...);
    case asymmetric:
        return std::forward<Functor>(f).template operator()<asymmetric_t>(std::forward<Args>(args)...);
    default:
        throw MissingCaseForEnumError{"Calculation symmetry selector", calculation_symmetry};
    }
}

template <class Functor, class... Args>
decltype(auto) calculation_type_func_selector(CalculationType calculation_type, Functor&& f, Args&&... args) {
    using enum CalculationType;

    switch (calculation_type) {
    case CalculationType::power_flow:
        return std::forward<Functor>(f).template operator()<power_flow_t>(std::forward<Args>(args)...);
    case CalculationType::state_estimation:
        return std::forward<Functor>(f).template operator()<state_estimation_t>(std::forward<Args>(args)...);
    case CalculationType::short_circuit:
        return std::forward<Functor>(f).template operator()<short_circuit_t>(std::forward<Args>(args)...);
    default:
        throw MissingCaseForEnumError{"CalculationType", calculation_type};
    }
}

template <class Functor, class... Args>
decltype(auto) calculation_type_symmetry_func_selector(CalculationType calculation_type,
                                                       CalculationSymmetry calculation_symmetry, Functor&& f,
                                                       Args&&... args) {
    calculation_type_func_selector(
        calculation_type,
        []<calculation_type_tag calculation_type, typename Functor_, typename... Args_>(
            CalculationSymmetry calculation_symmetry_, Functor_&& f_, Args_&&... args_) {
            calculation_symmetry_func_selector(
                calculation_symmetry_,
                []<symmetry_tag sym, typename SubFunctor, typename... SubArgs>(SubFunctor&& sub_f,
                                                                               SubArgs&&... sub_args) {
                    std::forward<SubFunctor>(sub_f).template operator()<calculation_type, sym>(
                        std::forward<SubArgs>(sub_args)...);
                },
                std::forward<Functor_>(f_), std::forward<Args_>(args_)...);
        },
        calculation_symmetry, std::forward<Functor>(f), std::forward<Args>(args)...);
}

template <typename T, typename sym> struct Calculator;
template <symmetry_tag sym> struct Calculator<power_flow_t, sym> {
    template <typename State>
    static auto preparer(State const& state, ComponentToMathCoupling& /*comp_coup*/,
                         MainModelOptions const& /*options*/) {
        return [&state](Idx n_math_solvers) { return main_core::prepare_power_flow_input<sym>(state, n_math_solvers); };
    }
    static auto solver(CalculationMethod calculation_method, MainModelOptions const& options, Logger& logger) {
        return [calculation_method, err_tol = options.err_tol, max_iter = options.max_iter,
                &logger](MathSolverProxy<sym>& solver, YBus<sym> const& y_bus, PowerFlowInput<sym> const& input) {
            return solver.get().run_power_flow(input, err_tol, max_iter, logger, calculation_method, y_bus);
        };
    }
};
template <symmetry_tag sym> struct Calculator<state_estimation_t, sym> {
    template <typename State>
    static auto preparer(State const& state, ComponentToMathCoupling& /*comp_coup*/,
                         MainModelOptions const& /*options*/) {
        return [&state](Idx n_math_solvers) {
            return main_core::prepare_state_estimation_input<sym>(state, n_math_solvers);
        };
    }
    static auto solver(CalculationMethod calculation_method, MainModelOptions const& options, Logger& logger) {
        return [calculation_method, err_tol = options.err_tol, max_iter = options.max_iter,
                &logger](MathSolverProxy<sym>& solver, YBus<sym> const& y_bus, StateEstimationInput<sym> const& input) {
            return solver.get().run_state_estimation(input, err_tol, max_iter, logger, calculation_method, y_bus);
        };
    }
};

template <symmetry_tag sym> struct Calculator<short_circuit_t, sym> {
    template <typename State>
    static auto preparer(State const& state, ComponentToMathCoupling& comp_coup, MainModelOptions const& options) {
        return [&state, &comp_coup, voltage_scaling = options.short_circuit_voltage_scaling](Idx n_math_solvers) {
            // assert(solvers_cache_status.is_topology_valid());
            // assert(solvers_cache_status.template is_parameter_valid<sym>());
            return main_core::prepare_short_circuit_input<sym>(state, comp_coup, n_math_solvers, voltage_scaling);
        };
    }
    static auto solver(CalculationMethod calculation_method, MainModelOptions const& /*options*/, Logger& logger) {
        return [calculation_method, &logger](MathSolverProxy<sym>& solver, YBus<sym> const& y_bus,
                                             ShortCircuitInput const& input) {
            return solver.get().run_short_circuit(input, logger, calculation_method, y_bus);
        };
    }
};

} // namespace power_grid_model
