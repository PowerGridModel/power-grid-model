// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "validation_test_utils.hpp"

#include <power_grid_model/validation/batch.hpp>

#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/validation/context.hpp>
#include <power_grid_model/validation/issues.hpp>
#include <power_grid_model/validation/registry.hpp>

#include <doctest/doctest.h>

#include <array>
#include <vector>

TEST_CASE("Batch validation orchestration") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using namespace power_grid_model::validation::detail;
    using namespace test;

    SUBCASE("run only batch rules and emit one base scope") {
        std::array<TestInput, 1> const rows{{{.id = 3, .value = 1.0}}};
        auto input = make_empty_dataset(false, "input");
        input.add_buffer(TestComponent::name, rows.size(), rows.size(), nullptr, rows.data());
        auto const update = make_empty_dataset(true, "update");
        std::vector<int> calls;
        RuleDefinition input_rule{.code = RuleCode::missing_value,
                                  .fields = {"value"},
                                  .apply = [&calls](ValidationContext const&, Issues&) { calls.push_back(1); }};
        RuleDefinition batch_rule{
            .code = RuleCode::not_boolean,
            .fields = {"status"},
            .apply = [&calls](ValidationContext const& context, Issues& issues) {
                calls.push_back(2);
                CHECK(context.options.calculation_type == CalculationType::state_estimation);
                CHECK(context.options.calculation_symmetry == CalculationSymmetry::asymmetric);
                issues.push_back({.rule = RuleCode::not_boolean, .component = "test", .field = "status", .ids = {3}});
            }};
        ValidationRegistry const registry{
            {.component = "test", .input_rules = {input_rule}, .batch_rules = {batch_rule}}};

        auto const report =
            validate_batch_with_registry<TestComponentsTuple>(input, update,
                                                              {.calculation_type = CalculationType::state_estimation,
                                                               .calculation_symmetry = CalculationSymmetry::asymmetric},
                                                              registry);

        CHECK(calls == std::vector<int>{2});
        CHECK(report.is_batch);
        REQUIRE(report.scopes.size() == 1);
        CHECK(report.scopes.front().scenario == na_Idx);
        REQUIRE(report.scopes.front().issues.size() == 1);
        CHECK(report.scopes.front().issues.front().rule == RuleCode::not_boolean);
        CHECK(report.scopes.front().issues.front().ids == std::vector<ID>{3});
    }

    SUBCASE("exit early when input IDs are duplicated") {
        std::array<TestInput, 3> const rows{{{.id = 7}, {.id = 7}, {.id = 10}}};
        auto input = make_empty_dataset(false, "input");
        input.add_buffer(TestComponent::name, rows.size(), rows.size(), nullptr, rows.data());
        auto const update = make_empty_dataset(true, "update");

        ValidationRegistry const registry{{.component = TestComponent::name, .input_rules = {}, .batch_rules = {}}};

        auto const report = validate_batch_with_registry<TestComponentsTuple>(input, update, {}, registry);

        CHECK(report.is_batch == true);
        REQUIRE(report.scopes.size() == 1);
        REQUIRE(report.scopes.front().issues.size() == 1);
        CHECK(report.scopes.front().issues.front().rule == RuleCode::not_unique);
        CHECK(report.scopes.front().issues.front().component == TestComponent::name);
        CHECK(report.scopes.front().issues.front().field == TestInput::id_name);
        CHECK(report.scopes.front().issues.front().ids == std::vector<ID>{7});
    }

    SUBCASE("validate dataset types before running rules") {
        auto const input = make_empty_dataset(false, "input");
        auto const update = make_empty_dataset(false, "update");
        bool called = false;
        RuleDefinition rule{.code = RuleCode::not_boolean,
                            .fields = {"status"},
                            .apply = [&called](ValidationContext const&, Issues&) { called = true; }};
        ValidationRegistry const registry{{.component = "test", .input_rules = {}, .batch_rules = {rule}}};

        CHECK_THROWS_AS(validate_batch_with_registry<TestComponentsTuple>(input, update, {}, registry),
                        InvalidArguments);
        CHECK_FALSE(called);
    }
}
