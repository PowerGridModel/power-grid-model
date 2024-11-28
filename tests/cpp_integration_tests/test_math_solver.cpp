// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// In this unit test the powerflow, state estimation and short circuit solvers are tested

#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/common/three_phase_tensor.hpp>
#include <power_grid_model/math_solver/math_solver.hpp>
#include <power_grid_model/math_solver/newton_raphson_pf_solver.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {
namespace {
using CalculationMethod::iterative_current;
using CalculationMethod::iterative_linear;
using CalculationMethod::linear;
using CalculationMethod::linear_current;
using CalculationMethod::newton_raphson;

template <symmetry_tag sym> void check_close(auto const& x, auto const& y, auto const& tolerance) {
    if constexpr (is_symmetric_v<sym>) {
        CHECK(cabs((x) - (y)) < (tolerance));
    } else {
        CHECK((cabs((x) - (y)) < (tolerance)).all());
    }
}

template <symmetry_tag sym> void check_close(auto const& x, auto const& y) {
    check_close<sym>(x, y, numerical_tolerance);
}
void check_close(auto const& x, auto const& y, auto const& tolerance) { check_close<symmetric_t>(x, y, tolerance); }
void check_close(auto const& x, auto const& y) { check_close<symmetric_t>(x, y); }
} // namespace

TEST_CASE("Test block") {
    SUBCASE("symmetric") {
        math_solver::newton_raphson_pf::PFJacBlock<symmetric_t> b{};
        b.h() += 1.0;
        b.n() += 2.0;
        b.m() += 3.0;
        b.l() += 4.0;
        CHECK(b.h() == 1.0);
        CHECK(b.n() == 2.0);
        CHECK(b.m() == 3.0);
        CHECK(b.l() == 4.0);
    }

    SUBCASE("Asymmetric") {
        math_solver::newton_raphson_pf::PFJacBlock<asymmetric_t> b{};
        RealTensor<asymmetric_t> const h{1.0};
        RealTensor<asymmetric_t> const n{2.0};
        RealTensor<asymmetric_t> const m{3.0};
        RealTensor<asymmetric_t> const l{4.0};
        b.h() += h;
        b.n() += n;
        b.m() += m;
        b.l() += l;
        check_close<asymmetric_t>(b.h(), h, numerical_tolerance);
        check_close<asymmetric_t>(b.n(), n, numerical_tolerance);
        check_close<asymmetric_t>(b.m(), m, numerical_tolerance);
        check_close<asymmetric_t>(b.l(), l, numerical_tolerance);
    }
}

namespace {

template <symmetry_tag sym>
void assert_output(SolverOutput<sym> const& output, SolverOutput<sym> const& output_ref, bool normalize_phase = false,
                   double tolerance = numerical_tolerance) {
    DoubleComplex const phase_offset = normalize_phase ? std::exp(1.0i / 180.0 * pi) : 1.0;
    for (size_t i = 0; i != output.u.size(); ++i) {
        check_close<sym>(output.u[i], output_ref.u[i] * phase_offset, tolerance);
    }
    for (size_t i = 0; i != output.bus_injection.size(); ++i) {
        check_close<sym>(output.bus_injection[i], output_ref.bus_injection[i], tolerance);
    }
    for (size_t i = 0; i != output.branch.size(); ++i) {
        check_close<sym>(output.branch[i].s_f, output_ref.branch[i].s_f, tolerance);
        check_close<sym>(output.branch[i].s_t, output_ref.branch[i].s_t, tolerance);
        check_close<sym>(output.branch[i].i_f, output_ref.branch[i].i_f * phase_offset, tolerance);
        check_close<sym>(output.branch[i].i_t, output_ref.branch[i].i_t * phase_offset, tolerance);
    }
    for (size_t i = 0; i != output.source.size(); ++i) {
        check_close<sym>(output.source[i].s, output_ref.source[i].s, tolerance);
        check_close<sym>(output.source[i].i, output_ref.source[i].i * phase_offset, tolerance);
    }
    for (size_t i = 0; i != output.load_gen.size(); ++i) {
        check_close<sym>(output.load_gen[i].s, output_ref.load_gen[i].s, tolerance);
        check_close<sym>(output.load_gen[i].i, output_ref.load_gen[i].i * phase_offset, tolerance);
    }
    for (size_t i = 0; i != output.shunt.size(); ++i) {
        check_close<sym>(output.shunt[i].s, output_ref.shunt[i].s, tolerance);
        check_close<sym>(output.shunt[i].i, output_ref.shunt[i].i * phase_offset, tolerance);
    }
}

} // namespace

TEST_CASE(
    "Test math solver") { // most of these should be template test cases with instantiations for the individual solvers
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
    MathModelParam<symmetric_t> param;
    PowerFlowInput<symmetric_t> pf_input;
    SolverOutput<symmetric_t> output_ref;
    // voltage
    double const vref = 1.1;
    double const v0 = 1.08;
    double const v1 = 0.97;
    double const v2 = 0.90;
    constexpr double deg = deg_30 / 30.0;
    DoubleComplex const u0 = v0 * std::exp(-1.0i * deg);
    DoubleComplex const u1 = v1 * std::exp(-4.0i * deg);
    DoubleComplex const u2 = v2 * std::exp(-37.0i * deg);
    output_ref.u = {u0, u1, u2};
    // branch parameter
    DoubleComplex const shift = std::exp(1.0i * shift_val);
    DoubleComplex const y0 = 1.0 - 2.0i;
    DoubleComplex const ys0 = 0.05 + 0.2i;
    DoubleComplex const y1 = 3.0 - 4.0i;
    param.branch_param = {{y0 + ys0, -y0, -y0, y0 + ys0}, {y1, -y1 * shift, -y1 * conj(shift), y1}};
    // branch result
    output_ref.branch.resize(2);
    output_ref.branch[0].i_f = (u0 - u1) * y0 + u0 * ys0;
    output_ref.branch[0].i_t = (u1 - u0) * y0 + u1 * ys0;
    output_ref.branch[0].s_f = conj(output_ref.branch[0].i_f) * u0;
    output_ref.branch[0].s_t = conj(output_ref.branch[0].i_t) * u1;
    output_ref.branch[1].i_f = (u1 - u2 * shift) * y1;
    output_ref.branch[1].i_t = (u2 - u1 * conj(shift)) * y1;
    output_ref.branch[1].s_f = conj(output_ref.branch[1].i_f) * u1;
    output_ref.branch[1].s_t = conj(output_ref.branch[1].i_t) * u2;
    // source input
    DoubleComplex const uref = vref;
    DoubleComplex const yref = 10.0 - 50.0i;
    pf_input.source = {vref};
    // source param and result
    param.source_param = {SourceCalcParam{yref, yref}};
    output_ref.source.resize(1);
    output_ref.source[0].i = yref * (uref - u0);
    output_ref.source[0].s = conj(output_ref.source[0].i) * u0;
    // injection of bus0 and bus1
    DoubleComplex const i0_load_inj = -output_ref.source[0].i + output_ref.branch[0].i_f;
    DoubleComplex const i1_load_inj = output_ref.branch[0].i_t + output_ref.branch[1].i_f;
    DoubleComplex const s0_load_inj = conj(i0_load_inj) * u0;
    DoubleComplex const s1_load_inj = conj(i1_load_inj) * u1;
    // injection of shunt0 at bus2
    DoubleComplex const i2_shunt_inj = output_ref.branch[1].i_t;
    // shunt param and result
    DoubleComplex const ys = -i2_shunt_inj / u2;
    param.shunt_param = {ys};
    output_ref.shunt = {{conj(i2_shunt_inj) * u2, i2_shunt_inj}};
    // load input and result, load6 is disconnected
    pf_input.s_injection = {s0_load_inj / 3.0,
                            s0_load_inj / 3.0 / v0,
                            s0_load_inj / 3.0 / v0 / v0,
                            s1_load_inj / 3.0,
                            s1_load_inj / 3.0 / v1,
                            s1_load_inj / 3.0 / v1 / v1,
                            0.0};
    output_ref.load_gen = {{s0_load_inj / 3.0, i0_load_inj / 3.0},
                           {s0_load_inj / 3.0, i0_load_inj / 3.0},
                           {s0_load_inj / 3.0, i0_load_inj / 3.0},
                           {s1_load_inj / 3.0, i1_load_inj / 3.0},
                           {s1_load_inj / 3.0, i1_load_inj / 3.0},
                           {s1_load_inj / 3.0, i1_load_inj / 3.0},
                           {0.0, 0.0}};
    // bus injection
    output_ref.bus_injection = {output_ref.branch[0].s_f, output_ref.branch[0].s_t + output_ref.branch[1].s_f, 0};

    // const z
    PowerFlowInput<symmetric_t> pf_input_z = pf_input;
    SolverOutput<symmetric_t> output_ref_z = output_ref;
    for (size_t i = 0; i < 6; i++) {
        if (i % 3 == 2) {
            pf_input_z.s_injection[i] *= 3.0;
            output_ref_z.load_gen[i].i *= 3.0;
            output_ref_z.load_gen[i].s *= 3.0;
        } else {
            pf_input_z.s_injection[i] = 0.0;
            output_ref_z.load_gen[i] = {};
        }
    }

    // asymmetric param
    // network param
    MathModelParam<asymmetric_t> param_asym;
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
    param_asym.branch_param = {{y0a + ys0a, -y0a, -y0a, y0a + ys0a}, {y1_1, y1_3, y1_3t, y1_1}};
    // shunt
    DoubleComplex const ys_0 = ys * 0.2;
    ComplexTensor<asymmetric_t> ysa{2.0 * ys + ys_0, ys_0 - ys};
    ysa /= 3.0;
    param_asym.shunt_param = {ysa};
    // source
    param_asym.source_param = {SourceCalcParam{yref, yref}};

    // load and source
    PowerFlowInput<asymmetric_t> pf_input_asym;
    pf_input_asym.source = {vref};
    pf_input_asym.s_injection.resize(pf_input.s_injection.size());
    for (size_t i = 0; i < pf_input.s_injection.size(); i++) {
        pf_input_asym.s_injection[i] = RealValue<asymmetric_t>{real(pf_input.s_injection[i])} +
                                       1.0i * RealValue<asymmetric_t>{imag(pf_input.s_injection[i])};
    }

    // output
    SolverOutput<asymmetric_t> output_ref_asym;
    output_ref_asym.u.resize(output_ref.u.size());
    for (size_t i = 0; i != output_ref.u.size(); ++i) {
        output_ref_asym.u[i] = ComplexValue<asymmetric_t>{output_ref.u[i]};
    }
    output_ref_asym.branch.resize(output_ref.branch.size());
    for (size_t i = 0; i != output_ref.branch.size(); ++i) {
        output_ref_asym.branch[i].s_f = output_ref.branch[i].s_f * RealValue<asymmetric_t>{1.0};
        output_ref_asym.branch[i].s_t = output_ref.branch[i].s_t * RealValue<asymmetric_t>{1.0};
        output_ref_asym.branch[i].i_f = ComplexValue<asymmetric_t>{output_ref.branch[i].i_f};
        output_ref_asym.branch[i].i_t = ComplexValue<asymmetric_t>{output_ref.branch[i].i_t};
    }
    output_ref_asym.bus_injection.resize(output_ref.bus_injection.size());
    for (size_t i = 0; i != output_ref.bus_injection.size(); ++i) {
        output_ref_asym.bus_injection[i] = output_ref.bus_injection[i] * RealValue<asymmetric_t>{1.0};
    }
    output_ref_asym.source.resize(output_ref.source.size());
    for (size_t i = 0; i != output_ref.source.size(); ++i) {
        output_ref_asym.source[i].s = output_ref.source[i].s * RealValue<asymmetric_t>{1.0};
        output_ref_asym.source[i].i = ComplexValue<asymmetric_t>{output_ref.source[i].i};
    }
    output_ref_asym.load_gen.resize(output_ref.load_gen.size());
    for (size_t i = 0; i != output_ref.load_gen.size(); ++i) {
        output_ref_asym.load_gen[i].s = output_ref.load_gen[i].s * RealValue<asymmetric_t>{1.0};
        output_ref_asym.load_gen[i].i = ComplexValue<asymmetric_t>{output_ref.load_gen[i].i};
    }
    output_ref_asym.shunt.resize(output_ref.shunt.size());
    for (size_t i = 0; i != output_ref.shunt.size(); ++i) {
        output_ref_asym.shunt[i].s = output_ref.shunt[i].s * RealValue<asymmetric_t>{1.0};
        output_ref_asym.shunt[i].i = ComplexValue<asymmetric_t>{output_ref.shunt[i].i};
    }

    // const z
    PowerFlowInput<asymmetric_t> pf_input_asym_z = pf_input_asym;
    SolverOutput<asymmetric_t> output_ref_asym_z = output_ref_asym;
    for (size_t i = 0; i < 6; i++) {
        if (i % 3 == 2) {
            pf_input_asym_z.s_injection[i] *= 3.0;
            output_ref_asym_z.load_gen[i].i *= 3.0;
            output_ref_asym_z.load_gen[i].s *= 3.0;
        } else {
            pf_input_asym_z.s_injection[i] = ComplexValue<asymmetric_t>{0.0};
            output_ref_asym_z.load_gen[i] = {};
        }
    }

    // topo and param ptr
    auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
    auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
    auto param_asym_ptr = std::make_shared<MathModelParam<asymmetric_t> const>(param_asym);
    YBus<symmetric_t> y_bus_sym{topo_ptr, param_ptr};
    YBus<asymmetric_t> const y_bus_asym{topo_ptr, param_asym_ptr};

    // state estimation input
    // symmetric, with u angle, with u angle and const z, without u angle
    StateEstimationInput<symmetric_t> se_input_angle;
    se_input_angle.shunt_status = {1};
    se_input_angle.load_gen_status = {1, 1, 1, 1, 1, 1, 0};
    se_input_angle.source_status = {1};
    se_input_angle.measured_voltage = {{output_ref.u[0], 1.0}, {output_ref.u[2], 1.0}, {output_ref.u[2], 1.0}};
    se_input_angle.measured_bus_injection = {
        {output_ref.source[0].s + output_ref.load_gen[0].s + output_ref.load_gen[1].s + output_ref.load_gen[2].s, 0.5,
         0.5}};
    se_input_angle.measured_source_power = {{output_ref.source[0].s, 0.5, 0.5}, {output_ref.source[0].s, 0.5, 0.5}};
    se_input_angle.measured_load_gen_power = {
        {output_ref.load_gen[3].s, 0.5, 0.5},
        {output_ref.load_gen[4].s, 0.5, 0.5},
        {output_ref.load_gen[5].s, 0.5, 0.5},
        {500.0, 0.5, 0.5},
    };
    se_input_angle.measured_shunt_power = {
        {output_ref.shunt[0].s, 0.5, 0.5},
    };

    se_input_angle.measured_branch_from_power = {
        {output_ref.branch[0].s_f, 0.5, 0.5},
    };
    se_input_angle.measured_branch_to_power = {
        {output_ref.branch[0].s_t, 0.5, 0.5},
        {output_ref.branch[0].s_t, 0.5, 0.5},
        {output_ref.branch[1].s_t, 0.5, 0.5},
    };

    SUBCASE("Test wrong calculation type") {
        MathSolver<symmetric_t> solver{topo_ptr};
        CalculationInfo info;
        CHECK_THROWS_AS(solver.run_power_flow(pf_input, 1e-12, 20, info, iterative_linear, y_bus_sym),
                        InvalidCalculationMethod);
        CHECK_THROWS_AS(solver.run_state_estimation(se_input_angle, 1e-10, 20, info, linear, y_bus_sym),
                        InvalidCalculationMethod);
    }
}

TEST_CASE("Math solver, zero variance test") {
    /*
    network, v means voltage measured
    variance always 1.0

    bus_1 --branch0-- bus_0(v) --yref-- source
    bus_1 = bus_0 = 1.0
    */
    MathModelTopology topo;
    topo.slack_bus = 1;
    topo.phase_shift = {0.0, 0.0};
    topo.branch_bus_idx = {{0, 1}};
    topo.sources_per_bus = {from_sparse, {0, 0, 1}};
    topo.shunts_per_bus = {from_sparse, {0, 0, 0}};
    topo.load_gens_per_bus = {from_sparse, {0, 0, 0}};
    topo.voltage_sensors_per_bus = {from_sparse, {0, 0, 1}};
    topo.power_sensors_per_bus = {from_sparse, {0, 0, 0}};
    topo.power_sensors_per_source = {from_sparse, {0, 0}};
    topo.power_sensors_per_load_gen = {from_sparse, {0}};
    topo.power_sensors_per_shunt = {from_sparse, {0}};
    topo.power_sensors_per_branch_from = {from_sparse, {0, 0}};
    topo.power_sensors_per_branch_to = {from_sparse, {0, 0}};
    MathModelParam<symmetric_t> param;
    param.branch_param = {{1.0, -1.0, -1.0, 1.0}};
    auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
    auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
    YBus<symmetric_t> const y_bus_sym{topo_ptr, param_ptr};

    StateEstimationInput<symmetric_t> se_input;
    se_input.source_status = {1};
    se_input.measured_voltage = {{1.0, 1.0}};

    MathSolver<symmetric_t> solver{topo_ptr};
    CalculationInfo info;
    SolverOutput<symmetric_t> output;

    SUBCASE("iterative linear") {
        output = solver.run_state_estimation(se_input, 1e-10, 20, info, iterative_linear, y_bus_sym);
    }
    SUBCASE("Newton-Raphson") {
        output = solver.run_state_estimation(se_input, 1e-10, 20, info, newton_raphson, y_bus_sym);
    }

    // check both voltage
    check_close(output.u[0], 1.0);
    check_close(output.u[1], 1.0);
}

TEST_CASE("Math solver, measurements") {
    /*
    network

     bus_0 --branch_0-- bus_1
        |                    |
    source_0               load_0

    */
    MathModelTopology topo;
    topo.slack_bus = 0;
    topo.phase_shift = {0.0, 0.0};
    topo.branch_bus_idx = {{0, 1}};
    topo.sources_per_bus = {from_sparse, {0, 1, 1}};
    topo.shunts_per_bus = {from_sparse, {0, 0, 0}};
    topo.load_gens_per_bus = {from_sparse, {0, 0, 1}};

    topo.voltage_sensors_per_bus = {from_sparse, {0, 1, 1}};
    topo.power_sensors_per_bus = {from_sparse, {0, 0, 0}};
    topo.power_sensors_per_source = {from_sparse, {0, 0}};
    topo.power_sensors_per_load_gen = {from_sparse, {0, 0}};
    topo.power_sensors_per_shunt = {from_sparse, {0}};
    topo.power_sensors_per_branch_from = {from_sparse, {0, 0}};
    topo.power_sensors_per_branch_to = {from_sparse, {0, 0}};

    MathModelParam<symmetric_t> param;
    param.branch_param = {{1.0e3, -1.0e3, -1.0e3, 1.0e3}};

    StateEstimationInput<symmetric_t> se_input;
    se_input.source_status = {1};
    se_input.load_gen_status = {1};
    se_input.measured_voltage = {{1.0, 0.1}};

    CalculationInfo info;
    SolverOutput<symmetric_t> output;

    SUBCASE("Source and branch") {
        /*
        network, v means voltage measured, p means power measured

         bus_0(v) -(p)-branch_0-- bus_1
            |                       |
        source_0(p)               load_0

        */
        topo.power_sensors_per_source = {from_sparse, {0, 1}};
        topo.power_sensors_per_branch_from = {from_sparse, {0, 1}};

        se_input.measured_source_power = {{1.93, 0.05, 0.05}};
        se_input.measured_branch_from_power = {{1.97, 0.05, 0.05}};

        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<symmetric_t> const y_bus_sym{topo_ptr, param_ptr};

        MathSolver<symmetric_t> solver{topo_ptr};

        SUBCASE("iterative linear") {
            output = solver.run_state_estimation(se_input, 1e-10, 20, info, iterative_linear, y_bus_sym);
        }
        SUBCASE("Newton-Raphson") {
            output = solver.run_state_estimation(se_input, 1e-10, 20, info, newton_raphson, y_bus_sym);
        }

        CHECK(real(output.bus_injection[0]) == doctest::Approx(1.95));
        CHECK(real(output.source[0].s) == doctest::Approx(1.95));
        CHECK(real(output.branch[0].s_f) == doctest::Approx(1.95));
    }

    SUBCASE("Load and branch") {
        /*
        network, v means voltage measured, p means power measured

         bus_0(v) --branch_0-(p)- bus_1
           |                        |
        source_0                 load_0(p)

        */
        topo.power_sensors_per_load_gen = {from_sparse, {0, 1}};
        topo.power_sensors_per_branch_to = {from_sparse, {0, 1}};

        se_input.measured_load_gen_power = {{-1.93, 0.05, 0.05}};
        se_input.measured_branch_to_power = {{-1.97, 0.05, 0.05}};

        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<symmetric_t> const y_bus_sym{topo_ptr, param_ptr};
        MathSolver<symmetric_t> solver{topo_ptr};

        SUBCASE("iterative linear") {
            output = solver.run_state_estimation(se_input, 1e-10, 20, info, iterative_linear, y_bus_sym);
        }
        SUBCASE("Newton-Raphson") {
            output = solver.run_state_estimation(se_input, 1e-10, 20, info, newton_raphson, y_bus_sym);
        }

        CHECK(real(output.bus_injection[1]) == doctest::Approx(-1.95));
        CHECK(real(output.load_gen[0].s) == doctest::Approx(-1.95));
        CHECK(real(output.branch[0].s_t) == doctest::Approx(-1.95));
    }

    SUBCASE("Node injection and source") {
        /*
        network, v means voltage measured, p means power measured

         bus_0(vp) -(p)-branch_0-- bus_1
            |                        |
        source_0(p)                load_0

        */
        topo.power_sensors_per_bus = {from_sparse, {0, 1, 1}};
        topo.power_sensors_per_source = {from_sparse, {0, 1}};
        topo.power_sensors_per_branch_from = {from_sparse, {0, 1}};

        se_input.measured_bus_injection = {{2.2, 0.1, 0.1}};
        se_input.measured_source_power = {{1.93, 0.05, 0.05}};
        se_input.measured_branch_from_power = {{1.97, 0.05, 0.05}};

        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<symmetric_t> const y_bus_sym{topo_ptr, param_ptr};
        MathSolver<symmetric_t> solver{topo_ptr};

        SUBCASE("iterative linear") {
            output = solver.run_state_estimation(se_input, 1e-10, 20, info, iterative_linear, y_bus_sym);
        }
        SUBCASE("Newton-Raphson") {
            output = solver.run_state_estimation(se_input, 1e-10, 20, info, newton_raphson, y_bus_sym);
        }

        CHECK(real(output.bus_injection[0]) == doctest::Approx(2.0));
        CHECK(real(output.source[0].s) == doctest::Approx(2.0));
        CHECK(real(output.branch[0].s_f) == doctest::Approx(2.0));
    }

    SUBCASE("Node injection, source and branch") {
        /*
        network, v means voltage measured, p means power measured

         bus_0(vp) -(p)-branch_0-- bus_1
            |                        |
        source_0(p)                load_0

        */
        topo.power_sensors_per_bus = {from_sparse, {0, 1, 1}};
        topo.power_sensors_per_source = {from_sparse, {0, 1}};
        topo.power_sensors_per_branch_from = {from_sparse, {0, 1}};

        se_input.measured_bus_injection = {{2.2, 0.1, 0.1}};
        se_input.measured_source_power = {{1.93, 0.05, 0.05}};
        se_input.measured_branch_from_power = {{1.97, 0.05, 0.05}};

        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<symmetric_t> const y_bus_sym{topo_ptr, param_ptr};
        MathSolver<symmetric_t> solver{topo_ptr};

        SUBCASE("iterative linear") {
            output = solver.run_state_estimation(se_input, 1e-10, 20, info, iterative_linear, y_bus_sym);
        }
        SUBCASE("Newton-Raphson") {
            output = solver.run_state_estimation(se_input, 1e-10, 20, info, newton_raphson, y_bus_sym);
        }

        CHECK(real(output.bus_injection[0]) == doctest::Approx(2.0));
        CHECK(real(output.source[0].s) == doctest::Approx(2.0));
        CHECK(real(output.branch[0].s_f) == doctest::Approx(2.0));
    }

    SUBCASE("Node injection, load and branch") {
        /*
        network, v means voltage measured, p means power measured

         bus_0(v) --branch_0-(p)- bus_1(p)
           |                        |
        source_0                 load_0(p)

        */
        topo.power_sensors_per_bus = {from_sparse, {0, 0, 1}};
        topo.power_sensors_per_load_gen = {from_sparse, {0, 1}};
        topo.power_sensors_per_branch_to = {from_sparse, {0, 1}};

        se_input.measured_bus_injection = {{-2.2, 0.1, 0.1}};
        se_input.measured_load_gen_power = {{-1.93, 0.05, 0.05}};
        se_input.measured_branch_to_power = {{-1.97, 0.05, 0.05}};

        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<symmetric_t> const y_bus_sym{topo_ptr, param_ptr};
        MathSolver<symmetric_t> solver{topo_ptr};

        SUBCASE("iterative linear") {
            output = solver.run_state_estimation(se_input, 1e-10, 20, info, iterative_linear, y_bus_sym);
        }
        SUBCASE("Newton-Raphson") {
            output = solver.run_state_estimation(se_input, 1e-10, 20, info, newton_raphson, y_bus_sym);
        }

        CHECK(real(output.bus_injection[1]) == doctest::Approx(-2.0));
        CHECK(real(output.load_gen[0].s) == doctest::Approx(-2.0));
        CHECK(real(output.branch[0].s_t) == doctest::Approx(-2.0));
    }

    SUBCASE("Load and gen") {
        /*
        network, v means voltage measured, p means power measured

         bus_0(v) --branch_0-- bus_1
           |                    /   \
        source_0          load_0(p)  gen_1(p)

        */

        topo.load_gens_per_bus = {from_sparse, {0, 0, 2}};
        topo.power_sensors_per_load_gen = {from_sparse, {0, 1, 2}};

        se_input.load_gen_status = {1, 1};
        se_input.measured_load_gen_power = {{-3.0, 0.05, 0.05}, {1.0, 0.05, 0.05}};

        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<symmetric_t> const y_bus_sym{topo_ptr, param_ptr};
        MathSolver<symmetric_t> solver{topo_ptr};

        SUBCASE("iterative linear") {
            output = solver.run_state_estimation(se_input, 1e-10, 20, info, iterative_linear, y_bus_sym);
        }
        SUBCASE("Newton-Raphson") {
            output = solver.run_state_estimation(se_input, 1e-10, 20, info, newton_raphson, y_bus_sym);
        }

        CHECK(real(output.bus_injection[1]) == doctest::Approx(-2.0));
        CHECK(real(output.branch[0].s_t) == doctest::Approx(-2.0));
        CHECK(real(output.load_gen[0].s) == doctest::Approx(-3.0));
        CHECK(real(output.load_gen[1].s) == doctest::Approx(1.0));
    }

    SUBCASE("Node injection, load and gen") {
        /*
        network, v means voltage measured, p means power measured

         bus_0(v) --branch_0-- bus_1(p)
           |                    /   \
        source_0          load_0(p)  gen_1(p)
        */

        topo.voltage_sensors_per_bus = {from_sparse, {0, 1, 1}};
        topo.load_gens_per_bus = {from_sparse, {0, 0, 2}};
        topo.power_sensors_per_load_gen = {from_sparse, {0, 1, 2}};
        topo.power_sensors_per_bus = {from_sparse, {0, 0, 1}};

        se_input.load_gen_status = {1, 1};
        se_input.measured_load_gen_power = {{-1.8, 0.05, 0.05}, {0.9, 0.05, 0.05}};
        se_input.measured_bus_injection = {{-1.1, 0.1, 0.1}};

        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<symmetric_t> const y_bus_sym{topo_ptr, param_ptr};
        MathSolver<symmetric_t> solver{topo_ptr};

        SUBCASE("iterative linear") {
            output = solver.run_state_estimation(se_input, 1e-10, 20, info, iterative_linear, y_bus_sym);
        }
        SUBCASE("Newton-Raphson") {
            output = solver.run_state_estimation(se_input, 1e-10, 20, info, newton_raphson, y_bus_sym);
        }

        CHECK(real(output.bus_injection[1]) == doctest::Approx(-1.0));
        CHECK(real(output.load_gen[0].s) == doctest::Approx(-1.85));
        CHECK(real(output.load_gen[1].s) == doctest::Approx(0.85));
    }

    SUBCASE("Node injection, load and gen with different variances") {
        /*
        network, v means voltage measured, p means power measured

         bus_0(v) --branch_0-- bus_1(p)
           |                    /   \
        source_0          load_0(p)  gen_1(p)
        */

        topo.voltage_sensors_per_bus = {from_sparse, {0, 1, 1}};
        topo.load_gens_per_bus = {from_sparse, {0, 0, 2}};
        topo.power_sensors_per_load_gen = {from_sparse, {0, 1, 2}};
        topo.power_sensors_per_bus = {from_sparse, {0, 0, 1}};

        se_input.load_gen_status = {1, 1};
        se_input.measured_load_gen_power = {{-1.8, 0.05, 0.05}, {0.9, 0.025, 0.075}};
        se_input.measured_bus_injection = {{-1.1, 0.1, 0.1}};

        auto param_ptr = std::make_shared<MathModelParam<symmetric_t> const>(param);
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<symmetric_t> const y_bus_sym{topo_ptr, param_ptr};
        MathSolver<symmetric_t> solver{topo_ptr};

        SUBCASE("iterative linear") {
            output = solver.run_state_estimation(se_input, 1e-10, 20, info, iterative_linear, y_bus_sym);
        }
        SUBCASE("Newton-Raphson") {
            output = solver.run_state_estimation(se_input, 1e-10, 20, info, newton_raphson, y_bus_sym);
        }

        // the different aggregation of the load gen's P and Q measurements cause differences compared to the case with
        // identical variances
        CHECK(real(output.bus_injection[1]) > doctest::Approx(-1.0));
        CHECK(real(output.load_gen[0].s) < doctest::Approx(-1.85));
        CHECK(real(output.load_gen[1].s) > doctest::Approx(0.85));
    }

    const ComplexValue<symmetric_t> load_gen_s =
        std::accumulate(output.load_gen.begin(), output.load_gen.end(), ComplexValue<symmetric_t>{},
                        [](auto const& a, auto const& b) { return a + b.s; });

    CHECK(output.bus_injection[0] == output.branch[0].s_f);
    CHECK(output.bus_injection[0] == output.source[0].s);
    CHECK(output.bus_injection[1] == output.branch[0].s_t);
    CHECK(real(output.bus_injection[1]) == doctest::Approx(real(load_gen_s)));
}

} // namespace power_grid_model
