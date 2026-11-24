// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/validation/rules.hpp>

#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/validation/issues.hpp>

#include <doctest/doctest.h>

#include <array>
#include <limits>
#include <vector>

static_assert(power_grid_model::validation::rules::rule_c<power_grid_model::validation::rules::Required, double>);
static_assert(power_grid_model::validation::rules::rule_c<power_grid_model::validation::rules::Finite, double>);
static_assert(power_grid_model::validation::rules::rule_c<power_grid_model::validation::rules::Unique, bool>);
static_assert(!power_grid_model::validation::rules::rule_c<power_grid_model::validation::rules::NotBothZero, double>);

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

    SUBCASE("not-yet-implemented range-based rule reports NotImplementedError") {
        std::array<ID, 1> const ids{1};
        std::array<double, 1> const first_values{0.0};
        std::array<double, 1> const second_values{0.0};
        Issues issues;

        CHECK_THROWS_AS(rules::NotBothZero{}(first_values, second_values, ids, "test", "first", "second", issues),
                        NotImplementedError);
    }
}
