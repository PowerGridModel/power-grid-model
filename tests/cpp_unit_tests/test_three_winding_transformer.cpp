// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include "doctest/doctest.h"
#include "power_grid_model/component/three_winding_transformer.hpp"
#include "power_grid_model/component/transformer.hpp"

namespace power_grid_model {

using namespace std::complex_literals;

TEST_CASE("Test three winding transformer") {
    double const u1 = 138e3, u2 = 69e3, u3 = 13.8e3, s1 = 60e6, s2 = 50e6, s3 = 10e6;

    ThreeWindingTransformerInputBasics input_basics{
        {1, 2, 3, 4, true, true, true},  // Create branch3 {{id}, node_1, node_2, node_3, status_1, status_2, status_3}
        138e3,                           // u1
        69e3,                            // u2
        13.8e3,                          // u3
        60e6,                            // s1
        50e6,                            // s2
        10e6,                            // s3
        0.09,                            // uk12
        0.06,                            // uk13
        0.03,                            // uk23
        200e3,                           // pk_12
        150e3,                           // pk_13
        100e3,                           // pk_23
        0.005,                           // i0
        50e3,                            // p0
        WindingType::wye_n,              // winding_12
        WindingType::delta,              // winding_13
        WindingType::delta,              // winding_23
        1,                               // clock_12
        1,                               // clock_13
        Branch3Side::side_1,             // tap side
        2,                               // tap_pos
        -8,                              // tap_min
        10,                              // tap_max
        0,                               // tap_nom
        1380                             // tap size
    };

    ThreeWindingTransformerInput input{
        input_basics,
        nan,  // uk_12_min
        nan,  // uk_12_max
        nan,  // uk_13_min
        nan,  // uk_13_max
        nan,  // uk_23_min
        nan,  // uk_23_max
        nan,  // pk_12_min
        nan,  // pk_12_max
        nan,  // pk_13_min
        nan,  // pk_13_max
        nan,  // pk_23_min
        nan,  // pk_23_max
        1.0,  // r1_grounding
        4.0,  // x1_grounding
        nan,  // r2_grounding
        nan,  // x2_grounding
        nan,  // r3_grounding
        nan   // x3_grounding
    };

    // Check what transformers should be tested
    std::vector<ThreeWindingTransformer> vec;
    // 0 YNd1d1
    vec.emplace_back(input, 138e3, 69e3, 13.8e3);
    // 1 YNyn12d1
    input.winding_2 = WindingType::wye_n;
    input.clock_12 = 12;
    vec.emplace_back(input, 138e3, 69e3, 13.8e3);
    // 2 Yny12d1
    input.winding_2 = WindingType::wye;
    vec.emplace_back(input, 138e3, 69e3, 13.8e3);

    for (ThreeWindingTransformer& transformer3 : vec) {
        CHECK(transformer3.math_model_type() == ComponentType::branch3);
    }

    double const uk_t1 = 0.5 * (0.09 / 50 + 0.06 / 10 - 0.03 / 10) * 60;
    double const uk_t2 = 0.5 * (0.09 / 50 - 0.06 / 10 + 0.03 / 10) * 50;
    double const uk_t3 = 0.5 * (-0.09 / 50 + 0.06 / 10 + 0.03 / 10) * 10;

    double const pk_t1 = 0.5 * (200e3 / 50 / 50 + 150e3 / 10 / 10 - 100e3 / 10 / 10) * 60 * 60;
    double const pk_t2 = 0.5 * (200e3 / 50 / 50 - 150e3 / 10 / 10 + 100e3 / 10 / 10) * 50 * 50;
    double const pk_t3 = 0.5 * (-200e3 / 50 / 50 + 150e3 / 10 / 10 + 100e3 / 10 / 10) * 10 * 10;

    double const u_t1 = 138e3 + 1 * 2 * 1380;

    // ********************
    double const base_i_1 = base_power_3p / 138e3 / sqrt3;
    double const base_i_2 = base_power_3p / 69e3 / sqrt3;
    double const base_i_3 = base_power_3p / 13.8e3 / sqrt3;

    /*
    double const base_y_1 = base_i_1 / (138e3 / sqrt3);
    double const base_y_2 = base_i_2 / (69e3 / sqrt3);
    double const base_y_3 = base_i_3 / (13.8e3 / sqrt3);

    double const z_series_abs_1 = uk_t1 * 138e3 * 138e3 / 60e6;
    double const z_series_abs_2 = uk_t2 * 69e3 * 69e3 / 50e6;
    double const z_series_abs_3 = uk_t3 * 13.8e3 * 13.8e3 / 10e6;

    double const r_series_1 = pk_t1 * 138e3 * 138e3 / 60e6 / 60e6;
    double const r_series_2 = pk_t2 * 69e3 * 69e3 / 50e6 / 50e6;
    double const r_series_3 = pk_t3 * 13.8e3 * 13.8e3 / 10e6 / 10e6;

    DoubleComplex const z_series_1 =
        r_series_1 + 1.0i * std::sqrt(z_series_abs_1 * z_series_abs_1 - r_series_1 * r_series_1);
    DoubleComplex const z_series_2 =
        r_series_2 + 1.0i * std::sqrt(z_series_abs_2 * z_series_abs_2 - r_series_2 * r_series_2);
    DoubleComplex const z_series_3 =
        r_series_3 + 1.0i * std::sqrt(z_series_abs_3 * z_series_abs_3 - r_series_3 * r_series_3);

    DoubleComplex const y_t1 = 1.0 / z_series_1 / base_y_1;
    DoubleComplex const y_t2 = 1.0 / z_series_2 / base_y_2;
    DoubleComplex const y_t3 = 1.0 / z_series_3 / base_y_3;
    //*******************
    */

    // tap functioning
    // add for reverse tap and different side taps

    // Add test for grounding too

    TransformerInput T1_input{
        {{2}, 0, 1, true, true},  // {{id}, from_node, to_node, from_status, to_status}
        u_t1,                     // u1
        u_t1,                     // u2
        60e6,                     // sn
        uk_t1,                    // uk
        pk_t1,                    // pk
        0.005,                    // i0
        50e3,                     // p0
        WindingType::wye_n,       // winding_from
        WindingType::wye_n,       // winding_to
        0,                        // clock
        BranchSide::from,         // tap_side
        0,                        // tap_pos
        0,                        // tap_min
        0,                        // tap_max
        0,                        // tap_nom
        0.0,                      // tap_size
        nan,                      // uk_min
        nan,                      // uk_max
        nan,                      // pk_min
        nan,                      // pk_max
        1.0,                      // r_grounding_from
        4.0,                      // x_grounding_from
        0,                        // r_grounding_to
        0                         // x_grounding_to
    };
    TransformerInput T2_input{
        {{2}, 0, 1, true, true},  // {{id}, from_node, to_node, from_status, to_status}
        69e3,                     // u1
        u_t1,                     // u2
        50e6,                     // sn
        uk_t2,                    // uk
        pk_t2,                    // pk
        0.0,                      // i0
        0.0,                      // p0
        WindingType::delta,       // winding_from
        WindingType::wye_n,       // winding_to
        11,                       // clock, reversed
        BranchSide::from,         // tap_side
        0,                        // tap_pos
        0,                        // tap_min
        0,                        // tap_max
        0,                        // tap_nom
        0.0,                      // tap_size
        nan,                      // uk_min
        nan,                      // uk_max
        nan,                      // pk_min
        nan,                      // pk_max
        0,                        // r_grounding_from
        0,                        // x_grounding_from
        0,                        // r_grounding_to
        0                         // x_grounding_to
    };
    TransformerInput T3_input{
        {{2}, 0, 1, true, true},  // {{id}, from_node, to_node, from_status, to_status}
        13.8e3,                   // u1
        u_t1,                     // u2
        10e6,                     // sn
        uk_t3,                    // uk
        pk_t3,                    // pk
        0.0,                      // i0
        0.0,                      // p0
        WindingType::delta,       // winding_from
        WindingType::wye_n,       // winding_to
        11,                       // clock, reversed
        BranchSide::from,         // tap_side
        0,                        // tap_pos
        0,                        // tap_min
        0,                        // tap_max
        0,                        // tap_nom
        0.0,                      // tap_size
        nan,                      // uk_min
        nan,                      // uk_max
        nan,                      // pk_min
        nan,                      // pk_max
        0,                        // r_grounding_from
        0,                        // x_grounding_from
        0,                        // r_grounding_to
        0                         // x_grounding_to
    };

    Transformer t1{T1_input, 138e3, 138e3}, t2{T2_input, 138e3, 69e3}, t3{T3_input, 13.8e3, 138e3};
    std::array<Transformer, 3> calc_trafos{t1, t2, t3};

    std::array<Transformer, 3> conv_trafos = vec[0].convert_to_two_winding_transformers_pub();

    std::array<BranchCalcParam<true>, 3> calc_params{}, test_params = vec[0].calc_param<true>();
    // TODO: Replace loop of both sym and asym vec. CHeck for all loop
    for (size_t i = 0; i < 1; i++) {
        calc_params[i] = calc_trafos[i].calc_param<true>();
        for (size_t value_no = 0; value_no < calc_params[i].value.size(); ++value_no) {
            CHECK((cabs(calc_params[i].value[value_no] - test_params[i].value[value_no]) < numerical_tolerance));
        }
    }

    std::array<BranchCalcParam<false>, 3> asym_params{}, asym_test_params = vec[0].calc_param<false>();
    for (size_t i = 0; i < 1; i++) {
        asym_params[i] = calc_trafos[i].calc_param<false>();
        for (size_t value_no = 0; value_no < asym_params[i].value.size(); ++value_no) {
            CHECK((cabs(asym_params[i].value[value_no] - asym_test_params[i].value[value_no]) < numerical_tolerance)
                      .all());
        }
    }

    SUBCASE("Check output") {
        // TODO asym output check
        BranchMathOutput<true> b1_output, b2_output, b3_output;
        // Branch initialization: s_f, s_t, i_f, i_t
        // sum of s_t and i_t = 0
        b1_output = {(1.0 - 2.0i), (2.0 - 3.0i), (1.5 - 2.5i), (2.5 - 3.5i)};
        b2_output = {(2.0 - 3.0i), (-3.0 + 2.0i), (1.5 - 2.5i), (-4.0 + 1.5i)};
        b3_output = {(3.0 + 1.0i), (1.0 + 1.0i), (1.5 - 2.5i), (1.5 + 2.0i)};

        Branch3Output<true> sym_output = vec[0].get_output(b1_output, b2_output, b3_output);

        double const out_p_1 = base_power<true> * 1;
        double const out_q_1 = base_power<true> * (-2);
        double const out_i_1 = base_i_1 * cabs(b1_output.i_f);
        double const out_s_1 = base_power<true> * cabs(b1_output.s_f);

        double const out_p_2 = base_power<true> * 2;
        double const out_q_2 = base_power<true> * (-3);
        double const out_i_2 = base_i_2 * cabs(b2_output.i_f);
        double const out_s_2 = base_power<true> * cabs(b2_output.s_f);

        double const out_p_3 = base_power<true> * 3;
        double const out_q_3 = base_power<true> * 1;
        double const out_i_3 = base_i_3 * cabs(b3_output.i_f);
        double const out_s_3 = base_power<true> * cabs(b3_output.s_f);
        // max loading is at branch 3
        double const out_loading = 0.316227766;

        CHECK(sym_output.id == 1);
        CHECK(sym_output.energized);
        CHECK(sym_output.p_1 == doctest::Approx(out_p_1));
        CHECK(sym_output.q_1 == doctest::Approx(out_q_1));
        CHECK(sym_output.i_1 == doctest::Approx(out_i_1));
        CHECK(sym_output.s_1 == doctest::Approx(out_s_1));
        CHECK(sym_output.p_2 == doctest::Approx(out_p_2));
        CHECK(sym_output.q_2 == doctest::Approx(out_q_2));
        CHECK(sym_output.i_2 == doctest::Approx(out_i_2));
        CHECK(sym_output.s_2 == doctest::Approx(out_s_2));
        CHECK(sym_output.p_3 == doctest::Approx(out_p_3));
        CHECK(sym_output.q_3 == doctest::Approx(out_q_3));
        CHECK(sym_output.i_3 == doctest::Approx(out_i_3));
        CHECK(sym_output.s_3 == doctest::Approx(out_s_3));

        BranchMathOutput<false> asym_b1_output, asym_b2_output, asym_b3_output;
        asym_b1_output = {{(1.0 - 2.0i), (1.0 - 2.0i), (1.0 - 2.0i)},
                          {(2.0 - 3.0i), (2.0 - 3.0i), (2.0 - 3.0i)},
                          {(1.5 - 2.5i), (1.5 - 2.5i), (1.5 - 2.5i)},
                          {(2.5 - 3.5i), (2.5 - 3.5i), (2.5 - 3.5i)}};
        asym_b2_output = {{(2.0 - 3.0i), (2.0 - 3.0i), (2.0 - 3.0i)},
                          {(-3.0 + 2.0i), (-3.0 + 2.0i), (-3.0 + 2.0i)},
                          {(1.5 - 2.5i), (1.5 - 2.5i), (1.5 - 2.5i)},
                          {(-4.0 + 1.5i), (-4.0 + 1.5i), (-4.0 + 1.5i)}};
        asym_b3_output = {{(3.0 + 1.0i), (3.0 + 1.0i), (3.0 + 1.0i)},
                          {(1.0 + 1.0i), (1.0 + 1.0i), (1.0 + 1.0i)},
                          {(1.5 - 2.5i), (1.5 - 2.5i), (1.5 - 2.5i)},
                          {(1.5 + 2.0i), (1.5 + 2.0i), (1.5 + 2.0i)}};
    }

    SUBCASE("invalid input") {
        input.node_2 = 2;
        CHECK_THROWS_AS(ThreeWindingTransformer(input, 138e3, 69e3, 13.8e3), InvalidBranch3);
        input.node_2 = 3;

        // Should we check transformer windings and clocks and tap limits? covered from transformer
    }

    // check what gets covered by this
    SUBCASE("Test i base") {
        CHECK(vec[0].base_i_1() == doctest::Approx(base_i_1));
        CHECK(vec[0].base_i_2() == doctest::Approx(base_i_2));
        CHECK(vec[0].base_i_3() == doctest::Approx(base_i_3));
    }

    // Check if needed, what code is covered
    // TODO is anything more needed for updating
    SUBCASE("update - check changed") {
        SUBCASE("update tap") {
            auto changed = vec[0].update(ThreeWindingTransformerUpdate{{{1}, na_IntS, na_IntS, na_IntS}, -2});
            CHECK(!changed.topo);
            CHECK(changed.param);
        }
        SUBCASE("update status_1") {
            auto changed = vec[0].update(ThreeWindingTransformerUpdate{{{1}, false, true, true}, na_IntS});
            CHECK(changed.topo);
            CHECK(changed.param);
        }
        SUBCASE("update status_2") {
            auto changed = vec[0].update(ThreeWindingTransformerUpdate{{{1}, true, false, true}, na_IntS});
            CHECK(changed.topo);
            CHECK(changed.param);
        }
        SUBCASE("update status_3") {
            auto changed = vec[0].update(ThreeWindingTransformerUpdate{{{1}, true, true, false}, na_IntS});
            CHECK(changed.topo);
            CHECK(changed.param);
        }
        SUBCASE("update status") {
            auto changed = vec[0].update(ThreeWindingTransformerUpdate{{{1}, false, false, false}, na_IntS});
            CHECK(changed.topo);
            CHECK(changed.param);
        }
        SUBCASE("update status & tap") {
            auto changed = vec[0].update(ThreeWindingTransformerUpdate{{{1}, false, false, false}, -2});
            CHECK(changed.topo);
            CHECK(changed.param);
        }
        SUBCASE("update none") {
            auto changed = vec[0].update(ThreeWindingTransformerUpdate{{{1}, na_IntS, na_IntS, na_IntS}, na_IntS});
            CHECK(!changed.topo);
            CHECK(!changed.param);
        }
    }

    SUBCASE("3 winding specifics") {
    }
}

// output checking
// update test
// invalid inputs: same nodes, clock, tap position limits
// base_i check
// status setting check

}  // namespace power_grid_model
