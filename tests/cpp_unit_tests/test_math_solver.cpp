// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

// In this unit test the powerflow and state estimation solvers are tested

#include <power_grid_model/exception.hpp>
#include <power_grid_model/math_solver/math_solver.hpp>
#include <power_grid_model/math_solver/newton_raphson_pf_solver.hpp>
#include <power_grid_model/three_phase_tensor.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {
namespace {
using CalculationMethod::iterative_current;
using CalculationMethod::iterative_linear;
using CalculationMethod::linear;
using CalculationMethod::linear_current;
using CalculationMethod::newton_raphson;
using FaultType::single_phase_to_ground;
using FaultType::three_phase;
using FaultType::two_phase;
using FaultType::two_phase_to_ground;

template <bool sym> void check_close(auto const& x, auto const& y, auto const& tolerance) {
    if constexpr (sym) {
        CHECK(cabs((x) - (y)) < (tolerance));
    } else {
        CHECK((cabs((x) - (y)) < (tolerance)).all());
    }
}

template <bool sym> void check_close(auto const& x, auto const& y) { check_close<sym>(x, y, numerical_tolerance); }
void check_close(auto const& x, auto const& y, auto const& tolerance) { check_close<true>(x, y, tolerance); }
void check_close(auto const& x, auto const& y) { check_close<true>(x, y); }
} // namespace

TEST_CASE("Test block") {
    SUBCASE("symmetric") {
        math_model_impl::PFJacBlock<true> b{};
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
        math_model_impl::PFJacBlock<false> b{};
        RealTensor<false> const h{1.0};
        RealTensor<false> const n{2.0};
        RealTensor<false> const m{3.0};
        RealTensor<false> const l{4.0};
        b.h() += h;
        b.n() += n;
        b.m() += m;
        b.l() += l;
        check_close<false>(b.h(), h, numerical_tolerance);
        check_close<false>(b.n(), n, numerical_tolerance);
        check_close<false>(b.m(), m, numerical_tolerance);
        check_close<false>(b.l(), l, numerical_tolerance);
    }
}

namespace {

template <bool sym>
void assert_output(MathOutput<sym> const& output, MathOutput<sym> const& output_ref, bool normalize_phase = false,
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

template <bool sym>
void assert_sc_output(ShortCircuitMathOutput<sym> const& output, ShortCircuitMathOutput<sym> const& output_ref,
                      double tolerance = numerical_tolerance) {
    for (size_t i = 0; i != output.u_bus.size(); ++i) {
        check_close<sym>(output.u_bus[i], output_ref.u_bus[i], tolerance);
    }
    for (size_t i = 0; i != output.branch.size(); ++i) {
        check_close<sym>(output.branch[i].i_f, output_ref.branch[i].i_f, tolerance);
    }
    for (size_t i = 0; i != output.branch.size(); ++i) {
        check_close<sym>(output.branch[i].i_t, output_ref.branch[i].i_t, tolerance);
    }
    for (size_t i = 0; i != output.fault.size(); ++i) {
        check_close<sym>(output.fault[i].i_fault, output_ref.fault[i].i_fault, tolerance);
    }
    for (size_t i = 0; i != output.source.size(); ++i) {
        check_close<sym>(output.source[i].i, output_ref.source[i].i, tolerance);
    }
}

} // namespace

TEST_CASE("Test math solver") {
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
    topo.slack_bus_ = 0;
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
    MathModelParam<true> param;
    PowerFlowInput<true> pf_input;
    MathOutput<true> output_ref;
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
    param.source_param = {yref};
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
    PowerFlowInput<true> pf_input_z = pf_input;
    MathOutput<true> output_ref_z = output_ref;
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
    MathModelParam<false> param_asym;
    // branch
    DoubleComplex const y0_0 = 0.5 + 0.5i;
    ComplexTensor<false> y0a{2.0 * y0 + y0_0, y0_0 - y0};
    y0a /= 3.0;
    ComplexTensor<false> const ys0a{ys0, 0.0};
    ComplexTensor<false> y1_1{2.0 * y1, -y1};
    y1_1 /= 3.0;
    ComplexTensor<false> y1_3;
    y1_3 << -y1, y1, 0.0, 0.0, -y1, y1, y1, 0.0, -y1;
    y1_3 /= sqrt3;
    ComplexTensor<false> y1_3t = (y1_3.matrix().transpose()).array();
    param_asym.branch_param = {{y0a + ys0a, -y0a, -y0a, y0a + ys0a}, {y1_1, y1_3, y1_3t, y1_1}};
    // shunt
    DoubleComplex const ys_0 = ys * 0.2;
    ComplexTensor<false> ysa{2.0 * ys + ys_0, ys_0 - ys};
    ysa /= 3.0;
    param_asym.shunt_param = {ysa};
    // source
    param_asym.source_param = {ComplexTensor<false>{yref}};

    // load and source
    PowerFlowInput<false> pf_input_asym;
    pf_input_asym.source = {vref};
    pf_input_asym.s_injection.resize(pf_input.s_injection.size());
    for (size_t i = 0; i < pf_input.s_injection.size(); i++) {
        pf_input_asym.s_injection[i] =
            RealValue<false>{real(pf_input.s_injection[i])} + 1.0i * RealValue<false>{imag(pf_input.s_injection[i])};
    }

    // output
    MathOutput<false> output_ref_asym;
    output_ref_asym.u.resize(output_ref.u.size());
    for (size_t i = 0; i != output_ref.u.size(); ++i) {
        output_ref_asym.u[i] = ComplexValue<false>{output_ref.u[i]};
    }
    output_ref_asym.branch.resize(output_ref.branch.size());
    for (size_t i = 0; i != output_ref.branch.size(); ++i) {
        output_ref_asym.branch[i].s_f = output_ref.branch[i].s_f * RealValue<false>{1.0};
        output_ref_asym.branch[i].s_t = output_ref.branch[i].s_t * RealValue<false>{1.0};
        output_ref_asym.branch[i].i_f = ComplexValue<false>{output_ref.branch[i].i_f};
        output_ref_asym.branch[i].i_t = ComplexValue<false>{output_ref.branch[i].i_t};
    }
    output_ref_asym.bus_injection.resize(output_ref.bus_injection.size());
    for (size_t i = 0; i != output_ref.bus_injection.size(); ++i) {
        output_ref_asym.bus_injection[i] = output_ref.bus_injection[i] * RealValue<false>{1.0};
    }
    output_ref_asym.source.resize(output_ref.source.size());
    for (size_t i = 0; i != output_ref.source.size(); ++i) {
        output_ref_asym.source[i].s = output_ref.source[i].s * RealValue<false>{1.0};
        output_ref_asym.source[i].i = ComplexValue<false>{output_ref.source[i].i};
    }
    output_ref_asym.load_gen.resize(output_ref.load_gen.size());
    for (size_t i = 0; i != output_ref.load_gen.size(); ++i) {
        output_ref_asym.load_gen[i].s = output_ref.load_gen[i].s * RealValue<false>{1.0};
        output_ref_asym.load_gen[i].i = ComplexValue<false>{output_ref.load_gen[i].i};
    }
    output_ref_asym.shunt.resize(output_ref.shunt.size());
    for (size_t i = 0; i != output_ref.shunt.size(); ++i) {
        output_ref_asym.shunt[i].s = output_ref.shunt[i].s * RealValue<false>{1.0};
        output_ref_asym.shunt[i].i = ComplexValue<false>{output_ref.shunt[i].i};
    }

    // const z
    PowerFlowInput<false> pf_input_asym_z = pf_input_asym;
    MathOutput<false> output_ref_asym_z = output_ref_asym;
    for (size_t i = 0; i < 6; i++) {
        if (i % 3 == 2) {
            pf_input_asym_z.s_injection[i] *= 3.0;
            output_ref_asym_z.load_gen[i].i *= 3.0;
            output_ref_asym_z.load_gen[i].s *= 3.0;
        } else {
            pf_input_asym_z.s_injection[i] = ComplexValue<false>{0.0};
            output_ref_asym_z.load_gen[i] = {};
        }
    }

    // topo and param ptr
    auto param_ptr = std::make_shared<MathModelParam<true> const>(param);
    auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
    auto param_asym_ptr = std::make_shared<MathModelParam<false> const>(param_asym);
    YBus<true> y_bus_sym{topo_ptr, param_ptr};
    YBus<false> const y_bus_asym{topo_ptr, param_asym_ptr};

    // state estimation input
    // symmetric, with u angle, with u angle and const z, without u angle
    StateEstimationInput<true> se_input_angle;
    se_input_angle.shunt_status = {1};
    se_input_angle.load_gen_status = {1, 1, 1, 1, 1, 1, 0};
    se_input_angle.source_status = {1};
    se_input_angle.measured_voltage = {{output_ref.u[0], 1.0}, {output_ref.u[2], 1.0}, {output_ref.u[2], 1.0}};
    se_input_angle.measured_bus_injection = {
        {output_ref.source[0].s + output_ref.load_gen[0].s + output_ref.load_gen[1].s + output_ref.load_gen[2].s, 1.0}};
    se_input_angle.measured_source_power = {{output_ref.source[0].s, 1.0}, {output_ref.source[0].s, 1.0}};
    se_input_angle.measured_load_gen_power = {
        {output_ref.load_gen[3].s, 1.0},
        {output_ref.load_gen[4].s, 1.0},
        {output_ref.load_gen[5].s, 1.0},
        {500.0, 1.0},
    };
    se_input_angle.measured_shunt_power = {
        {output_ref.shunt[0].s, 1.0},
    };

    se_input_angle.measured_branch_from_power = {
        {output_ref.branch[0].s_f, 1.0},
    };
    se_input_angle.measured_branch_to_power = {
        {output_ref.branch[0].s_t, 1.0},
        {output_ref.branch[0].s_t, 1.0},
        {output_ref.branch[1].s_t, 1.0},
    };
    // no angle, keep the angle of 2nd measurement of bus2, which will be ignored
    StateEstimationInput<true> se_input_no_angle = se_input_angle;
    se_input_no_angle.measured_voltage[0].value = DoubleComplex{cabs(se_input_no_angle.measured_voltage[0].value), nan};
    se_input_no_angle.measured_voltage[1].value = DoubleComplex{cabs(se_input_no_angle.measured_voltage[1].value), nan};

    // with angle, const z
    StateEstimationInput<true> se_input_angle_const_z = se_input_angle;
    // set open for load 01, 34, scale load 5 (sensor 2)
    se_input_angle_const_z.load_gen_status[0] = 0;
    se_input_angle_const_z.load_gen_status[1] = 0;
    se_input_angle_const_z.load_gen_status[3] = 0;
    se_input_angle_const_z.load_gen_status[4] = 0;
    se_input_angle_const_z.measured_load_gen_power[2].value *= 3.0;

    // asymmetric, with u angle, with u angle and const z, without u angle
    StateEstimationInput<false> se_input_asym_angle;
    se_input_asym_angle.shunt_status = {1};
    se_input_asym_angle.load_gen_status = {1, 1, 1, 1, 1, 1, 0};
    se_input_asym_angle.source_status = {1};
    se_input_asym_angle.measured_voltage = {{ComplexValue<false>{output_ref.u[0]}, 1.0},
                                            {ComplexValue<false>{output_ref.u[2]}, 1.0},
                                            {ComplexValue<false>{output_ref.u[2]}, 1.0}};
    se_input_asym_angle.measured_bus_injection = {
        {(output_ref.source[0].s + output_ref.load_gen[0].s + output_ref.load_gen[1].s + output_ref.load_gen[2].s) *
             RealValue<false>{1.0},
         1.0}};
    se_input_asym_angle.measured_source_power = {{output_ref.source[0].s * RealValue<false>{1.0}, 1.0},
                                                 {output_ref.source[0].s * RealValue<false>{1.0}, 1.0}};
    se_input_asym_angle.measured_load_gen_power = {
        {output_ref.load_gen[3].s * RealValue<false>{1.0}, 1.0},
        {output_ref.load_gen[4].s * RealValue<false>{1.0}, 1.0},
        {output_ref.load_gen[5].s * RealValue<false>{1.0}, 1.0},
        {500.0 * RealValue<false>{1.0}, 1.0},
    };
    se_input_asym_angle.measured_shunt_power = {
        {output_ref.shunt[0].s * RealValue<false>{1.0}, 1.0},
    };

    se_input_asym_angle.measured_branch_from_power = {
        {output_ref.branch[0].s_f * RealValue<false>{1.0}, 1.0},
    };
    se_input_asym_angle.measured_branch_to_power = {
        {output_ref.branch[0].s_t * RealValue<false>{1.0}, 1.0},
        {output_ref.branch[0].s_t * RealValue<false>{1.0}, 1.0},
        {output_ref.branch[1].s_t * RealValue<false>{1.0}, 1.0},
    };
    // no angle, keep the angle of 2nd measurement of bus2, which will be ignored
    StateEstimationInput<false> se_input_asym_no_angle = se_input_asym_angle;
    se_input_asym_no_angle.measured_voltage[0].value =
        cabs(se_input_asym_no_angle.measured_voltage[0].value) + DoubleComplex{0.0, nan};
    se_input_asym_no_angle.measured_voltage[1].value =
        cabs(se_input_asym_no_angle.measured_voltage[1].value) + DoubleComplex{0.0, nan};
    // with angle, const z
    StateEstimationInput<false> se_input_asym_angle_const_z = se_input_asym_angle;
    // set open for load 01, 34, scale load 5 (sensor 2)
    se_input_asym_angle_const_z.load_gen_status[0] = 0;
    se_input_asym_angle_const_z.load_gen_status[1] = 0;
    se_input_asym_angle_const_z.load_gen_status[3] = 0;
    se_input_asym_angle_const_z.load_gen_status[4] = 0;
    se_input_asym_angle_const_z.measured_load_gen_power[2].value *= 3.0;

    SUBCASE("Test symmetric pf solver") {
        MathSolver<true> solver{topo_ptr};
        CalculationInfo info;
        MathOutput<true> output = solver.run_power_flow(pf_input, 1e-12, 20, info, newton_raphson, y_bus_sym);
        // verify
        assert_output(output, output_ref);
        // copy
        MathSolver<true> solver2{solver};
        solver2.clear_solver();
        output = solver2.run_power_flow(pf_input, 1e-12, 20, info, newton_raphson, y_bus_sym);
        // verify
        assert_output(output, output_ref);
        // move
        MathSolver<true> solver3{std::move(solver)};
        output = solver3.run_power_flow(pf_input, 1e-12, 20, info, newton_raphson, y_bus_sym);
        // verify
        assert_output(output, output_ref);
    }

    SUBCASE("Test symmetric iterative current pf solver") {
        MathSolver<true> solver{topo_ptr};
        CalculationInfo info;
        MathOutput<true> const output = solver.run_power_flow(pf_input, 1e-12, 20, info, iterative_current, y_bus_sym);
        // verify
        assert_output(output, output_ref);
    }

    SUBCASE("Test symmetric linear current pf solver") {
        // low precision
        constexpr auto error_tolerance{5e-3};
        constexpr auto result_tolerance{5e-2};

        MathSolver<true> solver{topo_ptr};
        CalculationInfo info;
        MathOutput<true> const output =
            solver.run_power_flow(pf_input, error_tolerance, 20, info, linear_current, y_bus_sym);
        // verify
        assert_output(output, output_ref, false, result_tolerance);
    }

    SUBCASE("Test wrong calculation type") {
        MathSolver<true> solver{topo_ptr};
        CalculationInfo info;
        CHECK_THROWS_AS(solver.run_power_flow(pf_input, 1e-12, 20, info, iterative_linear, y_bus_sym),
                        InvalidCalculationMethod);
        CHECK_THROWS_AS(solver.run_state_estimation(se_input_angle, 1e-10, 20, info, linear, y_bus_sym),
                        InvalidCalculationMethod);
    }

    SUBCASE("Test const z pf solver") {
        MathSolver<true> solver{topo_ptr};
        CalculationInfo info;

        // const z
        MathOutput<true> const output = solver.run_power_flow(pf_input_z, 1e-12, 20, info, linear, y_bus_sym);
        // verify
        assert_output(output, output_ref_z);
    }

    SUBCASE("Test not converge") {
        MathSolver<true> solver{topo_ptr};
        CalculationInfo info;
        pf_input.s_injection[6] = 1e6;
        CHECK_THROWS_AS(solver.run_power_flow(pf_input, 1e-12, 20, info, newton_raphson, y_bus_sym), IterationDiverge);
    }

    SUBCASE("Test singular ybus") {
        std::vector<CalculationMethod> const methods{linear, newton_raphson, linear_current, iterative_current};

        param.branch_param[0] = BranchCalcParam<true>{};
        param.branch_param[1] = BranchCalcParam<true>{};
        param.shunt_param[0] = 0.0;
        y_bus_sym.update_admittance(std::make_shared<MathModelParam<true> const>(param));
        MathSolver<true> solver{topo_ptr};
        CalculationInfo info;

        for (auto method : methods) {
            CAPTURE(method);
            CHECK_THROWS_AS(solver.run_power_flow(pf_input, 1e-12, 20, info, method, y_bus_sym), SparseMatrixError);
        }
    }

    SUBCASE("Test asymmetric pf solver") {
        MathSolver<false> solver{topo_ptr};
        CalculationInfo info;
        MathOutput<false> const output =
            solver.run_power_flow(pf_input_asym, 1e-12, 20, info, newton_raphson, y_bus_asym);
        // verify
        assert_output(output, output_ref_asym);
    }

    SUBCASE("Test iterative current asymmetric pf solver") {
        MathSolver<false> solver{topo_ptr};
        CalculationInfo info;
        MathOutput<false> const output =
            solver.run_power_flow(pf_input_asym, 1e-12, 20, info, iterative_current, y_bus_asym);
        // verify
        assert_output(output, output_ref_asym);
    }

    SUBCASE("Test asym const z pf solver") {
        MathSolver<false> solver{topo_ptr};
        CalculationInfo info;
        // const z
        MathOutput<false> const output = solver.run_power_flow(pf_input_asym_z, 1e-12, 20, info, linear, y_bus_asym);
        // verify
        assert_output(output, output_ref_asym_z);
    }

    SUBCASE("Test sym se with angle") {
        MathSolver<true> solver{topo_ptr};
        CalculationInfo info;
        MathOutput<true> const output =
            solver.run_state_estimation(se_input_angle, 1e-10, 20, info, iterative_linear, y_bus_sym);
        // verify
        assert_output(output, output_ref);
    }

    SUBCASE("Test sym se without angle") {
        MathSolver<true> solver{topo_ptr};
        CalculationInfo info;
        MathOutput<true> const output =
            solver.run_state_estimation(se_input_no_angle, 1e-10, 20, info, iterative_linear, y_bus_sym);
        // verify
        assert_output(output, output_ref, true);
    }

    SUBCASE("Test sym se with angle, const z") {
        MathSolver<true> solver{topo_ptr};
        CalculationInfo info;
        MathOutput<true> const output =
            solver.run_state_estimation(se_input_angle_const_z, 1e-10, 20, info, iterative_linear, y_bus_sym);
        // verify
        assert_output(output, output_ref_z);
    }

    SUBCASE("Test asym se with angle") {
        MathSolver<false> solver{topo_ptr};
        CalculationInfo info;
        MathOutput<false> const output =
            solver.run_state_estimation(se_input_asym_angle, 1e-10, 20, info, iterative_linear, y_bus_asym);
        // verify
        assert_output(output, output_ref_asym);
    }

    SUBCASE("Test asym se without angle") {
        MathSolver<false> solver{topo_ptr};
        CalculationInfo info;
        MathOutput<false> const output =
            solver.run_state_estimation(se_input_asym_no_angle, 1e-10, 20, info, iterative_linear, y_bus_asym);
        // verify
        assert_output(output, output_ref_asym, true);
    }

    SUBCASE("Test asym se with angle, const z") {
        MathSolver<false> solver{topo_ptr};
        CalculationInfo info;
        MathOutput<false> const output = solver.run_state_estimation(se_input_asym_angle_const_z, 1e-10, 20, info,

                                                                     iterative_linear, y_bus_asym);
        // verify
        assert_output(output, output_ref_asym_z);
    }
}

namespace {

ShortCircuitInput create_sc_test_input(FaultType fault_type, FaultPhase fault_phase, DoubleComplex const& y_fault,
                                       double const vref, IdxVector const& fault_bus_indptr) {
    ShortCircuitInput sc_input;
    sc_input.fault_bus_indptr = fault_bus_indptr;
    sc_input.source = {vref};
    sc_input.faults = {{y_fault, fault_type, fault_phase}};
    return sc_input;
}

template <bool sym> constexpr ShortCircuitMathOutput<sym> blank_sc_output(DoubleComplex vref) {
    ShortCircuitMathOutput<sym> sc_output;
    sc_output.u_bus = {ComplexValue<sym>(vref), ComplexValue<sym>(vref)};
    sc_output.fault = {{ComplexValue<sym>{}}};
    sc_output.branch = {BranchShortCircuitMathOutput<sym>{.i_f = {ComplexValue<sym>{}}, .i_t = {ComplexValue<sym>{}}}};
    sc_output.source = {{ComplexValue<sym>{}}};
    return sc_output;
}

template <bool sym>
constexpr ShortCircuitMathOutput<sym> create_math_sc_output(ComplexValue<sym> u0, ComplexValue<sym> u1,
                                                            ComplexValue<sym> if_abc) {
    ShortCircuitMathOutput<sym> sc_output;
    sc_output.u_bus = {u0, u1};
    sc_output.fault = {{if_abc}};
    sc_output.branch = {BranchShortCircuitMathOutput<sym>{.i_f = if_abc, .i_t = -if_abc}};
    sc_output.source = {{if_abc}};
    return sc_output;
}

template <bool sym>
ShortCircuitMathOutput<sym> create_sc_test_output(FaultType fault_type, DoubleComplex const& z_fault,
                                                  DoubleComplex const& z0, DoubleComplex const& z0_0, double const vref,
                                                  DoubleComplex const& zref) {

    if constexpr (sym) {
        DoubleComplex const if_abc = vref / (z0 + zref + z_fault);
        DoubleComplex const u0 = vref - if_abc * zref;
        DoubleComplex const u1 = u0 - if_abc * z0;
        return create_math_sc_output<true>(u0, u1, if_abc);
    } else {
        ComplexValue<false> if_abc{};
        switch (fault_type) {
        case three_phase: {
            DoubleComplex const if_3ph = vref / (z0 + zref + z_fault);
            if_abc = ComplexValue<false>(if_3ph);
            break;
        }
        case single_phase_to_ground: {
            DoubleComplex const if_1phg = vref / (2.0 * (zref + z0) + (z0_0 + zref) + 3.0 * z_fault);
            if_abc = ComplexValue<false>(3.0 * if_1phg, 0.0, 0.0);
            break;
        }
        case two_phase: {
            DoubleComplex const if_2ph = (-1i * sqrt3) * vref / (2.0 * (zref + z0) + z_fault);
            if_abc = ComplexValue<false>(0.0, if_2ph, -if_2ph);
            break;
        }
        case two_phase_to_ground: {
            DoubleComplex const y2phg_0 = 1.0 / (zref + z0_0 + 3.0 * z_fault);
            DoubleComplex const y2phg_12 = 1.0 / (zref + z0);
            DoubleComplex const y2phg_sum = 2.0 * y2phg_12 + y2phg_0;
            DoubleComplex const i_0 = vref * (-y2phg_0 * y2phg_12 / y2phg_sum);
            DoubleComplex const i_1 = vref * ((-y2phg_12 * y2phg_12 / y2phg_sum) + y2phg_12);
            DoubleComplex const i_2 = vref * (-y2phg_12 * y2phg_12 / y2phg_sum);
            if_abc = ComplexValue<false>{i_0 + i_1 + i_2, i_0 + i_1 * a * a + i_2 * a, i_0 + i_1 * a + i_2 * a * a};
            break;
        }
        default:
            throw InvalidShortCircuitType{false, fault_type};
        }
        ComplexValue<false> const vref_asym{vref};
        ComplexValue<false> const u0 = vref_asym - if_abc * zref;
        DoubleComplex const z_self{(2.0 * z0 + z0_0) / 3.0};
        DoubleComplex const z_mutual{(z0_0 - z0) / 3.0};
        ComplexValue<false> const u_drop{
            if_abc(0) * z_self + (if_abc(1) + if_abc(2)) * z_mutual,
            if_abc(1) * z_self + (if_abc(0) + if_abc(2)) * z_mutual,
            if_abc(2) * z_self + (if_abc(0) + if_abc(1)) * z_mutual,
        };
        ComplexValue<false> const u1 = u0 - u_drop;
        return create_math_sc_output<false>(u0, u1, if_abc);
    }
}

} // namespace

TEST_CASE("Short circuit solver") {
    // Test case grid
    // source -- bus --- line -- bus -- fault(type varying as per subcase)

    // Grid for short circuit
    MathModelTopology topo_sc;
    topo_sc.slack_bus_ = 0;
    topo_sc.phase_shift = {0.0, 0.0};
    topo_sc.branch_bus_idx = {{0, 1}};
    topo_sc.sources_per_bus = {from_sparse, {0, 1, 1}};
    topo_sc.shunts_per_bus = {from_sparse, {0, 0, 0}};
    topo_sc.load_gens_per_bus = {from_sparse, {0, 0, 0}};
    IdxVector const fault_bus_indptr{0, 0, 1};

    // Impedance / admittances
    // source
    double const vref = 1.1;
    DoubleComplex const yref{10.0 - 50.0i};
    DoubleComplex const zref{1.0 / yref};
    // line
    DoubleComplex const y0{1.0 - 2.0i};
    DoubleComplex const y0_0{0.5 + 0.5i};
    DoubleComplex const z0{1.0 / y0};
    DoubleComplex const z0_0{1.0 / y0_0};
    // fault
    DoubleComplex const z_fault{1.0 + 1.0i};
    DoubleComplex const y_fault{1.0 / z_fault};
    DoubleComplex const z_fault_solid{0.0 + 0.0i};
    DoubleComplex const y_fault_solid{std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()};

    // params sym
    MathModelParam<true> param_sc_sym;
    param_sc_sym.branch_param = {{y0, -y0, -y0, y0}};
    param_sc_sym.source_param = {yref};

    // params asym
    MathModelParam<false> param_sc_asym;
    ComplexTensor<false> const y0a{(2.0 * y0 + y0_0) / 3.0, (y0_0 - y0) / 3.0};
    param_sc_asym.branch_param = {{y0a, -y0a, -y0a, y0a}};
    ComplexTensor<false> const yref_asym{yref};
    param_sc_asym.source_param = {yref_asym};

    // topo and param ptr
    auto topo_sc_ptr = std::make_shared<MathModelTopology const>(topo_sc);
    auto param_sym_ptr = std::make_shared<MathModelParam<true> const>(param_sc_sym);
    auto param_asym_ptr = std::make_shared<MathModelParam<false> const>(param_sc_asym);

    SUBCASE("Test short circuit solver 3ph") {
        YBus<false> y_bus_asym{topo_sc_ptr, param_asym_ptr};
        MathSolver<false> solver{topo_sc_ptr};
        auto sc_input = create_sc_test_input(three_phase, FaultPhase::abc, y_fault, vref, fault_bus_indptr);
        auto sc_output_ref = create_sc_test_output<false>(three_phase, z_fault, z0, z0_0, vref, zref);
        CalculationInfo info;
        auto output = solver.run_short_circuit(sc_input, info, CalculationMethod::iec60909, y_bus_asym);
        assert_sc_output<false>(output, sc_output_ref);

        auto sc_input_default =
            create_sc_test_input(three_phase, FaultPhase::default_value, y_fault, vref, fault_bus_indptr);
        CHECK_THROWS_AS(solver.run_short_circuit(sc_input_default, info, CalculationMethod::iec60909, y_bus_asym),
                        InvalidShortCircuitPhaseOrType);
    }

    SUBCASE("Test short circuit solver 3ph solid fault") {
        YBus<false> const y_bus_asym{topo_sc_ptr, param_asym_ptr};
        MathSolver<false> solver{topo_sc_ptr};
        auto sc_input = create_sc_test_input(three_phase, FaultPhase::abc, y_fault_solid, vref, fault_bus_indptr);
        auto sc_output_ref = create_sc_test_output<false>(three_phase, z_fault_solid, z0, z0_0, vref, zref);
        CalculationInfo info;
        auto output = solver.run_short_circuit(sc_input, info, CalculationMethod::iec60909, y_bus_asym);
        assert_sc_output<false>(output, sc_output_ref);
    }

    SUBCASE("Test short circuit solver 3ph sym params") {
        YBus<true> const y_bus_sym{topo_sc_ptr, param_sym_ptr};
        MathSolver<true> solver{topo_sc_ptr};
        auto sc_input = create_sc_test_input(three_phase, FaultPhase::abc, y_fault, vref, fault_bus_indptr);
        auto sc_output_ref = create_sc_test_output<true>(three_phase, z_fault, z0, z0_0, vref, zref);
        CalculationInfo info;
        auto output = solver.run_short_circuit(sc_input, info, CalculationMethod::iec60909, y_bus_sym);
        assert_sc_output<true>(output, sc_output_ref);

        auto sc_input_default =
            create_sc_test_input(three_phase, FaultPhase::default_value, y_fault, vref, fault_bus_indptr);
        CHECK_THROWS_AS(solver.run_short_circuit(sc_input_default, info, CalculationMethod::iec60909, y_bus_sym),
                        InvalidShortCircuitPhaseOrType);
    }

    SUBCASE("Test short circuit solver 3ph sym params solid fault") {
        YBus<true> const y_bus_sym{topo_sc_ptr, param_sym_ptr};
        MathSolver<true> solver{topo_sc_ptr};
        auto sc_input = create_sc_test_input(three_phase, FaultPhase::abc, y_fault_solid, vref, fault_bus_indptr);
        auto sc_output_ref = create_sc_test_output<true>(three_phase, z_fault_solid, z0, z0_0, vref, zref);
        CalculationInfo info;
        auto output = solver.run_short_circuit(sc_input, info, CalculationMethod::iec60909, y_bus_sym);
        assert_sc_output<true>(output, sc_output_ref);
    }

    SUBCASE("Test short circuit solver 1phg") {
        YBus<false> const y_bus_asym{topo_sc_ptr, param_asym_ptr};
        MathSolver<false> solver{topo_sc_ptr};
        auto sc_input = create_sc_test_input(single_phase_to_ground, FaultPhase::a, y_fault, vref, fault_bus_indptr);
        auto sc_output_ref = create_sc_test_output<false>(single_phase_to_ground, z_fault, z0, z0_0, vref, zref);
        CalculationInfo info;
        auto output = solver.run_short_circuit(sc_input, info, CalculationMethod::iec60909, y_bus_asym);
        assert_sc_output<false>(output, sc_output_ref);

        auto sc_input_default =
            create_sc_test_input(single_phase_to_ground, FaultPhase::default_value, y_fault, vref, fault_bus_indptr);
        CHECK_THROWS_AS(solver.run_short_circuit(sc_input_default, info, CalculationMethod::iec60909, y_bus_asym),
                        InvalidShortCircuitPhaseOrType);
    }

    SUBCASE("Test short circuit solver 1phg solid fault") {
        YBus<false> const y_bus_asym{topo_sc_ptr, param_asym_ptr};
        MathSolver<false> solver{topo_sc_ptr};
        auto sc_input =
            create_sc_test_input(single_phase_to_ground, FaultPhase::a, y_fault_solid, vref, fault_bus_indptr);
        auto sc_output_ref = create_sc_test_output<false>(single_phase_to_ground, z_fault_solid, z0, z0_0, vref, zref);
        CalculationInfo info;
        auto output = solver.run_short_circuit(sc_input, info, CalculationMethod::iec60909, y_bus_asym);
        assert_sc_output<false>(output, sc_output_ref);
    }

    SUBCASE("Test short circuit solver 2ph") {
        YBus<false> const y_bus_asym{topo_sc_ptr, param_asym_ptr};
        MathSolver<false> solver{topo_sc_ptr};
        auto sc_input = create_sc_test_input(two_phase, FaultPhase::bc, y_fault, vref, fault_bus_indptr);
        auto sc_output_ref = create_sc_test_output<false>(two_phase, z_fault, z0, z0_0, vref, zref);
        CalculationInfo info;
        auto output = solver.run_short_circuit(sc_input, info, CalculationMethod::iec60909, y_bus_asym);
        assert_sc_output<false>(output, sc_output_ref);

        auto sc_input_default =
            create_sc_test_input(two_phase, FaultPhase::default_value, y_fault, vref, fault_bus_indptr);
        CHECK_THROWS_AS(solver.run_short_circuit(sc_input_default, info, CalculationMethod::iec60909, y_bus_asym),
                        InvalidShortCircuitPhaseOrType);
    }

    SUBCASE("Test short circuit solver 2ph solid fault") {
        YBus<false> const y_bus_asym{topo_sc_ptr, param_asym_ptr};
        MathSolver<false> solver{topo_sc_ptr};
        auto sc_input = create_sc_test_input(two_phase, FaultPhase::bc, y_fault_solid, vref, fault_bus_indptr);
        auto sc_output_ref = create_sc_test_output<false>(two_phase, z_fault_solid, z0, z0_0, vref, zref);
        CalculationInfo info;
        auto output = solver.run_short_circuit(sc_input, info, CalculationMethod::iec60909, y_bus_asym);
        assert_sc_output<false>(output, sc_output_ref);
    }

    SUBCASE("Test short circuit solver 2phg") {
        YBus<false> const y_bus_asym{topo_sc_ptr, param_asym_ptr};
        MathSolver<false> solver{topo_sc_ptr};
        auto sc_input = create_sc_test_input(two_phase_to_ground, FaultPhase::bc, y_fault, vref, fault_bus_indptr);
        auto sc_output_ref = create_sc_test_output<false>(two_phase_to_ground, z_fault, z0, z0_0, vref, zref);
        CalculationInfo info;
        auto output = solver.run_short_circuit(sc_input, info, CalculationMethod::iec60909, y_bus_asym);
        assert_sc_output<false>(output, sc_output_ref);

        auto sc_input_default =
            create_sc_test_input(two_phase_to_ground, FaultPhase::default_value, y_fault, vref, fault_bus_indptr);
        CHECK_THROWS_AS(solver.run_short_circuit(sc_input_default, info, CalculationMethod::iec60909, y_bus_asym),
                        InvalidShortCircuitPhaseOrType);
    }

    SUBCASE("Test short circuit solver 2phg solid") {
        YBus<false> const y_bus_asym{topo_sc_ptr, param_asym_ptr};
        MathSolver<false> solver{topo_sc_ptr};
        auto sc_input =
            create_sc_test_input(two_phase_to_ground, FaultPhase::bc, y_fault_solid, vref, fault_bus_indptr);
        auto sc_output_ref = create_sc_test_output<false>(two_phase_to_ground, z_fault_solid, z0, z0_0, vref, zref);
        CalculationInfo info;
        auto output = solver.run_short_circuit(sc_input, info, CalculationMethod::iec60909, y_bus_asym);
        assert_sc_output<false>(output, sc_output_ref);
    }

    SUBCASE("Test short circuit solver no faults") {
        YBus<true> const y_bus_sym{topo_sc_ptr, param_sym_ptr};
        YBus<false> const y_bus_asym{topo_sc_ptr, param_asym_ptr};
        MathSolver<false> solver_asym{topo_sc_ptr};
        ShortCircuitInput sc_input;
        sc_input.source = {vref};
        auto asym_sc_output_ref = blank_sc_output<false>(vref);
        CalculationInfo info;
        auto asym_output = solver_asym.run_short_circuit(sc_input, info, CalculationMethod::iec60909, y_bus_asym);
        assert_sc_output<false>(asym_output, asym_sc_output_ref);

        MathSolver<true> solver_sym{topo_sc_ptr};
        auto sym_sc_output_ref = blank_sc_output<true>(vref);
        auto sym_output = solver_sym.run_short_circuit(sc_input, info, CalculationMethod::iec60909, y_bus_sym);
        assert_sc_output<true>(sym_output, sym_sc_output_ref);
    }

    SUBCASE("Test fault on source bus") {
        // Grid for short circuit
        MathModelTopology topo_comp;
        topo_sc.slack_bus_ = 0;
        topo_comp.phase_shift = {0.0};
        topo_comp.branch_bus_idx = {};
        topo_comp.sources_per_bus = {from_sparse, {0, 1}};
        topo_comp.shunts_per_bus = {from_sparse, {0, 0}};
        topo_comp.load_gens_per_bus = {from_sparse, {0, 0}};
        IdxVector const fault_bus_indptr_2 = {0, 1};
        // params source injection
        MathModelParam<false> asym_param_comp;
        asym_param_comp.source_param = {yref_asym};
        MathModelParam<true> sym_param_comp;
        sym_param_comp.source_param = {yref};
        // topo and param ptr
        auto topo_comp_ptr = std::make_shared<MathModelTopology const>(topo_comp);
        auto asym_param_comp_ptr = std::make_shared<MathModelParam<false> const>(asym_param_comp);
        auto sym_param_comp_ptr = std::make_shared<MathModelParam<true> const>(sym_param_comp);
        MathSolver<false> solver{topo_comp_ptr};
        MathSolver<true> sym_solver{topo_comp_ptr};
        YBus<false> y_bus_asym{topo_comp_ptr, asym_param_comp_ptr};
        YBus<true> y_bus_sym{topo_comp_ptr, sym_param_comp_ptr};

        DoubleComplex const if_comp = vref / (zref + z_fault);
        DoubleComplex const uf_comp = vref - if_comp * zref;
        DoubleComplex const if_comp_solid = vref / (zref + z_fault_solid);
        DoubleComplex const uf_comp_solid = vref - if_comp_solid * zref;

        DoubleComplex const if_b_comp = (vref * (a * a - a)) / (2.0 * zref + z_fault);
        DoubleComplex const uf_b_comp = vref * a * a - if_b_comp * zref;
        DoubleComplex const uf_c_comp = vref * a + if_b_comp * zref;

        DoubleComplex const if_b_comp_solid = (vref * (a * a - a)) / (2.0 * zref + z_fault_solid);
        DoubleComplex const uf_b_comp_solid = vref * a * a - if_b_comp_solid * zref;
        DoubleComplex const uf_c_comp_solid = vref * a + if_b_comp_solid * zref;

        DoubleComplex const uf_b_2phg = (vref * (a * a + a)) * z_fault / (zref + 2.0 * z_fault);
        DoubleComplex const if_b_2phg = (vref * a * a - uf_b_2phg) / zref;
        DoubleComplex const if_c_2phg = (vref * a - uf_b_2phg) / zref;
        DoubleComplex const uf_b_2phg_solid = 0.0 + 0.0i;
        DoubleComplex const if_b_2phg_solid = vref * a * a / zref;
        DoubleComplex const if_c_2phg_solid = vref * a / zref;

        SUBCASE("Source on 3ph sym fault") {
            ShortCircuitMathOutput<true> sc_output_ref;
            sc_output_ref.u_bus = {uf_comp};
            sc_output_ref.fault = {{if_comp}};
            sc_output_ref.branch = {};
            sc_output_ref.source = {{if_comp}};

            auto sc_input = create_sc_test_input(three_phase, FaultPhase::abc, y_fault, vref, fault_bus_indptr_2);
            CalculationInfo info;
            auto output = sym_solver.run_short_circuit(sc_input, info, CalculationMethod::iec60909, y_bus_sym);
            assert_sc_output<true>(output, sc_output_ref);
        }

        SUBCASE("Source on 3ph sym solid fault") {
            ShortCircuitMathOutput<true> sc_output_ref;
            sc_output_ref.u_bus = {DoubleComplex{uf_comp_solid}};
            sc_output_ref.fault = {{if_comp_solid}};
            sc_output_ref.branch = {};
            sc_output_ref.source = {{if_comp_solid}};

            auto sc_input = create_sc_test_input(three_phase, FaultPhase::abc, y_fault_solid, vref, fault_bus_indptr_2);
            CalculationInfo info;
            auto output = sym_solver.run_short_circuit(sc_input, info, CalculationMethod::iec60909, y_bus_sym);
            assert_sc_output<true>(output, sc_output_ref);
        }

        SUBCASE("Source on 3ph fault") {
            ShortCircuitMathOutput<false> sc_output_ref;
            sc_output_ref.u_bus = {ComplexValue<false>{uf_comp}};
            sc_output_ref.fault = {{ComplexValue<false>{if_comp}}};
            sc_output_ref.branch = {};
            sc_output_ref.source = {{ComplexValue<false>{if_comp}}};

            auto sc_input = create_sc_test_input(three_phase, FaultPhase::abc, y_fault, vref, fault_bus_indptr_2);
            CalculationInfo info;
            auto output = solver.run_short_circuit(sc_input, info, CalculationMethod::iec60909, y_bus_asym);
            assert_sc_output<false>(output, sc_output_ref);
        }

        SUBCASE("Source on 1phg fault") {
            ShortCircuitMathOutput<false> sc_output_ref;
            sc_output_ref.u_bus = {ComplexValue<false>{uf_comp, vref * a * a, vref * a}};
            sc_output_ref.fault = {{ComplexValue<false>{if_comp, 0, 0}}};
            sc_output_ref.branch = {};
            sc_output_ref.source = {{ComplexValue<false>{if_comp, 0, 0}}};

            auto sc_input =
                create_sc_test_input(single_phase_to_ground, FaultPhase::a, y_fault, vref, fault_bus_indptr_2);
            CalculationInfo info;
            auto output = solver.run_short_circuit(sc_input, info, CalculationMethod::iec60909, y_bus_asym);
            assert_sc_output<false>(output, sc_output_ref);
        }

        SUBCASE("Source on 1phg solid fault") {
            ShortCircuitMathOutput<false> sc_output_ref;
            sc_output_ref.u_bus = {ComplexValue<false>{uf_comp_solid, vref * a * a, vref * a}};
            sc_output_ref.fault = {{ComplexValue<false>{if_comp_solid, 0, 0}}};
            sc_output_ref.branch = {};
            sc_output_ref.source = {{ComplexValue<false>{if_comp_solid, 0, 0}}};

            auto sc_input =
                create_sc_test_input(single_phase_to_ground, FaultPhase::a, y_fault_solid, vref, fault_bus_indptr_2);
            CalculationInfo info;
            auto output = solver.run_short_circuit(sc_input, info, CalculationMethod::iec60909, y_bus_asym);
            assert_sc_output<false>(output, sc_output_ref);
        }

        SUBCASE("Source on 2ph fault") {
            ShortCircuitMathOutput<false> sc_output_ref;
            sc_output_ref.u_bus = {ComplexValue<false>{vref, uf_b_comp, uf_c_comp}};
            sc_output_ref.fault = {{ComplexValue<false>{0.0, if_b_comp, -if_b_comp}}};
            sc_output_ref.branch = {};
            sc_output_ref.source = {{ComplexValue<false>{0.0, if_b_comp, -if_b_comp}}};

            auto sc_input = create_sc_test_input(two_phase, FaultPhase::bc, y_fault, vref, fault_bus_indptr_2);
            CalculationInfo info;
            auto output = solver.run_short_circuit(sc_input, info, CalculationMethod::iec60909, y_bus_asym);
            assert_sc_output<false>(output, sc_output_ref);
        }

        SUBCASE("Source on 2ph solid fault") {
            ShortCircuitMathOutput<false> sc_output_ref;
            sc_output_ref.u_bus = {ComplexValue<false>{vref, uf_b_comp_solid, uf_c_comp_solid}};
            sc_output_ref.fault = {{ComplexValue<false>{0.0, if_b_comp_solid, -if_b_comp_solid}}};
            sc_output_ref.branch = {};
            sc_output_ref.source = {{ComplexValue<false>{0.0, if_b_comp_solid, -if_b_comp_solid}}};

            auto sc_input = create_sc_test_input(two_phase, FaultPhase::bc, y_fault_solid, vref, fault_bus_indptr_2);
            CalculationInfo info;
            auto output = solver.run_short_circuit(sc_input, info, CalculationMethod::iec60909, y_bus_asym);
            assert_sc_output<false>(output, sc_output_ref);
        }

        SUBCASE("Source on 2phg fault") {
            ShortCircuitMathOutput<false> sc_output_ref;
            sc_output_ref.u_bus = {ComplexValue<false>{vref, uf_b_2phg, uf_b_2phg}};
            sc_output_ref.fault = {{ComplexValue<false>{0.0, if_b_2phg, if_c_2phg}}};
            sc_output_ref.branch = {};
            sc_output_ref.source = {{ComplexValue<false>{0.0, if_b_2phg, if_c_2phg}}};

            auto sc_input =
                create_sc_test_input(two_phase_to_ground, FaultPhase::bc, y_fault, vref, fault_bus_indptr_2);
            CalculationInfo info;
            auto output = solver.run_short_circuit(sc_input, info, CalculationMethod::iec60909, y_bus_asym);
            assert_sc_output<false>(output, sc_output_ref);
        }

        SUBCASE("Source on 2phg solid fault") {
            ShortCircuitMathOutput<false> sc_output_ref;
            sc_output_ref.u_bus = {ComplexValue<false>{vref, uf_b_2phg_solid, uf_b_2phg_solid}};
            sc_output_ref.fault = {{ComplexValue<false>{0.0, if_b_2phg_solid, if_c_2phg_solid}}};
            sc_output_ref.branch = {};
            sc_output_ref.source = {{ComplexValue<false>{0.0, if_b_2phg_solid, if_c_2phg_solid}}};

            auto sc_input =
                create_sc_test_input(two_phase_to_ground, FaultPhase::bc, y_fault_solid, vref, fault_bus_indptr_2);
            CalculationInfo info;
            auto output = solver.run_short_circuit(sc_input, info, CalculationMethod::iec60909, y_bus_asym);
            assert_sc_output<false>(output, sc_output_ref);
        }
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
    topo.slack_bus_ = 1;
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
    MathModelParam<true> param;
    param.branch_param = {{1.0, -1.0, -1.0, 1.0}};
    auto param_ptr = std::make_shared<MathModelParam<true> const>(param);
    auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
    YBus<true> y_bus_sym{topo_ptr, param_ptr};

    StateEstimationInput<true> se_input;
    se_input.source_status = {1};
    se_input.measured_voltage = {{1.0, 1.0}};

    MathSolver<true> solver{topo_ptr};
    CalculationInfo info;
    MathOutput<true> output = solver.run_state_estimation(se_input, 1e-10, 20, info, iterative_linear, y_bus_sym);

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
    topo.slack_bus_ = 0;
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

    MathModelParam<true> param;
    param.branch_param = {{1.0e3, -1.0e3, -1.0e3, 1.0e3}};

    StateEstimationInput<true> se_input;
    se_input.source_status = {1};
    se_input.load_gen_status = {1};
    se_input.measured_voltage = {{1.0, 0.1}};

    CalculationInfo info;
    MathOutput<true> output;

    SUBCASE("Source and branch") {
        /*
        network, v means voltage measured, p means power measured

         bus_0(v) -(p)-branch_0-- bus_1
            |                       |
        source_0(p)               load_0

        */
        topo.power_sensors_per_source = {from_sparse, {0, 1}};
        topo.power_sensors_per_branch_from = {from_sparse, {0, 1}};

        se_input.measured_source_power = {{1.93, 0.1}};
        se_input.measured_branch_from_power = {{1.97, 0.1}};

        auto param_ptr = std::make_shared<MathModelParam<true> const>(param);
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<true> const y_bus_sym{topo_ptr, param_ptr};
        MathSolver<true> solver{topo_ptr};
        output = solver.run_state_estimation(se_input, 1e-10, 20, info, iterative_linear, y_bus_sym);

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

        se_input.measured_load_gen_power = {{-1.93, 0.1}};
        se_input.measured_branch_to_power = {{-1.97, 0.1}};

        auto param_ptr = std::make_shared<MathModelParam<true> const>(param);
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<true> const y_bus_sym{topo_ptr, param_ptr};
        MathSolver<true> solver{topo_ptr};
        output = solver.run_state_estimation(se_input, 1e-10, 20, info, iterative_linear, y_bus_sym);

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

        se_input.measured_bus_injection = {{2.2, 0.2}};
        se_input.measured_source_power = {{1.93, 0.1}};
        se_input.measured_branch_from_power = {{1.97, 0.1}};

        auto param_ptr = std::make_shared<MathModelParam<true> const>(param);
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<true> const y_bus_sym{topo_ptr, param_ptr};
        MathSolver<true> solver{topo_ptr};
        output = solver.run_state_estimation(se_input, 1e-10, 20, info, iterative_linear, y_bus_sym);

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

        se_input.measured_bus_injection = {{2.2, 0.2}};
        se_input.measured_source_power = {{1.93, 0.1}};
        se_input.measured_branch_from_power = {{1.97, 0.1}};

        auto param_ptr = std::make_shared<MathModelParam<true> const>(param);
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<true> const y_bus_sym{topo_ptr, param_ptr};
        MathSolver<true> solver{topo_ptr};
        output = solver.run_state_estimation(se_input, 1e-10, 20, info, iterative_linear, y_bus_sym);

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

        se_input.measured_bus_injection = {{-2.2, 0.2}};
        se_input.measured_load_gen_power = {{-1.93, 0.1}};
        se_input.measured_branch_to_power = {{-1.97, 0.1}};

        auto param_ptr = std::make_shared<MathModelParam<true> const>(param);
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<true> const y_bus_sym{topo_ptr, param_ptr};
        MathSolver<true> solver{topo_ptr};
        output = solver.run_state_estimation(se_input, 1e-10, 20, info, iterative_linear, y_bus_sym);

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
        se_input.measured_load_gen_power = {{-3.0, 0.1}, {1.0, 0.1}};

        auto param_ptr = std::make_shared<MathModelParam<true> const>(param);
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<true> const y_bus_sym{topo_ptr, param_ptr};
        MathSolver<true> solver{topo_ptr};
        output = solver.run_state_estimation(se_input, 1e-10, 20, info, iterative_linear, y_bus_sym);

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
        se_input.measured_load_gen_power = {{-1.8, 0.1}, {0.9, 0.1}};
        se_input.measured_bus_injection = {{-1.1, 0.2}};

        auto param_ptr = std::make_shared<MathModelParam<true> const>(param);
        auto topo_ptr = std::make_shared<MathModelTopology const>(topo);
        YBus<true> const y_bus_sym{topo_ptr, param_ptr};
        MathSolver<true> solver{topo_ptr};
        output = solver.run_state_estimation(se_input, 1e-10, 20, info, iterative_linear, y_bus_sym);

        CHECK(real(output.bus_injection[1]) == doctest::Approx(-1.0));
        CHECK(real(output.load_gen[0].s) == doctest::Approx(-1.85));
        CHECK(real(output.load_gen[1].s) == doctest::Approx(0.85));
    }

    const ComplexValue<true> load_gen_s =
        std::accumulate(output.load_gen.begin(), output.load_gen.end(), ComplexValue<true>{},
                        [](auto const& a, auto const& b) { return a + b.s; });

    CHECK(output.bus_injection[0] == output.branch[0].s_f);
    CHECK(output.bus_injection[0] == output.source[0].s);
    CHECK(output.bus_injection[1] == output.branch[0].s_t);
    CHECK(real(output.bus_injection[1]) == doctest::Approx(real(load_gen_s)));
}

} // namespace power_grid_model
