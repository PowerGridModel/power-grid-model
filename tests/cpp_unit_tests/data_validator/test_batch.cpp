// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "power_grid_model/common/common.hpp"
#include "power_grid_model/common/exception.hpp"
#include "power_grid_model/validation/context.hpp"
#include "power_grid_model/validation/issues.hpp"
#include "power_grid_model/validation/report.hpp"
#include "validation_test_utils.hpp"

#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/validation/batch.hpp>

#include <doctest/doctest.h>

#include <array>
#include <vector>

TEST_CASE("Batch orchestration runs only batch rules and emits one base scope") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using test::make_empty_dataset;
    using test::TestComponent;
    using test::TestInput;

    std::array<TestInput, 1> const rows{{{.id = 3, .value = 1.0}}};
    auto input = make_empty_dataset(false, "input");
    input.add_buffer(TestComponent::name, rows.size(), rows.size(), nullptr, rows.data());
    auto const update = make_empty_dataset(true, "update");
    std::vector<int> calls;
    RuleDefinition input_rule{.code = RuleCode::missing_value,
                              .fields = {"value"},
                              .apply = [&calls](ValidationContext const&, Issues&) { calls.push_back(1); }};
    RuleDefinition batch_rule{.code = RuleCode::not_boolean,
                              .fields = {"status"},
                              .apply = [&calls](ValidationContext const& context, Issues& issues) {
                                  calls.push_back(2);
                                  CHECK(context.calculation_type == CalculationType::state_estimation);
                                  issues.push_back(
                                      {.rule = "not_boolean", .component = "test", .field = "status", .ids = {3}});
                              }};
    ValidationRegistry const registry{{.component = "test", .input_rules = {input_rule}, .batch_rules = {batch_rule}}};

    auto const report = power_grid_model::validation::detail::validate_batch_with_registry(
        input, update, false, CalculationType::state_estimation, registry);

    CHECK(calls == std::vector<int>{2});
    CHECK(report.type == ValidationType::batch);
    REQUIRE(report.scopes.size() == 1);
    CHECK(report.scopes.front().status == ValidationStatus::failed);
    CHECK(report.scopes.front().scenario == na_Idx);
    REQUIRE(report.scopes.front().issues.size() == 1);
    CHECK(report.scopes.front().issues.front().rule == "not_boolean");
    CHECK(report.scopes.front().issues.front().ids == std::vector<ID>{3});
}

TEST_CASE("Batch orchestration validates dataset types before running rules") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using test::make_empty_dataset;

    auto const input = make_empty_dataset(false, "input");
    auto const update = make_empty_dataset(false, "update");
    bool called = false;
    RuleDefinition rule{.code = RuleCode::not_boolean,
                        .fields = {"status"},
                        .apply = [&called](ValidationContext const&, Issues&) { called = true; }};
    ValidationRegistry const registry{{.component = "test", .input_rules = {}, .batch_rules = {rule}}};

    CHECK_THROWS_AS(power_grid_model::validation::detail::validate_batch_with_registry(
                        input, update, true, CalculationType::power_flow, registry),
                    InvalidArguments);
    CHECK_FALSE(called);
}
