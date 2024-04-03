// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/container.hpp>
#include <power_grid_model/optimizer/optimizer.hpp>

#include <doctest/doctest.h>

namespace power_grid_model::optimizer {
namespace {
using StubComponentContainer = Container<Transformer, ThreeWindingTransformer>;

using StubState = main_core::MainModelState<StubComponentContainer>;
static_assert(main_core::main_model_state_c<StubState>);

struct StubStateCalculatorResultType {
    Idx x{};
};

struct StubUpdateType {};

using StubStateCalculator = StubStateCalculatorResultType (*)(StubState const& /* state */,
                                                              CalculationMethod /* method */);
using SymStubSteadyStateCalculator = std::vector<MathOutput<symmetric_t>> (*)(StubState const& /* state */,
                                                                              CalculationMethod /* method */);
using AsymStubSteadyStateCalculator = std::vector<MathOutput<asymmetric_t>> (*)(StubState const& /* state */,
                                                                                CalculationMethod /* method */);
using StubUpdate = void (*)(StubUpdateType const& /* update_data */);
using ConstDatasetUpdate = void (*)(ConstDataset const& /* update_data */);

static_assert(std::invocable<StubStateCalculator, StubState const&, CalculationMethod>);
static_assert(std::same_as<std::invoke_result_t<StubStateCalculator, StubState const&, CalculationMethod>,
                           StubStateCalculatorResultType>);
static_assert(std::invocable<SymStubSteadyStateCalculator, StubState const&, CalculationMethod>);
static_assert(std::same_as<std::invoke_result_t<SymStubSteadyStateCalculator, StubState const&, CalculationMethod>,
                           std::vector<MathOutput<symmetric_t>>>);
static_assert(std::invocable<SymStubSteadyStateCalculator, StubState const&, CalculationMethod>);
static_assert(std::same_as<std::invoke_result_t<AsymStubSteadyStateCalculator, StubState const&, CalculationMethod>,
                           std::vector<MathOutput<asymmetric_t>>>);
static_assert(std::invocable<StubUpdate, StubUpdateType const&>);
static_assert(std::invocable<ConstDatasetUpdate, ConstDataset const&>);

static_assert(optimizer_c<NoOptimizer<StubStateCalculator, StubState>>);
static_assert(optimizer_c<TapPositionOptimizer<SymStubSteadyStateCalculator, ConstDatasetUpdate, StubState>>);
static_assert(optimizer_c<TapPositionOptimizer<AsymStubSteadyStateCalculator, ConstDatasetUpdate, StubState>>);

constexpr auto mock_state_calculator(StubState const& /* state */, CalculationMethod /* method */) {
    return StubStateCalculatorResultType{.x = 1};
}
static_assert(std::convertible_to<decltype(mock_state_calculator), StubStateCalculator>);

template <symmetry_tag sym>
constexpr auto stub_steady_state_state_calculator(StubState const& /* state */, CalculationMethod /* method */) {
    return std::vector<MathOutput<sym>>{};
}
static_assert(
    std::convertible_to<decltype(stub_steady_state_state_calculator<symmetric_t>), SymStubSteadyStateCalculator>);
static_assert(
    std::convertible_to<decltype(stub_steady_state_state_calculator<asymmetric_t>), AsymStubSteadyStateCalculator>);

constexpr void stub_update(StubUpdateType const& /* update_data */){
    // stub
};
static_assert(std::convertible_to<decltype(stub_update), StubUpdate>);

constexpr void stub_const_dataset_update(ConstDataset const& /* update_data */){
    // stub
};
static_assert(std::convertible_to<decltype(stub_const_dataset_update), ConstDatasetUpdate>);

constexpr auto strategies = [] {
    using enum OptimizerStrategy;
    return std::array{any, global_minimum, global_maximum, local_minimum, local_maximum};
}();

constexpr auto calculation_methods = [] {
    using enum CalculationMethod;
    return std::array{default_method,    linear,         linear_current, iterative_linear,
                      iterative_current, newton_raphson, iec60909};
}();

constexpr auto strategies_and_methods = [] {
    std::array<std::pair<OptimizerStrategy, CalculationMethod>, strategies.size() * calculation_methods.size()> result;
    size_t idx{};
    for (auto strategy : strategies) {
        for (auto method : calculation_methods) {
            result[idx++] = std::make_pair(strategy, method);
        }
    }
    return result;
}();
} // namespace

TEST_CASE("Test no-op optimizer") {
    for (auto method : calculation_methods) {
        CAPTURE(method);
        auto optimizer = NoOptimizer<StubStateCalculator, StubState>{mock_state_calculator};
        CHECK(optimizer.optimize({}, method).x == 1);
        CHECK(optimizer.optimize({}, method).x == 1);
    }
}

TEST_CASE("Test tap position optimizer") {
    SUBCASE("symmetric") {
        for (auto [strategy, method] : strategies_and_methods) {
            CAPTURE(strategy);
            CAPTURE(method);
            auto optimizer = TapPositionOptimizer<SymStubSteadyStateCalculator, ConstDatasetUpdate, StubState>{
                stub_steady_state_state_calculator<symmetric_t>, stub_const_dataset_update, strategy};
            CHECK(optimizer.optimize({}, method).empty());
        }
    }
    SUBCASE("asymmetric") {
        for (auto [strategy, method] : strategies_and_methods) {
            CAPTURE(strategy);
            CAPTURE(method);
            auto optimizer = TapPositionOptimizer<AsymStubSteadyStateCalculator, ConstDatasetUpdate, StubState>{
                stub_steady_state_state_calculator<asymmetric_t>, stub_const_dataset_update, strategy};
            CHECK(optimizer.optimize({}, method).empty());
        }
    }
}

TEST_CASE("Test get optimizer") {
    using enum OptimizerType;

    SUBCASE("Stub state calculator") {
        SUBCASE("Noop") {
            for (auto [strategy, method] : strategies_and_methods) {
                CAPTURE(strategy);
                CAPTURE(method);
                auto optimizer = get_optimizer<StubState, StubUpdateType>(no_optimization, strategy,
                                                                          mock_state_calculator, stub_update);
                CHECK(optimizer->optimize({}, method).x == 1);
            }
        }

        SUBCASE("Not implemented type") {
            for (auto strategy : strategies) {
                CAPTURE(strategy);
                CHECK_THROWS_AS((get_optimizer<StubState, StubUpdateType>(automatic_tap_adjustment, strategy,
                                                                          mock_state_calculator, stub_update)),
                                MissingCaseForEnumError<OptimizerType>);
            }
        }
    }

    SUBCASE("Symmetric state calculator") {
        auto const get_instance = [](OptimizerType optimizer_type, OptimizerStrategy strategy) {
            return get_optimizer<StubState, ConstDataset>(
                optimizer_type, strategy, stub_steady_state_state_calculator<symmetric_t>, stub_const_dataset_update);
        };

        SUBCASE("Noop") {
            for (auto [strategy, method] : strategies_and_methods) {
                CAPTURE(strategy);
                CAPTURE(method);
                auto optimizer = get_instance(no_optimization, strategy);
                CHECK(optimizer->optimize({}, method).empty());
            }
        }
        SUBCASE("Automatic tap adjustment") {
            for (auto [strategy, method] : strategies_and_methods) {
                CAPTURE(strategy);
                CAPTURE(method);
                auto optimizer = get_instance(automatic_tap_adjustment, strategy);

                auto tap_optimizer = std::dynamic_pointer_cast<
                    TapPositionOptimizer<SymStubSteadyStateCalculator, ConstDatasetUpdate, StubState>>(optimizer);
                REQUIRE(tap_optimizer != nullptr);
                CHECK(tap_optimizer->get_strategy() == strategy);

                CHECK(optimizer->optimize({}, method).empty());
            }
        }
    }
}
} // namespace power_grid_model::optimizer
