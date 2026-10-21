// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "power_grid_model/common/common.hpp"
#include "power_grid_model/common/enum.hpp"
#include "power_grid_model/validation/context.hpp"
#include "power_grid_model/validation/issues.hpp"
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

    std::array<NodeInput, 4> const nodes{{{.id = 1, .u_rated = power_grid_model::nan},
                                          {.id = 2, .u_rated = std::numeric_limits<double>::infinity()},
                                          {.id = 3, .u_rated = 0.0},
                                          {.id = 4, .u_rated = -1.0}}};
    auto dataset = test::make_empty_dataset(false, "input");
    dataset.add_buffer("node", nodes.size(), nodes.size(), nullptr, nodes.data());
    Issues issues;

    ValidationContext const context{
        .input = dataset, .update = nullptr, .symmetric = true, .calculation_type = CalculationType::power_flow};
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

    ValidationContext const context{
        .input = dataset, .update = nullptr, .symmetric = true, .calculation_type = CalculationType::power_flow};
    power_grid_model::validation::detail::apply_rules(node_rule_table().input_rules, context, issues);

    REQUIRE(issues.size() == 1);
    CHECK(issues.front().rule == "missing_value");
    CHECK(issues.front().ids == std::vector<ID>{10});

    auto const no_nodes = test::make_empty_dataset(false, "input");
    issues.clear();
    ValidationContext const no_nodes_context{
        .input = no_nodes, .update = nullptr, .symmetric = true, .calculation_type = CalculationType::power_flow};
    power_grid_model::validation::detail::apply_rules(node_rule_table().input_rules, no_nodes_context, issues);
    CHECK(issues.empty());
}
