// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/validation/issues.hpp>

#include <doctest/doctest.h>

#include <vector>

TEST_CASE("Adding an issue with no ids leaves the issue list unchanged") {
    using namespace power_grid_model::validation;

    Issues issues;
    add_issue(issues, "missing_value", "node", "u_rated");

    CHECK(issues.empty());
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
