// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include "doctest/doctest.h"
#include "power_grid_model/component/three_winding_transformer.hpp"
#include "power_grid_model/component/transformer.hpp"

namespace power_grid_model {

using namespace std::complex_literals;

TEST_CASE("Test three winding transformer") {
    ThreeWindingTransformerInputBasics input_basics{
        {{1}, 2, 3, 4, true, true, true},  // Create branch3 {{id}, node_1, node_2, node_3, status_1, status_2,
                                           // status_3}
        138e3,                             // u1
        69e3,                              // u2
        13.8e3,                            // u3
        60e6,                              // s1
        50e6,                              // s2
        10e6,                              // s3
        0.09,                              // uk12
        0.06,                              // uk13
        0.03,                              // uk23
        200e3,                             // pk_12
        150e3,                             // pk_13
        100e3,                             // pk_23
        0.1,                               // i0
        50e3,                              // p0
        WindingType::wye_n,                // winding_12
        WindingType::delta,                // winding_13
        WindingType::delta,                // winding_23
        1,                                 // clock_12
        1,                                 // clock_13
        Branch3Side::side_1,               // tap side
        2,                                 // tap_pos
        -8,                                // tap_min
        10,                                // tap_max
        0,                                 // tap_nom
        1380                               // tap size
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
    // 0 YN d1 d1
    vec.emplace_back(input, 138e3, 69e3, 13.8e3);
    // 1 d YN1 YN1
    input.winding_1 = WindingType::delta;
    input.winding_2 = WindingType::wye_n;
    input.winding_3 = WindingType::wye_n;
    vec.emplace_back(input, 138e3, 69e3, 13.8e3);
    // 2 YN yn12 d1
    input.winding_1 = WindingType::wye_n;
    input.winding_3 = WindingType::delta;
    input.clock_12 = 12;
    input.r_grounding_2 = 2.0;
    input.x_grounding_2 = 6.0;
    vec.emplace_back(input, 138e3, 69e3, 13.8e3);
    // 3 Yn y12 d1
    input.winding_2 = WindingType::wye;
    vec.emplace_back(input, 138e3, 69e3, 13.8e3);
    // 4 tap max limit and side 2
    input.tap_side = Branch3Side::side_2;
    input.tap_pos = 12;
    vec.emplace_back(input, 138e3, 69e3, 13.8e3);
    // 5 tap min limit and side 3
    input.tap_side = Branch3Side::side_3;
    input.tap_pos = -14;
    vec.emplace_back(input, 138e3, 69e3, 13.8e3);
    // 6 reverse tap
    input.tap_pos = 2;
    input.tap_max = -10;
    input.tap_min = 8;
    vec.emplace_back(input, 138e3, 69e3, 13.8e3);
    // 7 uk,pk min and max
    input.uk_12_min = 0.08;
    input.uk_12_max = 0.09;
    input.uk_13_min = 0.07;
    input.uk_13_max = 0.05;
    input.uk_23_min = 0.02;
    input.uk_23_max = 0.04;
    input.pk_12_min = 180e3;
    input.pk_12_max = 220e3;
    input.pk_13_min = 130e3;
    input.pk_13_max = 170e3;
    input.pk_23_min = 80e3;
    input.pk_23_max = 120e3;
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

    // add for reverse tap and different side taps

    // Add test for grounding too

    TransformerInput T1_input{
        {{2}, 0, 1, true, true},  // {{id}, from_node, to_node, from_status, to_status}
        u_t1,                     // u1
        u_t1,                     // u2
        60e6,                     // sn
        uk_t1,                    // uk
        pk_t1,                    // pk
        0.1,                      // i0
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

    auto make_trafos = [](TransformerInput T1, TransformerInput T2, TransformerInput T3) {
        Transformer t1{T1, 138e3, 138e3}, t2{T2, 69e3, 138e3}, t3{T3, 13.8e3, 138e3};
        return std::array<Transformer, 3>{t1, t2, t3};
    };

    std::vector<std::array<Transformer, 3>> trafos_vec;
    // 0 YN d1 d1
    trafos_vec.emplace_back(make_trafos(T1_input, T2_input, T3_input));
    // 1 D YN1 YN1
    T2_input.winding_to = WindingType::delta;
    T3_input.winding_to = WindingType::delta;
    T2_input.winding_from = WindingType::wye_n;
    T3_input.winding_from = WindingType::wye_n;
    trafos_vec.emplace_back(make_trafos(T1_input, T2_input, T3_input));
    // 2 YN yn12 d1
    T2_input.winding_from = WindingType::wye_n;
    T3_input.winding_from = WindingType::delta;
    T2_input.winding_to = WindingType::wye_n;
    T3_input.winding_to = WindingType::wye_n;
    T2_input.clock = 12;
    T2_input.r_grounding_from = 2.0;
    T2_input.x_grounding_from = 6.0;
    trafos_vec.emplace_back(make_trafos(T1_input, T2_input, T3_input));
    // 3 Yn y12 d1
    T2_input.winding_from = WindingType::wye;
    trafos_vec.emplace_back(make_trafos(T1_input, T2_input, T3_input));
    // 4 tap max limit and side 2
    T1_input.u1 = 138e3;
    T1_input.u2 = 138e3;
    T2_input.u1 = 69e3 + 1 * 10 * 1380;
    T2_input.u2 = 138e3;
    T3_input.u2 = 138e3;
    trafos_vec.emplace_back(make_trafos(T1_input, T2_input, T3_input));
    // 5 tap min limit and side 3
    T2_input.u1 = 69e3;
    T3_input.u1 = 13.8e3 + 1 * (-8) * 1380;
    trafos_vec.emplace_back(make_trafos(T1_input, T2_input, T3_input));
    // 6 reverse tap
    T3_input.u1 = 13.8e3 + (-1) * 2 * 1380;
    trafos_vec.emplace_back(make_trafos(T1_input, T2_input, T3_input));
    // 7 uk,pk min and max
    // calculated manually
    T1_input.uk = 0.1575;
    T2_input.uk = -0.04375;
    T3_input.uk = 0.03625;
    T1_input.pk = 1040.4e3;
    T2_input.pk = -527.5e3;
    T3_input.pk = 116.1e3;
    trafos_vec.emplace_back(make_trafos(T1_input, T2_input, T3_input));

    // sym admittances of converted 3 2wdg transformers of 3wdg transformer vector
    for (size_t trafo = 0; trafo < trafos_vec.size(); ++trafo) {
        std::array<BranchCalcParam<true>, 3> calc_params, test_params = vec[trafo].calc_param<true>();
        for (size_t i = 0; i < 3; ++i) {
            calc_params[i] = trafos_vec[trafo][i].calc_param<true>();
        }
        for (size_t i = 0; i < 3; ++i) {
            for (size_t value_no = 0; value_no < 3; ++value_no) {
                CHECK(cabs(calc_params[i].value[value_no] - test_params[i].value[value_no]) < numerical_tolerance);
            }
        }
    }
    // asym admittance
    for (size_t trafo = 0; trafo < trafos_vec.size(); ++trafo) {
        std::array<BranchCalcParam<false>, 3> calc_params, test_params = vec[trafo].calc_param<false>();
        for (size_t i = 0; i < 3; ++i) {
            calc_params[i] = trafos_vec[trafo][i].calc_param<false>();
        }
        for (size_t i = 0; i < 3; i++) {
            for (size_t value_no = 0; value_no < calc_params[i].value.size(); ++value_no) {
                CHECK((cabs(calc_params[i].value[value_no] - test_params[i].value[value_no]) < numerical_tolerance)
                          .all());
            }
        }
    }

    // Check phase shift
    std::array<double, 3> test_shift = vec[0].phase_shift();
    std::array<double, 3> shift = {0.0, -1 * deg_30, -1 * deg_30};
    for (size_t i = 0; i < 3; i++) {
        CHECK(test_shift[i] == shift[i]);
    }

    SUBCASE("Check output of branch 3") {
        // TODO asym output check
        BranchMathOutput<true> b1_output, b2_output, b3_output;
        // Branch initialization: s_f, s_t, i_f, i_t
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
        CHECK(sym_output.loading == doctest::Approx(out_loading));

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

        Branch3Output<false> asym_output = vec[0].get_output(asym_b1_output, asym_b2_output, asym_b3_output);

        CHECK(asym_output.id == 1);
        CHECK(asym_output.energized);
        CHECK(asym_output.p_1(0) == doctest::Approx(out_p_1 / 3));
        CHECK(asym_output.q_1(1) == doctest::Approx(out_q_1 / 3));
        CHECK(asym_output.i_1(2) == doctest::Approx(out_i_1));
        CHECK(asym_output.s_1(0) == doctest::Approx(out_s_1 / 3));
        CHECK(asym_output.p_2(1) == doctest::Approx(out_p_2 / 3));
        CHECK(asym_output.q_2(2) == doctest::Approx(out_q_2 / 3));
        CHECK(asym_output.i_2(0) == doctest::Approx(out_i_2));
        CHECK(asym_output.s_2(1) == doctest::Approx(out_s_2 / 3));
        CHECK(asym_output.p_3(2) == doctest::Approx(out_p_3 / 3));
        CHECK(asym_output.q_3(0) == doctest::Approx(out_q_3 / 3));
        CHECK(asym_output.i_3(1) == doctest::Approx(out_i_3));
        CHECK(asym_output.s_3(2) == doctest::Approx(out_s_3 / 3));
        CHECK(asym_output.loading == doctest::Approx(out_loading));
    }

    SUBCASE("No source results") {
        Branch3Output<false> output = vec[0].get_null_output<false>();
        CHECK(output.id == 1);
        CHECK(!output.energized);
        CHECK(output.p_1(0) == 0);
        CHECK(output.q_1(1) == 0);
        CHECK(output.i_1(2) == 0);
        CHECK(output.s_1(0) == 0);
        CHECK(output.p_2(1) == 0);
        CHECK(output.q_2(2) == 0);
        CHECK(output.i_2(0) == 0);
        CHECK(output.s_2(1) == 0);
        CHECK(output.p_3(2) == 0);
        CHECK(output.q_3(0) == 0);
        CHECK(output.i_3(1) == 0);
        CHECK(output.s_3(2) == 0);
        CHECK(output.loading == 0);
    }

    SUBCASE("invalid input") {
        input.node_2 = 2;
        CHECK_THROWS_AS(ThreeWindingTransformer(input, 138e3, 69e3, 13.8e3), InvalidBranch3);
        input.node_2 = 3;
    }

    SUBCASE("Test i base") {
        CHECK(vec[0].base_i_1() == doctest::Approx(base_i_1));
        CHECK(vec[0].base_i_2() == doctest::Approx(base_i_2));
        CHECK(vec[0].base_i_3() == doctest::Approx(base_i_3));
    }

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
}

}  // namespace power_grid_model
