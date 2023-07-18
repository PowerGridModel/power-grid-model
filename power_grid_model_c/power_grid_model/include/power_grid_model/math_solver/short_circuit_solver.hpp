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
          n_source_{topo_ptr->n_source()},
          source_bus_indptr_{topo_ptr, &topo_ptr->source_bus_indptr},
          fault_bus_indptr_{topo_ptr, &topo_ptr->fault_bus_indptr},
          mat_data_(y_bus.nnz_lu()),
          sparse_solver_{y_bus.shared_indptr_lu(), y_bus.shared_indices_lu(), y_bus.shared_diag_lu()},
          perm_(n_bus_) {
    }

    ShortCircuitMathOutput<sym> run_short_circuit(double source_voltage_ref, YBus<sym> const& y_bus,
                                                  ShortCircuitInput const& input) {
        // For one calculation all faults should be of the same type and have the same phase
        if (!all_fault_type_phase_equal(input.faults)) {
            throw InvalidShortCircuitPhaseOrType{};
        }
        FaultPhase const fault_phase =
            input.faults.empty() ? FaultPhase::default_value : input.faults.front().fault_phase;
        FaultType const fault_type = input.faults.empty() ? FaultType::nan : input.faults.front().fault_type;
        // set phase 1 and 2 index for single and two phase faults
        auto const [phase_1, phase_2] = set_phase_index(fault_phase);

        // getter
        ComplexTensorVector<sym> const& ydata = y_bus.admittance();
        IdxVector const& bus_entry = y_bus.lu_diag();
        // output
        ShortCircuitMathOutput<sym> output;
        output.u_bus.resize(n_bus_);
        output.i_fault.resize(n_fault_);
        output.i_source.resize(n_source_);

        // copy y_bus data
        std::transform(y_bus.map_lu_y_bus().cbegin(), y_bus.map_lu_y_bus().cend(), mat_data_.begin(), [&](Idx k) {
            if (k == -1) {
                return ComplexTensor<sym>{};
            }
            else {
                return ydata[k];
            }
        });

        // prepare matrix + rhs
        IdxVector infinite_admittance_fault_counter(n_bus_);
        IdxVector const& source_bus_indptr = *source_bus_indptr_;
        IdxVector const& fault_bus_indptr = *fault_bus_indptr_;
        // loop through all buses
        for (Idx bus_number = 0; bus_number != n_bus_; ++bus_number) {
            Idx const diagonal_position = bus_entry[bus_number];
            // add all sources
            for (Idx source_number = source_bus_indptr[bus_number]; source_number != source_bus_indptr[bus_number + 1];
                 ++source_number) {
                ComplexTensor<sym> const y_source = y_bus.math_model_param().source_param[source_number];
                mat_data_[diagonal_position] += y_source;  // add y_source to the diagonal of Ybus
                output.u_bus[bus_number] +=
                    dot(y_source, ComplexValue<sym>{input.source[source_number] *
                                                    source_voltage_ref});  // rhs += Y_source * U_source * c
            }
            // add all faults
            for (Idx fault_number = fault_bus_indptr[bus_number]; fault_number != fault_bus_indptr[bus_number + 1];
                 ++fault_number) {
                DoubleComplex const y_fault = input.faults[fault_number].y_fault;
                if (std::isinf(y_fault.real())) {
                    assert(std::isinf(y_fault.imag()));
                    infinite_admittance_fault_counter[bus_number] += 1;
                    if (fault_type == FaultType::three_phase) {  // three phase fault
                        for (Idx data_index = y_bus.row_indptr_lu()[bus_number];
                             data_index != y_bus.row_indptr_lu()[bus_number + 1]; ++data_index) {
                            Idx const col_data_index = y_bus.lu_transpose_entry()[data_index];
                            // mat_data[:,bus] = 0
                            mat_data_[col_data_index] = (ComplexTensor<sym>)(0);
                        }
                        // mat_data[bus,bus] = -1
                        mat_data_[diagonal_position] = (ComplexTensor<sym>)(-1);
                        output.u_bus[bus_number] = (ComplexValue<sym>)(0);  // update rhs
                    }
                    if constexpr (!sym) {
                        if (fault_type == FaultType::single_phase_to_ground) {
                            for (Idx data_index = y_bus.row_indptr_lu()[bus_number];
                                 data_index != y_bus.row_indptr_lu()[bus_number + 1]; ++data_index) {
                                Idx const col_data_index = y_bus.lu_transpose_entry()[data_index];
                                // mat_data[:,bus][:, phase_1] = 0
                                mat_data_[col_data_index].col(phase_1) = 0;
                            }
                            // mat_data[bus,bus][phase_1, phase_1] = -1
                            mat_data_[diagonal_position](phase_1, phase_1) = -1;
                            output.u_bus[bus_number](phase_1) = 0;  // update rhs
                        }
                        else if (fault_type == FaultType::two_phase) {
                            for (Idx data_index = y_bus.row_indptr_lu()[bus_number];
                                 data_index != y_bus.row_indptr_lu()[bus_number + 1]; ++data_index) {
                                Idx const col_data_index = y_bus.lu_transpose_entry()[data_index];
                                // mat_data[:,bus][:, phase_1] += mat_data[:,bus][:, phase_2]
                                // mat_data[:,bus][:, phase_2] = 0
                                mat_data_[col_data_index].col(phase_1) += mat_data_[col_data_index].col(phase_2);
                                mat_data_[col_data_index].col(phase_2) = 0;
                            }
                            // mat_data[bus,bus][phase_1, phase_2] = -1
                            // mat_data[bus,bus][phase_2, phase_2] = 1
                            mat_data_[diagonal_position](phase_1, phase_2) = -1;
                            mat_data_[diagonal_position](phase_2, phase_2) = 1;
                            // update rhs
                            output.u_bus[bus_number](phase_2) += output.u_bus[bus_number](phase_1);
                            output.u_bus[bus_number](phase_1) = 0;
                        }
                        else if (fault_type == FaultType::two_phase_to_ground) {
                            for (Idx data_index = y_bus.row_indptr_lu()[bus_number];
                                 data_index != y_bus.row_indptr_lu()[bus_number + 1]; ++data_index) {
                                Idx const col_data_index = y_bus.lu_transpose_entry()[data_index];
                                // mat_data[:,bus][:, phase_1] = 0
                                // mat_data[:,bus][:, phase_2] = 0
                                mat_data_[col_data_index].col(phase_1) = 0;
                                mat_data_[col_data_index].col(phase_2) = 0;
                            }
                            // mat_data[bus,bus][phase_1, phase_1] = -1
                            // mat_data[bus,bus][phase_2, phase_2] = -1
                            mat_data_[diagonal_position](phase_1, phase_1) = -1;
                            mat_data_[diagonal_position](phase_2, phase_2) = -1;
                            // update rhs
                            output.u_bus[bus_number](phase_1) = 0;
                            output.u_bus[bus_number](phase_2) = 0;
                        }
                        else {
                            assert((fault_type == FaultType::three_phase));
                            continue;
                        }
                    }
                    // If there is a fault with infinite admittance, there is no need to add other faults to that bus
                    break;
                }
                else {
                    assert(!std::isinf(y_fault.imag()));
                    if (fault_type == FaultType::three_phase) {  // three phase fault
                        // mat_data[bus,bus] += y_fault
                        mat_data_[diagonal_position] += (ComplexTensor<sym>)(y_fault);
                    }
                    if constexpr (!sym) {
                        if (fault_type == FaultType::single_phase_to_ground) {
                            // mat_data[bus,bus][phase_1, phase_1] += y_fault
                            mat_data_[diagonal_position](phase_1, phase_1) += y_fault;
                        }
                        else if (fault_type == FaultType::two_phase) {
                            // mat_data[bus,bus][phase_1, phase_1] += y_fault
                            // mat_data[bus,bus][phase_2, phase_2] += y_fault
                            // mat_data[bus,bus][phase_1, phase_2] -= y_fault
                            // mat_data[bus,bus][phase_2, phase_1] -= y_fault
                            mat_data_[diagonal_position](phase_1, phase_1) += y_fault;
                            mat_data_[diagonal_position](phase_2, phase_2) += y_fault;
                            mat_data_[diagonal_position](phase_1, phase_2) -= y_fault;
                            mat_data_[diagonal_position](phase_2, phase_1) -= y_fault;
                        }
                        else if (fault_type == FaultType::two_phase_to_ground) {
                            // mat_data[bus,bus][phase_1, phase_1] += 2 * y_fault
                            // mat_data[bus,bus][phase_2, phase_2] += 2 * y_fault
                            // mat_data[bus,bus][phase_1, phase_2] = -= y_fault
                            // mat_data[bus,bus][phase_2, phase_1] = -= y_fault
                            mat_data_[diagonal_position](phase_1, phase_1) += 2.0 * y_fault;
                            mat_data_[diagonal_position](phase_2, phase_2) += 2.0 * y_fault;
                            mat_data_[diagonal_position](phase_1, phase_2) -= y_fault;
                            mat_data_[diagonal_position](phase_2, phase_1) -= y_fault;
                        }
                        else {
                            assert((fault_type == FaultType::three_phase));
                            continue;
                        }
                    }
                }
            }
        }

        // solve matrix
        sparse_solver_.prefactorize_and_solve(mat_data_, perm_, output.u_bus, output.u_bus);

        // post processing
        calculate_result(y_bus, input, output, infinite_admittance_fault_counter, fault_type, phase_1, phase_2,
                         source_voltage_ref);

        return output;
    }

   private:
    Idx n_bus_;
    Idx n_fault_;
    Idx n_source_;
    // shared topo data
    std::shared_ptr<IdxVector const> source_bus_indptr_;
    std::shared_ptr<IdxVector const> fault_bus_indptr_;
    // sparse linear equation
    ComplexTensorVector<sym> mat_data_;
    // sparse solver
    SparseLUSolver<ComplexTensor<sym>, ComplexValue<sym>, ComplexValue<sym>> sparse_solver_;
    typename SparseLUSolver<ComplexTensor<sym>, ComplexValue<sym>, ComplexValue<sym>>::BlockPermArray perm_;

    void calculate_result(YBus<sym> const& y_bus, ShortCircuitInput const& input, ShortCircuitMathOutput<sym>& output,
                          IdxVector const& infinite_admittance_fault_counter, FaultType const fault_type,
                          int const& phase_1, int const& phase_2, double const& source_voltage_ref) {
        // loop through all buses
        for (Idx bus_number = 0; bus_number != n_bus_; ++bus_number) {
            ComplexValue<sym> const x_tmp = output.u_bus[bus_number];  // save x to temp variable
            double const infinite_admittance_fault_counter_bus =
                static_cast<double>(infinite_admittance_fault_counter[bus_number]);
            for (Idx fault_number = (*fault_bus_indptr_)[bus_number];
                 fault_number != (*fault_bus_indptr_)[bus_number + 1]; ++fault_number) {
                DoubleComplex const y_fault = input.faults[fault_number].y_fault;
                if (std::isinf(y_fault.real())) {
                    assert(std::isinf(y_fault.imag()));
                    if (fault_type == FaultType::three_phase) {  // three phase fault
                        output.i_fault[fault_number] =
                            (ComplexValue<sym>)(-1.0 * x_tmp /
                                                infinite_admittance_fault_counter_bus);  // injection is negative to
                                                                                         // fault
                        output.u_bus[bus_number] = (ComplexValue<sym>)(0.0);
                    }
                    if constexpr (!sym) {
                        if (fault_type == FaultType::single_phase_to_ground) {
                            output.i_fault[fault_number](phase_1) =
                                -1.0 * x_tmp[phase_1] / infinite_admittance_fault_counter_bus;
                            output.u_bus[bus_number](phase_1) = 0.0;
                        }
                        else if (fault_type == FaultType::two_phase) {
                            output.i_fault[fault_number](phase_1) =
                                -1.0 * x_tmp[phase_2] / infinite_admittance_fault_counter_bus;
                            output.i_fault[fault_number](phase_2) = -1.0 * output.i_fault[fault_number](phase_1);
                            output.u_bus[bus_number](phase_2) = output.u_bus[bus_number](phase_1);
                        }
                        else if (fault_type == FaultType::two_phase_to_ground) {
                            output.i_fault[fault_number](phase_1) =
                                -1.0 * x_tmp[phase_1] / infinite_admittance_fault_counter_bus;
                            output.i_fault[fault_number](phase_2) =
                                -1.0 * x_tmp[phase_2] / infinite_admittance_fault_counter_bus;
                            output.u_bus[bus_number](phase_1) = 0.0;
                            output.u_bus[bus_number](phase_2) = 0.0;
                        }
                        else {
                            assert((fault_type == FaultType::three_phase));
                            continue;
                        }
                    }
                }
                else {
                    assert(!std::isinf(y_fault.imag()));
                    if (infinite_admittance_fault_counter[bus_number] > 0) {
                        // ignore fault objects with impedance, when there is a fault with infinite admittance on bus
                        continue;
                    }
                    if (fault_type == FaultType::three_phase) {  // three phase fault
                        output.i_fault[fault_number] = (ComplexValue<sym>)(y_fault * x_tmp);
                    }
                    if constexpr (!sym) {
                        if (fault_type == FaultType::single_phase_to_ground) {
                            output.i_fault[fault_number](phase_1) = y_fault * x_tmp[phase_1];
                        }
                        else if (fault_type == FaultType::two_phase) {
                            output.i_fault[fault_number](phase_1) = y_fault * x_tmp[phase_1] - y_fault * x_tmp[phase_2];
                            output.i_fault[fault_number](phase_2) = y_fault * x_tmp[phase_2] - y_fault * x_tmp[phase_1];
                        }
                        else if (fault_type == FaultType::two_phase_to_ground) {
                            output.i_fault[fault_number](phase_1) =
                                2.0 * y_fault * x_tmp[phase_1] - y_fault * x_tmp[phase_2];
                            output.i_fault[fault_number](phase_2) =
                                2.0 * y_fault * x_tmp[phase_2] - y_fault * x_tmp[phase_1];
                        }
                        else {
                            assert((fault_type == FaultType::three_phase));
                            continue;
                        }
                    }
                }
            }
            ComplexValue<sym> i_source_bus{};  // if asym, already initialized to zero
            for (Idx source_number = (*source_bus_indptr_)[bus_number];
                 source_number != (*source_bus_indptr_)[bus_number + 1]; ++source_number) {
                ComplexTensor<sym> const y_source = y_bus.math_model_param().source_param[source_number];
                output.i_source[source_number] = dot(
                    y_source,
                    (ComplexValue<sym>{input.source[source_number] * source_voltage_ref} - output.u_bus[bus_number]));
                i_source_bus += output.i_source[source_number];
            }

            // compensate source current into hard fault
            for (Idx fault_number = (*fault_bus_indptr_)[bus_number];
                 fault_number != (*fault_bus_indptr_)[bus_number + 1]; ++fault_number) {
                DoubleComplex const y_fault = input.faults[fault_number].y_fault;
                if (std::isinf(y_fault.real())) {
                    assert(std::isinf(y_fault.imag()));
                    if (fault_type == FaultType::three_phase) {
                        output.i_fault[fault_number] +=
                            (ComplexValue<sym>)(i_source_bus / infinite_admittance_fault_counter_bus);
                    }
                    if constexpr (!sym) {
                        if (fault_type == FaultType::single_phase_to_ground) {
                            output.i_fault[fault_number](phase_1) +=
                                i_source_bus[phase_1] / infinite_admittance_fault_counter_bus;
                        }
                        else if (fault_type == FaultType::two_phase) {
                            output.i_fault[fault_number](phase_1) -= i_source_bus[phase_1];
                            output.i_fault[fault_number](phase_2) -= i_source_bus[phase_2];
                        }
                        else if (fault_type == FaultType::two_phase_to_ground) {
                            output.i_fault[fault_number](phase_1) += i_source_bus[phase_1];
                            output.i_fault[fault_number](phase_2) += i_source_bus[phase_2];
                        }
                        else {
                            assert((fault_type == FaultType::three_phase));
                            continue;
                        }
                    }
                }
            }
        }

        // TODO calculate i_branch_from, i_branch_to, i_shunt
    }

    auto set_phase_index(FaultPhase fault_phase) {
        IntS phase_1{-1};
        IntS phase_2{-1};

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

        return std::make_pair(phase_1, phase_2);
    }

    bool all_fault_type_phase_equal(std::vector<FaultCalcParam> const& vec) {
        if (vec.empty()) {
            return false;
        }

        FaultCalcParam const first = vec.front();

        return std::all_of(cbegin(vec), cend(vec), [first](FaultCalcParam const& param) {
            return (param.fault_type == first.fault_type) && (param.fault_phase == first.fault_phase);
        });
    }
};

template class ShortCircuitSolver<true>;
template class ShortCircuitSolver<false>;

}  // namespace math_model_impl

template <bool sym>
using ShortCircuitSolver = math_model_impl::ShortCircuitSolver<sym>;

}  // namespace power_grid_model

#endif