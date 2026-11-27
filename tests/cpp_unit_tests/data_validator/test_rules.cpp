// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/validation/rules.hpp>

#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/validation/context.hpp>
#include <power_grid_model/validation/issues.hpp>

#include <doctest/doctest.h>

#include <array>
#include <concepts>
#include <limits>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

static_assert(power_grid_model::validation::rules::rule_c<power_grid_model::validation::rules::Required, double>);
static_assert(power_grid_model::validation::rules::rule_c<power_grid_model::validation::rules::Finite, double>);
static_assert(power_grid_model::validation::rules::rule_c<power_grid_model::validation::rules::Unique, bool>);
static_assert(!power_grid_model::validation::rules::rule_c<power_grid_model::validation::rules::NotBothZero, double>);

static_assert(
    std::invocable<power_grid_model::validation::rules::FilteredValidId<int> const&, power_grid_model::ID, int const&>);
static_assert(!std::invocable<power_grid_model::validation::rules::FilteredValidId<int> const&, power_grid_model::ID,
                              double const&>);

TEST_CASE("Validation rules") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;

    SUBCASE("reports missing floating, ID, and status values") {
        std::array<ID, 2> const ids{1, 2};
        Issues issues;

        std::array<double, 2> const floating_values{power_grid_model::nan, 1.0};
        rules::apply(rules::is_required, floating_values, ids, "test", "floating", issues);

        std::array<ID, 2> const id_values{na_IntID, 4};
        rules::apply(rules::is_required, id_values, ids, "test", "id", issues);

        std::array<IntS, 2> const status_values{na_IntS, 1};
        rules::apply(rules::is_required, status_values, ids, "test", "status", issues);

        REQUIRE(issues.size() == 3);
        CHECK(issues[0].rule == RuleCode::missing_value);
        CHECK(issues[0].field == "floating");
        CHECK(issues[0].ids == std::vector<ID>{1});
        CHECK(issues[1].field == "id");
        CHECK(issues[1].ids == std::vector<ID>{1});
        CHECK(issues[2].field == "status");
        CHECK(issues[2].ids == std::vector<ID>{1});
    }

    SUBCASE("reports nothing for empty ranges and accepts standard containers") {
        std::array<double, 0> const no_values{};
        std::array<ID, 0> const no_ids{};
        Issues issues;
        rules::apply(rules::is_required, no_values, no_ids, "test", "value", issues);
        CHECK(issues.empty());

        std::vector<double> const values{2.0, power_grid_model::nan, 3.0};
        std::vector<ID> const ids{7, 8, 9};
        rules::apply(rules::is_required, values, ids, "test", "value", issues);

        REQUIRE(issues.size() == 1);
        CHECK(issues.front().ids == std::vector<ID>{8});
    }

    SUBCASE("finite reports both infinities but not NaN or finite values") {
        std::array<double, 4> const values{std::numeric_limits<double>::infinity(),
                                           -std::numeric_limits<double>::infinity(), power_grid_model::nan, 5.0};
        std::array<ID, 4> const ids{1, 2, 3, 4};
        Issues issues;

        rules::apply(rules::is_finite, values, ids, "test", "value", issues);

        REQUIRE(issues.size() == 1);
        CHECK(issues.front().rule == RuleCode::infinity);
        CHECK(issues.front().ids == std::vector<ID>{1, 2});
    }

    SUBCASE("greater-than-zero reports zero and negative values, but not NaN or infinity") {
        std::array<double, 5> const values{0.0, -0.0, -1.0, power_grid_model::nan,
                                           std::numeric_limits<double>::infinity()};
        std::array<ID, 5> const ids{1, 2, 3, 4, 5};
        Issues issues;

        rules::apply(rules::is_greater_than_zero, values, ids, "test", "value", issues);

        REQUIRE(issues.size() == 1);
        CHECK(issues.front().rule == RuleCode::not_greater_than_zero);
        CHECK(issues.front().ids == std::vector<ID>{1, 2, 3});
    }

    SUBCASE("boolean reports values other than zero and one, including missing status") {
        std::array<IntS, 5> const values{0, 1, 2, -1, na_IntS};
        std::array<ID, 5> const ids{10, 11, 12, 13, 14};
        Issues issues;

        rules::apply(rules::is_boolean, values, ids, "test", "status", issues);

        REQUIRE(issues.size() == 1);
        CHECK(issues.front().rule == RuleCode::not_boolean);
        CHECK(issues.front().ids == std::vector<ID>{12, 13, 14});
    }
    SUBCASE("unique reports values that are not unique") {
        using namespace power_grid_model;
        using namespace power_grid_model::validation;

        std::array<bool, 3> const is_unique{true, false, true};
        std::array<ID, 3> const ids{1, 2, 3};
        Issues issues;

        rules::apply(rules::Unique{}, is_unique, ids, "test", "id", issues);

        REQUIRE(issues.size() == 1);
        CHECK(issues.front().rule == RuleCode::not_unique);
        CHECK(issues.front().ids == std::vector<ID>{2});
    }

    SUBCASE("filtered valid id only checks ids with a filter value in the table") {
        IDToComponentElementMap const id_map{{1, {.component_type = "node", .element_idx = 0}},
                                             {2, {.component_type = "source", .element_idx = 0}}};
        static constexpr std::array<std::string_view, 1> nodes{"node"};
        static constexpr std::array<std::string_view, 1> sources{"source"};
        static constexpr std::array<std::pair<int, std::span<std::string_view const>>, 2> table{
            {{10, nodes}, {20, sources}}};
        rules::FilteredValidId<int> const rule{.id_to_component_element_map = id_map,
                                               .filter_to_ref_components = table};

        CHECK_FALSE(rule(1, 10));
        CHECK_FALSE(rule(2, 20));
        CHECK(rule(2, 10));
        CHECK(rule(1, 20));
        CHECK(rule(3, 10));
        CHECK(rule(na_IntID, 20));
        CHECK_FALSE(rule(3, 30));
    }

    SUBCASE("not-yet-implemented range-based rule reports NotImplementedError") {
        std::array<ID, 1> const ids{1};
        std::array<double, 1> const first_values{0.0};
        std::array<double, 1> const second_values{0.0};
        Issues issues;

        CHECK_THROWS_AS(rules::NotBothZero{}(first_values, second_values, ids, "test", "first", "second", issues),
                        NotImplementedError);
    }
}
