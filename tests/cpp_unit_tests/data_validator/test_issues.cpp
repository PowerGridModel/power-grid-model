// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "power_grid_model/common/common.hpp"
#include "power_grid_model/common/exception.hpp"
#include <power_grid_model/validation/issues.hpp>

#include <doctest/doctest.h>

#include <string_view>
#include <vector>

TEST_CASE("Rule codes have stable report names") {
    using namespace power_grid_model::validation;

    CHECK(std::string_view{to_string(RuleCode::missing_value)} == "missing_value");
    CHECK(std::string_view{to_string(RuleCode::infinity)} == "infinity");
    CHECK(std::string_view{to_string(RuleCode::not_greater_than_zero)} == "not_greater_than_zero");
    CHECK(std::string_view{to_string(RuleCode::not_boolean)} == "not_boolean");
    CHECK_THROWS_AS(to_string(static_cast<RuleCode>(255)), power_grid_model::MissingCaseForEnumError);
}

TEST_CASE("Adding an issue with no ids leaves the issue list unchanged") {
    using namespace power_grid_model::validation;

    Issues issues;
    add_issue(issues, "missing_value", "node", "u_rated");

    CHECK(issues.empty());
}

TEST_CASE("Adding an issue from its rule code preserves the serialized rule name") {
    using namespace power_grid_model::validation;

    Issues issues;
    add_issue(issues, RuleCode::not_boolean, "line", "from_status", {4});

    REQUIRE(issues.size() == 1);
    CHECK(issues.front().rule == "not_boolean");
    CHECK(issues.front().component == "line");
    CHECK(issues.front().field == "from_status");
}

TEST_CASE("Adding issues preserves their report fields and append order") {
    using namespace power_grid_model::validation;

    Issues issues;
    add_issue(issues, "missing_value", "node", "u_rated", {3, 7});
    add_issue(issues, "infinity", "line", "r1", {9});

    REQUIRE(issues.size() == 2);
    CHECK(issues[0].rule == "missing_value");
    CHECK(issues[0].component == "node");
    CHECK(issues[0].field == "u_rated");
    CHECK(issues[0].ids == std::vector<power_grid_model::ID>{3, 7});
    CHECK(issues[1].rule == "infinity");
    CHECK(issues[1].component == "line");
    CHECK(issues[1].field == "r1");
    CHECK(issues[1].ids == std::vector<power_grid_model::ID>{9});
}
