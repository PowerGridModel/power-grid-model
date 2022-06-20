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


Steps:
Initialize U_2 with flat start and phase shifts accounted
Add source admittance to Y bus matrix
while maximum deviation > error tolerance
    Calculate I_2 with U_2 of previous iteration as per load/gen types.
    Solve MU_2 = RHS using prefactorization.
    Find maximum deviation in voltage
    Update U_2
(Invalidate prefactorization if parameters change, ie y bus values changes)

*/

#include "../calculation_parameters.hpp"
#include "../exception.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"
#include "../timer.hpp"
#include "block_matrix.hpp"
#include "bsr_solver.hpp"
#include "iterative_pf_solver.hpp"
#include "y_bus.hpp"

namespace power_grid_model {

// hide implementation in inside namespace
namespace math_model_impl {

// solver
template <bool sym>
class IterativecurrentPFSolver : public IterativePFSolver<sym> {
   private:
    // block size 1 for symmetric, 3 for asym
    static constexpr Idx bsr_block_size_ = sym ? 1 : 3;

   public:
    IterativecurrentPFSolver(YBus<sym> const& y_bus, std::shared_ptr<MathModelTopology const> const& topo_ptr)
        : IterativePFSolver<sym>{y_bus, topo_ptr},
          updated_u_(y_bus.size()),
          rhs_(y_bus.size()),
          mat_data_(y_bus.nnz()),
          loaded_mat_data_(false),
          bsr_solver_{y_bus.size(), bsr_block_size_, y_bus.shared_indptr(), y_bus.shared_indices()} {
    }

    MathOutput<sym> run_power_flow(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input, double err_tol,
                                   Idx max_iter, CalculationInfo& calculation_info) {
        // Get y bus data along with its entry
        ComplexTensorVector<sym> const& ydata = y_bus.admittance();
        IdxVector const& bus_entry = y_bus.bus_entry();
        Idx n_bus = this->n_bus_;

        // Why not use the private variables?
        IdxVector const& source_bus_indptr = *this->source_bus_indptr_;
        std::vector<double> const& phase_shift = *this->phase_shift_;

        // prepare output
        MathOutput<sym> output;
        output.u.resize(n_bus);
        double max_dev = std::numeric_limits<double>::max();

        Timer main_timer(calculation_info, 2220, "Math solver");

        // initialize
        Timer sub_timer(calculation_info, 2221, "Initialize calculation");
        // average u_ref of all sources
        DoubleComplex const u_ref = [&]() {
            DoubleComplex sum_u_ref = 0.0;
            for (Idx bus = 0; bus != n_bus; ++bus) {
                for (Idx source = source_bus_indptr[bus]; source != source_bus_indptr[bus + 1]; ++source) {
                    sum_u_ref += input.source[source] * std::exp(1.0i * -phase_shift[bus]);  // offset phase shift
                }
            }
            return sum_u_ref / (double)input.source.size();
        }();
        for (Idx i = 0; i != n_bus; ++i) {
            // always flat start
            // consider phase shift
            output.u[i] = ComplexValue<sym>{u_ref * std::exp(1.0i * phase_shift[i])};
        }

        // Build y bus data with source impedance
        // copy y bus data.
        if (!loaded_mat_data_) {
            std::copy(ydata.begin(), ydata.end(), mat_data_.begin());
            // loop bus
            for (Idx bus_number = 0; bus_number != n_bus; ++bus_number) {
                Idx const data_sequence = bus_entry[bus_number];
                // loop sources
                for (Idx source_number = source_bus_indptr[bus_number];
                     source_number != source_bus_indptr[bus_number + 1]; ++source_number) {
                    // YBus_diag += Y_source
                    mat_data_[data_sequence] += y_bus.math_model_param().source_param[source_number];
                }
            }
            loaded_mat_data_ = true;
        }
        sub_timer.stop();

        // start calculation
        // iteration
        Idx num_iter = 0;
        while (max_dev > err_tol) {
            if (num_iter++ == max_iter) {
                throw IterationDiverge{max_iter, max_dev, err_tol};
            }
            // Calculate injected current
            sub_timer = Timer(calculation_info, 2222, "Calculate injected current");
            // set rhs to zero for iteration start
            std::fill(rhs_.begin(), rhs_.end(), ComplexValue<sym>{0.0});
            // Calculate RHS
            calculate_injected_current(y_bus, input, output.u);
            sub_timer = Timer(calculation_info, 2223, "Solve sparse linear equation");
            bsr_solver_.solve(mat_data_.data(), rhs_.data(), updated_u_.data(), true);
            // Calculate maximum deviation from previous iteration
            sub_timer = Timer(calculation_info, 2224, "Iterate unknown");
            max_dev = iterate_unknown(output.u);
            sub_timer.stop();
        }

        // calculate math result
        sub_timer = Timer(calculation_info, 2225, "Calculate Math Result");
        this->calculate_result(y_bus, input, output);

        // Manually stop timers to avoid "Max number of iterations" to be included in the timing.
        sub_timer.stop();
        main_timer.stop();

        const auto key = Timer::make_key(2226, "Max number of iterations");
        calculation_info[key] = std::max(calculation_info[key], (double)num_iter);

        return output;
    }

    void reset_lhs() {
        // Invalidate prefactorization when parameters change
        bsr_solver_.invalidate_prefactorization();
        loaded_mat_data_ = false;
    }

   private:
    ComplexValueVector<sym> updated_u_;
    ComplexValueVector<sym> rhs_;
    ComplexTensorVector<sym> mat_data_;
    bool loaded_mat_data_;
    BSRSolver<DoubleComplex> bsr_solver_;

    void calculate_injected_current(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input,
                                    ComplexValueVector<sym> const& u) {
        IdxVector const& load_gen_bus_indptr = *this->load_gen_bus_indptr_;
        IdxVector const& source_bus_indptr = *this->source_bus_indptr_;
        std::vector<LoadGenType> const& load_gen_type = *this->load_gen_type_;
        Idx n_bus = this->n_bus_;

        // rhs = I_inj + L'U
        // loop buses: i
        for (Idx bus_number = 0; bus_number != n_bus; ++bus_number) {
            // loop loads/generation: j
            for (Idx load_number = load_gen_bus_indptr[bus_number]; load_number != load_gen_bus_indptr[bus_number + 1];
                 ++load_number) {
                // load type
                LoadGenType const type = load_gen_type[load_number];
                switch (type) {
                    case LoadGenType::const_pq:
                        // I_inj_i = conj(S_inj_j/U_i) for constant PQ type
                        rhs_[bus_number] += conj(input.s_injection[load_number] / u[bus_number]);
                        break;
                    case LoadGenType::const_y:
                        // I_inj_i = conj((S_inj_j * abs(U_i)^2) / U_i) = conj((S_inj_j) * U_i for const impedance type
                        rhs_[bus_number] += conj(input.s_injection[load_number]) * u[bus_number];
                        break;
                    case LoadGenType::const_i:
                        // I_inj_i = conj(S_inj_j*abs(U_i)/U_i) for const current type
                        rhs_[bus_number] += conj(input.s_injection[load_number] * cabs(u[bus_number]) / u[bus_number]);
                        break;
                    default:
                        throw MissingCaseForEnumError("Injection current calculation", type);
                }
            }
            // loop sources: j
            for (Idx source_number = source_bus_indptr[bus_number]; source_number != source_bus_indptr[bus_number + 1];
                 ++source_number) {
                // -L'U = Y_source_j * U_ref_j
                rhs_[bus_number] += dot(y_bus.math_model_param().source_param[source_number],
                                        ComplexValue<sym>{input.source[source_number]});
            }
        }
    }

    double iterate_unknown(ComplexValueVector<sym>& u) {
        Idx n_bus = this->n_bus_;
        double max_dev = 0.0;
        // loop all buses
        for (Idx bus_number = 0; bus_number != n_bus; ++bus_number) {
            // Get maximum iteration for a bus
            double const dev = max_val(cabs(updated_u_[bus_number] - u[bus_number]));
            // Keep maximum deviation of all buses
            max_dev = std::max(dev, max_dev);
            // assign updated values
            u[bus_number] = updated_u_[bus_number];
        }
        return max_dev;
    }
};

template class IterativecurrentPFSolver<true>;
template class IterativecurrentPFSolver<false>;

}  // namespace math_model_impl

template <bool sym>
using IterativecurrentPFSolver = math_model_impl::IterativecurrentPFSolver<sym>;

}  // namespace power_grid_model

#endif
