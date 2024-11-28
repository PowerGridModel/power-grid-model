// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// In this unit test the powerflow solvers are tested

#include "test_math_solver_common.hpp"

#include <power_grid_model/common/calculation_info.hpp>
#include <power_grid_model/math_solver/sparse_lu_solver.hpp>
#include <power_grid_model/math_solver/y_bus.hpp>

namespace power_grid_model {
template <typename SolverType>
inline auto run_power_flow(SolverType& solver, YBus<typename SolverType::sym> const& y_bus,
                           PowerFlowInput<typename SolverType::sym> const& input, double err_tol, Idx max_iter,
                           CalculationInfo& calculation_info) {
    if constexpr (SolverType::is_iterative) {
        return solver.run_power_flow(y_bus, input, err_tol, max_iter, calculation_info);
    } else {
        return solver.run_power_flow(y_bus, input, calculation_info);
    }
};

TEST_CASE_TEMPLATE_DEFINE("Test math solver - PF", SolverType, test_math_solver_pf_id) {
    /*
    network, v means voltage measured, p means power measured, pp means double measured
    variance always 1.0
                                                          shunt0 (ys) (p)
     (pp)                     (y0, ys0)           (y1)         |
    source --yref-- bus0(vp) -p-branch0-pp- bus1 --branch1-p-  bus2(vv)
                     |                      |                   |
                  load012                load345 (p)          load6 (not connected) (p, rubbish value)
                                          for const z,
                                       rubbish value for load3/4

    uref = 1.10
    u0 = 1.08 -1deg
    u1 = 0.97 -4deg
    u2 = 0.90 -37deg
    */
    using sym = SolverType::sym;

    // build topo
    double const shift_val = deg_30;
    MathModelTopology topo;
    topo.slack_bus = 0;
    topo.phase_shift = {0.0, 0.0, -shift_val};
    topo.branch_bus_idx = {{0, 1}, {1, 2}};
    topo.sources_per_bus = {from_sparse, {0, 1, 1, 1}};
    topo.shunts_per_bus = {from_sparse, {0, 0, 0, 1}};
    topo.load_gens_per_bus = {from_sparse, {0, 3, 6, 7}};
    topo.load_gen_type = {
        LoadGenType::const_pq, LoadGenType::const_i, LoadGenType::const_y,
        LoadGenType::const_pq, LoadGenType::const_i, LoadGenType::const_y,
        LoadGenType::const_pq // not connected
    };
    topo.voltage_sensors_per_bus = {from_sparse, {0, 1, 1, 3}};
    topo.power_sensors_per_bus = {from_sparse, {0, 1, 1, 1}};
    topo.power_sensors_per_source = {from_sparse, {0, 2}};
    topo.power_sensors_per_load_gen = {from_sparse, {0, 0, 0, 0, 1, 2, 3, 4}};
    topo.power_sensors_per_shunt = {from_sparse, {0, 1}};
    topo.power_sensors_per_branch_from = {from_sparse, {0, 1, 1}};
    topo.power_sensors_per_branch_to = {from_sparse, {0, 2, 3}};

    // build param, pf input, output, backwards
    // voltage
    double const vref = 1.1;
    double const v0 = 1.08;
    double const v1 = 0.97;
    double const v2 = 0.90;
    constexpr double deg = deg_30 / 30.0;
    DoubleComplex const u0 = v0 * std::exp(-1.0i * deg);
    DoubleComplex const u1 = v1 * std::exp(-4.0i * deg);
    DoubleComplex const u2 = v2 * std::exp(-37.0i * deg);
    // branch
    DoubleComplex const shift = std::exp(1.0i * shift_val);
    DoubleComplex const y0 = 1.0 - 2.0i;
    DoubleComplex const ys0 = 0.05 + 0.2i;
    DoubleComplex const y1 = 3.0 - 4.0i;
    DoubleComplex const branch0_i_f = (u0 - u1) * y0 + u0 * ys0;
    DoubleComplex const branch0_i_t = (u1 - u0) * y0 + u1 * ys0;
    DoubleComplex const branch1_i_f = (u1 - u2 * shift) * y1;
    DoubleComplex const branch1_i_t = (u2 - u1 * conj(shift)) * y1;
    // source input
    DoubleComplex const uref = vref;
    DoubleComplex const yref = 10.0 - 50.0i;
    DoubleComplex const source_inj = yref * (uref - u0);
    // injection of bus0 and bus1
    DoubleComplex const i0_load_inj = -source_inj + branch0_i_f;
    DoubleComplex const i1_load_inj = branch0_i_t + branch1_i_f;
    DoubleComplex const s0_load_inj = conj(i0_load_inj) * u0;
    DoubleComplex const s1_load_inj = conj(i1_load_inj) * u1;
    // injection of shunt0 at bus2
    DoubleComplex const i2_shunt_inj = branch1_i_t;
    DoubleComplex const ys = -i2_shunt_inj / u2;

    PowerFlowInput<sym> pf_input = [&] {
        PowerFlowInput<sym> result;
        ComplexValueVector<symmetric_t> sym_s_injection = {s0_load_inj / 3.0,
                                                           s0_load_inj / 3.0 / v0,
                                                           s0_load_inj / 3.0 / v0 / v0,
                                                           s1_load_inj / 3.0,
                                                           s1_load_inj / 3.0 / v1,
                                                           s1_load_inj / 3.0 / v1 / v1,
                                                           0.0};

        result.source = {vref};
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
    }();

    // output
    auto const get_sym_output_ref = [&] {
        SolverOutput<symmetric_t> result;
        result.u = {u0, u1, u2};
        // branch result
        result.branch.resize(2);
        result.branch[0].i_f = branch0_i_f;
        result.branch[0].i_t = branch0_i_t;
        result.branch[0].s_f = conj(result.branch[0].i_f) * u0;
        result.branch[0].s_t = conj(result.branch[0].i_t) * u1;
        result.branch[1].i_f = branch1_i_f;
        result.branch[1].i_t = branch1_i_t;
        result.branch[1].s_f = conj(result.branch[1].i_f) * u1;
        result.branch[1].s_t = conj(result.branch[1].i_t) * u2;
        // source result
        result.source.resize(1);
        result.source[0].i = source_inj;
        result.source[0].s = conj(result.source[0].i) * u0;
        // shunt result
        result.shunt = {{conj(i2_shunt_inj) * u2, i2_shunt_inj}};
        // load input and result, load6 is disconnected
        result.load_gen = {{s0_load_inj / 3.0, i0_load_inj / 3.0},
                           {s0_load_inj / 3.0, i0_load_inj / 3.0},
                           {s0_load_inj / 3.0, i0_load_inj / 3.0},
                           {s1_load_inj / 3.0, i1_load_inj / 3.0},
                           {s1_load_inj / 3.0, i1_load_inj / 3.0},
                           {s1_load_inj / 3.0, i1_load_inj / 3.0},
                           {0.0, 0.0}};
        // bus injection
        result.bus_injection = {result.branch[0].s_f, result.branch[0].s_t + result.branch[1].s_f, 0};

        return result;
    };
    auto const get_asym_output_ref = [&] {
        SolverOutput<asymmetric_t> result;

        SolverOutput<symmetric_t> sym_result = get_sym_output_ref();
        result.u.resize(sym_result.u.size());
        for (size_t i = 0; i != sym_result.u.size(); ++i) {
            result.u[i] = ComplexValue<asymmetric_t>{sym_result.u[i]};
        }
        result.branch.resize(sym_result.branch.size());
        for (size_t i = 0; i != sym_result.branch.size(); ++i) {
            result.branch[i].s_f = sym_result.branch[i].s_f * RealValue<asymmetric_t>{1.0};
            result.branch[i].s_t = sym_result.branch[i].s_t * RealValue<asymmetric_t>{1.0};
            result.branch[i].i_f = ComplexValue<asymmetric_t>{sym_result.branch[i].i_f};
            result.branch[i].i_t = ComplexValue<asymmetric_t>{sym_result.branch[i].i_t};
        }
        result.bus_injection.resize(sym_result.bus_injection.size());
        for (size_t i = 0; i != sym_result.bus_injection.size(); ++i) {
            result.bus_injection[i] = sym_result.bus_injection[i] * RealValue<asymmetric_t>{1.0};
        }
        result.source.resize(sym_result.source.size());
        for (size_t i = 0; i != sym_result.source.size(); ++i) {
            result.source[i].s = sym_result.source[i].s * RealValue<asymmetric_t>{1.0};
            result.source[i].i = ComplexValue<asymmetric_t>{sym_result.source[i].i};
        }
        result.load_gen.resize(sym_result.load_gen.size());
        for (size_t i = 0; i != sym_result.load_gen.size(); ++i) {
            result.load_gen[i].s = sym_result.load_gen[i].s * RealValue<asymmetric_t>{1.0};
            result.load_gen[i].i = ComplexValue<asymmetric_t>{sym_result.load_gen[i].i};
        }
        result.shunt.resize(sym_result.shunt.size());
        for (size_t i = 0; i != sym_result.shunt.size(); ++i) {
            result.shunt[i].s = sym_result.shunt[i].s * RealValue<asymmetric_t>{1.0};
            result.shunt[i].i = ComplexValue<asymmetric_t>{sym_result.shunt[i].i};
        }
        return result;
    };
    SolverOutput<sym> output_ref = [&] {
        if constexpr (is_symmetric_v<sym>) {
            return get_sym_output_ref();
        } else {
            return get_asym_output_ref();
        }
    }();

    // const z
    PowerFlowInput<sym> pf_input_z = [&] {
        PowerFlowInput<sym> result{pf_input};
        for (size_t i = 0; i < 6; i++) {
            if (i % 3 == 2) {
                result.s_injection[i] *= 3.0;
            } else {
                result.s_injection[i] = ComplexValue<sym>{0.0};
            }
        }
        return result;
    }();
    SolverOutput<sym> output_ref_z = [&] {
        SolverOutput<sym> result{output_ref};
        for (size_t i = 0; i < 6; i++) {
            if (i % 3 == 2) {
                result.load_gen[i].i *= 3.0;
                result.load_gen[i].s *= 3.0;
            } else {
                result.load_gen[i] = {};
            }
        }
        return result;
    }();

    // network param
    MathModelParam<sym> param = [&] {
        MathModelParam<sym> result;
        if constexpr (is_symmetric_v<sym>) {
            // branch parameter
            result.branch_param = {{y0 + ys0, -y0, -y0, y0 + ys0}, {y1, -y1 * shift, -y1 * conj(shift), y1}};
            // shunt
            result.shunt_param = {ys};
            // source
            result.source_param = {SourceCalcParam{yref, yref}};
        } else {
            // branch
            DoubleComplex const y0_0 = 0.5 + 0.5i;
            ComplexTensor<asymmetric_t> y0a{2.0 * y0 + y0_0, y0_0 - y0};
            y0a /= 3.0;
            ComplexTensor<asymmetric_t> const ys0a{ys0, 0.0};
            ComplexTensor<asymmetric_t> y1_1{2.0 * y1, -y1};
            y1_1 /= 3.0;
            ComplexTensor<asymmetric_t> y1_3;
            y1_3 << -y1, y1, 0.0, 0.0, -y1, y1, y1, 0.0, -y1;
            y1_3 /= sqrt3;
            ComplexTensor<asymmetric_t> y1_3t = (y1_3.matrix().transpose()).array();
            result.branch_param = {{y0a + ys0a, -y0a, -y0a, y0a + ys0a}, {y1_1, y1_3, y1_3t, y1_1}};

            // shunt
            DoubleComplex const ys_0 = ys * 0.2;
            ComplexTensor<asymmetric_t> ysa{2.0 * ys + ys_0, ys_0 - ys};
            ysa /= 3.0;
            result.shunt_param = {ysa};

            // source
            result.source_param = {SourceCalcParam{yref, yref}};
        }
        return result;
    }();

    // topo and param ptr
    auto param_ptr = std::make_shared<MathModelParam<sym> const>(param);
    auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
    YBus<sym> y_bus{topo_ptr, param_ptr};

    SUBCASE("Test pf solver") {
        constexpr auto error_tolerance{1e-12};
        constexpr auto num_iter{20};
        constexpr auto result_tolerance =
            SolverType::is_iterative ? 1e-12 : 0.15; // linear methods may be very inaccurate

        SolverType solver{y_bus, topo_ptr};
        CalculationInfo info;
        SolverOutput<sym> output = run_power_flow(solver, y_bus, pf_input, error_tolerance, num_iter, info);
        assert_output(output, output_ref, false, result_tolerance);
    }

    SUBCASE("Test const z pf solver") {
        SolverType solver{y_bus, topo_ptr};
        CalculationInfo info;

        // const z
        SolverOutput<sym> const output = run_power_flow(solver, y_bus, pf_input_z, 1e-12, 20, info);
        assert_output(output, output_ref_z); // for const z, all methods (including linear) should be accurate
    }

    if constexpr (SolverType::is_iterative) {
        SUBCASE("Test pf solver with single iteration") {
            // low precision
            constexpr auto error_tolerance{std::numeric_limits<double>::infinity()};
            constexpr auto result_tolerance{0.15};

            SolverType solver{y_bus, topo_ptr};
            CalculationInfo info;
            SolverOutput<sym> const output = run_power_flow(solver, y_bus, pf_input, error_tolerance, 1, info);
            assert_output(output, output_ref, false, result_tolerance);
        }
        SUBCASE("Test not converge") {
            SolverType solver{y_bus, topo_ptr};
            CalculationInfo info;
            pf_input.s_injection[6] = ComplexValue<sym>{1e6};
            CHECK_THROWS_AS(run_power_flow(solver, y_bus, pf_input, 1e-12, 20, info), IterationDiverge);
        }
    }

    SUBCASE("Test singular ybus") {
        param.branch_param[0] = BranchCalcParam<sym>{};
        param.branch_param[1] = BranchCalcParam<sym>{};
        param.shunt_param[0] = ComplexTensor<sym>{};
        y_bus.update_admittance(std::make_shared<MathModelParam<sym> const>(param));
        SolverType solver{y_bus, topo_ptr};
        CalculationInfo info;

        CHECK_THROWS_AS(run_power_flow(solver, y_bus, pf_input, 1e-12, 20, info), SparseMatrixError);
    }
}

} // namespace power_grid_model
