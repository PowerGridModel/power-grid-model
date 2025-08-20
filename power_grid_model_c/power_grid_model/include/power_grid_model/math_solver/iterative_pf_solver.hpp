// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

/*
 * Class to house common functions of newton raphson and iterative current method
 */

// Check if all includes needed
#include "common_solver_functions.hpp"
#include "y_bus.hpp"

#include "../calculation_parameters.hpp"
#include "../common/common.hpp"
#include "../common/exception.hpp"
#include "../common/three_phase_tensor.hpp"
#include "../common/timer.hpp"

namespace power_grid_model::math_solver {

// solver
template <symmetry_tag sym, typename DerivedSolver> class IterativePFSolver {
  public:
    friend DerivedSolver;
    SolverOutput<sym> run_power_flow(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input, double err_tol,
                                     Idx max_iter, CalculationInfo& calculation_info) {
        // get derived reference for derived solver class
        auto derived_solver = static_cast<DerivedSolver&>(*this);

        // prepare
        SolverOutput<sym> output;
        output.u.resize(n_bus_);
        double max_dev = std::numeric_limits<double>::infinity();

        Timer main_timer{calculation_info, LogEvent::math_solver};

        // initialize
        {
            Timer const sub_timer{calculation_info, LogEvent::initialize_calculation};
            // Further initialization specific to the derived solver
            derived_solver.initialize_derived_solver(y_bus, input, output);
        }

        // start calculation
        // iteration
        Idx num_iter = 0;
        while (max_dev > err_tol || num_iter == 0) {
            if (num_iter++ == max_iter) {
                throw IterationDiverge{max_iter, max_dev, err_tol};
            }
            {
                // Prepare the matrices of linear equations to be solved
                Timer const sub_timer{calculation_info, LogEvent::prepare_matrices};
                derived_solver.prepare_matrix_and_rhs(y_bus, input, output.u);
            }
            {
                // Solve the linear equations
                Timer const sub_timer{calculation_info, LogEvent::solve_sparse_linear_equation};
                derived_solver.solve_matrix();
            }
            {
                // Calculate maximum deviation of voltage at any bus
                Timer const sub_timer{calculation_info, LogEvent::iterate_unknown};
                max_dev = derived_solver.iterate_unknown(output.u);
            }
        }

        // calculate math result
        {
            Timer const sub_timer{calculation_info, LogEvent::calculate_math_result};
            calculate_result(y_bus, input, output);
        }
        // Manually stop timers to avoid "Max number of iterations" to be included in the timing.
        main_timer.stop();

        calculation_info[LogEvent::iterative_pf_solver_max_num_iter] =
            std::max(calculation_info[LogEvent::iterative_pf_solver_max_num_iter], static_cast<double>(num_iter));

        return output;
    }

    void calculate_result(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input, SolverOutput<sym>& output) {
        detail::calculate_pf_result(y_bus, input, *sources_per_bus_, *load_gens_per_bus_, output,
                                    [this](Idx i) { return (*load_gen_type_)[i]; });
    }

  private:
    Idx n_bus_;
    std::shared_ptr<DoubleVector const> phase_shift_;
    std::shared_ptr<SparseGroupedIdxVector const> load_gens_per_bus_;
    std::shared_ptr<DenseGroupedIdxVector const> sources_per_bus_;
    std::shared_ptr<std::vector<LoadGenType> const> load_gen_type_;
    IterativePFSolver(YBus<sym> const& y_bus, std::shared_ptr<MathModelTopology const> const& topo_ptr)
        : n_bus_{y_bus.size()},
          phase_shift_{topo_ptr, &topo_ptr->phase_shift},
          load_gens_per_bus_{topo_ptr, &topo_ptr->load_gens_per_bus},
          sources_per_bus_{topo_ptr, &topo_ptr->sources_per_bus},
          load_gen_type_{topo_ptr, &topo_ptr->load_gen_type} {}
};

} // namespace power_grid_model::math_solver
