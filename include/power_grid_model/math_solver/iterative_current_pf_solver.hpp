// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

// Check if the name means anything
#pragma once
#ifndef POWER_GRID_MODEL_MATH_SOLVER_ITERATIVE_CURRENT_PF_SOLVER_HPP
#define POWER_GRID_MODEL_MATH_SOLVER_ITERATIVE_CURRENT_PF_SOLVER_HPP

/*
Iterative Power Flow
I = YU
Split this with U_1 having all source buses, U_2 having all other buses
[I_1] = [ K  L ] [U_1]
[I_2]   [ L' M ] [U_2]

I_2 = MU_2 + L'U_1
MU_2 = I_2 - L'U_1 = RHS


Initialize U_2 with flat start and phase shifts accounted
while maximum deviation > error tolerance
    Calculate I_2 with U_2 of previous iteration
    Solve MU_2 = RHS
    Find maximum deviation in voltage
    Update U_2

*/

#include "../calculation_parameters.hpp"
#include "../exception.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"
#include "../timer.hpp"
#include "block_matrix.hpp"
#include "bsr_solver.hpp"
#include "y_bus.hpp"

namespace power_grid_model {

// hide implementation in inside namespace
namespace math_model_impl {

// solver
template <bool sym>
class IterativecurrentPFSolver {
   private:
    // block size 1 for symmetric, 3 for asym
    static constexpr Idx bsr_block_size_ = sym ? 1 : 3;

   public:
    IterativecurrentPFSolver(YBus<sym> const& y_bus, std::shared_ptr<MathModelTopology const> const& topo_ptr)
        : n_bus_{y_bus.size()},
          phase_shift_{topo_ptr, &topo_ptr->phase_shift},
          load_gen_bus_indptr_{topo_ptr, &topo_ptr->load_gen_bus_indptr},
          source_bus_indptr_{topo_ptr, &topo_ptr->source_bus_indptr},
          load_gen_type_{topo_ptr, &topo_ptr->load_gen_type},
          updated_u_(y_bus.size()),
          rhs_(n_bus_),
          mat_data_(y_bus.nnz()),
          bsr_solver_{y_bus.size(), bsr_block_size_, y_bus.shared_indptr(), y_bus.shared_indices()} {
    }

    MathOutput<sym> run_power_flow(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input, double err_tol,
                                   Idx max_iter, CalculationInfo& calculation_info) {
        // Get y bus data along with its entry
        ComplexTensorVector<sym> const& ydata = y_bus.admittance();
        IdxVector const& bus_entry = y_bus.bus_entry();
        // phase shifts
        std::vector<double> const& phase_shift = *phase_shift_;
        // prepare output
        MathOutput<sym> output;
        output.u.resize(n_bus_);
        double max_dev = std::numeric_limits<double>::max();

        Timer main_timer(calculation_info, 2220, "Math solver");

        // initialize
        Timer sub_timer(calculation_info, 2221, "Initialize calculation");
        // average u_ref of all sources
        double const u_ref = std::transform_reduce(input.source.cbegin(), input.source.cend(), 0.0, std::plus{},
                                                   [](SourceCalcParam<sym> const& source) {
                                                       return source.u_ref;
                                                   }) /
                             input.source.size();
        for (Idx i = 0; i != n_bus_; ++i) {
            // always flat start
            // consider phase shift
            RealValue<sym> theta;
            if constexpr (sym) {
                theta = phase_shift[i];
            }
            else {
                theta << phase_shift[i], (phase_shift[i] - deg_120), (phase_shift[i] - deg_240);
            }
            ComplexValue<sym> const phase_shift_complex = exp(1.0i * theta);
            output.u[i] = u_ref * phase_shift_complex;
        }

        // Build y bus data with source impedance
        // copy y bus data.
        std::copy(ydata.begin(), ydata.end(), mat_data_.begin());
        prefactorize_y_data(y_bus, input);

        // start calculation
        // iteration
        Idx num_iter = 0;
        while (max_dev > err_tol) {
            if (num_iter++ == max_iter) {
                throw IterationDiverge{max_iter, max_dev, err_tol};
            }
            sub_timer = Timer(calculation_info, 2222, "Calculate injected current");
            // set rhs to zero for iteration start
            std::fill(rhs_.begin(), rhs_.end(), ComplexValue<sym>{0.0});
            // copy y bus data.
            //std::copy(ydata.begin(), ydata.end(), mat_data_.begin());
        
            sub_timer.stop();
            // Calculate RHS
            calculate_injected_current(y_bus, input, output.u);
            sub_timer = Timer(calculation_info, 2223, "Solve sparse linear equation");
            bsr_solver_.solve(mat_data_.data(), rhs_.data(), updated_u_.data());
            sub_timer = Timer(calculation_info, 2224, "Iterate unknown");
            // Calculate maximum deviation from previous iteration
            max_dev = iterate_unknown(output.u);
            sub_timer.stop();
        }

        // Invalidate prefactorization when y bus data changes
        // Invalidate after every calculation now, change later
        bsr_solver_.invalidate_prefactorization();

        // calculate math result
        sub_timer = Timer(calculation_info, 2225, "Calculate Math Result");
        calculate_result(y_bus, input, output);

        // Manually stop timers to avoid "Max number of iterations" to be included in the timing.
        sub_timer.stop();
        main_timer.stop();

        const auto key = Timer::make_key(2226, "Max number of iterations");
        calculation_info[key] = std::max(calculation_info[key], (double)num_iter);

        return output;
    }

   private:
    Idx n_bus_;
    // shared topo data
    std::shared_ptr<DoubleVector const> phase_shift_;
    std::shared_ptr<IdxVector const> load_gen_bus_indptr_;
    std::shared_ptr<IdxVector const> source_bus_indptr_;
    std::shared_ptr<std::vector<LoadGenType> const> load_gen_type_;
    //std::vector < ComplexPower < sym>> updated_u_;
    ComplexValueVector<sym> updated_u_;
    ComplexValueVector<sym> rhs_;
    ComplexTensorVector<sym> mat_data_;
    BSRSolver<DoubleComplex> bsr_solver_;

    void calculate_injected_current(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input,
                                    ComplexValueVector<sym> const& u) {
        IdxVector const& load_gen_bus_indptr = *load_gen_bus_indptr_;
        IdxVector const& source_bus_indptr = *source_bus_indptr_;
        std::vector<LoadGenType> const& load_gen_type = *load_gen_type_;
        //ComplexTensorVector<sym> const& ydata = y_bus.admittance();
        IdxVector const& bus_entry = y_bus.bus_entry();

        // loop individual load/source
        for (Idx bus_number = 0; bus_number != n_bus_; ++bus_number) {
            Idx const data_sequence = bus_entry[bus_number];    // Do something for sequence
            // loop load
            for (Idx load_number = load_gen_bus_indptr[bus_number]; load_number != load_gen_bus_indptr[bus_number + 1]; ++load_number) {
                // load type
                LoadGenType const type = load_gen_type[load_number];
                switch (type) {
                    case LoadGenType::const_pq:
                        // I_inj = conj(S_inj/U) for constant PQ type
                        rhs_[bus_number] += conj(input.s_injection[load_number] / u[bus_number]);
                        break;
                    case LoadGenType::const_y:
                        // I_inj = conj((S_inj * abs(U*U)) / U) for const impedance type
                        rhs_[bus_number] += conj(input.s_injection[load_number] * cabs(u[bus_number] * u[bus_number]) / u[bus_number]);
                        break;
                    case LoadGenType::const_i:
                        // I_inj = conj(S_inj*abs(U)/U) for const current type
                        rhs_[bus_number] += conj(input.s_injection[load_number] * cabs(u[bus_number]) / u[bus_number]);
                        break;
                    default:
                        throw MissingCaseForEnumError("Injection current calculation", type);
                }
            }
            // loop sources
            for (Idx source_number = source_bus_indptr[bus_number]; source_number != source_bus_indptr[bus_number + 1];
                 ++source_number) {
                // YBus_diag += Y_source
                //mat_data_[data_sequence] += input.source[source_number].y_ref;
                // rhs += Y_source_j * U_ref_j
                rhs_[bus_number] +=
                    dot(input.source[source_number].y_ref, ComplexValue<sym>{input.source[source_number].u_ref});
            }
        }
    }

    void prefactorize_y_data(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input) {
        IdxVector const& source_bus_indptr = *source_bus_indptr_;
        //ComplexTensorVector<sym> const& ydata = y_bus.admittance();
        IdxVector const& bus_entry = y_bus.bus_entry();

        // loop individual load/source
        for (Idx bus_number = 0; bus_number != n_bus_; ++bus_number) {
            Idx const data_sequence = bus_entry[bus_number];  // Do something for sequence
            // loop sources
            for (Idx source_number = source_bus_indptr[bus_number]; source_number != source_bus_indptr[bus_number + 1];
                 ++source_number) {
                // YBus_diag += Y_source
                mat_data_[data_sequence] += input.source[source_number].y_ref;            }
        }
        // Prefactorize solver
        bsr_solver_.prefactorize(mat_data_.data());
    }

    double iterate_unknown(ComplexValueVector<sym>& u) {
        double max_dev = 0.0;
        // loop all buses
        for (Idx bus_number = 0; bus_number != n_bus_; ++bus_number) {
            // Get maximum iteration for a bus
            double const dev = max_val(cabs(updated_u_[bus_number] - u[bus_number]));
            // Keep maximum deviation of all buses
            max_dev = std::max(dev, max_dev);
            // assign
            u[bus_number] = updated_u_[bus_number];
        }
        return max_dev;
    }

    void calculate_result(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input, MathOutput<sym>& output) {
        // pending to correct
        // call y bus
        output.branch = y_bus.calculate_branch_flow(output.u);
        output.shunt = y_bus.calculate_shunt_flow(output.u);

        // prepare source and load gen
        output.source.resize(source_bus_indptr_->back());
        output.load_gen.resize(load_gen_bus_indptr_->back());
        // loop all bus
        for (Idx bus = 0; bus != n_bus_; ++bus) {
            // source
            for (Idx source = (*source_bus_indptr_)[bus]; source != (*source_bus_indptr_)[bus + 1]; ++source) {
                ComplexValue<sym> const u_ref{input.source[source].u_ref};
                ComplexTensor<sym> const y_ref = input.source[source].y_ref;
                output.source[source].i = dot(y_ref, u_ref - output.u[bus]);
                output.source[source].s = output.u[bus] * conj(output.source[source].i);
            }

            // load_gen
            for (Idx load_gen = (*load_gen_bus_indptr_)[bus]; load_gen != (*load_gen_bus_indptr_)[bus + 1];
                 ++load_gen) {
                LoadGenType const type = (*load_gen_type_)[load_gen];
                switch (type) {
                    case LoadGenType::const_pq:
                        // always same power
                        output.load_gen[load_gen].s = input.s_injection[load_gen];
                        break;
                    case LoadGenType::const_y:
                        // power is quadratic relation to voltage
                        output.load_gen[load_gen].s = input.s_injection[load_gen] * cabs(output.u[bus] * output.u[bus]);
                        break;
                    case LoadGenType::const_i:
                        // power is linear relation to voltage
                        output.load_gen[load_gen].s = input.s_injection[load_gen] * cabs(output.u[bus]);
                        break;
                    default:
                        throw MissingCaseForEnumError("Power injection", type);
                }
                output.load_gen[load_gen].i = conj(output.load_gen[load_gen].s / output.u[bus]);
            }
        }
    }
};

template class IterativecurrentPFSolver<true>;
template class IterativecurrentPFSolver<false>;

}  // namespace math_model_impl

template <bool sym>
using IterativecurrentPFSolver = math_model_impl::IterativecurrentPFSolver<sym>;

}  // namespace power_grid_model

#endif
