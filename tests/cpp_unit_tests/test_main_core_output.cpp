// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/main_core/output.hpp>
#include <power_grid_model/main_core/state.hpp>
#include <power_grid_model/main_core/state_queries.hpp>

#include <doctest/doctest.h>

namespace power_grid_model::main_core {
TEST_CASE("Test main core output") {
    SUBCASE("TransformerTapRegulator") {
        using ComponentContainer = Container<ExtraRetrievableTypes<Base, Regulator>, TransformerTapRegulator>;
        using State = MainModelState<ComponentContainer>;
        using SymOutput = MathOutput<std::vector<SolverOutput<symmetric_t>>>;
        using AsymOutput = MathOutput<std::vector<SolverOutput<asymmetric_t>>>;

        State state;
        emplace_component<TransformerTapRegulator>(state.components, 0,
                                                   TransformerTapRegulatorInput{.id = 0, .regulated_object = 2},
                                                   ComponentType::test, 10e3);
        emplace_component<TransformerTapRegulator>(state.components, 1,
                                                   TransformerTapRegulatorInput{.id = 1, .regulated_object = 3},
                                                   ComponentType::test, 10e3);
        state.components.set_construction_complete();

        auto comp_topo = std::make_shared<ComponentTopology>();
        comp_topo->regulated_object_idx = {2, 3};
        state.comp_topo = std::make_shared<ComponentTopology const>(std::move(*comp_topo));

        std::vector<TransformerTapRegulatorOutput> output(state.components.template size<TransformerTapRegulator>());

        SUBCASE("No regulation") {
            SUBCASE("Symmetric") {
                output_result<TransformerTapRegulator, ComponentContainer>(state, SymOutput{}, output);
            }
            SUBCASE("Asymmetric") {
                output_result<TransformerTapRegulator, ComponentContainer>(state, AsymOutput{}, output);
            }
            CHECK(output[0].id == 0);
            CHECK(output[0].energized == 0);
            CHECK(output[0].tap_pos == na_IntS);
            CHECK(output[1].id == 1);
            CHECK(output[1].energized == 0);
            CHECK(output[1].tap_pos == na_IntS);
        }
        SUBCASE("One regulated") {
            OptimizerOutput const optimizer_output{
                .transformer_tap_positions = {{.transformer_id = 3, .tap_position = 1}}};
            SUBCASE("Symmetric") {
                output_result<TransformerTapRegulator, ComponentContainer>(
                    state, SymOutput{.solver_output = {}, .optimizer_output = optimizer_output}, output);
            }
            SUBCASE("Asymmetric") {
                output_result<TransformerTapRegulator, ComponentContainer>(
                    state, AsymOutput{.solver_output = {}, .optimizer_output = optimizer_output}, output);
            }
            CHECK(output[0].id == 0);
            CHECK(output[0].energized == 0);
            CHECK(output[0].tap_pos == na_IntS);
            CHECK(output[1].id == 1);
            CHECK(output[1].energized == 1);
            CHECK(output[1].tap_pos == 1);
        }
        SUBCASE("Two regulated") {
            OptimizerOutput const optimizer_output{
                .transformer_tap_positions = {{.transformer_id = 3, .tap_position = 1},
                                              {.transformer_id = 4, .tap_position = 2},
                                              {.transformer_id = 2, .tap_position = 3}}};
            SUBCASE("Symmetric") {
                output_result<TransformerTapRegulator, ComponentContainer>(
                    state, SymOutput{.solver_output = {}, .optimizer_output = optimizer_output}, output);
            }
            SUBCASE("Asymmetric") {
                output_result<TransformerTapRegulator, ComponentContainer>(
                    state, AsymOutput{.solver_output = {}, .optimizer_output = optimizer_output}, output);
            }
            CHECK(output[0].id == 0);
            CHECK(output[0].energized == 1);
            CHECK(output[0].tap_pos == 3);
            CHECK(output[1].id == 1);
            CHECK(output[1].energized == 1);
            CHECK(output[1].tap_pos == 1);
        }
    }
}
} // namespace power_grid_model::main_core
