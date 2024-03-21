// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "base_optimizer.hpp"

#include "../auxiliary/dataset.hpp"
#include "../common/enum.hpp"

namespace power_grid_model::optimizer {

namespace detail {
template <main_core::main_model_state_c State> auto rank_transformers(State const& state) {
    // TODO: rank Idx2D of transformers as listed in the container
    return std::vector<Idx2D>{};
}
} // namespace detail

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

    TapPositionOptimizer(Calculator calculate, StateUpdater update, OptimizerStrategy strategy)
        : calculate_{std::move(calculate)}, update_{std::move(update)}, strategy_{strategy} {}

    virtual auto optimize(State const& state) -> ResultType final {
        auto const order = detail::rank_transformers(state);
        return optimize(state, order);
    }

    constexpr auto strategy() { return strategy_; }

  private:
    auto optimize(State const& state, std::vector<Idx2D> const& order) -> ResultType { throw PowerGridError{}; }

    Calculator calculate_;
    StateUpdater update_;
    OptimizerStrategy strategy_;
};

} // namespace power_grid_model::optimizer
