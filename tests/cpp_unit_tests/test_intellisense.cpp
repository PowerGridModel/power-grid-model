// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/test_intellisense.hpp>

#include <doctest/doctest.h>

#include <iostream>

namespace {
using power_grid_model::test_intellisense::base_input_c;
using power_grid_model::test_intellisense::Derived;
using power_grid_model::test_intellisense::derived_input_c;
using power_grid_model::test_intellisense::ID;

void update_base(base_input_c auto& base) { ++base.id; }

void update_derived(derived_input_c auto& derived) {
    ++derived.id;
    derived.u_rated += 1.0;
}
} // namespace

TEST_CASE("Test implicit conversion") {
    Derived derived;
    CHECK(derived.id == 0);

    update_base(derived);
    CHECK(derived.id == 1);

    update_derived(derived);
    CHECK(derived.id == 2);
}
