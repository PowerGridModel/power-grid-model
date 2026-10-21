// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "power_grid_model/validation/context.hpp"
#include "validation_test_utils.hpp"

#include <power_grid_model/auxiliary/input.hpp>
#include <power_grid_model/auxiliary/meta_data.hpp>
#include <power_grid_model/component/line.hpp>
#include <power_grid_model/component/node.hpp>

#include <doctest/doctest.h>

#include <array>
#include <limits>
#include <utility>
#include <vector>

TEST_CASE("Unary rule adapter maps one field to an issue") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using test::make_empty_dataset;

    auto const rule = make_unary_rule<Node, &NodeInput::u_rated>("u_rated", rules::Required{});
    CHECK(rule.code == RuleCode::missing_value);
    REQUIRE(rule.fields.size() == 1);
    CHECK(rule.fields.front().name == "u_rated");
    CHECK(rule.fields.front().type == meta_data::ctype_v<double>);

    SUBCASE("row data") {
        std::array<NodeInput, 2> const nodes{
            {{.id = 41, .u_rated = power_grid_model::nan}, {.id = 42, .u_rated = 1.0}}};
        auto dataset = make_empty_dataset(false, "input");
        dataset.add_buffer("node", nodes.size(), nodes.size(), nullptr, nodes.data());
        ValidationContext const context{
            .input = dataset, .update = nullptr, .symmetric = true, .calculation_type = CalculationType::power_flow};
        Issues issues;

        rule.apply(context, issues);

        REQUIRE(issues.size() == 1);
        CHECK(issues.front().component == "node");
        CHECK(issues.front().field == "u_rated");
        CHECK(issues.front().ids == std::vector<ID>{41});
    }

    SUBCASE("columnar data") {
        std::array<ID, 2> const ids{41, 42};
        std::array<double, 2> const values{power_grid_model::nan, 1.0};
        auto dataset = make_empty_dataset(false, "input");
        dataset.add_buffer("node", ids.size(), ids.size(), nullptr, nullptr);
        dataset.add_attribute_buffer("node", "id", ids.data());
        dataset.add_attribute_buffer("node", "u_rated", values.data());
        ValidationContext const context{
            .input = dataset, .update = nullptr, .symmetric = true, .calculation_type = CalculationType::power_flow};
        Issues issues;

        rule.apply(context, issues);

        REQUIRE(issues.size() == 1);
        CHECK(issues.front().ids == std::vector<ID>{41});
    }

    SUBCASE("missing column defaults to the field's missing value") {
        std::array<ID, 1> const ids{41};
        auto dataset = make_empty_dataset(false, "input");
        dataset.add_buffer("node", ids.size(), ids.size(), nullptr, nullptr);
        dataset.add_attribute_buffer("node", "id", ids.data());
        ValidationContext const context{
            .input = dataset, .update = nullptr, .symmetric = true, .calculation_type = CalculationType::power_flow};
        Issues issues;

        rule.apply(context, issues);

        REQUIRE(issues.size() == 1);
        CHECK(issues.front().ids == std::vector<ID>{41});
    }
}

TEST_CASE("Range rule adapter maps duplicate values to component IDs") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using test::make_empty_dataset;

    std::array<LineInput, 4> const lines{{{.id = 10}, {.id = 11}, {.id = 10}, {.id = 12}}};
    auto dataset = make_empty_dataset(false, "input");
    dataset.add_buffer("line", lines.size(), lines.size(), nullptr, lines.data());
    ValidationContext const context{
        .input = dataset, .update = nullptr, .symmetric = true, .calculation_type = CalculationType::power_flow};
    auto const rule = make_range_rule<Line, &LineInput::id>("id", rules::Unique{});
    Issues issues;

    rule.apply(context, issues);

    REQUIRE(issues.size() == 1);
    CHECK(issues.front().rule == "not_unique");
    CHECK(issues.front().component == "line");
    CHECK(issues.front().field == "id");
    CHECK(issues.front().ids == std::vector<ID>{10, 10});
}

TEST_CASE("ID reference adapter uses the configured target lookup and reports referencing IDs") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using test::make_empty_dataset;

    std::array<NodeInput, 2> const nodes{{{.id = 5, .u_rated = 1.0}, {.id = 6, .u_rated = 1.0}}};
    std::array<LineInput, 2> const lines{{{.id = 10, .from_node = 5}, {.id = 11, .from_node = 99}}};
    auto dataset = make_empty_dataset(false, "input");
    dataset.add_buffer("node", nodes.size(), nodes.size(), nullptr, nodes.data());
    dataset.add_buffer("line", lines.size(), lines.size(), nullptr, lines.data());
    InputValidationLookup lookup;
    std::array<ID, 2> const node_ids{5, 6};
    std::array<ID, 2> const line_ids{10, 11};
    lookup.add_component_ids(Node::name, node_ids);
    lookup.add_component_ids(Line::name, line_ids);
    ValidationContext const context{.input = dataset,
                                    .update = nullptr,
                                    .symmetric = true,
                                    .calculation_type = CalculationType::power_flow,
                                    .input_id_lookup = std::move(lookup)};
    auto const rule = make_id_reference_rule<Line, &LineInput::from_node>("from_node", Node::name);
    Issues issues;

    rule.apply(context, issues);

    REQUIRE(issues.size() == 1);
    CHECK(issues.front().rule == "invalid_id");
    CHECK(issues.front().component == "line");
    CHECK(issues.front().field == "from_node");
    CHECK(issues.front().ids == std::vector<ID>{11});
}

TEST_CASE("ID reference adapter reads a columnar reference field") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using test::make_empty_dataset;

    std::array<ID, 2> const node_ids{5, 6};
    std::array<ID, 2> const line_ids{10, 11};
    std::array<ID, 2> const from_node{6, 99};
    auto dataset = make_empty_dataset(false, "input");
    dataset.add_buffer("node", node_ids.size(), node_ids.size(), nullptr, nullptr);
    dataset.add_attribute_buffer("node", "id", node_ids.data());
    dataset.add_buffer("line", line_ids.size(), line_ids.size(), nullptr, nullptr);
    dataset.add_attribute_buffer("line", "id", line_ids.data());
    dataset.add_attribute_buffer("line", "from_node", from_node.data());
    InputValidationLookup lookup;
    lookup.add_component_ids(Node::name, node_ids);
    ValidationContext const context{.input = dataset,
                                    .update = nullptr,
                                    .symmetric = true,
                                    .calculation_type = CalculationType::power_flow,
                                    .input_id_lookup = std::move(lookup)};
    auto const rule = make_id_reference_rule<Line, &LineInput::from_node>("from_node", Node::name);
    Issues issues;

    rule.apply(context, issues);

    REQUIRE(issues.size() == 1);
    CHECK(issues.front().ids == std::vector<ID>{11});
}
