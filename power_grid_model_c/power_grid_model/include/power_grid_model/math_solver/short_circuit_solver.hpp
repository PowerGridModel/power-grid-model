// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MATH_SOLVER_SHORT_CIRCUIT_SOLVER_HPP
#define POWER_GRID_MODEL_MATH_SOLVER_SHORT_CIRCUIT_SOLVER_HPP

#include "sparse_lu_solver.hpp"
#include "y_bus.hpp"

#include "../calculation_parameters.hpp"
#include "../enum.hpp"
#include "../exception.hpp"

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
          fault_bus_indptr_{topo_ptr, &topo_ptr->fault_bus_indptr},
          mat_data_(y_bus.nnz_lu()),
          sparse_solver_{y_bus.shared_indptr_lu(), y_bus.shared_indices_lu(), y_bus.shared_diag_lu()},
          perm_(n_bus_) {
    }

    ShortCircuitMathOutput<sym> run_short_circuit(double source_voltage_ref, YBus<sym> const& y_bus,
                                                  ShortCircuitInput const& input) {
        // For one calculation all faults should be of the same type and have the same phase
        assert_all_fault_type_phase_equal_(input.faults);
        const FaultPhase fault_phase = input.faults[0].fault_phase;
        const FaultType fault_type = input.faults[0].fault_type;
        // set phase 1 and 2 index for single and two phase faults
        int phase_1{-1};
        int phase_2{-1};
        set_phase_index_(phase_1, phase_2, fault_phase);

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
        IdxVector const& source_bus_indptr = *source_bus_indptr_;
        IdxVector const& fault_bus_indptr = *fault_bus_indptr_;
        // loop through all buses
        for (Idx bus_number = 0; bus_number != n_bus_; ++bus_number) {
            Idx const diagonal_position = bus_entry[bus_number];
            // add all sources
            for (Idx source_number = source_bus_indptr[bus_number]; source_number != source_bus_indptr[bus_number + 1];
                 ++source_number) {
                ComplexTensor<sym> y_source = y_bus.math_model_param().source_param[source_number];
                mat_data_[diagonal_position] += y_source;  // add y_source to the diagonal of Ybus
                rhs[bus_number] +=
                    y_source * input.source[source_number] * source_voltage_ref;  // Y_source * U_source * c
            }
            // add all faults
            for (Idx fault_number = fault_bus_indptr[bus_number]; fault_number != fault_bus_indptr[bus_number + 1];
                 ++fault_number) {
                DoubleComplex y_fault = input.faults[fault_number].y_fault;
                if (std::isinf(y_fault.real())) {
                    assert(std::isinf(y_fault.imag());
                    zero_fault_counter[bus_number] += 1;
                    if constexpr (sym) {  // three phase fault
                        for (Idx data_index = y_bus.row_indptr_lu()[bus_number];
                             data_index != y_bus.row_indptr_lu()[bus_number + 1]; ++data_index) {
                            Idx row_number = y_bus.col_indices_lu()[data_index];
                            Idx col_data_index = y_bus.lu_transpose_entry()[data_index];
                            // mat_data[:,bus] = 0
                            // mat_data[bus,bus] = -1
                            if (row_number != bus_number) {
                                mat_data[col_data_index] = 0;
                            }
                            else {
                                mat_data_[col_data_index] = -1;
                            }
                        }
                        rhs[bus_number] = 0;
                    }
                    else if (fault_type == FaultType::single_phase_to_ground) {
                        for (Idx data_index = y_bus.row_indptr_lu()[bus_number];
                             data_index != y_bus.row_indptr_lu()[bus_number + 1]; ++data_index) {
                            Idx row_number = y_bus.col_indices_lu()[data_index];
                            Idx col_data_index = y_bus.lu_transpose_entry()[data_index];
                            // mat_data[:,bus][:, phase_1] = 0
                            // mat_data[bus,bus][phase_1, phase_1] = -1
                            mat_data[col_data_index].col(phase_1) = 0;
                            if (row_number == bus_number) {
                                mat_data_[col_data_index](phase_1, phase_1) = -1;
                            }
                        }
                        rhs[bus_number](phase_1) = 0;
                    }
                    else if (fault_type == FaultType::two_phase) {
                        for (Idx data_index = y_bus.row_indptr_lu()[bus_number];
                             data_index != y_bus.row_indptr_lu()[bus_number + 1]; ++data_index) {
                            Idx row_number = y_bus.col_indices_lu()[data_index];
                            Idx col_data_index = y_bus.lu_transpose_entry()[data_index];
                            // mat_data[:,bus][:, phase_1] += mat_data[:,bus][:, phase_2]
                            // mat_data[:,bus][:, phase_2] = 0
                            // mat_data[bus,bus][phase_1, phase_2] = -1
                            // mat_data[bus,bus][phase_2, phase_2] = 1
                            mat_data[col_data_index].col(phase_1) += mat_data[col_data_index].col(phase_1);
                            mat_data[col_data_index].col(phase_2) = 0;
                            if (row_number == bus_number) {
                                mat_data_[col_data_index](phase_1, phase_2) = -1;
                                mat_data_[col_data_index](phase_2, phase_2) = 1;
                            }
                        }
                        rhs[bus_number](phase_2) += rhs[bus_number](phase_1);
                        rhs[bus_number](phase_1) = 0;
                    }
                    else {
                        assert((fault_type == FaultType::two_phase_to_ground));
                        for (Idx data_index = y_bus.row_indptr_lu()[bus_number];
                             data_index != y_bus.row_indptr_lu()[bus_number + 1]; ++data_index) {
                            Idx row_number = y_bus.col_indices_lu()[data_index];
                            Idx col_data_index = y_bus.lu_transpose_entry()[data_index];
                            // mat_data[:,bus][:, phase_1] = 0
                            // mat_data[:,bus][:, phase_2] = 0
                            // mat_data[bus,bus][phase_1, phase_1] = -1
                            // mat_data[bus,bus][phase_2, phase_2] = -1
                            mat_data[col_data_index].col(phase_1) = 0;
                            mat_data[col_data_index].col(phase_2) = 0;
                            if (row_number == bus_number) {
                                mat_data_[col_data_index](phase_1, phase_1) = -1;
                                mat_data_[col_data_index](phase_2, phase_2) = -1;
                            }
                        }
                        rhs[bus_number](phase_1) = 0;
                        rhs[bus_number](phase_2) = 0;
                    }
                    // If there is a fault with infinite admittance, there is no need to add other faults to that bus
                    break;
                }
                else {
                    assert(!std::isinf(y_fault.imag());
                    if constexpr (sym) {  // three phase fault
                        for (Idx data_index = y_bus.row_indptr_lu()[bus_number];
                             data_index != y_bus.row_indptr_lu()[bus_number + 1]; ++data_index) {
                            Idx row_number = y_bus.col_indices_lu()[data_index];
                            Idx col_data_index = y_bus.lu_transpose_entry()[data_index];
                            // mat_data[bus,bus] += y_fault
                            if (row_number == bus_number) {
                                mat_data[col_data_index] += y_fault;
                            }
                        }
                    }
                    else if (fault_type == FaultType::single_phase_to_ground) {
                        for (Idx data_index = y_bus.row_indptr_lu()[bus_number];
                             data_index != y_bus.row_indptr_lu()[bus_number + 1]; ++data_index) {
                            Idx row_number = y_bus.col_indices_lu()[data_index];
                            Idx col_data_index = y_bus.lu_transpose_entry()[data_index];
                            // mat_data[bus,bus][phase_1, phase_1] += y_fault
                            if (row_number == bus_number) {
                                mat_data_[col_data_index](phase_1, phase_1) += y_fault;
                            }
                        }
                    }
                    else if (fault_type == FaultType::two_phase) {
                        for (Idx data_index = y_bus.row_indptr_lu()[bus_number];
                             data_index != y_bus.row_indptr_lu()[bus_number + 1]; ++data_index) {
                            Idx row_number = y_bus.col_indices_lu()[data_index];
                            Idx col_data_index = y_bus.lu_transpose_entry()[data_index];
                            // mat_data[bus,bus][phase_1, phase_1] += y_fault
                            // mat_data[bus,bus][phase_2, phase_2] += y_fault
                            // mat_data[bus,bus][phase_1, phase_2] -= y_fault
                            // mat_data[bus,bus][phase_2, phase_1] -= y_fault
                            if (row_number == bus_number) {
                                mat_data_[col_data_index](phase_1, phase_1) += y_fault;
                                mat_data_[col_data_index](phase_2, phase_2) += y_fault;
                                mat_data_[col_data_index](phase_1, phase_2) -= y_fault;
                                mat_data_[col_data_index](phase_2, phase_1) -= y_fault;
                            }
                        }
                    }
                    else {
                        assert((fault_type == FaultType::two_phase_to_ground));
                        for (Idx data_index = y_bus.row_indptr_lu()[bus_number];
                             data_index != y_bus.row_indptr_lu()[bus_number + 1]; ++data_index) {
                            Idx row_number = y_bus.col_indices_lu()[data_index];
                            Idx col_data_index = y_bus.lu_transpose_entry()[data_index];
                            // mat_data[bus,bus][phase_1, phase_1] += 2 * y_fault
                            // mat_data[bus,bus][phase_2, phase_2] += 2 * y_fault
                            // mat_data[bus,bus][phase_1, phase_2] = -= y_fault
                            // mat_data[bus,bus][phase_2, phase_1] = -= y_fault
                            if (row_number == bus_number) {
                                mat_data_[col_data_index](phase_1, phase_1) += 2 * y_fault;
                                mat_data_[col_data_index](phase_2, phase_2) += 2 * y_fault;
                                mat_data_[col_data_index](phase_1, phase_2) -= y_fault;
                                mat_data_[col_data_index](phase_2, phase_1) -= y_fault;
                            }
                        }
                    }
                }
            }
        }

        // solve matrix

        // post processing
    }

   private:
    Idx n_bus_;
    Idx n_fault_;
    // shared topo data
    std::shared_ptr<IdxVector const> source_bus_indptr_;
    std::shared_ptr<IdxVector const> fault_bus_indptr_;
    // sparse linear equation
    ComplexTensorVector<sym> mat_data_;
    // sparse solver
    SparseLUSolver<ComplexTensor<sym>, ComplexValue<sym>, ComplexValue<sym>> sparse_solver_;
    typename SparseLUSolver<ComplexTensor<sym>, ComplexValue<sym>, ComplexValue<sym>>::BlockPermArray perm_;

    void set_phase_index_(double& phase_1, double& phase_2, FaultPhase fault_phase) {
        // This function updates the phase index for single and two phase faults
        if (fault_phase == FaultPhase::a) {
            phase_1 = 0;
        }
        else if (fault_phase == FaultPhase::b) {
            phase_1 = 1;
        }
        else if (fault_phase == FaultPhase::c) {
            phase_1 = 2;
        }
        else if (fault_phase == FaultPhase::ab) {
            phase_1 = 0;
            phase_2 = 1;
        }
        else if (fault_phase == FaultPhase::ac) {
            phase_1 = 0;
            phase_2 = 2;
        }
        else if (fault_phase == FaultPhase::bc) {
            phase_1 = 1;
            phase_2 = 2;
        }
    }

    void assert_all_fault_type_phase_equal_(const std::vector<FaultCalcParam>& vec) {
        assert(!vec.empty());  // Assert that the vector is not empty

        const FaultPhase phase = vec[0].fault_phase;
        const FaultType type = vec[0].fault_type;
        for (size_t i = 1; i < vec.size(); ++i) {
            assert(vec[i].fault_phase == phase);  // Assert that each phase is equal to the first phase
            assert(vec[i].fault_type == type);    // Assert that each type is equal to the first type
        }
    }
};

}  // namespace math_model_impl

}  // namespace power_grid_model

#endif