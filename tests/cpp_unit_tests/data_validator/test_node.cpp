// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "validation_test_utils.hpp"

#include <power_grid_model/auxiliary/input.hpp>
#include <power_grid_model/validation/node.hpp>

#include <doctest/doctest.h>

#include <array>
#include <limits>
#include <vector>

TEST_CASE("Node validation reports missing, infinite, and nonpositive rated voltages") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;

    std::array<NodeInput, 4> const nodes{
        {{1, power_grid_model::nan}, {2, std::numeric_limits<double>::infinity()}, {3, 0.0}, {4, -1.0}}};
    auto dataset = test::make_empty_dataset(false, "input");
    dataset.add_buffer("node", nodes.size(), nodes.size(), nullptr, nodes.data());
    Issues issues;

    ValidationContext const context{dataset, nullptr, true, CalculationType::power_flow};
    power_grid_model::validation::detail::apply_rules(node_rule_table().input_rules, context, issues);

    REQUIRE(issues.size() == 3);
    CHECK(issues[0].rule == "missing_value");
    CHECK(issues[0].component == "node");
    CHECK(issues[0].field == "u_rated");
    CHECK(issues[0].ids == std::vector<ID>{1});
    CHECK(issues[1].rule == "infinity");
    CHECK(issues[1].ids == std::vector<ID>{2});
    CHECK(issues[2].rule == "not_greater_than_zero");
    CHECK(issues[2].ids == std::vector<ID>{3, 4});
}

TEST_CASE("Node validation supports columnar datasets and absent components") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;

    std::array<ID, 2> const ids{10, 11};
    std::array<double, 2> const rated_voltage{power_grid_model::nan, 2.0};
    auto dataset = test::make_empty_dataset(false, "input");
    dataset.add_buffer("node", ids.size(), ids.size(), nullptr, nullptr);
    dataset.add_attribute_buffer("node", "id", ids.data());
    dataset.add_attribute_buffer("node", "u_rated", rated_voltage.data());
    Issues issues;

    ValidationContext const context{dataset, nullptr, true, CalculationType::power_flow};
    power_grid_model::validation::detail::apply_rules(node_rule_table().input_rules, context, issues);

    REQUIRE(issues.size() == 1);
    CHECK(issues.front().rule == "missing_value");
    CHECK(issues.front().ids == std::vector<ID>{10});

    auto const no_nodes = test::make_empty_dataset(false, "input");
    issues.clear();
    ValidationContext const no_nodes_context{no_nodes, nullptr, true, CalculationType::power_flow};
    power_grid_model::validation::detail::apply_rules(node_rule_table().input_rules, no_nodes_context, issues);
    CHECK(issues.empty());
}
