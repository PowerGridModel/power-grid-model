// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

// Check if the name means anything
#pragma once
#ifndef POWER_GRID_MODEL_MATH_SOLVER_ITERATIVE_CURRENT_PF_SOLVER_HPP
#define POWER_GRID_MODEL_MATH_SOLVER_ITERATIVE_CURRENT_PF_SOLVER_HPP

/*
Iterative Power Flow

I_inj = YU

Steps:
Initialize U with flat start and phase shifts accounted
Source admittance is not included in Y bus matrix here. Include that to complete the Y bus matrix.
while maximum deviation > error tolerance
    Calculate I_inj with U of previous iteration as per load/gen types.
    Solve YU = I_inj using prefactorization.
    Find maximum deviation in voltage buses U
    Update U
(Invalidate prefactorization if parameters change, ie y bus values changes)

Prefactorization:
The Y bus matrix is only factorized once and the same result is used in subsequent iteration.
Same factorization is also used in subsequent batches if Y bus matrix does not change in the new batch.

Calculating Injected current:
For each bus i
    For source on bus i, I_inj_i = y_ref * u_uref
    For Loads on bus i:
        If type is constant PQ: I_inj_i = conj(S_inj_j/U_i)
        If type is constant impedance: I_inj_i = conj((S_inj_j * abs(U_i)^2) / U_i) = conj((S_inj_j) * U_i
        If type is constant current: I_inj_i = conj(S_inj_j*abs(U_i)/U_i)


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
class IterativeCurrentPFSolver : public IterativePFSolver<sym, IterativeCurrentPFSolver<sym>> {
   private:
    // block size 1 for symmetric, 3 for asym
    static constexpr Idx bsr_block_size_ = sym ? 1 : 3;

   public:
    IterativeCurrentPFSolver(YBus<sym> const& y_bus, std::shared_ptr<MathModelTopology const> const& topo_ptr)
        : IterativePFSolver<sym, IterativeCurrentPFSolver>{y_bus, topo_ptr},
          updated_u_(y_bus.size()),
          rhs_(y_bus.size()),
          mat_data_(y_bus.nnz()),
          loaded_mat_data_(false),
          bsr_solver_{y_bus.size(), bsr_block_size_, y_bus.shared_indptr(), y_bus.shared_indices()} {
    }

    // For iterative current, add source admittance to Y bus and set variable for prepared y bus to true
    void initialize_derived_solver(YBus<sym> const& y_bus, MathOutput<sym> output) {
        (void)(output);
        IdxVector const& source_bus_indptr = *this->source_bus_indptr_;
        ComplexTensorVector<sym> const& ydata = y_bus.admittance();
        IdxVector const& bus_entry = y_bus.bus_entry();

        // Build y bus data with source admittance
        // copy y bus data.
        if (!loaded_mat_data_) {
            std::copy(ydata.begin(), ydata.end(), mat_data_.begin());
            // loop bus
            for (Idx bus_number = 0; bus_number != this->n_bus_; ++bus_number) {
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
    }

    // Prepare matrix calculates injected current ie. RHS of solver for each iteration.
    void prepare_matrix(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input, ComplexValueVector<sym> const& u) {
        IdxVector const& load_gen_bus_indptr = *this->load_gen_bus_indptr_;
        IdxVector const& source_bus_indptr = *this->source_bus_indptr_;
        std::vector<LoadGenType> const& load_gen_type = *this->load_gen_type_;

        // set rhs to zero for iteration start
        std::fill(rhs_.begin(), rhs_.end(), ComplexValue<sym>{0.0});

        // rhs = I_inj + L'U
        // loop buses: i
        for (Idx bus_number = 0; bus_number != this->n_bus_; ++bus_number) {
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

    // Solve the linear equations I_inj = YU
    void solve_matrix() {
        bsr_solver_.solve(mat_data_.data(), rhs_.data(), updated_u_.data(), true);
    }

    // Find maximum deviation in voltage among all buses
    double iterate_unknown(ComplexValueVector<sym>& u) {
        double max_dev = 0.0;
        // loop all buses
        for (Idx bus_number = 0; bus_number != this->n_bus_; ++bus_number) {
            // Get maximum iteration for a bus
            double const dev = max_val(cabs(updated_u_[bus_number] - u[bus_number]));
            // Keep maximum deviation of all buses
            max_dev = std::max(dev, max_dev);
            // assign updated values
            u[bus_number] = updated_u_[bus_number];
        }
        return max_dev;
    }

    // Invalidate prefactorization if parameters change
    void reset_lhs() {
        bsr_solver_.invalidate_prefactorization();
        loaded_mat_data_ = false;
    }

   private:
    ComplexValueVector<sym> updated_u_;
    ComplexValueVector<sym> rhs_;
    ComplexTensorVector<sym> mat_data_;
    bool loaded_mat_data_;
    BSRSolver<DoubleComplex> bsr_solver_;
};

template class IterativeCurrentPFSolver<true>;
template class IterativeCurrentPFSolver<false>;

}  // namespace math_model_impl

template <bool sym>
using IterativeCurrentPFSolver = math_model_impl::IterativeCurrentPFSolver<sym>;

}  // namespace power_grid_model

#endif
