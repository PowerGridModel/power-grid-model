// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../auxiliary/dataset.hpp"
#include "../main_core/state.hpp"

#include <concepts>
#include <memory>

namespace power_grid_model::optimizer {
// template <typename T, typename State, typename Func, typename OutputType>
//     // requires main_core::main_model_state_c<State> && math_output_type<OutputType> &&
//     //              std::invocable<std::remove_cvref_t<Func>, State const&> &&
//     //              std::convertible_to<std::invoke_result_t<std::remove_cvref_t<Func>, State const&>, OutputType>
// concept optimizer_c = requires(State& state, Func state_calculator) {
//     { T::optimize(state, state_calculator); } -> std::same_as<OutputType>;
// };

namespace detail {
template <typename StateCalculator, typename State>
concept state_calculator_c = main_core::main_model_state_c<State> &&
                             std::invocable<std::remove_cvref_t<StateCalculator>, std::remove_cvref_t<State> const&>;

template <typename StateCalculator, typename State_>
    requires state_calculator_c<StateCalculator, State_>
struct state_calculator_type {
    using Calculator = StateCalculator;
    using State = State_;
    using result_type = std::invoke_result_t<std::remove_cvref_t<StateCalculator>, State_ const&>;
};

template <typename StateCalculator, typename State_>
using state_calculator_result_t = typename state_calculator_type<StateCalculator, State_>::result_type;

template <typename StateCalculator, typename State>
concept steady_state_calculator_c =
    steady_state_math_output_type<typename detail::state_calculator_result_t<StateCalculator, State>::value_type> &&
    std::same_as<detail::state_calculator_result_t<StateCalculator, State>,
                 std::vector<typename detail::state_calculator_result_t<StateCalculator, State>::value_type>>;

template <typename StateCalculator, typename State_>
    requires main_core::main_model_state_c<State_> &&
             std::invocable<std::remove_cvref_t<StateCalculator>, State_ const&>
class BaseOptimizer {
  public:
    using Calculator = StateCalculator;
    using State = State_;
    using ResultType = state_calculator_result_t<Calculator, State>;

    virtual auto optimize(State const& state) -> ResultType = 0;

    template <std::derived_from<BaseOptimizer> Optimizer, typename... Args>
        requires std::constructible_from<Optimizer, Args...>
    static auto make_shared(Args... args) -> std::shared_ptr<BaseOptimizer> {
        return std::static_pointer_cast<BaseOptimizer>(std::make_shared<Optimizer>(std::move(args)...));
    }
};
} // namespace detail

template <typename Optimizer>
concept optimizer_c =
    detail::state_calculator_c<typename Optimizer::Calculator, typename Optimizer::State> &&
    requires(Optimizer optimizer, Optimizer::State& state) {
        {
            optimizer.optimize(state)
            } -> std::same_as<std::invoke_result_t<typename Optimizer::Calculator,
                                                   std::add_lvalue_reference_t<typename Optimizer::State>>>;
    };

template <typename StateCalculator, typename State_>
class NoopOptimizer : public detail::BaseOptimizer<StateCalculator, State_> {
  public:
    using Base = detail::BaseOptimizer<StateCalculator, State_>;
    using typename Base::Calculator;
    using typename Base::ResultType;
    using typename Base::State;

    NoopOptimizer(Calculator func) : func_{std::move(func)} {}

    virtual auto optimize(State const& state) -> ResultType final { return func_(state); }

  private:
    Calculator func_;
};

template <typename StateCalculator, typename StateUpdater_, typename State_>
    requires detail::steady_state_calculator_c<StateCalculator, State_> &&
             std::invocable<std::remove_cvref_t<StateUpdater_>, ConstDataset const&>
class TapPositionOptimizer : public detail::BaseOptimizer<StateCalculator, State_> {
  public:
    using Base = detail::BaseOptimizer<StateCalculator, State_>;
    using typename Base::Calculator;
    using typename Base::ResultType;
    using typename Base::State;
    using StateUpdater = StateUpdater_;

    TapPositionOptimizer(Calculator calculate, StateUpdater update)
        : calculate_{std::move(calculate)}, update_{std::move(update)} {}

    virtual auto optimize(State const& state) -> ResultType final { throw PowerGridError{}; }

  private:
    Calculator calculate_;
    StateUpdater update_;
};

template <typename State, typename UpdateType, typename StateCalculator, typename StateUpdater>
    requires main_core::main_model_state_c<State> && std::invocable<std::remove_cvref_t<StateCalculator>, State&> &&
             std::invocable<std::remove_cvref_t<StateUpdater>, UpdateType>
constexpr auto get_optimizer(OptimizerType optimizer_type, StateCalculator calculate, StateUpdater update) {
    using enum OptimizerType;
    using namespace std::string_literals;
    using BaseOptimizer = detail::BaseOptimizer<StateCalculator, State>;

    switch (optimizer_type) {
    case noop:
        return BaseOptimizer::template make_shared<NoopOptimizer<StateCalculator, State>>(std::move(calculate));
    case automatic_tap_adjustment:
        if constexpr (detail::steady_state_calculator_c<StateCalculator, State> &&
                      std::invocable<std::remove_cvref_t<StateUpdater>, ConstDataset const&>) {
            return BaseOptimizer::template make_shared<TapPositionOptimizer<StateCalculator, StateUpdater, State>>(
                std::move(calculate), std::move(update));
        }
        [[fallthrough]];
    default:
        throw MissingCaseForEnumError{"optimizer::get_optimizer"s, optimizer_type};
    }
}
} // namespace power_grid_model::optimizer
