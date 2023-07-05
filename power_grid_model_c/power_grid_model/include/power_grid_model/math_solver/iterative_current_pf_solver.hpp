// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

// Check if the name means anything
#pragma once
#ifndef POWER_GRID_MODEL_MATH_SOLVER_ITERATIVE_CURRENT_PF_SOLVER_HPP
#define POWER_GRID_MODEL_MATH_SOLVER_ITERATIVE_CURRENT_PF_SOLVER_HPP

/*
Iterative Power Flow

Description:
    The algorithm similar to jacobi way of solving linear equations.
    Only I_inj is calculated fresh on each iteration based on latest values of U.
    Linear equation here: I_inj = YU

Prefactorization:
    If the Y bus matrix does not change, then there is no need for factorizing it again to solve linear equations.
    Hence it is done only once in the first iteration and same result is used in subsequent iterations.
    Same factorization is also used in subsequent batches

Steps:
    Initialize U with averaged u_ref, ie source voltage and phase shifts accounted
    Initialize solver
    while maximum deviation > error tolerance
        Calculate I_inj with U of previous iteration as per load/gen types.
        Solve YU = I_inj using prefactorization.
        Find maximum deviation in voltage buses U
        Update U
    Calculate output values from U result

    Initialize solver:
        Source admittance is not included in Y bus matrix here. Include that to complete the Y bus matrix.
        Invalidate prefactorization if parameters change, ie y bus values changes

    Calculating Injected current:
        Initialize I_inj = 0
        For each bus i
            For source on bus i, I_inj_i = y_ref * u_uref
            For Loads on bus i:
                If type is constant PQ: I_inj_i += conj(S_inj_j/U_i)
                If type is constant impedance: I_inj_i += conj((S_inj_j * abs(U_i)^2) / U_i) = conj((S_inj_j) * U_i
                If type is constant current: I_inj_i += conj(S_inj_j*abs(U_i)/U_i)

Nomenclature:
    I_inj : Injected current
    S_inj : Injected power
    Y : Y bus matrix
    U : Bus Voltage
    u_ref : reference voltage for source
    y_ref : Source admittance


*/

#include "block_matrix.hpp"
#include "iterative_pf_solver.hpp"
#include "sparse_lu_solver.hpp"
#include "y_bus.hpp"

#include "../calculation_parameters.hpp"
#include "../exception.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"
#include "../timer.hpp"

namespace power_grid_model {

// hide implementation in inside namespace
namespace math_model_impl {

// solver
template <bool sym>
class IterativeCurrentPFSolver : public IterativePFSolver<sym, IterativeCurrentPFSolver<sym>> {
   public:
    using BlockPermArray =
        typename SparseLUSolver<ComplexTensor<sym>, ComplexValue<sym>, ComplexValue<sym>>::BlockPermArray;

    IterativeCurrentPFSolver(YBus<sym> const& y_bus, std::shared_ptr<MathModelTopology const> const& topo_ptr)
        : IterativePFSolver<sym, IterativeCurrentPFSolver>{y_bus, topo_ptr},
          rhs_u_(y_bus.size()),
          y_data_ptr_(nullptr),
          sparse_solver_{y_bus.shared_indptr_lu(), y_bus.shared_indices_lu(), y_bus.shared_diag_lu()} {
    }

    // Add source admittance to Y bus and set variable for prepared y bus to true
    void initialize_derived_solver(YBus<sym> const& y_bus, MathOutput<sym> const&) {
        IdxVector const& source_bus_indptr = *this->source_bus_indptr_;
        ComplexTensorVector<sym> const& ydata = y_bus.admittance();
        IdxVector const& bus_entry = y_bus.lu_diag();
        // if Y bus is not up to date
        // re-build matrix and prefactorize Build y bus data with source admittance
        if (y_data_ptr_ != &y_bus.admittance()) {
            ComplexTensorVector<sym> mat_data(y_bus.nnz_lu());
            // copy y bus data
            std::transform(y_bus.map_lu_y_bus().cbegin(), y_bus.map_lu_y_bus().cend(), mat_data.begin(), [&](Idx k) {
                if (k == -1) {
                    return ComplexTensor<sym>{};
                }
                else {
                    return ydata[k];
                }
            });

            // loop bus
            for (Idx bus_number = 0; bus_number != this->n_bus_; ++bus_number) {
                Idx const data_sequence = bus_entry[bus_number];
                // loop sources
                for (Idx source_number = source_bus_indptr[bus_number];
                     source_number != source_bus_indptr[bus_number + 1]; ++source_number) {
                    // YBus_diag += Y_source
                    mat_data[data_sequence] += y_bus.math_model_param().source_param[source_number];
                }
            }
            // prefactorize
            BlockPermArray perm(this->n_bus_);
            sparse_solver_.prefactorize(mat_data, perm);
            // move pre-factorized version into shared ptr
            mat_data_ = std::make_shared<ComplexTensorVector<sym> const>(std::move(mat_data));
            perm_ = std::make_shared<BlockPermArray const>(std::move(perm));
            // cache pointer
            y_data_ptr_ = &y_bus.admittance();
        }
    }

    // Prepare matrix calculates injected current ie. RHS of solver for each iteration.
    void prepare_matrix_and_rhs(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input,
                                ComplexValueVector<sym> const& u) {
        IdxVector const& load_gen_bus_indptr = *this->load_gen_bus_indptr_;
        IdxVector const& source_bus_indptr = *this->source_bus_indptr_;
        std::vector<LoadGenType> const& load_gen_type = *this->load_gen_type_;

        // set rhs to zero for iteration start
        std::fill(rhs_u_.begin(), rhs_u_.end(), ComplexValue<sym>{0.0});

        // loop buses: i
        for (Idx bus_number = 0; bus_number != this->n_bus_; ++bus_number) {
            // loop loads/generation: j
            for (Idx load_number = load_gen_bus_indptr[bus_number]; load_number != load_gen_bus_indptr[bus_number + 1];
                 ++load_number) {
                // load type
                LoadGenType const type = load_gen_type[load_number];
                switch (type) {
                    using enum LoadGenType;

                    case const_pq:
                        // I_inj_i = conj(S_inj_j/U_i) for constant PQ type
                        rhs_u_[bus_number] += conj(input.s_injection[load_number] / u[bus_number]);
                        break;
                    case const_y:
                        // I_inj_i = conj((S_inj_j * abs(U_i)^2) / U_i) = conj((S_inj_j) * U_i for const impedance type
                        rhs_u_[bus_number] += conj(input.s_injection[load_number]) * u[bus_number];
                        break;
                    case const_i:
                        // I_inj_i = conj(S_inj_j*abs(U_i)/U_i) for const current type
                        rhs_u_[bus_number] +=
                            conj(input.s_injection[load_number] * cabs(u[bus_number]) / u[bus_number]);
                        break;
                    default:
                        throw MissingCaseForEnumError("Injection current calculation", type);
                }
            }
            // loop sources: j
            for (Idx source_number = source_bus_indptr[bus_number]; source_number != source_bus_indptr[bus_number + 1];
                 ++source_number) {
                // I_inj_i += Y_source_j * U_ref_j
                rhs_u_[bus_number] += dot(y_bus.math_model_param().source_param[source_number],
                                          ComplexValue<sym>{input.source[source_number]});
            }
        }
    }

    // Solve the linear equations I_inj = YU
    // inplace
    void solve_matrix() {
        sparse_solver_.solve_with_prefactorized_matrix(*mat_data_, *perm_, rhs_u_, rhs_u_);
    }

    // Find maximum deviation in voltage among all buses
    double iterate_unknown(ComplexValueVector<sym>& u) {
        double max_dev = 0.0;
        // loop all buses
        for (Idx bus_number = 0; bus_number != this->n_bus_; ++bus_number) {
            // Get maximum iteration for a bus
            double const dev = max_val(cabs(rhs_u_[bus_number] - u[bus_number]));
            // Keep maximum deviation of all buses
            max_dev = std::max(dev, max_dev);
            // assign updated values
            u[bus_number] = rhs_u_[bus_number];
        }
        return max_dev;
    }

   private:
    ComplexValueVector<sym> rhs_u_;
    std::shared_ptr<ComplexTensorVector<sym> const> mat_data_;
    ComplexTensorVector<sym> const* y_data_ptr_;
    // sparse solver
    SparseLUSolver<ComplexTensor<sym>, ComplexValue<sym>, ComplexValue<sym>> sparse_solver_;
    std::shared_ptr<BlockPermArray const> perm_;
};

template class IterativeCurrentPFSolver<true>;
template class IterativeCurrentPFSolver<false>;

}  // namespace math_model_impl

template <bool sym>
using IterativeCurrentPFSolver = math_model_impl::IterativeCurrentPFSolver<sym>;

}  // namespace power_grid_model

#endif
