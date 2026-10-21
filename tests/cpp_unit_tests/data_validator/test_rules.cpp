// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "power_grid_model/common/common.hpp"
#include "power_grid_model/validation/issues.hpp"
#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/validation/rules.hpp>

#include <doctest/doctest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

TEST_CASE("Required reports missing floating, ID, and status values") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;

    std::array<ID, 2> const ids{1, 2};
    Issues issues;

    std::array<double, 2> const floating_values{power_grid_model::nan, 1.0};
    rules::apply(rules::Required{}, floating_values, ids, "test", "floating", issues);

    std::array<ID, 2> const id_values{na_IntID, 4};
    rules::apply(rules::Required{}, id_values, ids, "test", "id", issues);

    std::array<IntS, 2> const status_values{na_IntS, 1};
    rules::apply(rules::Required{}, status_values, ids, "test", "status", issues);

    std::array<std::uint32_t, 2> const other_values{0, 1};
    rules::apply(rules::Required{}, other_values, ids, "test", "other", issues);

    REQUIRE(issues.size() == 3);
    CHECK(issues[0].rule == "missing_value");
    CHECK(issues[0].field == "floating");
    CHECK(issues[0].ids == std::vector<ID>{1});
    CHECK(issues[1].field == "id");
    CHECK(issues[1].ids == std::vector<ID>{1});
    CHECK(issues[2].field == "status");
    CHECK(issues[2].ids == std::vector<ID>{1});
}

TEST_CASE("Required reports nothing for empty ranges and accepts standard containers") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;

    std::array<double, 0> const no_values{};
    std::array<ID, 0> const no_ids{};
    Issues issues;
    rules::apply(rules::Required{}, no_values, no_ids, "test", "value", issues);
    CHECK(issues.empty());

    std::vector<double> const values{2.0, power_grid_model::nan, 3.0};
    std::vector<ID> const ids{7, 8, 9};
    rules::apply(rules::Required{}, values, ids, "test", "value", issues);

    REQUIRE(issues.size() == 1);
    CHECK(issues.front().ids == std::vector<ID>{8});
}

TEST_CASE("Required operates on strided attribute views") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;

    struct Row {
        ID id;
        double value;
    };
    std::array<Row, 2> const rows{{{.id = 10, .value = power_grid_model::nan}, {.id = 11, .value = 2.0}}};
    meta_data::AttributeView<ID> const ids{reinterpret_cast<char const*>(rows.data()), rows.size(), sizeof(Row),
                                           offsetof(Row, id), na_IntID};
    meta_data::AttributeView<double> const values{reinterpret_cast<char const*>(rows.data()), rows.size(), sizeof(Row),
                                                  offsetof(Row, value), power_grid_model::nan};
    Issues issues;

    rules::apply(rules::Required{}, values, ids, "test", "value", issues);

    REQUIRE(issues.size() == 1);
    CHECK(issues.front().ids == std::vector<ID>{10});
}

TEST_CASE("Finite reports both infinities but not NaN or finite values") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;

    std::array<double, 4> const values{std::numeric_limits<double>::infinity(),
                                       -std::numeric_limits<double>::infinity(), power_grid_model::nan, 5.0};
    std::array<ID, 4> const ids{1, 2, 3, 4};
    Issues issues;

    rules::apply(rules::Finite{}, values, ids, "test", "value", issues);

    REQUIRE(issues.size() == 1);
    CHECK(issues.front().rule == "infinity");
    CHECK(issues.front().ids == std::vector<ID>{1, 2});
}

TEST_CASE("Greater-than-zero reports zero and negative values, but not NaN or infinity") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;

    std::array<double, 5> const values{0.0, -0.0, -1.0, power_grid_model::nan, std::numeric_limits<double>::infinity()};
    std::array<ID, 5> const ids{1, 2, 3, 4, 5};
    Issues issues;

    rules::apply(rules::GreaterThanZero{}, values, ids, "test", "value", issues);

    REQUIRE(issues.size() == 1);
    CHECK(issues.front().rule == "not_greater_than_zero");
    CHECK(issues.front().ids == std::vector<ID>{1, 2, 3});
}

TEST_CASE("Boolean reports values other than zero and one, including missing status") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;

    std::array<IntS, 5> const values{0, 1, 2, -1, na_IntS};
    std::array<ID, 5> const ids{10, 11, 12, 13, 14};
    Issues issues;

    rules::apply(rules::Boolean{}, values, ids, "test", "status", issues);

    REQUIRE(issues.size() == 1);
    CHECK(issues.front().rule == "not_boolean");
    CHECK(issues.front().ids == std::vector<ID>{12, 13, 14});
}

TEST_CASE("Not-yet-implemented range-based rules report NotImplementedError") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;

    std::array<ID, 1> const ids{1};
    std::array<double, 1> const first_values{0.0};
    std::array<double, 1> const second_values{0.0};
    Issues issues;

    CHECK_THROWS_AS(rules::NotBothZero{}(first_values, second_values, ids, "test", "first", "second", issues),
                    NotImplementedError);
}
