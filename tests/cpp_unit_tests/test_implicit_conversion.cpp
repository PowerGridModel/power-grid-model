// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/test_implicit_conversion.hpp>

#include <doctest/doctest.h>

#include <iostream>

namespace {
using power_grid_model::test_class::Base;
using power_grid_model::test_class::Derived;
using power_grid_model::test_class::ID;
} // namespace

TEST_CASE("Test implicit conversion - const") {
    Base const base;
    Derived const derived;
    Base const& base_ref = derived;
    CHECK(base.id == derived.id);
    CHECK(base.id == base_ref.id);
}

TEST_CASE("Test implicit conversion - mutable") {
    Derived derived{0};
    Base& base_ref = derived;
    CHECK(derived.id == 0);
    CHECK(derived.id == base_ref.id);

    derived.id = 1;
    CHECK(derived.id == 1);
    CHECK(derived.id == base_ref.id);

    base_ref.id = 1;
    CHECK(derived.id == 1);
    CHECK(derived.id == base_ref.id);
}
