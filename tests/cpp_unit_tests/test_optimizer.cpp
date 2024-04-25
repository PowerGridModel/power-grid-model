// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "test_optimizer.hpp"

namespace power_grid_model::optimizer::test {
TEST_CASE("Test construct no-op optimizer") {
    for (auto method : calculation_methods) {
        CAPTURE(method);
        auto optimizer = NoOptimizer<StubStateCalculator, StubState>{mock_state_calculator};
        CHECK(optimizer.optimize({}, method).x == 1);
        CHECK(optimizer.optimize({}, method).x == 1);
    }
}

TEST_CASE("Test construct tap position optimizer") {
    StubState empty_state{};
    empty_state.components.set_construction_complete();

    SUBCASE("symmetric") {
        for (auto [strategy, method] : strategies_and_methods) {
            CAPTURE(strategy);
            CAPTURE(method);
            auto optimizer = TapPositionOptimizer<SymStubSteadyStateCalculator, ConstDatasetUpdate, StubState>{
                stub_steady_state_state_calculator<symmetric_t>, stub_const_dataset_update, strategy};
            CHECK(optimizer.optimize(empty_state, method).empty());
        }
    }
    SUBCASE("asymmetric") {
        for (auto [strategy, method] : strategies_and_methods) {
            CAPTURE(strategy);
            CAPTURE(method);
            auto optimizer = TapPositionOptimizer<AsymStubSteadyStateCalculator, ConstDatasetUpdate, StubState>{
                stub_steady_state_state_calculator<asymmetric_t>, stub_const_dataset_update, strategy};
            CHECK(optimizer.optimize(empty_state, method).empty());
        }
    }
}

TEST_CASE("Test get optimizer") {
    using enum OptimizerType;

    StubState empty_state;
    empty_state.components.set_construction_complete();

    SUBCASE("Stub state calculator") {
        SUBCASE("Noop") {
            for (auto [strategy, method] : strategies_and_methods) {
                CAPTURE(strategy);
                CAPTURE(method);
                auto optimizer = get_optimizer<StubState, StubUpdateType>(no_optimization, strategy,
                                                                          mock_state_calculator, stub_update);
                CHECK(optimizer->optimize(empty_state, method).x == 1);
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
                CHECK(optimizer->optimize(empty_state, method).empty());
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

                StubState empty_state{};
                empty_state.components.set_construction_complete();
                CHECK(optimizer->optimize(empty_state, method).empty());
            }
        }
    }
}
} // namespace power_grid_model::optimizer::test
