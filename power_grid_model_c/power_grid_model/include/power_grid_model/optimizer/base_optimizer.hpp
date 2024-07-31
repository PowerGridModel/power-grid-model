// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../main_core/state.hpp"

#include <concepts>

namespace power_grid_model::optimizer {

namespace detail {
template <typename StateCalculator, typename State>
concept state_calculator_c =
    main_core::main_model_state_c<State> &&
    std::invocable<std::remove_cvref_t<StateCalculator>, std::remove_cvref_t<State> const&, CalculationMethod>;

template <typename StateCalculator, typename State_>
    requires state_calculator_c<StateCalculator, State_>
struct state_calculator_type {
    using Calculator = StateCalculator;
    using State = State_;
    using result_type = std::invoke_result_t<std::remove_cvref_t<StateCalculator>, State_ const&, CalculationMethod>;
};

template <typename StateCalculator, typename State_>
using state_calculator_result_t = typename state_calculator_type<StateCalculator, State_>::result_type;

template <typename StateCalculator, typename State>
concept steady_state_calculator_c =
    steady_state_solver_output_type<typename detail::state_calculator_result_t<StateCalculator, State>::value_type> &&
    std::same_as<detail::state_calculator_result_t<StateCalculator, State>,
                 std::vector<typename detail::state_calculator_result_t<StateCalculator, State>::value_type>>;

template <typename StateCalculator, typename State_>
    requires state_calculator_c<StateCalculator, State_>
class BaseOptimizer {
  public:
    using Calculator = StateCalculator;
    using State = State_;
    using ResultType = state_calculator_result_t<Calculator, State>;

    BaseOptimizer() = default;
    BaseOptimizer(BaseOptimizer const&) = delete;
    BaseOptimizer(BaseOptimizer&&) noexcept = default;
    BaseOptimizer& operator=(BaseOptimizer const&) = delete;
    BaseOptimizer& operator=(BaseOptimizer&&) noexcept = default;
    virtual ~BaseOptimizer() = default;

    virtual auto optimize(State const& state, CalculationMethod method) -> MathOutput<ResultType> = 0;

    template <std::derived_from<BaseOptimizer> Optimizer, typename... Args>
        requires std::constructible_from<Optimizer, Args...>
    static auto make_shared(Args&&... args) -> std::shared_ptr<BaseOptimizer> {
        return std::static_pointer_cast<BaseOptimizer>(std::make_shared<Optimizer>(std::forward<Args>(args)...));
    }
};
} // namespace detail

template <typename Optimizer>
concept optimizer_c =
    detail::state_calculator_c<typename Optimizer::Calculator, typename Optimizer::State> &&
    requires(Optimizer optimizer, typename Optimizer::State const& state, CalculationMethod method) {
        {
            optimizer.optimize(state, method)
            } -> std::same_as<MathOutput<
                detail::state_calculator_result_t<typename Optimizer::Calculator, typename Optimizer::State>>>;
    };

template <typename StateCalculator, typename State_>
class NoOptimizer : public detail::BaseOptimizer<StateCalculator, State_> {
  public:
    using Base = detail::BaseOptimizer<StateCalculator, State_>;
    using typename Base::Calculator;
    using typename Base::ResultType;
    using typename Base::State;

    NoOptimizer(Calculator func) : func_{std::move(func)} {}

    auto optimize(State const& state, CalculationMethod method) -> MathOutput<ResultType> final {
        return {.solver_output = func_(state, method), .optimizer_output = {}};
    }

  private:
    Calculator func_;
};

} // namespace power_grid_model::optimizer
