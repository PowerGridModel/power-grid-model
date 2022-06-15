// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MATH_SOLVER_ITERATIVE_PF_SOLVER_HPP
#define POWER_GRID_MODEL_MATH_SOLVER_ITERATIVE_PF_SOLVER_HPP

/*
 * Class to house common functions of newton raphson and iterative current method
 */

// Check if all includes needed
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
class IterativePFSolver {

   public:
    IterativePFSolver(YBus<sym> const& y_bus, std::shared_ptr<MathModelTopology const> const& topo_ptr)
        : n_bus_{y_bus.size()},
          phase_shift_{topo_ptr, &topo_ptr->phase_shift},
          load_gen_bus_indptr_{topo_ptr, &topo_ptr->load_gen_bus_indptr},
          source_bus_indptr_{topo_ptr, &topo_ptr->source_bus_indptr},
          load_gen_type_{topo_ptr, &topo_ptr->load_gen_type} {
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
                ComplexValue<sym> const u_ref{input.source[source]};
                ComplexTensor<sym> const y_ref = y_bus.math_model_param().source_param[source];
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
                        output.load_gen[load_gen].s =
                            input.s_injection[load_gen] * cabs(output.u[bus]) * cabs(output.u[bus]);
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

    virtual MathOutput<sym> run_power_flow(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input, double err_tol,
                                           Idx max_iter, CalculationInfo& calculation_info) = 0;


   public:
    Idx n_bus_;
    std::shared_ptr<DoubleVector const> phase_shift_;
    std::shared_ptr<IdxVector const> load_gen_bus_indptr_;
    std::shared_ptr<IdxVector const> source_bus_indptr_;
    std::shared_ptr<std::vector<LoadGenType> const> load_gen_type_;
    


};

template class IterativePFSolver<true>;
template class IterativePFSolver<false>;

}  // namespace math_model_impl

template <bool sym>
using IterativePFSolver = math_model_impl::IterativePFSolver<sym>;

}  // namespace power_grid_model

#endif