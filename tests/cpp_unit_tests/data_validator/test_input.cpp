// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "power_grid_model/auxiliary/meta_data.hpp"
#include "power_grid_model/validation/context.hpp"
#include "power_grid_model/validation/issues.hpp"
#include "power_grid_model/validation/report.hpp"
#include "validation_test_utils.hpp"

#include <power_grid_model/auxiliary/input.hpp>
#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/component/node.hpp>
#include <power_grid_model/validation/input.hpp>
#include <power_grid_model/validation/line.hpp>
#include <power_grid_model/validation/node.hpp>

#include <doctest/doctest.h>

#include <array>
#include <cstddef>
#include <vector>

TEST_CASE("Input orchestration runs injected input rules in declaration order") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using test::make_empty_dataset;

    auto const dataset = make_empty_dataset(false, "input");
    std::vector<int> calls;
    RuleDefinition first{.code = RuleCode::missing_value,
                         .fields = {{.name = "first", .type = meta_data::ctype_v<double>}},
                         .apply = [&calls](ValidationContext const& context, Issues& issues) {
                             calls.push_back(1);
                             CHECK(context.update == nullptr);
                             CHECK(context.symmetric);
                             CHECK(context.calculation_type == CalculationType::power_flow);
                             issues.push_back(
                                 {.rule = "missing_value", .component = "test", .field = "first", .ids = {7}});
                         }};
    RuleDefinition second{.code = RuleCode::infinity,
                          .fields = {{.name = "second", .type = meta_data::ctype_v<double>}},
                          .apply = [&calls](ValidationContext const&, Issues&) { calls.push_back(2); }};
    RuleDefinition batch_rule{.code = RuleCode::not_boolean,
                              .fields = {{.name = "status", .type = meta_data::ctype_v<IntS>}},
                              .apply = [&calls](ValidationContext const&, Issues&) { calls.push_back(3); }};
    ValidationRegistry const registry{{.component = "test",
                                       .input_rules = {first, second},
                                       .batch_rules = {batch_rule},
                                       .build_input_id_lookup = [](ConstDataset const&, InputValidationLookup& lookup) {
                                           std::array<ID, 0> const ids{};
                                           lookup.add_component_ids("test", ids);
                                       }}};

    auto const report = power_grid_model::validation::detail::validate_input_with_registry(
        dataset, true, CalculationType::power_flow, registry);

    CHECK(calls == std::vector<int>{1, 2});
    CHECK(report.type == ValidationType::input);
    REQUIRE(report.scopes.size() == 1);
    CHECK(report.scopes.front().status == ValidationStatus::failed);
    CHECK(report.scopes.front().scenario == na_Idx);
    REQUIRE(report.scopes.front().issues.size() == 1);
    CHECK(report.scopes.front().issues.front().ids == std::vector<ID>{7});
}

TEST_CASE("Input orchestration reports a passed scope for an empty registry") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using test::make_empty_dataset;

    auto const dataset = make_empty_dataset(false, "input");
    auto const report = power_grid_model::validation::detail::validate_input_with_registry(
        dataset, false, CalculationType::state_estimation, {});

    CHECK(report.valid());
    CHECK(report.type == ValidationType::input);
    REQUIRE(report.scopes.size() == 1);
    CHECK(report.scopes.front().status == ValidationStatus::passed);
}

TEST_CASE("Input orchestration validates dataset type before running rules") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using test::make_empty_dataset;

    auto const update = make_empty_dataset(false, "update");
    bool called = false;
    RuleDefinition rule{.code = RuleCode::missing_value,
                        .fields = {{.name = "value", .type = meta_data::ctype_v<double>}},
                        .apply = [&called](ValidationContext const&, Issues&) { called = true; }};
    ValidationRegistry const registry{{.component = "test", .input_rules = {rule}, .batch_rules = {}}};

    CHECK_THROWS_AS(power_grid_model::validation::detail::validate_input_with_registry(
                        update, true, CalculationType::power_flow, registry),
                    InvalidArguments);
    CHECK_FALSE(called);
}

TEST_CASE("Input orchestration shares prepared component ID lookups across rules") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using test::make_empty_dataset;

    std::array<NodeInput, 2> const nodes{{{.id = 41, .u_rated = 1.0}, {.id = 42, .u_rated = 2.0}}};
    auto dataset = make_empty_dataset(false, "input");
    dataset.add_buffer("node", nodes.size(), nodes.size(), nullptr, nodes.data());

    std::size_t index_builds{};
    auto node_rules = node_rule_table();
    auto const build_node_lookup = node_rules.build_input_id_lookup;
    node_rules.build_input_id_lookup = [&index_builds, build_node_lookup](ConstDataset const& input,
                                                                          InputValidationLookup& lookup) {
        ++index_builds;
        build_node_lookup(input, lookup);
    };

    auto line_rules = line_rule_table();
    std::vector<ID> observed_ids;
    auto make_reference_rule = [&observed_ids](ID expected_id) {
        return RuleDefinition{.code = RuleCode::missing_value,
                              .fields = {{.name = "from_node", .type = meta_data::ctype_v<ID>}},
                              .apply = [&observed_ids, expected_id](ValidationContext const& context, Issues&) {
                                  CHECK(context.input_id_lookup.contains_component_id(Node::name, expected_id));
                                  CHECK_FALSE(context.input_id_lookup.contains_component_id(Node::name, na_IntID));
                                  observed_ids.push_back(expected_id);
                              }};
    };
    line_rules.input_rules.push_back(make_reference_rule(41));
    line_rules.input_rules.push_back(make_reference_rule(42));

    ValidationRegistry const registry{node_rules, line_rules};
    auto const report = power_grid_model::validation::detail::validate_input_with_registry(
        dataset, true, CalculationType::power_flow, registry);

    CHECK(index_builds == 1);
    CHECK(observed_ids == std::vector<ID>{41, 42});
    CHECK(report.valid());
}

TEST_CASE("Input orchestration prepares ID lookups for all registered components") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using test::make_empty_dataset;

    auto const dataset = make_empty_dataset(false, "input");
    std::size_t node_index_builds{};
    std::size_t line_index_builds{};
    auto node_rules = node_rule_table();
    auto const build_node_lookup = node_rules.build_input_id_lookup;
    node_rules.build_input_id_lookup = [&node_index_builds, build_node_lookup](ConstDataset const& input,
                                                                               InputValidationLookup& lookup) {
        ++node_index_builds;
        build_node_lookup(input, lookup);
    };
    auto line_rules = line_rule_table();
    auto const build_line_lookup = line_rules.build_input_id_lookup;
    line_rules.build_input_id_lookup = [&line_index_builds, build_line_lookup](ConstDataset const& input,
                                                                               InputValidationLookup& lookup) {
        ++line_index_builds;
        build_line_lookup(input, lookup);
    };
    ValidationRegistry const registry{node_rules, line_rules};

    auto const report = power_grid_model::validation::detail::validate_input_with_registry(
        dataset, true, CalculationType::power_flow, registry);

    CHECK(node_index_builds == 1);
    CHECK(line_index_builds == 1);
    CHECK(report.valid());
}

TEST_CASE("Input orchestration requires ID index builders for registered components") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using test::make_empty_dataset;

    auto const dataset = make_empty_dataset(false, "input");
    bool called = false;
    RuleDefinition rule{.code = RuleCode::missing_value,
                        .fields = {{.name = "reference", .type = meta_data::ctype_v<ID>}},
                        .apply = [&called](ValidationContext const&, Issues&) { called = true; }};
    ValidationRegistry const registry{{.component = "source", .input_rules = {rule}, .batch_rules = {}}};

    CHECK_THROWS_AS(power_grid_model::validation::detail::validate_input_with_registry(
                        dataset, true, CalculationType::power_flow, registry),
                    InvalidArguments);
    CHECK_FALSE(called);
}

TEST_CASE("Input validation reports local and cross-component duplicate IDs") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using test::make_empty_dataset;

    std::array<NodeInput, 3> const nodes{
        {{.id = 1, .u_rated = 1.0}, {.id = 1, .u_rated = 2.0}, {.id = 2, .u_rated = 3.0}}};
    std::array<LineInput, 1> const lines{
        {{.id = 1, .from_node = 2, .to_node = 3, .from_status = 1, .to_status = 1, .r1 = 0.1}}};
    auto dataset = make_empty_dataset(false, "input");
    dataset.add_buffer("node", nodes.size(), nodes.size(), nullptr, nodes.data());
    dataset.add_buffer("line", lines.size(), lines.size(), nullptr, lines.data());

    auto const report = validate_input_dataset(dataset, true, CalculationType::power_flow);

    REQUIRE(report.scopes.size() == 1);
    REQUIRE(report.scopes.front().issues.size() == 3);
    CHECK(report.scopes.front().issues[0].rule == "not_unique");
    CHECK(report.scopes.front().issues[0].component == "node");
    CHECK(report.scopes.front().issues[0].field == "id");
    CHECK(report.scopes.front().issues[0].ids == std::vector<ID>{1, 1});
    CHECK(report.scopes.front().issues[1].rule == "not_unique_across_components");
    CHECK(report.scopes.front().issues[1].component == "line");
    CHECK(report.scopes.front().issues[1].ids == std::vector<ID>{1});
    CHECK(report.scopes.front().issues[2].rule == "not_unique_across_components");
    CHECK(report.scopes.front().issues[2].component == "node");
    CHECK(report.scopes.front().issues[2].ids == std::vector<ID>{1});
}

TEST_CASE("Input validation checks line from_node references against node IDs") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using test::make_empty_dataset;

    std::array<NodeInput, 2> const nodes{{{.id = 1, .u_rated = 1.0}, {.id = 2, .u_rated = 2.0}}};
    std::array<LineInput, 2> const lines{
        {{.id = 10, .from_node = 1, .to_node = 2, .from_status = 1, .to_status = 1, .r1 = 0.1},
         {.id = 11, .from_node = 99, .to_node = 2, .from_status = 1, .to_status = 1, .r1 = 0.1}}};
    auto dataset = make_empty_dataset(false, "input");
    dataset.add_buffer("node", nodes.size(), nodes.size(), nullptr, nodes.data());
    dataset.add_buffer("line", lines.size(), lines.size(), nullptr, lines.data());

    auto const report = validate_input_dataset(dataset, true, CalculationType::power_flow);

    REQUIRE(report.scopes.size() == 1);
    REQUIRE(report.scopes.front().issues.size() == 1);
    CHECK(report.scopes.front().issues.front().rule == "invalid_id");
    CHECK(report.scopes.front().issues.front().component == "line");
    CHECK(report.scopes.front().issues.front().field == "from_node");
    CHECK(report.scopes.front().issues.front().ids == std::vector<ID>{11});
}
