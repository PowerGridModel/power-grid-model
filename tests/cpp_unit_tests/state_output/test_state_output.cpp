// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/auxiliary/meta_data.hpp>
#include <power_grid_model/auxiliary/meta_data_gen.hpp>
#include <power_grid_model/calculation_state.hpp>
#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/common/multi_threaded_logging.hpp>
#include <power_grid_model/common/text_logger.hpp>
#include <power_grid_model/main_model.hpp>
#include <power_grid_model/math_solver/math_solver.hpp>

#include <doctest/doctest.h>

#include <array>
#include <cmath>
#include <optional>
#include <sstream>
#include <string>
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
    CHECK(group.jacobian_history.empty());
}

TEST_CASE("Test State output - Capture Newton-Raphson Jacobian") {
    MathSolverDispatcher const dispatcher{math_solver::math_solver_tag<math_solver::MathSolver>{}};
    auto model = make_state_test_model(dispatcher);
    MultiThreadedTextLogger logger;
    model.set_logger(logger);
    auto options = MainModelOptions{};
    options.calculation_method = CalculationMethod::newton_raphson;
    auto state_output = calculate_state(model, options, {.jacobian = true});

    REQUIRE(state_output.has_value());
    REQUIRE(state_output->groups.size() == 1);
    auto const& group = state_output->groups[0];
    CHECK_FALSE(group.y_bus.has_value());
    REQUIRE(group.jacobian.has_value());
    CHECK(group.jacobian->blocks[0].size() == group.jacobian->blocks[1].size());
    CHECK(group.jacobian->blocks[0].size() == group.jacobian->blocks[2].size());
    CHECK(group.jacobian->blocks[0].size() == group.jacobian->blocks[3].size());
    CHECK(std::ranges::any_of(group.jacobian->blocks[0], [](double value) { return value != 0.0; }));
    CHECK(std::ranges::all_of(group.jacobian->blocks[0], [](double value) { return std::isfinite(value); }));

    auto const report = logger.report();
    constexpr std::string_view jacobian_tag{"Tag:4001: "};
    std::vector<std::string_view> jacobian_logs;
    auto record_start = report.find(jacobian_tag);
    while (record_start != std::string::npos) {
        auto const record_end = report.find("\n[", record_start + jacobian_tag.size());
        jacobian_logs.push_back(std::string_view{report}.substr(
            record_start + jacobian_tag.size(), record_end == std::string::npos ? std::string::npos
                                                                               : record_end - record_start - jacobian_tag.size()));
        record_start = report.find(jacobian_tag, record_start + jacobian_tag.size());
    }
    REQUIRE_FALSE(jacobian_logs.empty());
    CHECK(jacobian_logs.size() > 1);
    REQUIRE(group.jacobian_history.size() == jacobian_logs.size());
    auto parse_values = [](std::string_view jacobian_log, std::string_view field) {
        auto const field_start = jacobian_log.find(std::string{field} + "=");
        REQUIRE(field_start != std::string_view::npos);
        auto const value_start = field_start + field.size() + 1;
        auto const line_end = jacobian_log.find('\n', value_start);
        std::istringstream stream{std::string{jacobian_log.substr(value_start, line_end - value_start)}};
        std::vector<double> values;
        double value{};
        while (stream >> value) {
            values.push_back(value);
        }
        return values;
    };
    auto check_logged_values = [&parse_values](std::string_view jacobian_log, std::string_view field,
                                               std::vector<double> const& expected) {
        auto const logged = parse_values(jacobian_log, field);
        REQUIRE(logged.size() == expected.size());
        for (size_t i = 0; i < expected.size(); ++i) {
            CHECK(logged[i] == doctest::Approx(expected[i]).epsilon(1e-5));
        }
    };
    for (size_t iteration_idx = 0; iteration_idx < jacobian_logs.size(); ++iteration_idx) {
        auto const& jacobian = group.jacobian_history[iteration_idx];
        auto const jacobian_log = jacobian_logs[iteration_idx];
        CHECK(jacobian.iteration == static_cast<Idx>(iteration_idx + 1));
        check_logged_values(jacobian_log, "row_indptr_lu",
                            std::vector<double>(jacobian.row_indptr_lu.begin(), jacobian.row_indptr_lu.end()));
        check_logged_values(jacobian_log, "col_indices_lu",
                            std::vector<double>(jacobian.col_indices_lu.begin(), jacobian.col_indices_lu.end()));
        check_logged_values(jacobian_log, "jac_h", jacobian.blocks[0]);
        check_logged_values(jacobian_log, "jac_n", jacobian.blocks[1]);
        check_logged_values(jacobian_log, "jac_m", jacobian.blocks[2]);
        check_logged_values(jacobian_log, "jac_l", jacobian.blocks[3]);
    }
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
    CHECK(state_output->groups[0].jacobian_history.empty());
}

TEST_CASE("Test State output - Unrequested model state is absent") {
    MathSolverDispatcher const dispatcher{math_solver::math_solver_tag<math_solver::MathSolver>{}};
    auto model = make_state_test_model(dispatcher);
    auto state_output = calculate_state(model, MainModelOptions{}, {});

    CHECK_FALSE(state_output.has_value());
}
} // namespace power_grid_model
