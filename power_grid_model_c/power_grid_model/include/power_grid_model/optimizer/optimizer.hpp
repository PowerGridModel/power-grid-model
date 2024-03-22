// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "base_optimizer.hpp"
#include "tap_position_optimizer.hpp"

#include "../common/enum.hpp"

#include <memory>

namespace power_grid_model::optimizer {

template <typename State, typename UpdateType, typename StateCalculator, typename StateUpdater>
    requires main_core::main_model_state_c<State> && std::invocable<std::remove_cvref_t<StateCalculator>, State&> &&
             std::invocable<std::remove_cvref_t<StateUpdater>, UpdateType>
constexpr auto get_optimizer(OptimizerType optimizer_type, OptimizerStrategy strategy, StateCalculator calculator,
                             StateUpdater updater) {
    using enum OptimizerType;
    using namespace std::string_literals;
    using BaseOptimizer = detail::BaseOptimizer<StateCalculator, State>;

    switch (optimizer_type) {
    case no_optimization:
        return BaseOptimizer::template make_shared<NoOptimizer<StateCalculator, State>>(std::move(calculator));
    case automatic_tap_adjustment:
        if constexpr (detail::steady_state_calculator_c<StateCalculator, State> &&
                      std::invocable<std::remove_cvref_t<StateUpdater>, ConstDataset const&>) {
            return BaseOptimizer::template make_shared<TapPositionOptimizer<StateCalculator, StateUpdater, State>>(
                std::move(calculator), std::move(updater), strategy);
        }
        [[fallthrough]];
    default:
        throw MissingCaseForEnumError{"optimizer::get_optimizer"s, optimizer_type};
    }
}

} // namespace power_grid_model::optimizer
