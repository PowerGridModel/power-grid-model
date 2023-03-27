// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MATH_SOLVER_SHORT_CIRCUIT_SOLVER_HPP
#define POWER_GRID_MODEL_MATH_SOLVER_SHORT_CIRCUIT_SOLVER_HPP

#include "../calculation_parameters.hpp"
#include "../enum.hpp"
#include "../exception.hpp"
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
        // TODO: put the (a)sym checks below in separate (private) function
        // calculation type (sym/asym) should match the short circuit type (sym/asym)
        if constexpr (sym) {
            if (short_circuit_type != ShortCircuitType::three_phase) {
                throw InvalidShortCircuitType(sym, short_circuit_type)
            }
        }
        else {
            if (short_circuit_type == ShortCircuitType::three_phase) {
                throw InvalidShortCircuitType(sym, short_circuit_type)
            }
        }
        if (short_circuit_type == ShortCircuitType::three_phase) {
            if (short_circuit_phases != ShortCircuitPhases::abc)
                throw InvalidShortCircuitPhases(short_circuit_type, short_circuit_phases);
        }
        else {
            if (short_circuit_phases == ShortCircuitPhases::abc) {
                throw InvalidShortCircuitPhases(short_circuit_type, short_circuit_phases);
            }
        }
        // the number of phases in short_circuit_type should match the phases specified in short_circuit_phases

        // prepare matrix + rhs

        // solve matrix

        // post processing
    }
};

}  // namespace math_model_impl

}  // namespace power_grid_model

#endif