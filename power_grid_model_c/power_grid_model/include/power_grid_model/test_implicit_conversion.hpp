// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_EXCEPTION_HPP
#define POWER_GRID_MODEL_EXCEPTION_HPP

namespace power_grid_model::test_class {
using ID = int;

struct Base {
    ID id{};
};

struct Derived {
    ID id{};
    double u_rated{};

    inline operator Base&() { return *reinterpret_cast<Base*>(this); }
    inline operator Base const&() const { return *reinterpret_cast<Base const*>(this); }
};
} // namespace power_grid_model::test_class

#endif
