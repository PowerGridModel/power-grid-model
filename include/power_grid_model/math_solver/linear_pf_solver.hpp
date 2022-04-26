// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MATH_SOLVER_LINEAR_PF_SOLVER_HPP
#define POWER_GRID_MODEL_MATH_SOLVER_LINEAR_PF_SOLVER_HPP

/*
Linear PF solver for constant impedance

Model load as constant impedance/element_admittance
I_inj =  -U * Y_load
S_inj = U * conj(I_inj) = - U * conj(U) * conj(Y_load) = -V^2 * conj(Y_load)
S_base = -conj(Y_load)
Y_load = -conj(S_base)
YBus_diag += Y_load
YBus_diag += -conj(S_base)

Linear equation
[YBus] [U] = [rhs] = [I]

if no source
    rhs_i = 0
if there are sources
    rhs_i = I_i = sum{j as source} (- Y_source_j * U_i + Y_source_j * U_ref_j)
    reform equation
    YBus_diag_i += sum{j as source} (Y_source_j)
    rhs_i +=  sum{j as source} (Y_source_j * U_ref_j)

*/

#include "../calculation_parameters.hpp"
#include "../exception.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"
#include "../timer.hpp"
#include "bsr_solver.hpp"
#include "y_bus.hpp"

namespace power_grid_model {

template <bool sym>
class LinearPFSolver {
   private:
    // block size 1 for symmetric, 3 for asym
    static constexpr Idx bsr_block_size_ = sym ? 1 : 3;

   public:
    LinearPFSolver(YBus<sym> const& y_bus, std::shared_ptr<MathModelTopology const> const& topo_ptr)
        : n_bus_{y_bus.size()},
          load_gen_bus_indptr_{topo_ptr, &topo_ptr->load_gen_bus_indptr},
          source_bus_indptr_{topo_ptr, &topo_ptr->source_bus_indptr},
          mat_data_(y_bus.nnz()),
          rhs_(n_bus_),
          bsr_solver_{y_bus.size(), bsr_block_size_, y_bus.shared_indptr(), y_bus.shared_indices()} {
    }

    MathOutput<sym> run_power_flow(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input,
                                   CalculationInfo& calculation_info) {
        // getter
        ComplexTensorVector<sym> const& ydata = y_bus.admittance();
        IdxVector const& bus_entry = y_bus.bus_entry();
        // output
        MathOutput<sym> output;
        output.u.resize(n_bus_);

        Timer main_timer(calculation_info, 2220, "Math solver");

        // prepare matrix
        Timer sub_timer(calculation_info, 2221, "Prepare matrix");

        // copy y bus data
        std::copy(ydata.begin(), ydata.end(), mat_data_.begin());
        // set rhs to zero
        std::fill(rhs_.begin(), rhs_.end(), ComplexValue<sym>{0.0});

        // loop to all loads and sources, j as load number
        IdxVector const& load_gen_bus_idxptr = *load_gen_bus_indptr_;
        IdxVector const& source_bus_indptr = *source_bus_indptr_;
        for (Idx bus_number = 0; bus_number != n_bus_; ++bus_number) {
            Idx const data_sequence = bus_entry[bus_number];
            // loop loads
            for (Idx load_number = load_gen_bus_idxptr[bus_number]; load_number != load_gen_bus_idxptr[bus_number + 1];
                 ++load_number) {
                // YBus_diag += -conj(S_base)
                add_diag(mat_data_[data_sequence], -conj(input.s_injection[load_number]));
            }
            // loop sources
            for (Idx source_number = source_bus_indptr[bus_number]; source_number != source_bus_indptr[bus_number + 1];
                 ++source_number) {
                // YBus_diag += Y_source
                mat_data_[data_sequence] += y_bus.math_model_param().source_param[source_number];
                // rhs += Y_source_j * U_ref_j
                rhs_[bus_number] += dot(y_bus.math_model_param().source_param[source_number],
                                        ComplexValue<sym>{input.source[source_number]});
            }
        }

        // solve
        // u vector will have I_injection for slack bus for now
        sub_timer = Timer(calculation_info, 2222, "Solve sparse linear equation");
        bsr_solver_.solve(mat_data_.data(), rhs_.data(), output.u.data());

        // calculate math result
        sub_timer = Timer(calculation_info, 2223, "Calculate Math Result");
        calculate_result(y_bus, input, output);

        // output
        return output;
    }

   private:
    Idx n_bus_;
    // shared topo data
    std::shared_ptr<IdxVector const> load_gen_bus_indptr_;
    std::shared_ptr<IdxVector const> source_bus_indptr_;
    // sparse linear equation
    ComplexTensorVector<sym> mat_data_;
    ComplexValueVector<sym> rhs_;
    // sparse solver
    BSRSolver<DoubleComplex> bsr_solver_;

    void calculate_result(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input, MathOutput<sym>& output) {
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
                ComplexValue<sym> const u_ref{input.source[source]};
                ComplexTensor<sym> const y_ref = y_bus.math_model_param().source_param[source];
                output.source[source].i = dot(y_ref, u_ref - output.u[bus]);
                output.source[source].s = output.u[bus] * conj(output.source[source].i);
            }

            // load_gen
            for (Idx load_gen = (*load_gen_bus_indptr_)[bus]; load_gen != (*load_gen_bus_indptr_)[bus + 1];
                 ++load_gen) {
                // power is always quadratic relation to voltage for linear pf
                output.load_gen[load_gen].s = input.s_injection[load_gen] * abs2(output.u[bus]);
                output.load_gen[load_gen].i = conj(output.load_gen[load_gen].s / output.u[bus]);
            }
        }
    }
};

template class LinearPFSolver<true>;
template class LinearPFSolver<false>;

}  // namespace power_grid_model

#endif