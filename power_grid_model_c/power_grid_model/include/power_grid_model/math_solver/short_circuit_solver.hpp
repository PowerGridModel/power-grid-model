// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common_solver_functions.hpp"
#include "sparse_lu_solver.hpp"
#include "y_bus.hpp"

#include "../calculation_parameters.hpp"
#include "../common/enum.hpp"
#include "../common/exception.hpp"

namespace power_grid_model::math_solver {

// hide implementation in inside namespace
namespace short_circuit {

// solver
template <symmetry_tag sym> class ShortCircuitSolver {
    using BlockPermArray =
        typename SparseLUSolver<ComplexTensor<sym>, ComplexValue<sym>, ComplexValue<sym>>::BlockPermArray;

  public:
    ShortCircuitSolver(YBus<sym> const& y_bus, std::shared_ptr<MathModelTopology const> const& topo_ptr)
        : n_bus_{y_bus.size()},
          n_source_{topo_ptr->n_source()},
          sources_per_bus_{topo_ptr, &topo_ptr->sources_per_bus},
          mat_data_(y_bus.nnz_lu()),
          sparse_solver_{y_bus.shared_indptr_lu(), y_bus.shared_indices_lu(), y_bus.shared_diag_lu()},
          perm_{static_cast<BlockPermArray>(n_bus_)} {}

    ShortCircuitSolverOutput<sym> run_short_circuit(YBus<sym> const& y_bus, ShortCircuitInput const& input) {
        check_input_valid(input);

        auto const [fault_type, fault_phase] = extract_fault_type_phase(input.faults);

        // set phase 1 and 2 index for single and two phase faults
        auto const [phase_1, phase_2] = set_phase_index(fault_phase);

        // output
        ShortCircuitSolverOutput<sym> output;
        output.u_bus.resize(n_bus_);
        output.fault.resize(input.faults.size());
        output.source.resize(n_source_);

        IdxVector infinite_admittance_fault_counter(n_bus_);

        detail::copy_y_bus<sym>(y_bus, mat_data_);

        prepare_matrix_and_rhs(y_bus, input, output, infinite_admittance_fault_counter, fault_type, phase_1, phase_2);

        // solve matrix
        sparse_solver_.prefactorize_and_solve(mat_data_, perm_, output.u_bus, output.u_bus);

        // post processing
        calculate_result(y_bus, input, output, infinite_admittance_fault_counter, fault_type, phase_1, phase_2);

        return output;
    }

  private:
    Idx n_bus_;
    Idx n_fault_;
    Idx n_source_;
    // shared topo data
    std::shared_ptr<DenseGroupedIdxVector const> sources_per_bus_;
    // sparse linear equation
    ComplexTensorVector<sym> mat_data_;
    // sparse solver
    SparseLUSolver<ComplexTensor<sym>, ComplexValue<sym>, ComplexValue<sym>> sparse_solver_;
    BlockPermArray perm_;

    void prepare_matrix_and_rhs(YBus<sym> const& y_bus, ShortCircuitInput const& input,
                                ShortCircuitSolverOutput<sym>& output, IdxVector& infinite_admittance_fault_counter,
                                FaultType const& fault_type, IntS phase_1, IntS phase_2) {
        IdxVector const& bus_entry = y_bus.lu_diag();

        for (auto const& [bus_number, sources, faults] :
             enumerated_zip_sequence(*sources_per_bus_, input.fault_buses)) {
            Idx const diagonal_position = bus_entry[bus_number];
            auto& diagonal_element = mat_data_[diagonal_position];
            auto& u_bus = output.u_bus[bus_number];

            detail::add_sources<sym>(sources, bus_number, y_bus, input.source, diagonal_element, u_bus);

            add_faults(faults, bus_number, y_bus, input, diagonal_element, u_bus, infinite_admittance_fault_counter,
                       fault_type, phase_1, phase_2);
        }
    }

    void add_faults(IdxRange const& faults, Idx bus_number, YBus<sym> const& y_bus, ShortCircuitInput const& input,
                    ComplexTensor<sym>& diagonal_element, ComplexValue<sym>& u_bus,
                    IdxVector& infinite_admittance_fault_counter, FaultType const& fault_type, IntS phase_1,
                    IntS phase_2) {
        for (Idx const fault_number : faults) {
            DoubleComplex const y_fault = input.faults[fault_number].y_fault;
            if (std::isinf(y_fault.real())) {
                assert(std::isinf(y_fault.imag()));
                infinite_admittance_fault_counter[bus_number] += 1;
                add_fault_with_infinite_impedance(bus_number, y_bus, diagonal_element, u_bus, fault_type, phase_1,
                                                  phase_2);
                // If there is a fault with infinite admittance, there is no need to add other faults to that
                // bus
                break;
            }
            assert(!std::isinf(y_fault.imag()));
            add_fault(y_fault, bus_number, y_bus, diagonal_element, u_bus, fault_type, phase_1, phase_2);
        }
    }

    void add_fault_with_infinite_impedance(Idx bus_number, YBus<sym> const& y_bus, ComplexTensor<sym>& diagonal_element,
                                           ComplexValue<sym>& u_bus, FaultType const& fault_type, IntS phase_1,
                                           IntS phase_2) {
        using enum FaultType;

        if (fault_type == three_phase) { // three phase fault
            for (Idx data_index = y_bus.row_indptr_lu()[bus_number];
                 data_index != y_bus.row_indptr_lu()[bus_number + 1]; ++data_index) {
                Idx const col_data_index = y_bus.lu_transpose_entry()[data_index];
                // mat_data[:,bus] = 0
                mat_data_[col_data_index] = ComplexTensor<sym>{0};
            }
            // mat_data[bus,bus] = -1
            diagonal_element = ComplexTensor<sym>{-1};
            u_bus = ComplexValue<sym>{0}; // update rhs
        }
        if constexpr (!is_symmetric_v<sym>) {
            if (fault_type == single_phase_to_ground) {
                for (Idx data_index = y_bus.row_indptr_lu()[bus_number];
                     data_index != y_bus.row_indptr_lu()[bus_number + 1]; ++data_index) {
                    Idx const col_data_index = y_bus.lu_transpose_entry()[data_index];
                    // mat_data[:,bus][:, phase_1] = 0
                    mat_data_[col_data_index].col(phase_1) = 0;
                }
                // mat_data[bus,bus][phase_1, phase_1] = -1
                diagonal_element(phase_1, phase_1) = -1;
                u_bus(phase_1) = 0; // update rhs
            } else if (fault_type == two_phase) {
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
                diagonal_element(phase_1, phase_2) = -1;
                diagonal_element(phase_2, phase_2) = 1;
                // update rhs
                u_bus(phase_2) += u_bus(phase_1);
                u_bus(phase_1) = 0;
            } else if (fault_type == two_phase_to_ground) {
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
                diagonal_element(phase_1, phase_1) = -1;
                diagonal_element(phase_2, phase_2) = -1;
                // update rhs
                u_bus(phase_1) = 0;
                u_bus(phase_2) = 0;
            } else {
                assert((fault_type == three_phase));
            }
        }
    }

    void add_fault(DoubleComplex const& y_fault, Idx bus_number, YBus<sym> const& y_bus,
                   ComplexTensor<sym>& diagonal_element, ComplexValue<sym>& u_bus, FaultType const& fault_type,
                   IntS phase_1, IntS phase_2) {
        using enum FaultType;

        if (fault_type == three_phase) { // three phase fault
            // mat_data[bus,bus] += y_fault
            diagonal_element += ComplexTensor<sym>{y_fault};
        }
        if constexpr (!is_symmetric_v<sym>) {
            if (fault_type == single_phase_to_ground) {
                // mat_data[bus,bus][phase_1, phase_1] += y_fault
                diagonal_element(phase_1, phase_1) += y_fault;
            } else if (fault_type == two_phase) {
                // mat_data[bus,bus][phase_1, phase_1] += y_fault
                // mat_data[bus,bus][phase_2, phase_2] += y_fault
                // mat_data[bus,bus][phase_1, phase_2] -= y_fault
                // mat_data[bus,bus][phase_2, phase_1] -= y_fault
                diagonal_element(phase_1, phase_1) += y_fault;
                diagonal_element(phase_2, phase_2) += y_fault;
                diagonal_element(phase_1, phase_2) -= y_fault;
                diagonal_element(phase_2, phase_1) -= y_fault;
            } else if (fault_type == two_phase_to_ground) {
                for (Idx data_index = y_bus.row_indptr_lu()[bus_number];
                     data_index != y_bus.row_indptr_lu()[bus_number + 1]; ++data_index) {
                    Idx const col_data_index = y_bus.lu_transpose_entry()[data_index];
                    // mat_data[:,bus][:, phase_1] += mat_data[:,bus][:, phase_2]
                    // mat_data[:,bus][:, phase_2] = 0
                    mat_data_[col_data_index].col(phase_1) += mat_data_[col_data_index].col(phase_2);
                    mat_data_[col_data_index].col(phase_2) = 0;
                }
                // mat_data[bus,bus][phase_1, phase_2] = -1
                // mat_data[bus,bus][phase_2, phase_1] += y_fault
                // mat_data[bus,bus][phase_2, phase_2] = 1
                diagonal_element(phase_1, phase_2) = -1;
                diagonal_element(phase_2, phase_2) = 1;
                diagonal_element(phase_2, phase_1) += y_fault;
                // update rhs
                u_bus(phase_2) += u_bus(phase_1);
                u_bus(phase_1) = 0;
            } else {
                assert((fault_type == three_phase));
            }
        }
    }

    void calculate_result(YBus<sym> const& y_bus, ShortCircuitInput const& input, ShortCircuitSolverOutput<sym>& output,
                          IdxVector const& infinite_admittance_fault_counter, FaultType const fault_type,
                          int const phase_1, int const phase_2) const {
        using enum FaultType;

        for (auto const& [bus_number, faults, sources] :
             enumerated_zip_sequence(input.fault_buses, *sources_per_bus_)) {
            ComplexValue<sym> const x_bus_subtotal = output.u_bus[bus_number];
            auto const infinite_admittance_fault_counter_bus =
                static_cast<double>(infinite_admittance_fault_counter[bus_number]);

            for (Idx const fault_number : faults) {
                auto& i_fault = output.fault[fault_number].i_fault;
                auto& u_bus = output.u_bus[bus_number];
                DoubleComplex const y_fault = input.faults[fault_number].y_fault;

                if (std::isinf(y_fault.real())) {
                    assert(std::isinf(y_fault.imag()));
                    if (fault_type == three_phase) { // three phase fault
                        i_fault = -1.0 * static_cast<ComplexValue<sym>>(x_bus_subtotal) /
                                  infinite_admittance_fault_counter_bus; // injection is
                                                                         // negative to fault
                        u_bus = ComplexValue<sym>{0.0};
                    }
                    if constexpr (!is_symmetric_v<sym>) {
                        if (fault_type == single_phase_to_ground) {
                            i_fault(phase_1) = -1.0 * x_bus_subtotal[phase_1] / infinite_admittance_fault_counter_bus;
                            u_bus(phase_1) = 0.0;
                        } else if (fault_type == two_phase) {
                            i_fault(phase_1) = -1.0 * x_bus_subtotal[phase_2] / infinite_admittance_fault_counter_bus;
                            i_fault(phase_2) = -1.0 * i_fault(phase_1);
                            u_bus(phase_2) = u_bus(phase_1);
                        } else if (fault_type == two_phase_to_ground) {
                            i_fault(phase_1) = -1.0 * x_bus_subtotal[phase_1] / infinite_admittance_fault_counter_bus;
                            i_fault(phase_2) = -1.0 * x_bus_subtotal[phase_2] / infinite_admittance_fault_counter_bus;
                            u_bus(phase_1) = 0.0;
                            u_bus(phase_2) = 0.0;
                        } else {
                            assert((fault_type == three_phase));
                            continue;
                        }
                    }
                } else {
                    assert(!std::isinf(y_fault.imag()));
                    if (infinite_admittance_fault_counter[bus_number] > 0) {
                        // ignore fault objects with impedance, when there is a fault with infinite admittance on
                        // bus
                        continue;
                    }
                    if (fault_type == three_phase) { // three phase fault
                        i_fault = static_cast<ComplexValue<sym>>(y_fault * x_bus_subtotal);
                    }
                    if constexpr (!is_symmetric_v<sym>) {
                        if (fault_type == single_phase_to_ground) {
                            i_fault(phase_1) = y_fault * x_bus_subtotal[phase_1];
                        } else if (fault_type == two_phase) {
                            i_fault(phase_1) = y_fault * x_bus_subtotal[phase_1] - y_fault * x_bus_subtotal[phase_2];
                            i_fault(phase_2) = y_fault * x_bus_subtotal[phase_2] - y_fault * x_bus_subtotal[phase_1];
                        } else if (fault_type == two_phase_to_ground) {
                            i_fault(phase_1) = -1.0 * x_bus_subtotal[phase_2];
                            i_fault(phase_2) = -1.0 * i_fault(phase_1) + y_fault * x_bus_subtotal[phase_1];
                            u_bus(phase_2) = u_bus(phase_1);
                        } else {
                            assert((fault_type == three_phase));
                            continue;
                        }
                    }
                }
            }

            ComplexValue<sym> i_source_bus{};    // total source current in to the bus
            ComplexValue<sym> i_source_inject{}; // total raw source current as a Norton equivalent
            for (Idx const source_number : sources) {
                ComplexTensor<sym> const y_source = y_bus.math_model_param().source_param[source_number];
                ComplexValue<sym> const i_source_inject_single =
                    dot(y_source, ComplexValue<sym>{input.source[source_number]});
                output.source[source_number].i = i_source_inject_single - dot(y_source, output.u_bus[bus_number]);
                i_source_bus += output.source[source_number].i;
                i_source_inject += i_source_inject_single;
            }

            // compensate source current into hard fault
            // compensate source current into two phase to ground fault with impedance
            for (Idx const fault_number : faults) {
                auto& i_fault = output.fault[fault_number].i_fault;
                DoubleComplex const y_fault = input.faults[fault_number].y_fault;

                if (std::isinf(y_fault.real())) {
                    assert(std::isinf(y_fault.imag()));
                    if (fault_type == three_phase) {
                        i_fault += static_cast<ComplexValue<sym>>(i_source_bus / infinite_admittance_fault_counter_bus);
                    }
                    if constexpr (!is_symmetric_v<sym>) {
                        if (fault_type == single_phase_to_ground) {
                            i_fault(phase_1) += i_source_bus[phase_1] / infinite_admittance_fault_counter_bus;
                        } else if (fault_type == two_phase) {
                            i_fault(phase_1) += i_source_inject[phase_1] / infinite_admittance_fault_counter_bus;
                            // i_inj_1 + i_inj_2 = i_ref_1 + i_ref_2
                            // i_fault_2_p = i_inj_1
                            //      i_fault_2_p is the i_fault_2 status quo after the first fault loop
                            // i_inj_2 = - i_inj_1 + i_ref_1 + i_ref_2
                            // i_fault_2 = i_ref_2 - i_inj_2 = i_ref_2 + i_inj_1 - i_ref_1 - i_ref_2
                            //           = i_inj_1 - i_ref_1 = i_fault_2_p - i_ref_1
                            i_fault(phase_2) -= i_source_inject[phase_1] / infinite_admittance_fault_counter_bus;
                        } else if (fault_type == two_phase_to_ground) {
                            i_fault(phase_1) += i_source_bus[phase_1];
                            i_fault(phase_2) += i_source_bus[phase_2];
                        } else {
                            assert((fault_type == three_phase));
                            continue;
                        }
                    }
                } else {
                    // compensate for 2 phase to ground fault with impedance
                    assert(!std::isinf(y_fault.imag()));
                    if constexpr (!is_symmetric_v<sym>) {
                        if ((fault_type == two_phase_to_ground) && (infinite_admittance_fault_counter_bus == 0.0)) {
                            auto const finite_admittance_fault_counter_bus = static_cast<double>(faults.size());
                            // i_inj_1 + i_inj_2 = i_ref_1 + i_ref_2 - u_12 * y_fault
                            // i_fault_2_p = i_inj_1 + u_12 * y_fault
                            //      i_fault_2_p is the i_fault_2 status quo after the first fault loop
                            // i_inj_2 = - i_inj_1 + i_ref_1 + i_ref_2 - u_12 * y_fault
                            // i_fault_2 = i_ref_2 - i_inj_2 = i_ref_2 + i_inj_1 - i_ref_1 - i_ref_2 + u_12 *
                            // y_fault
                            //           = i_inj_1 + u_12 * y_fault - i_ref_1 = i_fault_2_p - i_ref_1
                            i_fault(phase_1) += i_source_inject[phase_1] / finite_admittance_fault_counter_bus;
                            i_fault(phase_2) -= i_source_inject[phase_1] / finite_admittance_fault_counter_bus;
                        }
                    }
                }
            }
        }

        output.branch = y_bus.template calculate_branch_flow<BranchShortCircuitSolverOutput<sym>>(output.u_bus);
        output.shunt = y_bus.template calculate_shunt_flow<ApplianceShortCircuitSolverOutput<sym>>(output.u_bus);
    }

    static constexpr auto set_phase_index(FaultPhase fault_phase) {
        IntS phase_1{-1};
        IntS phase_2{-1};

        // This function updates the phase index for single and two phase faults

        using enum FaultPhase;

        switch (fault_phase) {
        case a: {
            phase_1 = 0;
            break;
        }
        case b: {
            phase_1 = 1;
            break;
        }
        case c: {
            phase_1 = 2;
            break;
        }
        case ab: {
            phase_1 = 0;
            phase_2 = 1;
            break;
        }
        case ac: {
            phase_1 = 0;
            phase_2 = 2;
            break;
        }
        case bc: {
            phase_1 = 1;
            phase_2 = 2;
            break;
        }
        default:
            break;
        }

        return std::make_pair(phase_1, phase_2);
    }

    static constexpr auto extract_fault_type_phase(std::vector<FaultCalcParam> const& faults) {
        if (faults.empty()) {
            return std::pair{FaultType::nan, FaultPhase::nan};
        }

        auto const& first = faults.front();
        return std::pair{first.fault_type, first.fault_phase};
    }

    static void check_input_valid(ShortCircuitInput const& input) {
        if (!input.faults.empty()) {
            // For one calculation all faults should be of the same type and have the same phase
            if (!all_fault_type_phase_equal(input.faults)) {
                throw InvalidShortCircuitPhaseOrType{};
            }

            auto const& first = input.faults.front();
            if (first.fault_type == FaultType::nan || first.fault_phase == FaultPhase::default_value ||
                first.fault_phase == FaultPhase::nan) {
                throw InvalidShortCircuitPhaseOrType{};
            }
        }
    }

    static constexpr bool all_fault_type_phase_equal(std::vector<FaultCalcParam> const& vec) {
        if (vec.empty()) {
            return true;
        }

        return std::all_of(cbegin(vec), cend(vec), [first = vec.front()](FaultCalcParam const& param) {
            return (param.fault_type == first.fault_type) && (param.fault_phase == first.fault_phase);
        });
    }
};

template class ShortCircuitSolver<symmetric_t>;
template class ShortCircuitSolver<asymmetric_t>;

} // namespace short_circuit

using short_circuit::ShortCircuitSolver;

} // namespace power_grid_model::math_solver
