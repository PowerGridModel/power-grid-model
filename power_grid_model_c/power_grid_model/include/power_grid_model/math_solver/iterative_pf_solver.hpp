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
#include "../calculation_state.hpp"
#include "../common/common.hpp"
#include "../common/enum.hpp"
#include "../common/exception.hpp"
#include "../common/grouped_index_vector.hpp"
#include "../common/logging.hpp"
#include "../common/timer.hpp"

#include <functional>
#include <limits>
#include <vector>

namespace power_grid_model::math_solver {

// solver
template <symmetry_tag sym, typename DerivedSolver> class IterativePFSolver {
  public:
    friend DerivedSolver;
    void log_matrix(Logger& /*log*/, YBus<sym> const& /*y_bus*/, Idx /*iter*/) {}
    void copy_iteration_state_from(DerivedSolver const& solver) {
        nodal_states_ = static_cast<IterativePFSolver const&>(solver).nodal_states_;
    }

    void set_nodal_state_capture(bool capture) {
        capture_nodal_state_ = capture;
        nodal_states_.clear();
    }

    std::vector<ModelStateNodalState> get_nodal_states() const { return nodal_states_; }

    SolverOutput<sym> run_power_flow(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input, double err_tol,
                                     Idx max_iter, bool cache_run, Logger& log) {
        // get derived solver class
        auto derived_solver = static_cast<DerivedSolver&>(*this);

        // prepare
        SolverOutput<sym> output;
        output.u.resize(n_bus_);
        double max_dev = std::numeric_limits<double>::infinity();

        Timer main_timer{log, LogEvent::math_solver};

        // initialize
        {
            Timer const sub_timer{log, LogEvent::initialize_calculation};
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
                Timer const sub_timer{log, LogEvent::prepare_matrices};
                derived_solver.prepare_matrix_and_rhs(y_bus, input, output.u);
            }
            derived_solver.log_matrix(log, y_bus, num_iter);
            {
                // Solve the linear equations
                Timer const sub_timer{log, LogEvent::solve_sparse_linear_equation};
                derived_solver.solve_matrix();
            }
            {
                // Calculate maximum deviation of voltage at any bus
                Timer const sub_timer{log, LogEvent::iterate_unknown};
                max_dev = derived_solver.iterate_unknown(output.u, err_tol, cache_run);
            }
            if (capture_nodal_state_) {
                derived_solver.capture_iteration_nodal_state(output.u, num_iter);
            }
            log.log(LogEvent::iterate_unknown, [num_iter, max_dev] {
                return std::format("Iteration {:3}: max voltage deviation = {:.6e} p.u.", num_iter, max_dev);
            });
        }

        // calculate math result
        {
            Timer const sub_timer{log, LogEvent::calculate_math_result};
            if constexpr (requires { derived_solver.finalize_result(input, output); }) {
                derived_solver.finalize_result(input, output);
            }
            calculate_result(y_bus, input, output);
        }
        static_cast<DerivedSolver&>(*this).copy_iteration_state_from(derived_solver);
        // Manually stop timers to avoid "Max number of iterations" to be included in the timing.
        main_timer.stop();

        log.log(LogEvent::iterative_pf_solver_max_num_iter, num_iter);

        return output;
    }

    void calculate_result(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input, SolverOutput<sym>& output) {
        detail::calculate_pf_result(y_bus, input, sources_per_bus_.get(), load_gens_per_bus_.get(), output,
                                    [this](Idx i) { return (load_gen_type_.get())[i]; });
    }

    // Average reference voltage of all sources, with the topological phase shift of the source buses offset
    DoubleComplex average_source_voltage(PowerFlowInput<sym> const& input) const {
        std::vector<double> const& phase_shift = phase_shift_.get();
        DoubleComplex sum_u_ref = 0.0;
        for (auto const& [bus, sources] : enumerated_zip_sequence(sources_per_bus_.get())) {
            for (Idx const source : sources) {
                sum_u_ref += input.source[source] * std::exp(1.0i * -phase_shift[bus]); // offset phase shift
            }
        }
        return sum_u_ref / static_cast<double>(input.source.size());
    }

    // Initialize every bus to the average reference voltage of all sources, with the topological phase shift of each
    // bus accounted for
    void make_average_source_start(PowerFlowInput<sym> const& input, ComplexValueVector<sym>& output_u) const {
        std::vector<double> const& phase_shift = phase_shift_.get();
        DoubleComplex const u_ref = average_source_voltage(input);
        for (Idx i = 0; i != n_bus_; ++i) {
            // consider phase shift
            output_u[i] = ComplexValue<sym>{u_ref * std::exp(1.0i * phase_shift[i])};
        }
    }

    bool make_input_voltage_start(PowerFlowInput<sym> const& input, ComplexValueVector<sym>& output_u) const {
        if (input.initial_voltage.empty()) {
            return false;
        }
        if (std::ssize(input.initial_voltage) != n_bus_) {
            throw InvalidArguments{"Power-flow initial voltage vector must match the number of buses."};
        }
        for (Idx i = 0; i != n_bus_; ++i) {
            if (!input.initial_voltage[i]) {
                return false;
            }
        }
        for (Idx i = 0; i != n_bus_; ++i) {
            output_u[i] = *input.initial_voltage[i];
        }
        return true;
    }

    void apply_input_voltage_start(PowerFlowInput<sym> const& input, ComplexValueVector<sym>& output_u) const {
        if (input.initial_voltage.empty()) {
            return;
        }
        if (std::ssize(input.initial_voltage) != n_bus_) {
            throw InvalidArguments{"Power-flow initial voltage vector must match the number of buses."};
        }
        for (Idx i = 0; i != n_bus_; ++i) {
            if (input.initial_voltage[i]) {
                output_u[i] = *input.initial_voltage[i];
            }
        }
    }

  private:
    void capture_iteration_nodal_state(ComplexValueVector<sym> const& voltage, Idx iteration) {
        ModelStateNodalState state;
        state.iteration = iteration;
        constexpr size_t phases_per_bus = is_symmetric_v<sym> ? 1 : 3;
        state.voltage_magnitude.reserve(voltage.size() * phases_per_bus);
        state.voltage_angle.reserve(voltage.size() * phases_per_bus);
        for (auto const& bus_voltage : voltage) {
            if constexpr (is_symmetric_v<sym>) {
                state.voltage_magnitude.push_back(cabs(bus_voltage));
                state.voltage_angle.push_back(arg(bus_voltage));
            } else {
                for (Idx phase = 0; phase < 3; ++phase) {
                    state.voltage_magnitude.push_back(cabs(bus_voltage.coeff(phase)));
                    state.voltage_angle.push_back(arg(bus_voltage.coeff(phase)));
                }
            }
        }
        nodal_states_.push_back(std::move(state));
    }

    Idx n_bus_;
    std::reference_wrapper<DoubleVector const> phase_shift_;
    std::reference_wrapper<SparseGroupedIdxVector const> load_gens_per_bus_;
    std::reference_wrapper<DenseGroupedIdxVector const> sources_per_bus_;
    std::reference_wrapper<std::vector<LoadGenType> const> load_gen_type_;
    bool capture_nodal_state_{};
    std::vector<ModelStateNodalState> nodal_states_;
    IterativePFSolver(YBus<sym> const& y_bus, MathModelTopology const& topo)
        : n_bus_{y_bus.size()},
          phase_shift_{std::cref(topo.phase_shift)},
          load_gens_per_bus_{std::cref(topo.load_gens_per_bus)},
          sources_per_bus_{std::cref(topo.sources_per_bus)},
          load_gen_type_{std::cref(topo.load_gen_type)} {}
};

} // namespace power_grid_model::math_solver
