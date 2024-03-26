// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

/*
 * Class to house common functions of newton raphson and iterative current method
 */

// Check if all includes needed
#include "common_solver_functions.hpp"
#include "sparse_lu_solver.hpp"
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
    using LinearSparseSolverType = SparseLUSolver<ComplexTensor<sym>, ComplexValue<sym>, ComplexValue<sym>>;
    using LinearBlockPermArray = typename LinearSparseSolverType::BlockPermArray;

    friend DerivedSolver;

    MathOutput<sym> run_power_flow(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input, double err_tol,
                                   Idx max_iter, CalculationInfo& calculation_info) {
        // get derived reference for derived solver class
        auto derived_solver = static_cast<DerivedSolver&>(*this);

        // prepare
        MathOutput<sym> output;
        output.u.resize(n_bus_);
        double max_dev = std::numeric_limits<double>::infinity();

        Timer main_timer{calculation_info, 2220, "Math solver"};

        // initialize
        {
            Timer const sub_timer{calculation_info, 2221, "Initialize calculation"};
            make_flat_start(input, output.u);

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
        }

        // calculate math result
        {
            Timer const sub_timer{calculation_info, 2225, "Calculate math result"};
            calculate_result(y_bus, input, output);
        }
        // Manually stop timers to avoid "Max number of iterations" to be included in the timing.
        main_timer.stop();

        auto const key = Timer::make_key(2226, "Max number of iterations");
        calculation_info[key] = std::max(calculation_info[key], (double)num_iter);

        return output;
    }

    void calculate_result(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input, MathOutput<sym>& output) {
        detail::calculate_pf_result(y_bus, input, *sources_per_bus_, *load_gens_per_bus_, output,
                                    [this](Idx i) { return (*load_gen_type_)[i]; });
    }

    void parameters_changed(bool changed) { parameters_changed_ = parameters_changed_ || changed; }

    void prefactorize_linear_lhs(const YBus<sym>& y_bus) {
        // if Y bus is not up to date
        // re-build matrix and prefactorize Build y bus data with source admittance
        if (!parameters_changed_) {
            return;
        }
        ComplexTensorVector<sym> mat_data(y_bus.nnz_lu());
        detail::copy_y_bus<sym>(y_bus, mat_data);
        add_sources_linear_lhs(y_bus, mat_data);
        // prefactorize
        LinearBlockPermArray perm(n_bus_);
        linear_sparse_solver_.prefactorize(mat_data, perm);
        // move pre-factorized version into shared ptr
        linear_mat_data_ = std::make_shared<ComplexTensorVector<sym> const>(std::move(mat_data));
        linear_perm_ = std::make_shared<LinearBlockPermArray const>(std::move(perm));
        parameters_changed_ = false;
    }

    void make_flat_start(PowerFlowInput<sym> const& input, ComplexValueVector<sym>& output_u) {
        std::vector<double> const& phase_shift = *phase_shift_;
        // average u_ref of all sources
        DoubleComplex const u_ref = [&]() {
            DoubleComplex sum_u_ref = 0.0;
            for (auto const& [bus, sources] : enumerated_zip_sequence(*sources_per_bus_)) {
                for (Idx const source : sources) {
                    sum_u_ref += input.source[source] * std::exp(1.0i * -phase_shift[bus]); // offset phase shift
                }
            }
            return sum_u_ref / static_cast<double>(input.source.size());
        }();

        // assign u_ref as flat start
        for (Idx i = 0; i != n_bus_; ++i) {
            // consider phase shift
            output_u[i] = ComplexValue<sym>{u_ref * std::exp(1.0i * phase_shift[i])};
        }
    }

    void add_sources_linear_lhs(const YBus<sym>& y_bus, ComplexTensorVector<sym>& mat_data) {
        IdxVector const& bus_entry = y_bus.lu_diag();
        for (auto const& [bus_number, sources] : enumerated_zip_sequence(*sources_per_bus_)) {
            Idx const data_sequence = bus_entry[bus_number];
            for (auto source_number : sources) {
                // YBus_diag += Y_source
                mat_data[data_sequence] += y_bus.math_model_param().source_param[source_number];
            }
        }
    }

    void add_sources_linear_rhs(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input) {
        for (auto const& [bus_number, sources] : enumerated_zip_sequence(*sources_per_bus_)) {
            for (Idx const source_number : sources) {
                // I_inj_i += Y_source_j * U_ref_j
                linear_rhs_[bus_number] += dot(y_bus.math_model_param().source_param[source_number],
                                               ComplexValue<sym>{input.source[source_number]});
            }
        }
    }

    void add_loads_linear_rhs(PowerFlowInput<sym> const& input, ComplexValueVector<sym> const& u) {
        std::vector<LoadGenType> const& load_gen_type = *this->load_gen_type_;
        for (auto const& [bus_number, load_gens] : enumerated_zip_sequence(*this->load_gens_per_bus_)) {
            for (Idx const load_number : load_gens) {
                // load type
                LoadGenType const type = load_gen_type[load_number];
                switch (type) {
                    using enum LoadGenType;

                case const_pq:
                    // I_inj_i = conj(S_inj_j/U_i) for constant PQ type
                    linear_rhs_[bus_number] += conj(input.s_injection[load_number] / u[bus_number]);
                    break;
                case const_y:
                    // I_inj_i = conj((S_inj_j * abs(U_i)^2) / U_i) = conj((S_inj_j) * U_i for const impedance type
                    linear_rhs_[bus_number] += conj(input.s_injection[load_number]) * u[bus_number];
                    break;
                case const_i:
                    // I_inj_i = conj(S_inj_j*abs(U_i)/U_i) for const current type
                    linear_rhs_[bus_number] +=
                        conj(input.s_injection[load_number] * cabs(u[bus_number]) / u[bus_number]);
                    break;
                default:
                    throw MissingCaseForEnumError("Injection current calculation", type);
                }
            }
        }
    }

  private:
    Idx n_bus_;
    std::shared_ptr<DoubleVector const> phase_shift_;
    std::shared_ptr<SparseGroupedIdxVector const> load_gens_per_bus_;
    std::shared_ptr<DenseGroupedIdxVector const> sources_per_bus_;
    std::shared_ptr<std::vector<LoadGenType> const> load_gen_type_;
    bool parameters_changed_ = true;
    // Linear solver for initialization
    std::shared_ptr<ComplexTensorVector<sym> const> linear_mat_data_;
    ComplexValueVector<sym> linear_rhs_;
    LinearSparseSolverType linear_sparse_solver_;
    std::shared_ptr<LinearBlockPermArray const> linear_perm_;
    IterativePFSolver(YBus<sym> const& y_bus, std::shared_ptr<MathModelTopology const> const& topo_ptr)
        : n_bus_{y_bus.size()},
          phase_shift_{topo_ptr, &topo_ptr->phase_shift},
          load_gens_per_bus_{topo_ptr, &topo_ptr->load_gens_per_bus},
          sources_per_bus_{topo_ptr, &topo_ptr->sources_per_bus},
          load_gen_type_{topo_ptr, &topo_ptr->load_gen_type},
          linear_sparse_solver_{y_bus.shared_indptr_lu(), y_bus.shared_indices_lu(), y_bus.shared_diag_lu()},
          linear_rhs_(n_bus_) {}
};

} // namespace power_grid_model::math_solver
