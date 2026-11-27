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
#include <utility>
#include <vector>

static_assert(power_grid_model::validation::rules::rule_c<power_grid_model::validation::rules::Required, double>);
static_assert(power_grid_model::validation::rules::rule_c<power_grid_model::validation::rules::Finite, double>);
static_assert(power_grid_model::validation::rules::rule_c<power_grid_model::validation::rules::Unique, bool>);
static_assert(!power_grid_model::validation::rules::rule_c<power_grid_model::validation::rules::NotBothZero, double>);

namespace power_grid_model {
namespace {
enum class TestEnum : power_grid_model::IntS { foo = 0, bar = 1, baz = 3 };
enum class TestEnumWithNan : power_grid_model::IntS { foo = 0, bar = 1, nan = -128 };
constexpr bool is_valid(TestEnum value) {
    switch (value) {
        using enum TestEnum;
    case foo:
    case bar:
    case baz:
        return true;
    default:
        return false;
    }
}
constexpr bool is_valid(TestEnumWithNan value) {
    switch (value) {
        using enum TestEnumWithNan;
    case foo:
    case bar:
    case nan:
        return true;
    default:
        return false;
    }
}
} // namespace
} // namespace power_grid_model

TEST_CASE("Validation rules") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;

    SUBCASE("reports missing floating, ID, and status values") {
        constexpr auto ids = std::array{ID{1}, ID{2}};
        Issues issues;

        constexpr auto floating_values = std::array{power_grid_model::nan, 1.0};
        rules::apply(rules::is_required, floating_values, ids, "test", "floating", issues);

        constexpr auto id_values = std::array{na_IntID, ID{4}};
        rules::apply(rules::is_required, id_values, ids, "test", "id", issues);

        constexpr auto status_values = std::array{na_IntS, IntS{1}};
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
        constexpr std::array<double, 0> no_values{};
        constexpr std::array<ID, 0> no_ids{};
        static_assert(no_values.size() == no_ids.size());

        Issues issues;
        rules::apply(rules::is_required, no_values, no_ids, "test", "value", issues);
        CHECK(issues.empty());

        std::vector<double> const values{2.0, power_grid_model::nan, 3.0};
        std::vector<ID> const ids{7, 8, 9};
        REQUIRE(values.size() == ids.size());
        rules::apply(rules::is_required, values, ids, "test", "value", issues);

        REQUIRE(issues.size() == 1);
        CHECK(issues.front().ids == std::vector<ID>{8});
    }

    SUBCASE("finite reports both infinities but not NaN or finite values") {
        constexpr auto values = std::array{std::numeric_limits<double>::infinity(),
                                           -std::numeric_limits<double>::infinity(), power_grid_model::nan, 5.0};
        constexpr auto ids = std::array{ID{1}, ID{2}, ID{3}, ID{4}};
        static_assert(values.size() == ids.size());

        Issues issues;
        rules::apply(rules::is_finite, values, ids, "test", "value", issues);

        REQUIRE(issues.size() == 1);
        CHECK(issues.front().rule == RuleCode::infinity);
        CHECK(issues.front().ids == std::vector<ID>{1, 2});
    }

    SUBCASE("greater-than-zero reports zero and negative values, but not NaN or infinity") {
        constexpr auto values =
            std::array{0.0, -0.0, -1.0, power_grid_model::nan, std::numeric_limits<double>::infinity()};
        constexpr auto ids = std::array{ID{1}, ID{2}, ID{3}, ID{4}, ID{5}};
        static_assert(values.size() == ids.size());

        Issues issues;
        rules::apply(rules::is_greater_than_zero, values, ids, "test", "value", issues);

        REQUIRE(issues.size() == 1);
        CHECK(issues.front().rule == RuleCode::not_greater_than_zero);
        CHECK(issues.front().ids == std::vector<ID>{1, 2, 3});
    }

    SUBCASE("less-than-one reports greater than or equal to one and infinity, but not (1 - epsilon), 0, negative "
            "values, NaN") {
        constexpr auto one_minus_infinitesimal = 1.0 - std::numeric_limits<double>::epsilon();
        static_assert(one_minus_infinitesimal < 1.0);

        constexpr auto values = std::array{1.0,
                                           one_minus_infinitesimal,
                                           2.0,
                                           0.0,
                                           -0.0,
                                           -1.0,
                                           power_grid_model::nan,
                                           std::numeric_limits<double>::infinity()};
        constexpr auto ids = std::array{ID{1}, ID{2}, ID{3}, ID{4}, ID{5}, ID{6}, ID{7}, ID{8}};
        static_assert(values.size() == ids.size());

        Issues issues;
        rules::apply(rules::is_less_than_one, values, ids, "test", "value", issues);

        REQUIRE(issues.size() == 1);
        CHECK(issues.front().rule == RuleCode::not_less_than_one);
        CHECK(issues.front().ids == std::vector<ID>{1, 3, 8});
    }

    SUBCASE("is-in-inclusive-range reports values less than the provided lower bound and greater than the provided "
            "upper bound, but not the lower bound, upper bound, or anything in between") {
        constexpr auto values = std::array{0, 1, -1, 2, -3, 3};
        constexpr auto ids = std::array{ID{1}, ID{2}, ID{3}, ID{4}, ID{5}, ID{6}};
        static_assert(values.size() == ids.size());

        Issues issues;
        rules::apply(rules::is_in_inclusive_range<-1, 2>, values, ids, "test", "value", issues);

        REQUIRE(issues.size() == 1);
        CHECK(issues.front().rule == RuleCode::not_in_inclusive_range);
        CHECK(issues.front().ids == std::vector<ID>{5, 6});
    }

    SUBCASE("boolean reports values other than zero and one, including missing status") {
        constexpr auto values = std::array{IntS{0}, IntS{1}, IntS{2}, IntS{-1}, IntS{na_IntS}};
        constexpr auto ids = std::array{ID{10}, ID{11}, ID{12}, ID{13}, ID{14}};
        static_assert(values.size() == ids.size());

        Issues issues;
        rules::apply(rules::is_boolean, values, ids, "test", "status", issues);

        REQUIRE(issues.size() == 1);
        CHECK(issues.front().rule == RuleCode::not_boolean);
        CHECK(issues.front().ids == std::vector<ID>{12, 13, 14});
    }

    SUBCASE("enum reports values other than values allowed by is_valid, including missing status") {
        using enum TestEnum;

        constexpr auto values = std::array{
            std::to_underlying(foo),
            std::to_underlying(bar),
            std::to_underlying(baz),
            IntS{2},      // invalid
            IntS{-1},     // invalid
            IntS{na_IntS} // invalid
        };
        constexpr auto ids = std::array{ID{10}, ID{11}, ID{12}, ID{13}, ID{14}, ID{15}};
        static_assert(values.size() == ids.size());

        Issues issues;
        rules::apply(rules::is_enum<TestEnum>, values, ids, "test", "status", issues);

        REQUIRE(issues.size() == 1);
        CHECK(issues.front().rule == RuleCode::not_valid_enum);
        CHECK(issues.front().ids == std::vector<ID>{13, 14, 15});
    }

    SUBCASE("enum reports values other than values allowed by is_valid, excluding missing status if the enum has "
            "na_IntS support") {
        using enum TestEnumWithNan;

        constexpr auto values = std::array{std::to_underlying(foo),
                                           std::to_underlying(bar),
                                           std::to_underlying(nan),
                                           IntS{2},  // invalid
                                           IntS{-1}, // invalid
                                           IntS{na_IntS}};
        constexpr auto ids = std::array{ID{10}, ID{11}, ID{12}, ID{13}, ID{14}, ID{15}};
        static_assert(values.size() == ids.size());

        Issues issues;
        rules::apply(rules::is_enum<TestEnumWithNan>, values, ids, "test", "status", issues);

        REQUIRE(issues.size() == 1);
        CHECK(issues.front().rule == RuleCode::not_valid_enum);
        CHECK(issues.front().ids == std::vector<ID>{13, 14});
    }

    SUBCASE("unique reports values that are not unique") {
        using namespace power_grid_model;
        using namespace power_grid_model::validation;

        constexpr auto is_unique = std::array{true, false, true};
        constexpr auto ids = std::array{ID{1}, ID{2}, ID{3}};
        static_assert(is_unique.size() == ids.size());

        Issues issues;
        rules::apply(rules::Unique{}, is_unique, ids, "test", "id", issues);

        REQUIRE(issues.size() == 1);
        CHECK(issues.front().rule == RuleCode::not_unique);
        CHECK(issues.front().ids == std::vector<ID>{2});
    }

    SUBCASE("not-yet-implemented range-based rule reports NotImplementedError") {
        constexpr auto ids = std::array{ID{1}};
        constexpr auto first_values = std::array{0.0};
        constexpr auto second_values = std::array{0.0};
        static_assert(first_values.size() == ids.size());
        static_assert(second_values.size() == ids.size());

        Issues issues;

        CHECK_THROWS_AS(rules::NotBothZero{}(first_values, second_values, ids, "test", "first", "second", issues),
                        NotImplementedError);
    }
}
