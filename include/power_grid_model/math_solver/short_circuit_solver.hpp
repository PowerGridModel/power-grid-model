// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MATH_SOLVER_SHORT_CIRCUIT_SOLVER_HPP
#define POWER_GRID_MODEL_MATH_SOLVER_SHORT_CIRCUIT_SOLVER_HPP

#include "../calculation_parameters.hpp"
#include "../enum.hpp"
#include "y_bus.hpp"

namespace power_grid_model {

// hide implementation in inside namespace
namespace math_model_impl {

// solver
template <bool sym>
class ShortCircuitSolver {
   public:
    ShortCircuitSolver(YBus<sym> const& y_bus) {
    }

    ShortCircuitMathOutput<sym> run_short_circuit(ShortCircuitType short_circuit_type,
                                                  ShortCircuitPhases short_circuit_phases) {
        // check combination of sym and fault type AND type to phases, should match!

        // prepare matrix + rhs

        // solve matrix

        // post processing
    }
};

}  // namespace math_model_impl

}  // namespace power_grid_model

#endif