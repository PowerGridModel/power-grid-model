// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/validation/report.hpp>

#include <power_grid_model/common/common.hpp>

#include <doctest/doctest.h>

#include <vector>

TEST_CASE("Validation report validity follows scope issues") {
    using namespace power_grid_model::validation;

    ValidationReport report;
    CHECK(report.valid());

    report.scopes.push_back({.scenario = 0, .issues = {}});
    CHECK(report.valid());

    report.scopes.front().issues.push_back(
        {.rule = "missing_value", .component = "node", .field = "u_rated", .ids = {1}});
    CHECK_FALSE(report.valid());
}

TEST_CASE("Validation scopes default to an unscoped result") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;

    ValidationScope const scope;
    CHECK(scope.scenario == na_Idx);
    CHECK(scope.issues.empty());
}

TEST_CASE("Validation scope helper records scenario and issues") {
    using namespace power_grid_model::validation;

    auto const unscoped = make_validation_scope({});
    CHECK(unscoped.scenario == power_grid_model::na_Idx);
    CHECK(unscoped.issues.empty());

    auto const scenario_scoped = make_validation_scope({}, 2);
    CHECK(scenario_scoped.scenario == 2);
    CHECK(scenario_scoped.issues.empty());

    auto const report =
        make_validation_scope({{.rule = "missing_value", .component = "node", .field = "u_rated", .ids = {2}}});
    REQUIRE(report.issues.size() == 1);
    CHECK(report.issues.front().ids == std::vector<power_grid_model::ID>{2});
}
