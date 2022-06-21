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
#include "y_bus.hpp"
// #include "newton_raphson_pf_solver.hpp"
// #include "iterative_linear_se_solver.hpp"

namespace power_grid_model {

// hide implementation in inside namespace
namespace math_model_impl {

// solver
template <bool sym, typename DerivedSolver>
class IterativePFSolver {
   public:
    


    MathOutput<sym> run_power_flow(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input, double err_tol,
                                    Idx max_iter, CalculationInfo& calculation_info) {
        // get derived pointer for this
        DerivedSolver& derived_solver = static_cast<DerivedSolver&>(*this);
        // Extra variables?
        IdxVector const& source_bus_indptr = *this->source_bus_indptr_;
        std::vector<double> const& phase_shift = *this->phase_shift_;

        // prepare
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
        
        // assign u_ref as flat start
        for (Idx i = 0; i != n_bus_; ++i) {
            // consider phase shift
            output.u[i] = ComplexValue<sym>{u_ref * std::exp(1.0i * this->phase_shift_[i])};
        }

        derived_solver.initialize_unknown_polar(output);       // polar for NR complex nothing for iterative
        sub_timer.stop();

        derived_solver.initialize_matrix(y_bus);  // nothing for NR, factorize for iterative

        // start calculation
        // iteration
        Idx num_iter = 0;
        while (max_dev > err_tol) {
            if (num_iter++ == max_iter) {
                throw IterationDiverge{max_iter, max_dev, err_tol};
            }
            sub_timer = Timer(calculation_info, 2222, "Calculate jacobian and rhs");
            derived_solver.prepare_matrix_rhs(y_bus, input, output.u);  // jacobian for NR and injected for iterative current
            sub_timer = Timer(calculation_info, 2223, "Solve sparse linear equation");
            derived_solver.solve_matrix();  // bsr_solve for both
            sub_timer = Timer(calculation_info, 2224, "Iterate unknown");
            max_dev = derived_solver.iterate_unknown(output.u);  // compare both iterate unknowns use virtual-like
            sub_timer.stop();
        }

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


    /*
    // Not virtual but use from derived
    virtual void initialize_unknown();
    virtual void initialize_matrix();
    virtual void prepare_matrix_rhs();
    virtual void solve_matrix();
    virtual void iterate_unkown();
    virtual void ~IterativePFSolver();
    
    MathOutput<sym> run_power_flow(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input, double err_tol,
                                   Idx max_iter, CalculationInfo& calculation_info) {


        // intitialize_unknown
        // assign u_ref as flat start
        for (Idx i = 0; i != n_bus; ++i) {
            // consider phase shift
            output.u[i] = ComplexValue<sym>{u_ref * std::exp(1.0i * phase_shift[i])};
            x_[i].v = cabs(output.u[i]);
            x_[i].theta = arg(output.u[i]);
        }



        while (max_dev > err_tol) {
            if (num_iter++ == max_iter) {
                throw IterationDiverge{max_iter, max_dev, err_tol};
            }
            sub_timer = Timer(calculation_info, 2222, "Calculate jacobian and rhs");
            calculate_jacobian_and_deviation(y_bus, input, output.u);
            sub_timer = Timer(calculation_info, 2223, "Solve sparse linear equation");
            bsr_solver_.solve(data_jac_.data(), del_pq_.data(), del_x_.data());
            sub_timer = Timer(calculation_info, 2224, "Iterate unknown");
            max_dev = iterate_unknown(output.u);
            sub_timer.stop();
        }

    }
    */

    /*
    Idx get_n_bus() {
        return n_bus_;
    }
    std::shared_ptr<DoubleVector const> get_phase_shift() {
        return phase_shift_;
    }
    std::shared_ptr<IdxVector const> get_load_gen_bus_indptr() {
        return load_gen_bus_indptr_;
    }
    std::shared_ptr<IdxVector const> get_source_bus_indptr() {
        return source_bus_indptr_;
    }
    std::shared_ptr<std::vector<LoadGenType> const> get_load_gen_type() {
        return load_gen_type_;
    }
    */
    

   protected:
    Idx n_bus_;
    std::shared_ptr<DoubleVector const> phase_shift_;
    std::shared_ptr<IdxVector const> load_gen_bus_indptr_;
    std::shared_ptr<IdxVector const> source_bus_indptr_;
    std::shared_ptr<std::vector<LoadGenType> const> load_gen_type_;

   private:
    IterativePFSolver(YBus<sym> const& y_bus, std::shared_ptr<MathModelTopology const> const& topo_ptr)
        : n_bus_{y_bus.size()},
          phase_shift_{topo_ptr, &topo_ptr->phase_shift},
          load_gen_bus_indptr_{topo_ptr, &topo_ptr->load_gen_bus_indptr},
          source_bus_indptr_{topo_ptr, &topo_ptr->source_bus_indptr},
          load_gen_type_{topo_ptr, &topo_ptr->load_gen_type} {
    }
    friend DerivedSolver;

};

// template class IterativePFSolver<true>;
// template class IterativePFSolver<false>;



}  // namespace math_model_impl

template <bool sym, typename DerivedSolver>
using IterativePFSolver = math_model_impl::IterativePFSolver<sym, DerivedSolver>;

}  // namespace power_grid_model

#endif