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
    ShortCircuitSolver(YBus<sym> const& y_bus, std::shared_ptr<MathModelTopology const> const& topo_ptr)
        : n_bus_{y_bus.size()},
          n_fault_{topo_ptr->n_fault()},
          source_bus_indptr_{topo_ptr, &topo_ptr->source_bus_indptr},
          mat_data_(y_bus.nnz_lu()) {
    }

    ShortCircuitMathOutput<sym> run_short_circuit(ShortCircuitType short_circuit_type,
                                                  ShortCircuitPhases short_circuit_phases, YBus<sym> const& y_bus,
                                                  ShortCircuitInput const& input) {
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
        ComplexValueVector<sym> rhs(n_bus_){};
        IdxVector zero_fault_counter(n_bus_){};
        ComplexValueVector<sym> i_fault(n_fault_){};
        // loop through all sources and faults to update y_bus
        IdxVector const& source_bus_indptr = *source_bus_indptr_;
        for (Idx bus_number = 0; bus_number != n_bus_; ++bus_number) {
            Idx const data_sequence = bus_entry[bus_number];
            for (Idx source_number = source_bus_indptr[bus_number]; source_number != source_bus_indptr[bus_number + 1];
                 ++source_number) {
                // TODO: constants[bus] += y_source * U_source * c
                ComplexTensor<sym> y_source = y_bus.math_model_param().source_param[source_number];
                mat_data_[data_sequence] += y_source;
                rhs[bus_number] += y_source * input.source[source_number] * c;  // Y_source * U_source * c
                // TODO define u_source and c
            }
        }

        // solve matrix

        // post processing

        // TODO use Timer class??
    }

   private:
    Idx n_bus_;
    Idx n_fault_;
    // shared topo data
    std::shared_ptr<IdxVector const> source_bus_indptr_;
    // sparse linear equation
    ComplexTensorVector<sym> mat_data_;
};

}  // namespace math_model_impl

}  // namespace power_grid_model

#endif