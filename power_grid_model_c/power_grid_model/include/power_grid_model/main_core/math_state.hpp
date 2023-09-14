// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MAIN_CORE_MATH_STATE_HPP
#define POWER_GRID_MODEL_MAIN_CORE_MATH_STATE_HPP

#include "../math_solver/y_bus.hpp"

namespace power_grid_model::main_core {

struct MathState {
    std::vector<YBus<true>> y_bus_vec_sym;
    std::vector<YBus<false>> y_bus_vec_asym;
    std::vector<MathSolver<true>> math_solvers_sym;
    std::vector<MathSolver<false>> math_solvers_asym;
};

inline void clear(MathState& math_state) {
    math_state.math_solvers_sym.clear();
    math_state.math_solvers_asym.clear();
    math_state.y_bus_vec_sym.clear();
    math_state.y_bus_vec_asym.clear();
}

} // namespace power_grid_model::main_core

#endif
