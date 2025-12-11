// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// In this unit test the powerflow solvers are tested

#pragma once

#include "test_math_solver_common.hpp"

#include <power_grid_model/common/dummy_logging.hpp>
#include <power_grid_model/math_solver/sparse_lu_solver.hpp>
#include <power_grid_model/math_solver/y_bus.hpp>

namespace power_grid_model {
template <typename SolverType>
inline auto run_power_flow(SolverType& solver, YBus<typename SolverType::sym> const& y_bus,
                           PowerFlowInput<typename SolverType::sym> const& input, double err_tol, Idx max_iter,
                           Logger& log) {
    if constexpr (SolverType::is_iterative) {
        return solver.run_power_flow(y_bus, input, err_tol, max_iter, log);
    } else {
        return solver.run_power_flow(y_bus, input, log);
    }
};

template <symmetry_tag sym_type> struct PFSolverTestGrid : public SteadyStateSolverTestGrid<sym_type> {
    using sym = sym_type;

    auto sym_s_inj() const {
        auto const s0_load_injection = this->s0_load_inj;
        auto const s1_load_injection = this->s1_load_inj;
        auto const v_0 = this->v0;
        auto const v_1 = this->v1;
        return ComplexValueVector<symmetric_t>{s0_load_injection / 3.0,
                                               s0_load_injection / 3.0 / v_0,
                                               s0_load_injection / 3.0 / v_0 / v_0,
                                               s1_load_injection / 3.0,
                                               s1_load_injection / 3.0 / v_1,
                                               s1_load_injection / 3.0 / v_1 / v_1,
                                               0.0};
    }

    auto pf_input() const {
        PowerFlowInput<sym> result;

        auto sym_s_injection = sym_s_inj();

        result.source = {this->vref};
        if constexpr (is_symmetric_v<sym>) {
            result.s_injection = std::move(sym_s_injection);
        } else {
            result.s_injection.resize(sym_s_injection.size());
            for (size_t i = 0; i < sym_s_injection.size(); i++) {
                result.s_injection[i] = RealValue<asymmetric_t>{real(sym_s_injection[i])} +
                                        1.0i * RealValue<asymmetric_t>{imag(sym_s_injection[i])};
            }
        }
        return result;
    }

    auto pf_input_z() const {
        PowerFlowInput<sym> result{this->pf_input()};
        for (size_t i = 0; i < 6; i++) {
            if (i % 3 == 2) {
                result.s_injection[i] *= 3.0;
            } else {
                result.s_injection[i] = ComplexValue<sym>{0.0};
            }
        }
        return result;
    }
};

TEST_CASE_TEMPLATE_DEFINE("Test math solver - PF", SolverType, test_math_solver_pf_id) {
    using sym = typename SolverType::sym;
    using common::logging::NoLogger;

    PFSolverTestGrid<sym> const grid;

    // topo and param ptr
    auto param_ptr = std::make_shared<MathModelParam<sym> const>(grid.param());
    auto topo_ptr = std::make_shared<MathModelTopology const>(grid.topo());
    YBus<sym> y_bus{topo_ptr, param_ptr};

    SUBCASE("Test pf solver") {
        constexpr auto error_tolerance{1e-12};
        constexpr auto num_iter{20};
        constexpr auto result_tolerance =
            SolverType::is_iterative ? 1e-12 : 0.15; // linear methods may be very inaccurate

        SolverType solver{y_bus, topo_ptr};
        NoLogger log;

        PowerFlowInput<sym> const pf_input = grid.pf_input();
        SolverOutput<sym> const output = run_power_flow(solver, y_bus, pf_input, error_tolerance, num_iter, log);
        assert_output(output, grid.output_ref(), false, result_tolerance);
    }

    SUBCASE("Test const z pf solver") {
        SolverType solver{y_bus, topo_ptr};
        NoLogger log;

        // const z
        PowerFlowInput<sym> const pf_input_z = grid.pf_input_z();
        SolverOutput<sym> const output = run_power_flow(solver, y_bus, pf_input_z, 1e-12, 20, log);
        assert_output(output, grid.output_ref_z()); // for const z, all methods (including linear) should be accurate
    }

    if constexpr (SolverType::is_iterative) {
        SUBCASE("Test pf solver with single iteration") {
            // low precision
            constexpr auto error_tolerance{std::numeric_limits<double>::infinity()};
            constexpr auto result_tolerance{0.15};

            SolverType solver{y_bus, topo_ptr};
            NoLogger log;

            PowerFlowInput<sym> const pf_input = grid.pf_input();
            SolverOutput<sym> const output = run_power_flow(solver, y_bus, pf_input, error_tolerance, 1, log);
            assert_output(output, grid.output_ref(), false, result_tolerance);
        }
        SUBCASE("Test not converge") {
            SolverType solver{y_bus, topo_ptr};
            NoLogger log;

            PowerFlowInput<sym> pf_input = grid.pf_input();
            pf_input.s_injection[6] = ComplexValue<sym>{1e6};
            CHECK_THROWS_AS(run_power_flow(solver, y_bus, pf_input, 1e-12, 20, log), IterationDiverge);
        }
    }

    SUBCASE("Test singular ybus") {
        auto singular_param = grid.param();
        singular_param.branch_param[0] = BranchCalcParam<sym>{};
        singular_param.branch_param[1] = BranchCalcParam<sym>{};
        singular_param.shunt_param[0] = ComplexTensor<sym>{};
        y_bus.update_admittance(std::make_shared<MathModelParam<sym> const>(singular_param));
        SolverType solver{y_bus, topo_ptr};
        NoLogger log;

        PowerFlowInput<sym> const pf_input = grid.pf_input();
        CHECK_THROWS_AS(run_power_flow(solver, y_bus, pf_input, 1e-12, 20, log), SparseMatrixError);
    }
}

} // namespace power_grid_model
