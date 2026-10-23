// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "power_grid_model/auxiliary/meta_data.hpp"
#include "power_grid_model/common/common.hpp"
#include "power_grid_model/validation/context.hpp"
#include "power_grid_model/validation/issues.hpp"
#include "power_grid_model/validation/report.hpp"
#include "validation_test_utils.hpp"

#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/validation/input.hpp>

#include <doctest/doctest.h>

#include <array>
#include <vector>

TEST_CASE("Input orchestration runs injected input rules in declaration order") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using test::make_empty_dataset;
    using test::TestComponent;
    using test::TestInput;

    std::array<TestInput, 1> const rows{{{.id = 7, .value = 1.0}}};
    auto dataset = make_empty_dataset(false, "input");
    dataset.add_buffer(TestComponent::name, rows.size(), rows.size(), nullptr, rows.data());
    std::vector<int> calls;
    RuleDefinition first{.code = RuleCode::missing_value,
                         .fields = {{.name = "first", .type = meta_data::ctype_v<double>}},
                         .apply = [&calls](ValidationContext const& context, Issues& issues) {
                             calls.push_back(1);
                             CHECK(context.symmetric);
                             CHECK(context.calculation_type == CalculationType::power_flow);
                             issues.push_back(
                                 {.rule = "missing_value", .component = "test", .field = "first", .ids = {7}});
                         }};
    RuleDefinition second{.code = RuleCode::infinity,
                          .fields = {{.name = "second", .type = meta_data::ctype_v<double>}},
                          .apply = [&calls](ValidationContext const&, Issues&) { calls.push_back(2); }};
    RuleDefinition batch_rule{.code = RuleCode::not_boolean,
                              .fields = {{.name = "status", .type = meta_data::ctype_v<IntS>}},
                              .apply = [&calls](ValidationContext const&, Issues&) { calls.push_back(3); }};
    ValidationRegistry const registry{
        {.component = TestComponent::name, .input_rules = {first, second}, .batch_rules = {batch_rule}}};

    auto const report = power_grid_model::validation::detail::validate_input_with_registry(
        dataset, true, CalculationType::power_flow, registry);

    CHECK(calls == std::vector<int>{1, 2});
    CHECK(report.type == ValidationType::input);
    REQUIRE(report.scopes.size() == 1);
    CHECK(report.scopes.front().status == ValidationStatus::failed);
    CHECK(report.scopes.front().scenario == na_Idx);
    REQUIRE(report.scopes.front().issues.size() == 1);
    CHECK(report.scopes.front().issues.front().ids == std::vector<ID>{7});
}

TEST_CASE("Input orchestration skips rules for absent components") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using test::make_empty_dataset;
    using test::TestComponent;

    auto const dataset = make_empty_dataset(false, "input");
    bool called = false;
    RuleDefinition const rule{.code = RuleCode::missing_value,
                              .fields = {{.name = "value", .type = meta_data::ctype_v<double>}},
                              .apply = [&called](ValidationContext const&, Issues&) { called = true; }};
    ValidationRegistry const registry{{.component = TestComponent::name, .input_rules = {rule}, .batch_rules = {}}};

    auto const report = power_grid_model::validation::detail::validate_input_with_registry(
        dataset, true, CalculationType::power_flow, registry);

    CHECK_FALSE(called);
    CHECK(report.valid());
}

TEST_CASE("Input orchestration reports a passed scope for an empty registry") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using test::make_empty_dataset;

    auto const dataset = make_empty_dataset(false, "input");
    auto const report = power_grid_model::validation::detail::validate_input_with_registry(
        dataset, false, CalculationType::state_estimation, {});

    CHECK(report.valid());
    CHECK(report.type == ValidationType::input);
    REQUIRE(report.scopes.size() == 1);
    CHECK(report.scopes.front().status == ValidationStatus::passed);
}

TEST_CASE("Input validation skips components without IDs") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using test::make_empty_dataset;
    using test::TestComponent;
    using test::TestInput;

    std::array<double, 1> const values{power_grid_model::nan};
    auto dataset = make_empty_dataset(false, "input");
    dataset.add_buffer(TestComponent::name, values.size(), values.size(), nullptr, nullptr);
    dataset.add_attribute_buffer(TestComponent::name, TestInput::value_name, values.data());

    bool called = false;
    RuleDefinition const rule{.code = RuleCode::missing_value,
                              .fields = {{.name = TestInput::value_name, .type = meta_data::ctype_v<double>}},
                              .apply = [&called](ValidationContext const&, Issues&) { called = true; }};
    ValidationRegistry const registry{{.component = TestComponent::name, .input_rules = {rule}, .batch_rules = {}}};
    auto const report = power_grid_model::validation::detail::validate_input_with_registry(
        dataset, true, CalculationType::power_flow, registry);

    CHECK_FALSE(called);
    CHECK(report.valid());
}

TEST_CASE("Input orchestration validates dataset type before running rules") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using test::make_empty_dataset;

    auto const update = make_empty_dataset(false, "update");
    bool called = false;
    RuleDefinition rule{.code = RuleCode::missing_value,
                        .fields = {{.name = "value", .type = meta_data::ctype_v<double>}},
                        .apply = [&called](ValidationContext const&, Issues&) { called = true; }};
    ValidationRegistry const registry{{.component = "test", .input_rules = {rule}, .batch_rules = {}}};

    CHECK_THROWS_AS(power_grid_model::validation::detail::validate_input_with_registry(
                        update, true, CalculationType::power_flow, registry),
                    InvalidArguments);
    CHECK_FALSE(called);
}
