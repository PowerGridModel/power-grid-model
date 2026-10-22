// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "power_grid_model/validation/context.hpp"
#include "power_grid_model/validation/issues.hpp"
#include "validation_test_utils.hpp"

#include <power_grid_model/auxiliary/input.hpp>
#include <power_grid_model/validation/line.hpp>

#include <doctest/doctest.h>

#include <array>
#include <limits>
#include <vector>

TEST_CASE("Line batch rule table checks input status") {
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
