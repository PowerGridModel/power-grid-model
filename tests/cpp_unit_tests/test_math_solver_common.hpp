// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// In this unit test the powerflow solvers are tested

#pragma once

#include <power_grid_model/calculation_parameters.hpp>
#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/common/three_phase_tensor.hpp>

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

template <symmetry_tag sym_type> struct SteadyStateSolverTestGrid {
    /*
    network

                                                     shunt0 (ys)
                          (y0, ys0)           (y1)       |
    source --yref-- bus0 --branch0-- bus1 --branch1--  bus2
                     |                |                  |
                  load012          load345          load6 (not connected)

    uref = 1.10
    u0 = 1.08 -1deg
    u1 = 0.97 -4deg
    u2 = 0.90 -37deg
    */
    using sym = sym_type;

    static constexpr double shift_val = deg_30;

    // build topo
    auto topo() const {
        using enum LoadGenType;

        MathModelTopology result;
        result.slack_bus = 0;
        result.phase_shift = {0.0, 0.0, -shift_val};
        result.branch_bus_idx = {{0, 1}, {1, 2}};
        result.sources_per_bus = {from_sparse, {0, 1, 1, 1}};
        result.shunts_per_bus = {from_sparse, {0, 0, 0, 1}};
        result.load_gens_per_bus = {from_sparse, {0, 3, 6, 7}};
        result.load_gen_type = {
            const_pq, const_i, const_y, const_pq, const_i, const_y,
            const_pq // not connected
        };
        // sensors for se tests connected later only for se tests
        return result;
    };

    // build param, pf input, output, backwards
    // voltage
    static constexpr double vref = 1.1;
    static constexpr double v0 = 1.08;
    static constexpr double v1 = 0.97;
    static constexpr double v2 = 0.90;
    static constexpr double deg = deg_30 / 30.0;
    // NOLINTBEGIN(cppcoreguidelines-avoid-const-or-ref-data-members) // NOSONAR // should be constexpr but cant due to
    // std::exp
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
    DoubleComplex const ys = -i2_shunt_inj / u2; // output
    // NOLINTEND(cppcoreguidelines-avoid-const-or-ref-data-members) // NOSONAR

    SolverOutput<sym> output_ref() const {
        if constexpr (is_symmetric_v<sym>) {
            return get_sym_output_ref();
        } else {
            return get_asym_output_ref();
        }
    }

    // const z
    auto output_ref_z() const {
        SolverOutput<sym> result{output_ref()};
        for (size_t i = 0; i < 6; i++) {
            if (i % 3 == 2) {
                result.load_gen[i].i *= 3.0;
                result.load_gen[i].s *= 3.0;
            } else {
                result.load_gen[i] = {};
            }
        }
        return result;
    }

    // network param
    auto param() const {
        MathModelParam<sym> result;
        if constexpr (is_symmetric_v<sym>) {
            // branch parameter
            result.branch_param = {{y0 + ys0, -y0, -y0, y0 + ys0}, {y1, -y1 * shift, -y1 * conj(shift), y1}};
            // shunt
            result.shunt_param = {ys};
            // source
            result.source_param = {SourceCalcParam{.y1 = yref, .y0 = yref}};
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
            result.source_param = {SourceCalcParam{.y1 = yref, .y0 = yref}};
        }
        return result;
    }

  private:
    auto get_sym_output_ref() const {
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
        result.shunt = {{.s = conj(i2_shunt_inj) * u2, .i = i2_shunt_inj}};
        // load input and result, load6 is disconnected
        result.load_gen = {{.s = s0_load_inj / 3.0, .i = i0_load_inj / 3.0},
                           {.s = s0_load_inj / 3.0, .i = i0_load_inj / 3.0},
                           {.s = s0_load_inj / 3.0, .i = i0_load_inj / 3.0},
                           {.s = s1_load_inj / 3.0, .i = i1_load_inj / 3.0},
                           {.s = s1_load_inj / 3.0, .i = i1_load_inj / 3.0},
                           {.s = s1_load_inj / 3.0, .i = i1_load_inj / 3.0},
                           {.s = 0.0, .i = 0.0}};
        // bus injection
        result.bus_injection = {result.branch[0].s_f, result.branch[0].s_t + result.branch[1].s_f, 0};

        return result;
    };
    auto get_asym_output_ref() const {
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
};

} // namespace power_grid_model
