// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "validation_test_utils.hpp"

#include <power_grid_model/auxiliary/input.hpp>
#include <power_grid_model/validation/line.hpp>

#include <doctest/doctest.h>

#include <array>
#include <limits>
#include <vector>

TEST_CASE("Line input validation reports rule failures in declaration order") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;

    std::array<LineInput, 2> const lines{
        {{11, 0, 1, na_IntS, 1, power_grid_model::nan}, {12, 1, 2, 2, 1, std::numeric_limits<double>::infinity()}}};
    auto dataset = test::make_empty_dataset(false, "input");
    dataset.add_buffer("line", lines.size(), lines.size(), nullptr, lines.data());
    Issues issues;

    ValidationContext const context{dataset, nullptr, true, CalculationType::power_flow};
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
    std::array<IntS, 2> const from_status{0, 2};
    std::array<double, 2> const r1{0.1, std::numeric_limits<double>::infinity()};
    auto dataset = test::make_empty_dataset(false, "input");
    dataset.add_buffer("line", ids.size(), ids.size(), nullptr, nullptr);
    dataset.add_attribute_buffer("line", "id", ids.data());
    dataset.add_attribute_buffer("line", "from_status", from_status.data());
    dataset.add_attribute_buffer("line", "r1", r1.data());
    Issues issues;

    ValidationContext const context{dataset, nullptr, true, CalculationType::power_flow};
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

    std::array<LineInput, 1> const lines{{{31, 1, 2, 2, 1, std::numeric_limits<double>::infinity()}}};
    auto input_dataset = test::make_empty_dataset(false, "input");
    input_dataset.add_buffer("line", lines.size(), lines.size(), nullptr, lines.data());
    auto const update_dataset = test::make_empty_dataset(true, "update");
    Issues issues;

    ValidationContext const context{input_dataset, &update_dataset, true, CalculationType::power_flow};
    power_grid_model::validation::detail::apply_rules(line_rule_table().batch_rules, context, issues);

    REQUIRE(issues.size() == 1);
    CHECK(issues.front().rule == "not_boolean");
    CHECK(issues.front().component == "line");
    CHECK(issues.front().field == "from_status");
    CHECK(issues.front().ids == std::vector<ID>{31});
}
