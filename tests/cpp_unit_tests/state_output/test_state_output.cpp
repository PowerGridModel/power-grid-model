// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/auxiliary/meta_data.hpp>
#include <power_grid_model/auxiliary/meta_data_gen.hpp>
#include <power_grid_model/calculation_state.hpp>
#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/main_model.hpp>
#include <power_grid_model/math_solver/math_solver.hpp>

#include <doctest/doctest.h>

#include <array>
#include <cmath>
#include <optional>
#include <vector>

namespace power_grid_model {
namespace {
MainModel make_state_test_model(MathSolverDispatcher const& dispatcher) {
    std::vector<ID> const node_id{0};
    std::vector<double> const node_u_rated{100.0};

    std::vector<ID> const source_id{1};
    std::vector<ID> const source_node{0};
    std::vector<IntS> const source_status{1};
    std::vector<double> const source_u_ref{1.0};
    std::vector<double> const source_sk{1000.0};
    std::vector<double> const source_rx_ratio{0.0};

    std::vector<ID> const load_id{2};
    std::vector<ID> const load_node{0};
    std::vector<IntS> const load_status{1};
    std::vector<IntS> const load_type{2};
    std::vector<double> const load_p{0.0};
    std::vector<double> const load_q{500.0};

    auto const& metadata = meta_data::meta_data_gen::meta_data;
    ConstDataset input{false, 1, "input", metadata};
    input.add_buffer("node", 1, 1, nullptr, nullptr);
    input.add_attribute_buffer("node", "id", node_id.data());
    input.add_attribute_buffer("node", "u_rated", node_u_rated.data());
    input.add_buffer("source", 1, 1, nullptr, nullptr);
    input.add_attribute_buffer("source", "id", source_id.data());
    input.add_attribute_buffer("source", "node", source_node.data());
    input.add_attribute_buffer("source", "status", source_status.data());
    input.add_attribute_buffer("source", "u_ref", source_u_ref.data());
    input.add_attribute_buffer("source", "sk", source_sk.data());
    input.add_attribute_buffer("source", "rx_ratio", source_rx_ratio.data());
    input.add_buffer("sym_load", 1, 1, nullptr, nullptr);
    input.add_attribute_buffer("sym_load", "id", load_id.data());
    input.add_attribute_buffer("sym_load", "node", load_node.data());
    input.add_attribute_buffer("sym_load", "status", load_status.data());
    input.add_attribute_buffer("sym_load", "type", load_type.data());
    input.add_attribute_buffer("sym_load", "p_specified", load_p.data());
    input.add_attribute_buffer("sym_load", "q_specified", load_q.data());

    return MainModel{50.0, input, dispatcher};
}

std::optional<ModelStateOutput> calculate_state(MainModel& model, MainModelOptions const& options,
                                                ModelStateRequest request) {
    auto const& metadata = meta_data::meta_data_gen::meta_data;
    ConstDataset const update{false, 1, "update", metadata};
    MutableDataset output{false, 1, "sym_output", metadata};
    std::array<ModelStateRequest, 1> const requests{request};
    std::vector<std::optional<ModelStateOutput>> state_outputs(1);
    model.calculate_with_state(options, output, update, requests, state_outputs);
    return std::move(state_outputs[0]);
}
} // namespace

TEST_CASE("Test State output - Capture Y-bus and input-node mapping") {
    MathSolverDispatcher const dispatcher{math_solver::math_solver_tag<math_solver::MathSolver>{}};
    auto model = make_state_test_model(dispatcher);
    auto state_output = calculate_state(model, MainModelOptions{}, {.y_bus = true});

    REQUIRE(state_output.has_value());
    auto const& state = *state_output;
    REQUIRE(state.groups.size() == 1);
    CHECK(state.input_node_id == std::vector<ID>{0});
    CHECK(state.input_node_group == IdxVector{0});
    CHECK(state.input_node_bus == IdxVector{0});

    auto const& group = state.groups[0];
    CHECK(group.mapping.n_bus == 1);
    CHECK(group.mapping.bus_user_indptr == IdxVector{0, 1});
    CHECK(group.mapping.bus_user_sequence == IdxVector{0});
    CHECK(group.mapping.bus_user_id == std::vector<ID>{0});
    REQUIRE(group.y_bus.has_value());
    CHECK(group.y_bus->row_indptr == IdxVector{0, 1});
    CHECK(group.y_bus->col_indices == IdxVector{0});
    REQUIRE(group.y_bus->admittance_real.size() == 1);
    CHECK(std::isfinite(group.y_bus->admittance_real[0]));
    CHECK(std::isfinite(group.y_bus->admittance_imag[0]));
    CHECK(group.jacobians.empty());
}

TEST_CASE("Test State output - Capture Newton-Raphson Jacobian") {
    MathSolverDispatcher const dispatcher{math_solver::math_solver_tag<math_solver::MathSolver>{}};
    auto model = make_state_test_model(dispatcher);
    auto options = MainModelOptions{};
    options.calculation_method = CalculationMethod::newton_raphson;
    auto state_output = calculate_state(model, options, {.jacobian = true, .nodal_state = true});

    REQUIRE(state_output.has_value());
    REQUIRE(state_output->groups.size() == 1);
    auto const& group = state_output->groups[0];
    CHECK_FALSE(group.y_bus.has_value());
    constexpr std::array<std::array<double, 4>, 5> expected_jacobian_blocks{{
        {0.00066666666666666664, 0.0, 0.0, 0.00055555555555555545},
        {0.00053333333333333336, 0.0, 0.0, 0.00030222222222222217},
        {0.00050196078431372548, 0.0, 0.0, 0.00025294886582083806},
        {0.00050000762951094848, 0.0, 0.0, 0.00025001144438284152},
        {0.00050000000011641535, 0.0, 0.0, 0.00025000000017462302},
    }};
    REQUIRE(group.mapping.n_bus == 1);
    REQUIRE(group.jacobians.size() == expected_jacobian_blocks.size());
    REQUIRE(group.jacobian_structure.has_value());
    CHECK(group.jacobian_structure->row_indptr_lu == IdxVector{0, 1});
    CHECK(group.jacobian_structure->col_indices_lu == IdxVector{0});
    REQUIRE(group.nodal_states.size() == group.jacobians.size());
    for (size_t iteration_idx = 0; iteration_idx < group.jacobians.size(); ++iteration_idx) {
        auto const& jacobian = group.jacobians[iteration_idx];
        auto const& nodal_state = group.nodal_states[iteration_idx];
        CHECK(jacobian.iteration == static_cast<Idx>(iteration_idx + 1));
        CHECK(nodal_state.iteration == jacobian.iteration);
        auto const phases_per_bus = group.mapping.is_symmetric ? 1 : 3;
        auto const n_voltage_values = static_cast<size_t>(group.mapping.n_bus * phases_per_bus);
        CHECK(nodal_state.voltage_magnitude.size() == n_voltage_values);
        CHECK(nodal_state.voltage_angle.size() == n_voltage_values);
        CHECK(std::ranges::all_of(nodal_state.voltage_magnitude, [](double value) { return std::isfinite(value); }));
        CHECK(std::ranges::all_of(nodal_state.voltage_angle, [](double value) { return std::isfinite(value); }));
        CHECK(jacobian.blocks[0].size() == jacobian.blocks[1].size());
        CHECK(jacobian.blocks[0].size() == jacobian.blocks[2].size());
        CHECK(jacobian.blocks[0].size() == jacobian.blocks[3].size());
        CHECK(std::ranges::any_of(jacobian.blocks[0], [](double value) { return value != 0.0; }));
        for (size_t block_idx = 0; block_idx < jacobian.blocks.size(); ++block_idx) {
            auto const& block = jacobian.blocks[block_idx];
            REQUIRE(block.size() == 1);
            CHECK(std::isfinite(block[0]));
            CHECK(block[0] == doctest::Approx(expected_jacobian_blocks[iteration_idx][block_idx]).epsilon(1e-5));
        }
    }
}

TEST_CASE("Test State output - Capture nodal states independently") {
    MathSolverDispatcher const dispatcher{math_solver::math_solver_tag<math_solver::MathSolver>{}};
    auto model = make_state_test_model(dispatcher);
    auto options = MainModelOptions{};
    options.calculation_method = CalculationMethod::newton_raphson;
    auto state_output = calculate_state(model, options, {.nodal_state = true});

    REQUIRE(state_output.has_value());
    CHECK_FALSE(state_output->jacobian_requested);
    CHECK(state_output->nodal_state_requested);
    REQUIRE(state_output->groups.size() == 1);
    auto const& group = state_output->groups[0];
    CHECK(group.jacobians.empty());
    CHECK_FALSE(group.jacobian_structure.has_value());
    REQUIRE_FALSE(group.nodal_states.empty());
    for (size_t state_idx = 0; state_idx < group.nodal_states.size(); ++state_idx) {
        auto const& nodal_state = group.nodal_states[state_idx];
        CHECK(nodal_state.iteration == static_cast<Idx>(state_idx + 1));
        CHECK(nodal_state.voltage_magnitude.size() == static_cast<size_t>(group.mapping.n_bus));
        CHECK(nodal_state.voltage_angle.size() == static_cast<size_t>(group.mapping.n_bus));
    }
}

TEST_CASE("Test State output - Iterative current captures nodal states without Jacobians") {
    MathSolverDispatcher const dispatcher{math_solver::math_solver_tag<math_solver::MathSolver>{}};
    auto model = make_state_test_model(dispatcher);
    auto options = MainModelOptions{};
    options.calculation_method = CalculationMethod::iterative_current;
    auto state_output = calculate_state(model, options, {.nodal_state = true});

    REQUIRE(state_output.has_value());
    CHECK_FALSE(state_output->jacobian_requested);
    CHECK(state_output->nodal_state_requested);
    REQUIRE(state_output->groups.size() == 1);
    auto const& group = state_output->groups[0];
    CHECK(group.jacobians.empty());
    CHECK_FALSE(group.jacobian_structure.has_value());
    REQUIRE(group.nodal_states.size() > 1);
    for (size_t state_idx = 0; state_idx < group.nodal_states.size(); ++state_idx) {
        auto const& nodal_state = group.nodal_states[state_idx];
        CHECK(nodal_state.iteration == static_cast<Idx>(state_idx + 1));
        REQUIRE(nodal_state.voltage_magnitude.size() == static_cast<size_t>(group.mapping.n_bus));
        REQUIRE(nodal_state.voltage_angle.size() == static_cast<size_t>(group.mapping.n_bus));
        CHECK(std::ranges::all_of(nodal_state.voltage_magnitude, [](double value) { return std::isfinite(value); }));
        CHECK(std::ranges::all_of(nodal_state.voltage_angle, [](double value) { return std::isfinite(value); }));
    }
}

TEST_CASE("Test State output - Linear method has no Jacobian") {
    MathSolverDispatcher const dispatcher{math_solver::math_solver_tag<math_solver::MathSolver>{}};
    auto model = make_state_test_model(dispatcher);
    auto options = MainModelOptions{};
    options.calculation_method = CalculationMethod::linear;
    auto state_output = calculate_state(model, options, {.y_bus = true, .jacobian = true, .nodal_state = true});

    REQUIRE(state_output.has_value());
    REQUIRE(state_output->groups.size() == 1);
    CHECK(state_output->groups[0].y_bus.has_value());
    CHECK(state_output->groups[0].jacobians.empty());
    CHECK(state_output->groups[0].nodal_states.empty()); // linear method has only one iteration
    CHECK_FALSE(state_output->groups[0].jacobian_structure.has_value());
}

TEST_CASE("Test State output - Unrequested model state is absent") {
    MathSolverDispatcher const dispatcher{math_solver::math_solver_tag<math_solver::MathSolver>{}};
    auto model = make_state_test_model(dispatcher);
    auto state_output = calculate_state(model, MainModelOptions{}, {});

    CHECK_FALSE(state_output.has_value());
}
} // namespace power_grid_model
