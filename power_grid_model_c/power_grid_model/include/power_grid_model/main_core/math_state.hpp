// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MAIN_CORE_MATH_STATE_HPP
#define POWER_GRID_MODEL_MAIN_CORE_MATH_STATE_HPP

#include "../math_solver/y_bus.hpp"

namespace power_grid_model::main_core {

template <bool sym> struct MathState {

    std::vector<std::shared_ptr<YBus<sym>>> y_bus_vec;
    std::vector<MathSolver<sym>> math_solvers;
};

} // namespace power_grid_model::main_core

#endif