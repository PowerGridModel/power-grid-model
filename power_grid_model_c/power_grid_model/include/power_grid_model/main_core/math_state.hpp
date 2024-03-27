// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../math_solver/y_bus.hpp"

namespace power_grid_model::main_core {

struct MathState {
    std::vector<YBus<symmetric_t>> y_bus_vec_sym;
    std::vector<YBus<asymmetric_t>> y_bus_vec_asym;
    std::vector<MathSolver<symmetric_t>> math_solvers_sym;
    std::vector<MathSolver<asymmetric_t>> math_solvers_asym;
};

inline void clear(MathState& math_state) {
    math_state.math_solvers_sym.clear();
    math_state.math_solvers_asym.clear();
    math_state.y_bus_vec_sym.clear();
    math_state.y_bus_vec_asym.clear();
}

} // namespace power_grid_model::main_core
