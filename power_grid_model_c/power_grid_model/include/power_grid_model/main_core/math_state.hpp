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

template <symmetry_tag sym>
inline void update_y_bus(MathState& math_state, std::vector<MathModelParam<sym>> const& math_model_params) {
    auto& y_bus_vec = [&math_state]() -> auto& {
        if constexpr (is_symmetric_v<sym>) {
            return math_state.y_bus_vec_sym;
        } else {
            return math_state.y_bus_vec_asym;
        }
    }
    ();

    assert(y_bus_vec.size() == math_model_params.size());

    for (Idx i = 0; i != static_cast<Idx>(y_bus_vec.size()); ++i) {
        y_bus_vec[i].update_admittance(std::make_shared<MathModelParam<sym> const>(std::move(math_model_params[i])));
    }
}

template <symmetry_tag sym>
inline void update_y_bus(MathState& math_state, std::vector<MathModelParam<sym>> const& math_model_params,
                         std::vector<MathModelParamIncrement> const& math_model_param_increments) {
    auto& y_bus_vec = [&math_state]() -> auto& {
        if constexpr (is_symmetric_v<sym>) {
            return math_state.y_bus_vec_sym;
        } else {
            return math_state.y_bus_vec_asym;
        }
    }
    ();

    assert(y_bus_vec.size() == math_model_params.size());

    for (Idx i = 0; i != static_cast<Idx>(y_bus_vec.size()); ++i) {
        y_bus_vec[i].update_admittance_increment(
            std::make_shared<MathModelParam<sym> const>(std::move(math_model_params[i])),
            math_model_param_increments[i]);
    }
}

} // namespace power_grid_model::main_core
