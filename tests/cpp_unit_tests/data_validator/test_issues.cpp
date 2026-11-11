// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/validation/issues.hpp>
#include <power_grid_model/validation/report_json.hpp>

#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/exception.hpp>

#include <doctest/doctest.h>

#include <string_view>
#include <vector>

TEST_CASE("Validation issues and reports") {
    using namespace power_grid_model::validation;

    SUBCASE("issue JSON serializes stable rule names") {
        CHECK(std::string_view{to_string(RuleCode::missing_value)} == "missing_value");
        CHECK(std::string_view{to_string(RuleCode::infinity)} == "infinity");
        CHECK(std::string_view{to_string(RuleCode::not_greater_than_zero)} == "not_greater_than_zero");
        CHECK(std::string_view{to_string(RuleCode::not_boolean)} == "not_boolean");
        CHECK_THROWS_AS(to_string(static_cast<RuleCode>(255)), power_grid_model::MissingCaseForEnumError);
    }

    SUBCASE("report validity follows scope issues") {
        ValidationReport report;
        CHECK_FALSE(report.valid());

        report.scopes.push_back(
            {.scenario = 0,
             .issues = {{.rule = RuleCode::missing_value, .component = "node", .field = "u_rated", .ids = {1}}}});
        CHECK_FALSE(report.valid());
    }

    SUBCASE("an issue with no IDs leaves the issue list unchanged") {
        Issues issues;
        add_issue(issues, RuleCode::missing_value, "node", "u_rated");

        CHECK(issues.empty());
    }

    SUBCASE("an issue from its rule code preserves the rule code") {
        Issues issues;
        add_issue(issues, RuleCode::not_boolean, "line", "from_status", {4});

        REQUIRE(issues.size() == 1);
        CHECK(issues.front().rule == RuleCode::not_boolean);
        CHECK(issues.front().component == "line");
        CHECK(issues.front().field == "from_status");
    }

    SUBCASE("issues preserve report fields and append order") {
        Issues issues;
        add_issue(issues, RuleCode::missing_value, "node", "u_rated", {3, 7});
        add_issue(issues, RuleCode::infinity, "line", "r1", {9});

        REQUIRE(issues.size() == 2);
        CHECK(issues[0].rule == RuleCode::missing_value);
        CHECK(issues[0].component == "node");
        CHECK(issues[0].field == "u_rated");
        CHECK(issues[0].ids == std::vector<power_grid_model::ID>{3, 7});
        CHECK(issues[1].rule == RuleCode::infinity);
        CHECK(issues[1].component == "line");
        CHECK(issues[1].field == "r1");
        CHECK(issues[1].ids == std::vector<power_grid_model::ID>{9});
    }
}
