// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/validation/report.hpp>

#include <doctest/doctest.h>
#include <string_view>
#include <vector>

TEST_CASE("Validation report validity follows scope status and issues") {
    using namespace power_grid_model::validation;
    using enum ValidationStatus;

    ValidationReport report;
    CHECK(report.valid());

    report.scopes.push_back({.scenario = 0, .status = passed, .issues = {}});
    CHECK(report.valid());

    report.scopes.front().status = failed;
    CHECK_FALSE(report.valid());

    report.scopes.front().status = blocked;
    report.scopes.front().issues.clear();
    CHECK_FALSE(report.valid());

    report.scopes.front().status = passed;
    report.scopes.front().issues.push_back(
        {.rule = "missing_value", .component = "node", .field = "u_rated", .ids = {1}});
    CHECK_FALSE(report.valid());
}

TEST_CASE("Validation report types have stable names") {
    using namespace power_grid_model::validation;

    CHECK(std::string_view{to_string(ValidationType::input)} == "input");
    CHECK(std::string_view{to_string(ValidationType::batch)} == "batch");
    CHECK_THROWS_AS(to_string(static_cast<ValidationType>(255)), power_grid_model::MissingCaseForEnumError);
}

TEST_CASE("Validation scopes default to an unscoped passed result") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;

    ValidationScope const scope;
    CHECK(scope.scenario == na_Idx);
    CHECK(scope.status == ValidationStatus::passed);
    CHECK(scope.issues.empty());
}

TEST_CASE("Validation scope helper derives status from its issues") {
    using namespace power_grid_model::validation;

    auto const passed = make_validation_scope({});
    CHECK(passed.scenario == power_grid_model::na_Idx);
    CHECK(passed.status == ValidationStatus::passed);
    CHECK(passed.issues.empty());

    auto const failed =
        make_validation_scope({{.rule = "missing_value", .component = "node", .field = "u_rated", .ids = {2}}});
    CHECK(failed.status == ValidationStatus::failed);
    REQUIRE(failed.issues.size() == 1);
    CHECK(failed.issues.front().ids == std::vector<power_grid_model::ID>{2});
}
