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
    ShortCircuitSolver(YBus<sym> const& y_bus) : mat_data_(y_bus.nnz_lu()) {
    }

    ShortCircuitMathOutput<sym> run_short_circuit(ShortCircuitType short_circuit_type,
                                                  ShortCircuitPhases short_circuit_phases, YBus<sym> const& y_bus) {
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
        // the number of phases in short_circuit_type should match the phases specified in short_circuit_phases
        if (short_circuit_type == ShortCircuitType::three_phase) {
            if (short_circuit_phases != ShortCircuitPhases::abc)
                throw InvalidShortCircuitPhases(short_circuit_type, short_circuit_phases);
        }
        else {
            if (short_circuit_phases == ShortCircuitPhases::abc) {
                throw InvalidShortCircuitPhases(short_circuit_type, short_circuit_phases);
            }
        }

        // getter
        ComplexTensorVector<sym> const& ydata = y_bus.admittance();
        IdxVector const& bus_entry = y_bus.lu_diag();
        // output
        ShortCircuitMathOutput<sym> output;  // TODO: resize output values that are updated

        // copy y_bus data
        std::transform(y_bus.map_lu_y_bus().cbegin(), y_bus.map_lu_y_bus.cend(), mat_data_.begin(), [&](Idx k) {
            if (k == -1) {
                return ComplexTensor<sym>{};
            }
            else {
                return ydata[k];
            }
        });

        // prepare matrix + rhs

        // solve matrix

        // post processing

        // TODO use Timer class??
    }

   private:
    // sparse linear equation
    ComplexTensorVector<sym> mat_data_;
};

}  // namespace math_model_impl

}  // namespace power_grid_model

#endif