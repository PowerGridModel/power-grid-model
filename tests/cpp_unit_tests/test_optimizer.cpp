// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/optimizer/optimizer.hpp>

#include <doctest/doctest.h>

namespace power_grid_model::optimizer {
namespace {
struct StubComponentContainer {};

using StubState = main_core::MainModelState<StubComponentContainer>;
static_assert(main_core::main_model_state_c<StubState>);

struct StubStateCalculatorResultType {
    Idx x{};
};

struct StubUpdateType {};

using StubStateCalculator = StubStateCalculatorResultType (*)(StubState const& /* state */);
using SymStubSteadyStateCalculator = std::vector<MathOutput<symmetric_t>> (*)(StubState const& /* state */);
using AsymStubSteadyStateCalculator = std::vector<MathOutput<asymmetric_t>> (*)(StubState const& /* state */);
using StubUpdate = void (*)(StubUpdateType const& /* update_data */);
using ConstDatasetUpdate = void (*)(ConstDataset const& /* update_data */);

static_assert(std::invocable<StubStateCalculator, StubState const&>);
static_assert(std::same_as<std::invoke_result_t<StubStateCalculator, StubState const&>, StubStateCalculatorResultType>);
static_assert(std::invocable<SymStubSteadyStateCalculator, StubState const&>);
static_assert(std::same_as<std::invoke_result_t<SymStubSteadyStateCalculator, StubState const&>,
                           std::vector<MathOutput<symmetric_t>>>);
static_assert(std::invocable<SymStubSteadyStateCalculator, StubState const&>);
static_assert(std::same_as<std::invoke_result_t<AsymStubSteadyStateCalculator, StubState const&>,
                           std::vector<MathOutput<asymmetric_t>>>);
static_assert(std::invocable<StubUpdate, StubUpdateType const&>);
static_assert(std::invocable<ConstDatasetUpdate, ConstDataset const&>);

static_assert(optimizer_c<NoopOptimizer<StubStateCalculator, StubState>>);
static_assert(optimizer_c<TapPositionOptimizer<SymStubSteadyStateCalculator, ConstDatasetUpdate, StubState>>);
static_assert(optimizer_c<TapPositionOptimizer<AsymStubSteadyStateCalculator, ConstDatasetUpdate, StubState>>);

constexpr auto mock_state_calculator(StubState const& /* state */) { return StubStateCalculatorResultType{.x = 1}; }
static_assert(std::convertible_to<decltype(mock_state_calculator), StubStateCalculator>);

template <symmetry_tag sym> constexpr auto stub_steady_state_state_calculator(StubState const& /* state */) {
    return std::vector<MathOutput<sym>>{};
}
static_assert(
    std::convertible_to<decltype(stub_steady_state_state_calculator<symmetric_t>), SymStubSteadyStateCalculator>);
static_assert(
    std::convertible_to<decltype(stub_steady_state_state_calculator<asymmetric_t>), AsymStubSteadyStateCalculator>);

constexpr auto stub_update(StubUpdateType const& /* update_data */){};
static_assert(std::convertible_to<decltype(stub_update), StubUpdate>);

constexpr auto stub_const_dataset_update(ConstDataset const& /* update_data */){};
static_assert(std::convertible_to<decltype(stub_const_dataset_update), ConstDatasetUpdate>);
} // namespace

TEST_CASE("Test no-op optimizer") {
    auto optimizer = NoopOptimizer<StubStateCalculator, StubState>{mock_state_calculator};
    CHECK(optimizer.optimize({}).x == 1);
    CHECK(optimizer.optimize({}).x == 1);
}

TEST_CASE("Test tap position optimizer") {
    SUBCASE("symmetric") {
        auto optimizer = TapPositionOptimizer<SymStubSteadyStateCalculator, ConstDatasetUpdate, StubState>{
            stub_steady_state_state_calculator<symmetric_t>, stub_const_dataset_update};
        CHECK_THROWS_AS(optimizer.optimize({}), PowerGridError); // TODO to be overloaded
    }
    SUBCASE("asymmetric") {
        auto optimizer = TapPositionOptimizer<AsymStubSteadyStateCalculator, ConstDatasetUpdate, StubState>{
            stub_steady_state_state_calculator<asymmetric_t>, stub_const_dataset_update};
        CHECK_THROWS_AS(optimizer.optimize({}), PowerGridError); // TODO to be overloaded
    }
}

TEST_CASE("Test get optimizer") {
    using enum OptimizerType;

    SUBCASE("Stub state calculator") {
        SUBCASE("Noop") {
            auto optimizer = get_optimizer<StubState, StubUpdateType>(noop, mock_state_calculator, stub_update);
            CHECK(optimizer->optimize({}).x == 1);
        }

        SUBCASE("Not implemented type") {
            CHECK_THROWS_AS((get_optimizer<StubState, StubUpdateType>(automatic_tap_adjustment, mock_state_calculator,
                                                                      stub_update)),
                            MissingCaseForEnumError<OptimizerType>);
        }
    }

    SUBCASE("Symmetric state calculator") {
        auto const get_instance = [](OptimizerType optimizer_type) {
            return get_optimizer<StubState, ConstDataset>(
                optimizer_type, stub_steady_state_state_calculator<symmetric_t>, stub_const_dataset_update);
        };

        SUBCASE("Noop") {
            auto optimizer = get_instance(noop);
            CHECK(optimizer->optimize({}).empty());
        }
        SUBCASE("Automatic tap adjustment") {
            auto optimizer = get_instance(automatic_tap_adjustment);
            CHECK_THROWS_AS(optimizer->optimize({}), PowerGridError); // TODO to be overloaded
        }
    }
}
} // namespace power_grid_model::optimizer
