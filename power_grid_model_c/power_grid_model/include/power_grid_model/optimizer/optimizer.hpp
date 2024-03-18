// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../main_core/state.hpp"

#include <concepts>

namespace power_grid_model::optimizer {
// template <typename T, typename State, typename Func, typename OutputType>
//     requires main_core::main_model_state_c<State> && math_output_type<OutputType> &&
//                  std::invocable<std::remove_cvref_t<Func>, State const&> &&
//                  std::convertible_to<std::invoke_result_t<std::remove_cvref_t<Func>, State const&>, OutputType>
// concept optimizer_c = requires(State& state, Func state_calculator) {
//     { T::optimize(state, state_calculator); } -> std::same_as<OutputType>;
// };

template <typename Func, typename State>
    requires main_core::main_model_state_c<State> && std::invocable<std::remove_cvref_t<Func>, State const&>
class NoopOptimizer {
  public:
    NoopOptimizer(Func func) : func_{std::move(func)} {}

    auto optimize(State const& state) { return func_(state); }

  private:
    Func func_;
};

// template <typename Func, typename... Ts>
//     requires std::invocable<std::remove_cvref_t<Func>, Ts...> &&
//              steady_state_math_output_type<std::invoke_result_t<std::remove_cvref_t<Func>, Ts...>>
// class TapPositionOptimizer {
//   public:
//     TapPositionOptimizer(Func func) : func_{std::move(func)} {}

//     auto optimize(main_core::MainModelState const& state) {
//         return func(state);
//     }

//   private:
//     Func func_;
// };

// template <typename Func, typename... Ts>
//     requires std::invocable<std::remove_cvref_t<Func>, Ts...>
// optimizer(OptimizerType optimizer_type, Func func) {
//     using enum OptimizerType;
//     switch (optimizer_type) {
//     case noop_optimizer:
//         return [&](State const& state) { NoopOptimizer{func}::optimize(state) };
//     case tap_position_optimizer:
//         return [&](State const& state) { NoopOptimizer{func}::optimize(state) };
//     default:
//         throw;
//     }
// }
} // namespace power_grid_model::optimizer
