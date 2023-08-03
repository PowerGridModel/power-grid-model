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
template <bool sym, typename DerivedSolver>
class IterativePFSolver {
   public:
    friend DerivedSolver;
    MathOutput<sym> run_power_flow(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input, double err_tol,
                                   Idx max_iter, CalculationInfo& calculation_info) {
        // get derived reference for derived solver class
        DerivedSolver& derived_solver = static_cast<DerivedSolver&>(*this);
        IdxVector const& source_bus_indptr = *source_bus_indptr_;
        std::vector<double> const& phase_shift = *phase_shift_;

        // prepare
        MathOutput<sym> output;
        output.u.resize(n_bus_);
        double max_dev = std::numeric_limits<double>::infinity();

        Timer main_timer{calculation_info, 2220, "Math solver"};

        // initialize
        {
            Timer const sub_timer{calculation_info, 2221, "Initialize calculation"};
            // average u_ref of all sources
            DoubleComplex const u_ref = [&]() {
                DoubleComplex sum_u_ref = 0.0;
                for (Idx bus = 0; bus != n_bus_; ++bus) {
                    for (Idx source = source_bus_indptr[bus]; source != source_bus_indptr[bus + 1]; ++source) {
                        sum_u_ref += input.source[source] * std::exp(1.0i * -phase_shift[bus]);  // offset phase shift
                    }
                }
                return sum_u_ref / (double)input.source.size();
            }();

            // assign u_ref as flat start
            for (Idx i = 0; i != n_bus_; ++i) {
                // consider phase shift
                output.u[i] = ComplexValue<sym>{u_ref * std::exp(1.0i * phase_shift[i])};
            }

            // Further initialization specific to the derived solver
            derived_solver.initialize_derived_solver(y_bus, output);
        }

        // start calculation
        // iteration
        Idx num_iter = 0;
        do {
            if (num_iter++ == max_iter) {
                throw IterationDiverge{max_iter, max_dev, err_tol};
            }
            {
                // Prepare the matrices of linear equations to be solved
                Timer const sub_timer{calculation_info, 2222, "Prepare the matrices"};
                derived_solver.prepare_matrix_and_rhs(y_bus, input, output.u);
            }
            {
                // Solve the linear equations
                Timer const sub_timer{calculation_info, 2223, "Solve sparse linear equation"};
                derived_solver.solve_matrix();
            }
            {
                // Calculate maximum deviation of voltage at any bus
                Timer const sub_timer{calculation_info, 2224, "Iterate unknown"};
                max_dev = derived_solver.iterate_unknown(output.u);
            }
        } while (max_dev > err_tol);

        // calculate math result
        {
            Timer const sub_timer{calculation_info, 2225, "Calculate Math Result"};
            calculate_result(y_bus, input, output);
        }
        // Manually stop timers to avoid "Max number of iterations" to be included in the timing.
        main_timer.stop();

        const auto key = Timer::make_key(2226, "Max number of iterations");
        calculation_info[key] = std::max(calculation_info[key], (double)num_iter);

        return output;
    }

    void calculate_result(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input, MathOutput<sym>& output) {
        // pending to correct
        // call y bus
        output.branch = y_bus.template calculate_branch_flow<BranchMathOutput<sym>>(output.u);
        output.shunt = y_bus.template calculate_shunt_flow<ApplianceMathOutput<sym>>(output.u);

        // prepare source, load gen and bus_injection
        output.source.resize(source_bus_indptr_->back());
        output.load_gen.resize(load_gen_bus_indptr_->back());
        output.bus_injection.resize(n_bus_);

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
                    using enum LoadGenType;

                    case const_pq:
                        // always same power
                        output.load_gen[load_gen].s = input.s_injection[load_gen];
                        break;
                    case const_y:
                        // power is quadratic relation to voltage
                        output.load_gen[load_gen].s =
                            input.s_injection[load_gen] * cabs(output.u[bus]) * cabs(output.u[bus]);
                        break;
                    case const_i:
                        // power is linear relation to voltage
                        output.load_gen[load_gen].s = input.s_injection[load_gen] * cabs(output.u[bus]);
                        break;
                    default:
                        throw MissingCaseForEnumError("Power injection", type);
                }
                output.load_gen[load_gen].i = conj(output.load_gen[load_gen].s / output.u[bus]);
            }
        }
        output.bus_injection = y_bus.calculate_injection(output.u);
    }

   private:
    Idx n_bus_;
    std::shared_ptr<DoubleVector const> phase_shift_;
    std::shared_ptr<IdxVector const> load_gen_bus_indptr_;
    std::shared_ptr<IdxVector const> source_bus_indptr_;
    std::shared_ptr<std::vector<LoadGenType> const> load_gen_type_;
    IterativePFSolver(YBus<sym> const& y_bus, std::shared_ptr<MathModelTopology const> const& topo_ptr)
        : n_bus_{y_bus.size()},
          phase_shift_{topo_ptr, &topo_ptr->phase_shift},
          load_gen_bus_indptr_{topo_ptr, &topo_ptr->load_gen_bus_indptr},
          source_bus_indptr_{topo_ptr, &topo_ptr->source_bus_indptr},
          load_gen_type_{topo_ptr, &topo_ptr->load_gen_type} {
    }
};

}  // namespace math_model_impl

}  // namespace power_grid_model

#endif