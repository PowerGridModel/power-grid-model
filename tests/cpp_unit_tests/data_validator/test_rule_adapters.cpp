// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "validation_test_utils.hpp"

#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/validation/context.hpp>
#include <power_grid_model/validation/issues.hpp>
#include <power_grid_model/validation/rule_adapters.hpp>
#include <power_grid_model/validation/rules.hpp>

#include <doctest/doctest.h>

#include <array>
#include <vector>

namespace {
struct CountingRule {
    static constexpr power_grid_model::validation::RuleCode code{power_grid_model::validation::RuleCode::infinity};

    CountingRule(int* calls_, bool result_) : calls{calls_}, result{result_} {}

    int* calls;
    bool result;

    bool operator()(double /*unused*/) const {
        ++(*calls);
        return result;
    }
};
} // namespace

TEST_CASE("Unary rule adapter maps one field to an issue") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using test::make_empty_dataset;
    using test::TestComponent;
    using test::TestInput;

    auto const rule = make_unary_rule<TestComponent, &TestInput::value>("value", rules::Required{});
    CHECK(rule.code == RuleCode::missing_value);
    REQUIRE(rule.fields.size() == 1);
    CHECK(rule.fields.front() == TestInput::value_name);

    SUBCASE("row data") {
        std::array<TestInput, 2> const rows{{{.id = 41, .value = power_grid_model::nan}, {.id = 42, .value = 1.0}}};
        auto dataset = make_empty_dataset(false, "input");
        dataset.add_buffer(TestComponent::name, rows.size(), rows.size(), nullptr, rows.data());
        ValidationContext const context{
            .input = dataset, .symmetric = true, .calculation_type = CalculationType::power_flow};
        Issues issues;

        rule.apply(context, issues);

        REQUIRE(issues.size() == 1);
        CHECK(issues.front().component == TestComponent::name);
        CHECK(issues.front().field == TestInput::value_name);
        CHECK(issues.front().ids == std::vector<ID>{41});
    }

    SUBCASE("columnar data") {
        std::array<ID, 2> const ids{41, 42};
        std::array<double, 2> const values{power_grid_model::nan, 1.0};
        auto dataset = make_empty_dataset(false, "input");
        dataset.add_buffer(TestComponent::name, ids.size(), ids.size(), nullptr, nullptr);
        dataset.add_attribute_buffer(TestComponent::name, TestInput::id_name, ids.data());
        dataset.add_attribute_buffer(TestComponent::name, TestInput::value_name, values.data());
        ValidationContext const context{
            .input = dataset, .symmetric = true, .calculation_type = CalculationType::power_flow};
        Issues issues;

        rule.apply(context, issues);

        REQUIRE(issues.size() == 1);
        CHECK(issues.front().ids == std::vector<ID>{41});
    }

    SUBCASE("missing column defaults to the field's missing value") {
        std::array<ID, 1> const ids{41};
        auto dataset = make_empty_dataset(false, "input");
        dataset.add_buffer(TestComponent::name, ids.size(), ids.size(), nullptr, nullptr);
        dataset.add_attribute_buffer(TestComponent::name, TestInput::id_name, ids.data());
        ValidationContext const context{
            .input = dataset, .symmetric = true, .calculation_type = CalculationType::power_flow};
        Issues issues;

        rule.apply(context, issues);

        REQUIRE(issues.size() == 1);
        CHECK(issues.front().ids == std::vector<ID>{41});
    }
}

TEST_CASE("Unary rule adapter evaluates a missing attribute default once") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using test::make_empty_dataset;
    using test::TestComponent;
    using test::TestInput;

    std::array<ID, 2> const ids{41, 42};
    auto dataset = make_empty_dataset(false, "input");
    dataset.add_buffer(TestComponent::name, ids.size(), ids.size(), nullptr, nullptr);
    dataset.add_attribute_buffer(TestComponent::name, TestInput::id_name, ids.data());
    ValidationContext const context{
        .input = dataset, .symmetric = true, .calculation_type = CalculationType::power_flow};

    SUBCASE("default passes") {
        int calls{};
        auto const rule =
            make_unary_rule<TestComponent, &TestInput::value>(TestInput::value_name, CountingRule{&calls, false});
        Issues issues;

        rule.apply(context, issues);

        CHECK(issues.empty());
        CHECK(calls == 1);
    }

    SUBCASE("default fails") {
        int calls{};
        auto const rule =
            make_unary_rule<TestComponent, &TestInput::value>(TestInput::value_name, CountingRule{&calls, true});
        Issues issues;

        rule.apply(context, issues);

        REQUIRE(issues.size() == 1);
        CHECK(issues.front().ids == std::vector<ID>{41, 42});
        CHECK(calls == 1);
    }
}
