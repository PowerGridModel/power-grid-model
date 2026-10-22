// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "power_grid_model/validation/context.hpp"
#include "validation_test_utils.hpp"

#include <power_grid_model/auxiliary/meta_data.hpp>
#include <power_grid_model/component/node.hpp>

#include <doctest/doctest.h>

#include <array>
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
