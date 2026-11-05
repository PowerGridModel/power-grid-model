// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "validation_test_utils.hpp"

#include <power_grid_model/common/common.hpp>
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
        ValidationContext const context{.input = dataset};
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
        ValidationContext const context{.input = dataset};
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
        ValidationContext const context{.input = dataset};
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
    ValidationContext const context{.input = dataset};

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

TEST_CASE("Unary rule factory forwards each component span to the callable") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using test::make_empty_dataset;
    using test::TestComponent;
    using test::TestInput;

    bool is_callable_invoked{false};
    int called_on_component_counter{};
    auto const collect_failing_ids = [&is_callable_invoked](ValidationContext const& /*context*/, Issues& issues,
                                                            auto const& component_span, CountingRule const& rule) {
        is_callable_invoked = true;
        std::vector<ID> failing_ids;
        for (auto const id : validation::detail::get_ids_span<TestComponent>(component_span)) {
            if (rule(0.0)) { // dummy counting call
                failing_ids.push_back(id);
            }
        }
        add_issue(issues, CountingRule::code, TestComponent::name, TestInput::id_name, failing_ids);
    };
    auto const make_rule = [&called_on_component_counter, &collect_failing_ids](bool result) {
        return validation::detail::unary_rule_factory<TestComponent, CountingRule>(
            TestInput::id_name, ValidationDatasetType::input, collect_failing_ids,
            CountingRule{&called_on_component_counter, result});
    };
    Issues issues;
    bool const is_rule_not_satisfied = true;

    SUBCASE("rule definition") {
        auto const rule = make_rule(true);

        CHECK(rule.code == RuleCode::infinity);
        REQUIRE(rule.fields.size() == 1);
        CHECK(rule.fields.front() == TestInput::id_name);
        CHECK(!is_callable_invoked);
        CHECK(called_on_component_counter == 0);
    }

    SUBCASE("row data") {
        std::array<TestInput, 2> const rows{{{.id = 24, .value = 1.0}, {.id = 42, .value = 2.0}}};
        auto const size = rows.size();
        auto dataset = make_empty_dataset(false, "input");
        dataset.add_buffer(TestComponent::name, size, size, nullptr, rows.data());
        ValidationContext const context{.input = dataset};

        make_rule(is_rule_not_satisfied).apply(context, issues);

        CHECK(is_callable_invoked);
        CHECK(called_on_component_counter == size);
        REQUIRE(issues.size() == 1);
        CHECK(issues.front().component == TestComponent::name);
        CHECK(issues.front().field == TestInput::id_name);
        CHECK(issues.front().ids == std::vector<ID>{24, 42});
    }

    SUBCASE("columnar data") {
        std::array<ID, 2> const ids{53, 23};
        auto dataset = make_empty_dataset(false, "input");
        auto const size = ids.size();
        dataset.add_buffer(TestComponent::name, size, size, nullptr, nullptr);
        dataset.add_attribute_buffer(TestComponent::name, TestInput::id_name, ids.data());
        ValidationContext const context{.input = dataset};

        make_rule(!is_rule_not_satisfied).apply(context, issues);

        CHECK(is_callable_invoked);
        CHECK(called_on_component_counter == size);
        CHECK(issues.empty());
    }

    SUBCASE("empty dataset") {
        auto const dataset = make_empty_dataset(false, "input");
        ValidationContext const context{.input = dataset};

        make_rule(true).apply(context, issues);

        CHECK(is_callable_invoked);
        CHECK(called_on_component_counter == 0);
        CHECK(issues.empty());
    }
}
