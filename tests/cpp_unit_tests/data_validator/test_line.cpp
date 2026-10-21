// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "power_grid_model/common/common.hpp"
#include "power_grid_model/common/enum.hpp"
#include "power_grid_model/validation/context.hpp"
#include "power_grid_model/validation/issues.hpp"
#include "validation_test_utils.hpp"

#include <power_grid_model/auxiliary/input.hpp>
#include <power_grid_model/component/node.hpp>
#include <power_grid_model/validation/line.hpp>

#include <doctest/doctest.h>

#include <array>
#include <limits>
#include <utility>
#include <vector>

TEST_CASE("Line input validation reports rule failures in declaration order") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;

    std::array<LineInput, 2> const lines{
        {{.id = 11, .from_node = 0, .to_node = 1, .from_status = na_IntS, .to_status = 1, .r1 = power_grid_model::nan},
         {.id = 12,
          .from_node = 1,
          .to_node = 2,
          .from_status = 2,
          .to_status = 1,
          .r1 = std::numeric_limits<double>::infinity()}}};
    auto dataset = test::make_empty_dataset(false, "input");
    std::array<NodeInput, 2> const nodes{{{.id = 0, .u_rated = 1.0}, {.id = 1, .u_rated = 1.0}}};
    dataset.add_buffer("node", nodes.size(), nodes.size(), nullptr, nodes.data());
    dataset.add_buffer("line", lines.size(), lines.size(), nullptr, lines.data());
    InputValidationLookup input_id_lookup;
    power_grid_model::validation::detail::index_input_component_ids<Node>(dataset, input_id_lookup);
    Issues issues;

    ValidationContext const context{.input = dataset,
                                    .update = nullptr,
                                    .symmetric = true,
                                    .calculation_type = CalculationType::power_flow,
                                    .input_id_lookup = std::move(input_id_lookup)};
    power_grid_model::validation::detail::apply_rules(line_rule_table().input_rules, context, issues);

    REQUIRE(issues.size() == 4);
    CHECK(issues[0].rule == "missing_value");
    CHECK(issues[0].field == "from_status");
    CHECK(issues[0].ids == std::vector<ID>{11});
    CHECK(issues[1].rule == "missing_value");
    CHECK(issues[1].field == "r1");
    CHECK(issues[1].ids == std::vector<ID>{11});
    CHECK(issues[2].rule == "infinity");
    CHECK(issues[2].field == "r1");
    CHECK(issues[2].ids == std::vector<ID>{12});
    CHECK(issues[3].rule == "not_boolean");
    CHECK(issues[3].field == "from_status");
    CHECK(issues[3].ids == std::vector<ID>{11, 12});
}

TEST_CASE("Line input validation supports columnar datasets") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;

    std::array<ID, 2> const ids{21, 22};
    std::array<ID, 2> const node_ids{0, 1};
    std::array<ID, 2> const from_node{0, 1};
    std::array<IntS, 2> const from_status{0, 2};
    std::array<double, 2> const r1{0.1, std::numeric_limits<double>::infinity()};
    auto dataset = test::make_empty_dataset(false, "input");
    dataset.add_buffer("node", node_ids.size(), node_ids.size(), nullptr, nullptr);
    dataset.add_attribute_buffer("node", "id", node_ids.data());
    dataset.add_buffer("line", ids.size(), ids.size(), nullptr, nullptr);
    dataset.add_attribute_buffer("line", "id", ids.data());
    dataset.add_attribute_buffer("line", "from_node", from_node.data());
    dataset.add_attribute_buffer("line", "from_status", from_status.data());
    dataset.add_attribute_buffer("line", "r1", r1.data());
    InputValidationLookup input_id_lookup;
    power_grid_model::validation::detail::index_input_component_ids<Node>(dataset, input_id_lookup);
    Issues issues;

    ValidationContext const context{.input = dataset,
                                    .update = nullptr,
                                    .symmetric = true,
                                    .calculation_type = CalculationType::power_flow,
                                    .input_id_lookup = std::move(input_id_lookup)};
    power_grid_model::validation::detail::apply_rules(line_rule_table().input_rules, context, issues);

    REQUIRE(issues.size() == 2);
    CHECK(issues[0].rule == "infinity");
    CHECK(issues[0].ids == std::vector<ID>{22});
    CHECK(issues[1].rule == "not_boolean");
    CHECK(issues[1].ids == std::vector<ID>{22});
}

TEST_CASE("Line batch validation checks only input status") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;

    std::array<LineInput, 1> const lines{{{.id = 31,
                                           .from_node = 1,
                                           .to_node = 2,
                                           .from_status = 2,
                                           .to_status = 1,
                                           .r1 = std::numeric_limits<double>::infinity()}}};
    auto input_dataset = test::make_empty_dataset(false, "input");
    input_dataset.add_buffer("line", lines.size(), lines.size(), nullptr, lines.data());
    auto const update_dataset = test::make_empty_dataset(true, "update");
    Issues issues;

    ValidationContext const context{.input = input_dataset,
                                    .update = &update_dataset,
                                    .symmetric = true,
                                    .calculation_type = CalculationType::power_flow};
    power_grid_model::validation::detail::apply_rules(line_rule_table().batch_rules, context, issues);

    REQUIRE(issues.size() == 1);
    CHECK(issues.front().rule == "not_boolean");
    CHECK(issues.front().component == "line");
    CHECK(issues.front().field == "from_status");
    CHECK(issues.front().ids == std::vector<ID>{31});
}
