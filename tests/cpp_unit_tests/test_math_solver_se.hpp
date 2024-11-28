// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// In this unit test the powerflow solvers are tested

#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/common/three_phase_tensor.hpp>
#include <power_grid_model/math_solver/newton_raphson_pf_solver.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {
template <symmetry_tag sym> inline void check_close(auto const& x, auto const& y, auto const& tolerance) {
    if constexpr (is_symmetric_v<sym>) {
        CHECK(cabs((x) - (y)) < (tolerance));
    } else {
        CHECK((cabs((x) - (y)) < (tolerance)).all());
    }
}

template <symmetry_tag sym> inline void check_close(auto const& x, auto const& y) {
    check_close<sym>(x, y, numerical_tolerance);
}
inline void check_close(auto const& x, auto const& y, auto const& tolerance) {
    check_close<symmetric_t>(x, y, tolerance);
}
inline void check_close(auto const& x, auto const& y) { check_close<symmetric_t>(x, y); }

template <symmetry_tag sym>
inline void assert_output(SolverOutput<sym> const& output, SolverOutput<sym> const& output_ref,
                          bool normalize_phase = false, double tolerance = numerical_tolerance) {
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

TEST_CASE_TEMPLATE_DEFINE("Test math solver - SE", SolverType, test_math_solver_se_id) {
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

    auto const run = [](SolverType& solver, StateEstimationInput<sym> const& input, double err_tol, Idx max_iter,
                        CalculationInfo& calculation_info, YBus<sym> const& y_bus) {
        static_assert(SolverType::is_iterative); // otherwise, call different version
        return solver.run_state_estimation(y_bus, input, err_tol, max_iter, calculation_info);
    };

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

    // state estimation input
    // symmetric, with u angle
    StateEstimationInput<sym> se_input_angle = [&] {
        StateEstimationInput<sym> result;
        if constexpr (is_symmetric_v<sym>) {
            result.shunt_status = {1};
            result.load_gen_status = {1, 1, 1, 1, 1, 1, 0};
            result.source_status = {1};
            result.measured_voltage = {{output_ref.u[0], 1.0}, {output_ref.u[2], 1.0}, {output_ref.u[2], 1.0}};
            result.measured_bus_injection = {{output_ref.source[0].s + output_ref.load_gen[0].s +
                                                  output_ref.load_gen[1].s + output_ref.load_gen[2].s,
                                              0.5, 0.5}};
            result.measured_source_power = {{output_ref.source[0].s, 0.5, 0.5}, {output_ref.source[0].s, 0.5, 0.5}};
            result.measured_load_gen_power = {
                {output_ref.load_gen[3].s, 0.5, 0.5},
                {output_ref.load_gen[4].s, 0.5, 0.5},
                {output_ref.load_gen[5].s, 0.5, 0.5},
                {500.0, 0.5, 0.5},
            };
            result.measured_shunt_power = {
                {output_ref.shunt[0].s, 0.5, 0.5},
            };

            result.measured_branch_from_power = {
                {output_ref.branch[0].s_f, 0.5, 0.5},
            };
            result.measured_branch_to_power = {
                {output_ref.branch[0].s_t, 0.5, 0.5},
                {output_ref.branch[0].s_t, 0.5, 0.5},
                {output_ref.branch[1].s_t, 0.5, 0.5},
            };
        } else {
            result.shunt_status = {1};
            result.load_gen_status = {1, 1, 1, 1, 1, 1, 0};
            result.source_status = {1};
            result.measured_voltage = {{ComplexValue<asymmetric_t>{output_ref.u[0]}, 1.0},
                                       {ComplexValue<asymmetric_t>{output_ref.u[2]}, 1.0},
                                       {ComplexValue<asymmetric_t>{output_ref.u[2]}, 1.0}};
            result.measured_bus_injection = {{(output_ref.source[0].s + output_ref.load_gen[0].s +
                                               output_ref.load_gen[1].s + output_ref.load_gen[2].s) *
                                                  RealValue<asymmetric_t>{1.0},
                                              RealValue<asymmetric_t>{0.5}, RealValue<asymmetric_t>{0.5}}};
            result.measured_source_power = {{output_ref.source[0].s * RealValue<asymmetric_t>{1.0},
                                             RealValue<asymmetric_t>{0.5}, RealValue<asymmetric_t>{0.5}},
                                            {output_ref.source[0].s * RealValue<asymmetric_t>{1.0},
                                             RealValue<asymmetric_t>{0.5}, RealValue<asymmetric_t>{0.5}}};
            result.measured_load_gen_power = {
                {output_ref.load_gen[3].s * RealValue<asymmetric_t>{1.0}, RealValue<asymmetric_t>{0.5},
                 RealValue<asymmetric_t>{0.5}},
                {output_ref.load_gen[4].s * RealValue<asymmetric_t>{1.0}, RealValue<asymmetric_t>{0.5},
                 RealValue<asymmetric_t>{0.5}},
                {output_ref.load_gen[5].s * RealValue<asymmetric_t>{1.0}, RealValue<asymmetric_t>{0.5},
                 RealValue<asymmetric_t>{0.5}},
                {500.0 * RealValue<asymmetric_t>{1.0}, RealValue<asymmetric_t>{0.5}, RealValue<asymmetric_t>{0.5}},
            };
            result.measured_shunt_power = {
                {output_ref.shunt[0].s * RealValue<asymmetric_t>{1.0}, RealValue<asymmetric_t>{0.5},
                 RealValue<asymmetric_t>{0.5}},
            };

            result.measured_branch_from_power = {
                {output_ref.branch[0].s_f * RealValue<asymmetric_t>{1.0}, RealValue<asymmetric_t>{0.5},
                 RealValue<asymmetric_t>{0.5}},
            };
            result.measured_branch_to_power = {
                {output_ref.branch[0].s_t * RealValue<asymmetric_t>{1.0}, RealValue<asymmetric_t>{0.5},
                 RealValue<asymmetric_t>{0.5}},
                {output_ref.branch[0].s_t * RealValue<asymmetric_t>{1.0}, RealValue<asymmetric_t>{0.5},
                 RealValue<asymmetric_t>{0.5}},
                {output_ref.branch[1].s_t * RealValue<asymmetric_t>{1.0}, RealValue<asymmetric_t>{0.5},
                 RealValue<asymmetric_t>{0.5}},
            };
        }
        return result;
    }();

    // symmetric, without angle
    // no angle, keep the angle of 2nd measurement of bus2, which will be ignored
    StateEstimationInput<sym> se_input_no_angle = [&] {
        StateEstimationInput<sym> result = se_input_angle;
        if constexpr (is_symmetric_v<sym>) {
            result.measured_voltage[0].value = DoubleComplex{cabs(result.measured_voltage[0].value), nan};
            result.measured_voltage[1].value = DoubleComplex{cabs(result.measured_voltage[1].value), nan};
        } else { // TODO(mgovers): same as above; delete?
            result.measured_voltage[0].value = cabs(result.measured_voltage[0].value) + DoubleComplex{0.0, nan};
            result.measured_voltage[1].value = cabs(result.measured_voltage[1].value) + DoubleComplex{0.0, nan};
        }
        return result;
    }();

    // with angle, const z
    // set open for load 01, 34, scale load 5 (sensor 2)
    StateEstimationInput<sym> se_input_angle_const_z = [&] {
        StateEstimationInput<sym> result = se_input_angle;
        result.load_gen_status[0] = 0;
        result.load_gen_status[1] = 0;
        result.load_gen_status[3] = 0;
        result.load_gen_status[4] = 0;
        result.measured_load_gen_power[2].value *= 3.0;
        return result;
    }();

    SUBCASE("Test se with angle") {
        SolverType solver{y_bus, topo_ptr};
        CalculationInfo info;
        SolverOutput<sym> output;

        output = run(solver, se_input_angle, 1e-10, 20, info, y_bus);
        assert_output(output, output_ref);
    }

    SUBCASE("Test se without angle") {
        SolverType solver{y_bus, topo_ptr};
        CalculationInfo info;
        SolverOutput<sym> output;

        output = run(solver, se_input_no_angle, 1e-10, 20, info, y_bus);
        assert_output(output, output_ref, true);
    }

    SUBCASE("Test se with angle, const z") {
        SolverType solver{y_bus, topo_ptr};
        CalculationInfo info;
        SolverOutput<sym> output;

        output = run(solver, se_input_angle_const_z, 1e-10, 20, info, y_bus);
        assert_output(output, output_ref_z);
    }

    SUBCASE("Test se with angle and different power variances") {
        SolverType solver{y_bus, topo_ptr};
        CalculationInfo info;
        auto& branch_from_power = se_input_angle.measured_branch_from_power.front();
        branch_from_power.p_variance = RealValue<sym>{0.25};
        branch_from_power.q_variance = RealValue<sym>{0.75};
        SolverOutput<sym> output;

        output = run(solver, se_input_angle, 1e-10, 20, info, y_bus);
        assert_output(output, output_ref);
    }
}

} // namespace power_grid_model
