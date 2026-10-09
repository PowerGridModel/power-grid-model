// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/auxiliary/meta_data.hpp>
#include <power_grid_model/auxiliary/meta_data_gen.hpp>
#include <power_grid_model/calculation_state.hpp>
#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/common/multi_threaded_logging.hpp>
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
    CHECK_FALSE(group.jacobian.has_value());
}

TEST_CASE("Test State output - Capture Newton-Raphson Jacobian") {
    MathSolverDispatcher const dispatcher{math_solver::math_solver_tag<math_solver::MathSolver>{}};
    auto model = make_state_test_model(dispatcher);
    auto state_output = calculate_state(model, MainModelOptions{}, {.jacobian = true});

    REQUIRE(state_output.has_value());
    REQUIRE(state_output->groups.size() == 1);
    auto const& group = state_output->groups[0];
    CHECK_FALSE(group.y_bus.has_value());
    REQUIRE(group.jacobian.has_value());
    CHECK(group.jacobian->blocks[0].size() == group.jacobian->blocks[1].size());
    CHECK(group.jacobian->blocks[0].size() == group.jacobian->blocks[2].size());
    CHECK(group.jacobian->blocks[0].size() == group.jacobian->blocks[3].size());
}

TEST_CASE("Test State output - Linear method has no Jacobian") {
    MathSolverDispatcher const dispatcher{math_solver::math_solver_tag<math_solver::MathSolver>{}};
    auto model = make_state_test_model(dispatcher);
    auto options = MainModelOptions{};
    options.calculation_method = CalculationMethod::linear;
    auto state_output = calculate_state(model, options, {.y_bus = true, .jacobian = true});

    REQUIRE(state_output.has_value());
    REQUIRE(state_output->groups.size() == 1);
    CHECK(state_output->groups[0].y_bus.has_value());
    CHECK_FALSE(state_output->groups[0].jacobian.has_value());
}

TEST_CASE("Test State output - Unrequested model state is absent") {
    MathSolverDispatcher const dispatcher{math_solver::math_solver_tag<math_solver::MathSolver>{}};
    auto model = make_state_test_model(dispatcher);
    auto state_output = calculate_state(model, MainModelOptions{}, {});

    CHECK_FALSE(state_output.has_value());
}
} // namespace power_grid_model
