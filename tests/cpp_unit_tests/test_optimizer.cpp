// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "test_optimizer.hpp"

#include <power_grid_model/auxiliary/meta_data.hpp>

namespace power_grid_model::optimizer::test {
namespace {
namespace meta_gen = meta_data::meta_data_gen;
constexpr auto meta_data = meta_data::meta_data_gen::get_meta_data<
    ComponentList<Line, Link, Node, Transformer, ThreeWindingTransformer, TransformerTapRegulator, Source>,
    meta_data::meta_data_gen::dataset_mark<[] { return "update"; }, meta_data::update_getter_s>>::value;
} // namespace

TEST_CASE("Test construct no-op optimizer") {
    for (auto method : calculation_methods) {
        CAPTURE(method);
        auto optimizer = NoOptimizer<StubStateCalculator, StubState>{mock_state_calculator};
        CHECK(optimizer.optimize({}, method).solver_output.x == 1);
        CHECK(optimizer.optimize({}, method).solver_output.x == 1);
    }
}

TEST_CASE("Test construct tap position optimizer") {
    StubState empty_state{};
    empty_state.components.set_construction_complete();

    SUBCASE("symmetric") {
        for (auto strategy_method : strategies_and_methods) {
            for (auto search_method : search_methods) {
                if (strategy_method.strategy == OptimizerStrategy::any &&
                    search_method == SearchMethod::binary_search) {
                    continue;
                }
                CAPTURE(strategy_method.strategy);
                CAPTURE(strategy_method.method);
                CAPTURE(search_method);
                auto optimizer = TapPositionOptimizer<SymStubSteadyStateCalculator, ConstDatasetUpdate, StubState>{
                    stub_steady_state_state_calculator<symmetric_t>, stub_const_dataset_update,
                    strategy_method.strategy, meta_data, search_method};
                auto res = optimizer.optimize(empty_state, strategy_method.method);
                CHECK(optimizer.optimize(empty_state, strategy_method.method).solver_output.empty());
            }
        }
    }
    SUBCASE("asymmetric") {
        for (auto strategy_method : strategies_and_methods) {
            for (auto search_method : search_methods) {
                if (strategy_method.strategy == OptimizerStrategy::any &&
                    search_method == SearchMethod::binary_search) {
                    continue;
                }
                CAPTURE(strategy_method.strategy);
                CAPTURE(strategy_method.method);
                CAPTURE(search_method);
                auto optimizer = TapPositionOptimizer<AsymStubSteadyStateCalculator, ConstDatasetUpdate, StubState>{
                    stub_steady_state_state_calculator<asymmetric_t>, stub_const_dataset_update,
                    strategy_method.strategy, meta_data, search_method};
                CHECK(optimizer.optimize(empty_state, strategy_method.method).solver_output.empty());
            }
        }
    }
}

TEST_CASE("Test get optimizer") {
    using enum OptimizerType;

    StubState empty_state;
    empty_state.components.set_construction_complete();
    auto const search_methods = [] {
        using enum SearchMethod;
        return std::array{scanline, binary_search};
    }();

    SUBCASE("Stub state calculator") {
        SUBCASE("Noop") {
            for (auto strategy_method : strategies_and_methods) {
                CAPTURE(strategy_method.strategy);
                CAPTURE(strategy_method.method);
                auto optimizer = get_optimizer<StubState, StubUpdateType>(
                    no_optimization, strategy_method.strategy, mock_state_calculator, stub_update, meta_data);
                CHECK(optimizer->optimize(empty_state, strategy_method.method).solver_output.x == 1);
            }
        }

        SUBCASE("Not implemented type") {
            for (auto strategy : strategies) {
                CAPTURE(strategy);
                CHECK_THROWS_AS((get_optimizer<StubState, StubUpdateType>(
                                    automatic_tap_adjustment, strategy, mock_state_calculator, stub_update, meta_data)),
                                MissingCaseForEnumError);
            }
        }
    }

    SUBCASE("Symmetric state calculator") {
        auto const get_instance = [](OptimizerType optimizer_type, OptimizerStrategy strategy,
                                     SearchMethod search = SearchMethod::binary_search) {
            return get_optimizer<StubState, ConstDataset>(optimizer_type, strategy,
                                                          stub_steady_state_state_calculator<symmetric_t>,
                                                          stub_const_dataset_update, meta_data, search);
        };

        SUBCASE("Noop") {
            for (auto strategy_method : strategies_and_methods) {
                CAPTURE(strategy_method.strategy);
                CAPTURE(strategy_method.method);
                auto optimizer = get_instance(no_optimization, strategy_method.strategy);
                CHECK(optimizer->optimize(empty_state, strategy_method.method).solver_output.empty());
            }
        }
        SUBCASE("Automatic tap adjustment") {

            for (auto strategy_method : strategies_and_methods) {
                for (auto search_method : search_methods) {
                    CAPTURE(strategy_method.strategy);
                    CAPTURE(strategy_method.method);
                    CAPTURE(search_method);
                    if (strategy_method.strategy == OptimizerStrategy::any &&
                        search_method == SearchMethod::binary_search) {
                        continue;
                    }
                    auto optimizer = get_instance(automatic_tap_adjustment, strategy_method.strategy, search_method);

                    auto tap_optimizer = std::dynamic_pointer_cast<
                        TapPositionOptimizer<SymStubSteadyStateCalculator, ConstDatasetUpdate, StubState>>(optimizer);
                    REQUIRE(tap_optimizer != nullptr);
                    CHECK(tap_optimizer->get_strategy() == strategy_method.strategy);

                    StubState empty_state{};
                    empty_state.components.set_construction_complete();
                    CHECK(optimizer->optimize(empty_state, strategy_method.method).solver_output.empty());
                }
            }
        }
    }
}
} // namespace power_grid_model::optimizer::test
