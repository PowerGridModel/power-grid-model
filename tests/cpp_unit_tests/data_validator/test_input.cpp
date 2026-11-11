// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "validation_test_utils.hpp"

#include <power_grid_model/validation/input.hpp>

#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/validation/context.hpp>
#include <power_grid_model/validation/issues.hpp>
#include <power_grid_model/validation/registry.hpp>

#include <doctest/doctest.h>

#include <array>
#include <vector>

TEST_CASE("Input validation orchestration") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using namespace power_grid_model::validation::detail;
    using namespace power_grid_model::validation::test;

    SUBCASE("runs injected input rules in declaration order") {
        std::array<TestInput, 1> const rows{{{.id = 7, .value = 1.0}}};
        auto dataset = make_empty_dataset(false, "input");
        dataset.add_buffer(TestComponent::name, rows.size(), rows.size(), nullptr, rows.data());
        std::vector<int> calls;
        RuleDefinition first{
            .code = RuleCode::missing_value,
            .fields = {"first"},
            .apply = [&calls](ValidationContext const& context, Issues& issues) {
                calls.push_back(1);
                CHECK(context.options.calculation_symmetry == CalculationSymmetry::symmetric);
                CHECK(context.options.calculation_type == CalculationType::power_flow);
                issues.push_back({.rule = RuleCode::missing_value, .component = "test", .field = "first", .ids = {7}});
            }};
        RuleDefinition second{.code = RuleCode::infinity,
                              .fields = {"second"},
                              .apply = [&calls](ValidationContext const&, Issues&) { calls.push_back(2); }};
        RuleDefinition batch_rule{.code = RuleCode::not_boolean,
                                  .fields = {"status"},
                                  .apply = [&calls](ValidationContext const&, Issues&) { calls.push_back(3); }};
        ValidationRegistry const registry{
            {.component = TestComponent::name, .input_rules = {first, second}, .batch_rules = {batch_rule}}};

        auto const report = validate_input_with_registry<TestComponentsTuple>(dataset, {}, registry);

        CHECK(calls == std::vector<int>{1, 2});
        CHECK_FALSE(report.is_batch);
        REQUIRE(report.scopes.size() == 1);
        CHECK(report.scopes.front().scenario == na_Idx);
        REQUIRE(report.scopes.front().issues.size() == 1);
        CHECK(report.scopes.front().issues.front().ids == std::vector<ID>{7});
    }

    SUBCASE("exits early when input IDs are duplicated") {
        std::array<TestInput, 3> const rows{{{.id = 7}, {.id = 7}, {.id = 10}}};
        auto dataset = make_empty_dataset(false, "input");
        dataset.add_buffer(TestComponent::name, rows.size(), rows.size(), nullptr, rows.data());

        auto const report =
            power_grid_model::validation::detail::validate_input_with_registry<TestComponentsTuple>(dataset, {}, {});

        CHECK(report.is_batch == false);
        REQUIRE(report.scopes.size() == 1);
        REQUIRE(report.scopes.front().issues.size() == 1);
        CHECK(report.scopes.front().issues.front().rule == RuleCode::not_unique);
        CHECK(report.scopes.front().issues.front().component == TestComponent::name);
        CHECK(report.scopes.front().issues.front().field == TestInput::id_name);
        CHECK(report.scopes.front().issues.front().ids == std::vector<ID>{7});
    }

    SUBCASE("skips rules for absent components") {
        auto const dataset = make_empty_dataset(false, "input");
        bool called = false;
        RuleDefinition const rule{.code = RuleCode::missing_value,
                                  .fields = {"value"},
                                  .apply = [&called](ValidationContext const&, Issues&) { called = true; }};
        ValidationRegistry const registry{{.component = TestComponent::name, .input_rules = {rule}, .batch_rules = {}}};

        auto const report = validate_input_with_registry<TestComponentsTuple>(dataset, {}, registry);

        CHECK_FALSE(called);
        CHECK(report.valid());
    }

    SUBCASE("reports no scopes for an empty registry") {
        auto const dataset = make_empty_dataset(false, "input");
        auto const report = power_grid_model::validation::detail::validate_input_with_registry(
            dataset,
            {.calculation_type = CalculationType::state_estimation,
             .calculation_symmetry = CalculationSymmetry::asymmetric},
            {});

        CHECK(report.valid());
        CHECK_FALSE(report.is_batch);
        CHECK(report.scopes.empty());
    }

    SUBCASE("Input validation skips components without IDs") {
        // TODO(figueroa1395): Handle the case where the component has no IDs correctly
        std::array<double, 1> const values{power_grid_model::nan};
        auto dataset = make_empty_dataset(false, "input");
        dataset.add_buffer(TestComponent::name, values.size(), values.size(), nullptr, nullptr);
        dataset.add_attribute_buffer(TestComponent::name, TestInput::value_name, values.data());

        bool called = false;
        RuleDefinition const rule{.code = RuleCode::missing_value,
                                  .fields = {TestInput::value_name},
                                  .apply = [&called](ValidationContext const&, Issues&) { called = true; }};
        ValidationRegistry const registry{{.component = TestComponent::name, .input_rules = {rule}, .batch_rules = {}}};
        auto const report = validate_input_with_registry<TestComponentsTuple>(dataset, {}, registry);

        CHECK_FALSE(called);
        CHECK(report.valid());
    }

    SUBCASE("validates dataset type before running rules") {
        auto const update = make_empty_dataset(false, "update");
        bool called = false;
        RuleDefinition rule{.code = RuleCode::missing_value,
                            .fields = {"value"},
                            .apply = [&called](ValidationContext const&, Issues&) { called = true; }};
        ValidationRegistry const registry{{.component = "test", .input_rules = {rule}, .batch_rules = {}}};

        CHECK_THROWS_AS(validate_input_with_registry<TestComponentsTuple>(update, {}, registry), InvalidArguments);
        CHECK_FALSE(called);
    }
}
