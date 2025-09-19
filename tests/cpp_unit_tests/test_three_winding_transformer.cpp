// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/component/three_winding_transformer.hpp>
#include <power_grid_model/component/transformer.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {

using namespace std::complex_literals;

TEST_CASE("Test three winding transformer") {
    ThreeWindingTransformerInput input{.id = 1,
                                       .node_1 = 2,
                                       .node_2 = 3,
                                       .node_3 = 4,
                                       .status_1 = 1,
                                       .status_2 = 1,
                                       .status_3 = 1,
                                       .u1 = 138e3,
                                       .u2 = 69e3,
                                       .u3 = 13.8e3,
                                       .sn_1 = 60e6,
                                       .sn_2 = 50e6,
                                       .sn_3 = 10e6,
                                       .uk_12 = 0.09,
                                       .uk_13 = 0.06,
                                       .uk_23 = 0.03,
                                       .pk_12 = 200e3,
                                       .pk_13 = 150e3,
                                       .pk_23 = 100e3,
                                       .i0 = 0.1,
                                       .p0 = 50e3,
                                       .winding_1 = WindingType::wye_n,
                                       .winding_2 = WindingType::delta,
                                       .winding_3 = WindingType::delta,
                                       .clock_12 = 1,
                                       .clock_13 = 1,
                                       .tap_side = Branch3Side::side_1,
                                       .tap_pos = 2,
                                       .tap_min = -8,
                                       .tap_max = 10,
                                       .tap_nom = 0,
                                       .tap_size = 1380,
                                       .uk_12_min = nan,
                                       .uk_12_max = nan,
                                       .uk_13_min = nan,
                                       .uk_13_max = nan,
                                       .uk_23_min = nan,
                                       .uk_23_max = nan,
                                       .pk_12_min = nan,
                                       .pk_12_max = nan,
                                       .pk_13_min = nan,
                                       .pk_13_max = nan,
                                       .pk_23_min = nan,
                                       .pk_23_max = nan,
                                       .r_grounding_1 = 1.0,
                                       .x_grounding_1 = 4.0,
                                       .r_grounding_2 = nan,
                                       .x_grounding_2 = nan,
                                       .r_grounding_3 = nan,
                                       .x_grounding_3 = nan};

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

    SUBCASE("Test getters") {
        CHECK(vec[0].tap_pos() == 2);
        CHECK(vec[0].tap_side() == Branch3Side::side_1);
        CHECK(vec[0].tap_min() == -8);
        CHECK(vec[0].tap_max() == 10);
        CHECK(vec[0].tap_nom() == 0);
    }

    for (ThreeWindingTransformer const& transformer3 : vec) {
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

    TransformerInput T1_input{.id = 2,
                              .from_node = 0,
                              .to_node = 1,
                              .from_status = 1,
                              .to_status = 1,
                              .u1 = u_t1,
                              .u2 = u_t1,
                              .sn = 60e6,
                              .uk = uk_t1,
                              .pk = pk_t1,
                              .i0 = 0.1,
                              .p0 = 50e3,
                              .winding_from = WindingType::wye_n,
                              .winding_to = WindingType::wye_n,
                              .clock = 0,
                              .tap_side = BranchSide::from,
                              .tap_pos = 0,
                              .tap_min = 0,
                              .tap_max = 0,
                              .tap_nom = 0,
                              .tap_size = 0.0,
                              .uk_min = nan,
                              .uk_max = nan,
                              .pk_min = nan,
                              .pk_max = nan,
                              .r_grounding_from = 1.0,
                              .x_grounding_from = 4.0,
                              .r_grounding_to = 0,
                              .x_grounding_to = 0};
    TransformerInput T2_input{.id = 2,
                              .from_node = 0,
                              .to_node = 1,
                              .from_status = 1,
                              .to_status = 1,
                              .u1 = 69e3,
                              .u2 = u_t1,
                              .sn = 50e6,
                              .uk = uk_t2,
                              .pk = pk_t2,
                              .i0 = 0.0,
                              .p0 = 0.0,
                              .winding_from = WindingType::delta,
                              .winding_to = WindingType::wye_n,
                              .clock = 11, // reversed
                              .tap_side = BranchSide::from,
                              .tap_pos = 0,
                              .tap_min = 0,
                              .tap_max = 0,
                              .tap_nom = 0,
                              .tap_size = 0.0,
                              .uk_min = nan,
                              .uk_max = nan,
                              .pk_min = nan,
                              .pk_max = nan,
                              .r_grounding_from = 0,
                              .x_grounding_from = 0,
                              .r_grounding_to = 0,
                              .x_grounding_to = 0};
    TransformerInput T3_input{.id = 2,
                              .from_node = 0,
                              .to_node = 1,
                              .from_status = 1,
                              .to_status = 1,
                              .u1 = 13.8e3,
                              .u2 = u_t1,
                              .sn = 10e6,
                              .uk = uk_t3,
                              .pk = pk_t3,
                              .i0 = 0.0,
                              .p0 = 0.0,
                              .winding_from = WindingType::delta,
                              .winding_to = WindingType::wye_n,
                              .clock = 11, // reversed
                              .tap_side = BranchSide::from,
                              .tap_pos = 0,
                              .tap_min = 0,
                              .tap_max = 0,
                              .tap_nom = 0,
                              .tap_size = 0.0,
                              .uk_min = nan,
                              .uk_max = nan,
                              .pk_min = nan,
                              .pk_max = nan,
                              .r_grounding_from = 0,
                              .x_grounding_from = 0,
                              .r_grounding_to = 0,
                              .x_grounding_to = 0};

    auto make_trafos = [](TransformerInput T1, TransformerInput T2, TransformerInput T3) {
        Transformer const t1{T1, 138e3, 138e3};
        Transformer const t2{T2, 69e3, 138e3};
        Transformer const t3{T3, 13.8e3, 138e3};
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
        std::array<BranchCalcParam<symmetric_t>, 3> calc_params = vec[trafo].calc_param<symmetric_t>();
        std::array<BranchCalcParam<symmetric_t>, 3> test_params = vec[trafo].calc_param<symmetric_t>();
        for (size_t i = 0; i < 3; ++i) {
            calc_params[i] = trafos_vec[trafo][i].calc_param<symmetric_t>();
        }
        for (size_t i = 0; i < 3; ++i) {
            for (size_t value_no = 0; value_no < 3; ++value_no) {
                CHECK(cabs(calc_params[i].value[value_no] - test_params[i].value[value_no]) < numerical_tolerance);
            }
        }
    }
    // asym admittance
    for (size_t trafo = 0; trafo < trafos_vec.size(); ++trafo) {
        std::array<BranchCalcParam<asymmetric_t>, 3> calc_params = vec[trafo].calc_param<asymmetric_t>();
        std::array<BranchCalcParam<asymmetric_t>, 3> test_params = vec[trafo].calc_param<asymmetric_t>();
        for (size_t i = 0; i < 3; ++i) {
            calc_params[i] = trafos_vec[trafo][i].calc_param<asymmetric_t>();
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
        // Branch initialization: s_f, s_t, i_f, i_t
        BranchSolverOutput<symmetric_t> const b1_output{
            .s_f = (1.0 - 2.0i), .s_t = (2.0 - 3.0i), .i_f = (1.5 - 2.5i), .i_t = (2.5 - 3.5i)};
        BranchSolverOutput<symmetric_t> const b2_output{
            .s_f = (2.0 - 3.0i), .s_t = (-3.0 + 2.0i), .i_f = (1.5 - 2.5i), .i_t = (-4.0 + 1.5i)};
        BranchSolverOutput<symmetric_t> const b3_output{
            .s_f = (3.0 + 1.0i), .s_t = (1.0 + 1.0i), .i_f = (1.5 - 2.5i), .i_t = (1.5 + 2.0i)};

        Branch3Output<symmetric_t> const sym_output = vec[0].get_output(b1_output, b2_output, b3_output);

        double const out_p_1 = base_power<symmetric_t> * 1;
        double const out_q_1 = base_power<symmetric_t> * (-2);
        double const out_i_1 = base_i_1 * cabs(b1_output.i_f);
        double const out_s_1 = base_power<symmetric_t> * cabs(b1_output.s_f);

        double const out_p_2 = base_power<symmetric_t> * 2;
        double const out_q_2 = base_power<symmetric_t> * (-3);
        double const out_i_2 = base_i_2 * cabs(b2_output.i_f);
        double const out_s_2 = base_power<symmetric_t> * cabs(b2_output.s_f);

        double const out_p_3 = base_power<symmetric_t> * 3;
        double const out_q_3 = base_power<symmetric_t> * 1;
        double const out_i_3 = base_i_3 * cabs(b3_output.i_f);
        double const out_s_3 = base_power<symmetric_t> * cabs(b3_output.s_f);
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

        BranchSolverOutput<asymmetric_t> const asym_b1_output{.s_f = {(1.0 - 2.0i), (1.0 - 2.0i), (1.0 - 2.0i)},
                                                              .s_t = {(2.0 - 3.0i), (2.0 - 3.0i), (2.0 - 3.0i)},
                                                              .i_f = {(1.5 - 2.5i), (1.5 - 2.5i), (1.5 - 2.5i)},
                                                              .i_t = {(2.5 - 3.5i), (2.5 - 3.5i), (2.5 - 3.5i)}};
        BranchSolverOutput<asymmetric_t> const asym_b2_output{.s_f = {(2.0 - 3.0i), (2.0 - 3.0i), (2.0 - 3.0i)},
                                                              .s_t = {(-3.0 + 2.0i), (-3.0 + 2.0i), (-3.0 + 2.0i)},
                                                              .i_f = {(1.5 - 2.5i), (1.5 - 2.5i), (1.5 - 2.5i)},
                                                              .i_t = {(-4.0 + 1.5i), (-4.0 + 1.5i), (-4.0 + 1.5i)}};
        BranchSolverOutput<asymmetric_t> const asym_b3_output{.s_f = {(3.0 + 1.0i), (3.0 + 1.0i), (3.0 + 1.0i)},
                                                              .s_t = {(1.0 + 1.0i), (1.0 + 1.0i), (1.0 + 1.0i)},
                                                              .i_f = {(1.5 - 2.5i), (1.5 - 2.5i), (1.5 - 2.5i)},
                                                              .i_t = {(1.5 + 2.0i), (1.5 + 2.0i), (1.5 + 2.0i)}};

        Branch3Output<asymmetric_t> asym_output = vec[0].get_output(asym_b1_output, asym_b2_output, asym_b3_output);

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

    SUBCASE("Check asym short circuit output of branch 3") {
        ComplexValue<symmetric_t> const i_1{1.5 - 2.5i};
        ComplexValue<symmetric_t> const i_2{1.0 - 2.2i};
        ComplexValue<symmetric_t> const i_3{1.3 - 2.1i};
        ComplexValue<asymmetric_t> const i_1_asym{1.5 - 2.5i};
        ComplexValue<asymmetric_t> const i_2_asym{1.0 - 2.2i};
        ComplexValue<asymmetric_t> const i_3_asym{1.3 - 2.1i};

        Branch3ShortCircuitOutput asym_sc_output = vec[0].get_sc_output(i_1_asym, i_2_asym, i_3_asym);

        CHECK(asym_sc_output.id == 1);
        CHECK(asym_sc_output.energized == 1);
        CHECK(asym_sc_output.i_1(2) == doctest::Approx(cabs(i_1) * base_i_1));
        CHECK(asym_sc_output.i_2(0) == doctest::Approx(cabs(i_2) * base_i_2));
        CHECK(asym_sc_output.i_3(1) == doctest::Approx(cabs(i_3) * base_i_3));
        CHECK(asym_sc_output.i_1_angle(2) == doctest::Approx(arg(i_1) + deg_120));
        CHECK(asym_sc_output.i_2_angle(0) == doctest::Approx(arg(i_2)));
        CHECK(asym_sc_output.i_3_angle(1) == doctest::Approx(arg(i_3) - deg_120));
    }

    SUBCASE("Check sym short circuit output of branch 3") {
        ComplexValue<symmetric_t> const i_1{1.5 - 2.5i};
        ComplexValue<symmetric_t> const i_2{1.0 - 2.2i};
        ComplexValue<symmetric_t> const i_3{1.3 - 2.1i};

        BranchShortCircuitSolverOutput<symmetric_t> const sym_b1_output{.i_f = i_1, .i_t = ComplexValue<symmetric_t>{}};
        BranchShortCircuitSolverOutput<symmetric_t> const sym_b2_output{.i_f = i_2, .i_t = ComplexValue<symmetric_t>{}};
        BranchShortCircuitSolverOutput<symmetric_t> const sym_b3_output{.i_f = i_3, .i_t = ComplexValue<symmetric_t>{}};

        Branch3ShortCircuitOutput sym_sc_output = vec[0].get_sc_output(sym_b1_output, sym_b2_output, sym_b3_output);

        ComplexValue<asymmetric_t> const i_1_asym{1.5 - 2.5i};
        ComplexValue<asymmetric_t> const i_2_asym{1.0 - 2.2i};
        ComplexValue<asymmetric_t> const i_3_asym{1.3 - 2.1i};

        BranchShortCircuitSolverOutput<asymmetric_t> const asym_b1_output{.i_f = i_1_asym,
                                                                          .i_t = ComplexValue<asymmetric_t>{}};
        BranchShortCircuitSolverOutput<asymmetric_t> const asym_b2_output{.i_f = i_2_asym,
                                                                          .i_t = ComplexValue<asymmetric_t>{}};
        BranchShortCircuitSolverOutput<asymmetric_t> const asym_b3_output{.i_f = i_3_asym,
                                                                          .i_t = ComplexValue<asymmetric_t>{}};

        Branch3ShortCircuitOutput asym_sc_output = vec[0].get_sc_output(asym_b1_output, asym_b2_output, asym_b3_output);

        CHECK(sym_sc_output.id == asym_sc_output.id);
        CHECK(sym_sc_output.energized == asym_sc_output.energized);
        CHECK(sym_sc_output.i_1(2) == doctest::Approx(asym_sc_output.i_1(2)));
        CHECK(sym_sc_output.i_2(0) == doctest::Approx(asym_sc_output.i_2(0)));
        CHECK(sym_sc_output.i_3(1) == doctest::Approx(asym_sc_output.i_3(1)));
        CHECK(sym_sc_output.i_1_angle(2) == doctest::Approx(asym_sc_output.i_1_angle(2)));
        CHECK(sym_sc_output.i_2_angle(0) == doctest::Approx(asym_sc_output.i_2_angle(0)));
        CHECK(sym_sc_output.i_3_angle(1) == doctest::Approx(asym_sc_output.i_3_angle(1)));
    }

    SUBCASE("No source results") {
        Branch3Output<asymmetric_t> output = vec[0].get_null_output<asymmetric_t>();
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

    SUBCASE("No source short circuit results") {
        Branch3ShortCircuitOutput output = vec[0].get_null_sc_output();
        CHECK(output.id == 1);
        CHECK(!output.energized);
        CHECK(output.i_1(2) == 0);
        CHECK(output.i_2(0) == 0);
        CHECK(output.i_3(1) == 0);
        CHECK(output.i_1_angle(2) == 0);
        CHECK(output.i_2_angle(0) == 0);
        CHECK(output.i_3_angle(1) == 0);
    }

    SUBCASE("invalid input") {
        input.node_2 = 2;
        CHECK_THROWS_AS(ThreeWindingTransformer(input, 138e3, 69e3, 13.8e3), InvalidBranch3);
        input.node_2 = 3;
    }

    SUBCASE("Periodic clock input") {
        input.clock_12 = 24;
        input.clock_13 = 37;
        ThreeWindingTransformer const trafo_24_36(input, 138e3, 69e3, 13.8e3);
        CHECK(trafo_24_36.clock_12() == 0);
        CHECK(trafo_24_36.clock_13() == 1);

        input.clock_12 = -2;
        input.clock_13 = -13;
        ThreeWindingTransformer const trafo_m2_m13(input, 138e3, 69e3, 13.8e3);
        CHECK(trafo_m2_m13.clock_12() == 10);
        CHECK(trafo_m2_m13.clock_13() == 11);

        input.winding_2 = WindingType::delta;
        input.winding_3 = WindingType::delta;
        input.clock_12 = 25;
        input.clock_13 = 13;
        ThreeWindingTransformer const trafo_25_13(input, 138e3, 69e3, 13.8e3);
        CHECK(trafo_25_13.clock_12() == 1);
        CHECK(trafo_25_13.clock_13() == 1);
    }

    SUBCASE("Test i base") {
        CHECK(vec[0].base_i_1() == doctest::Approx(base_i_1));
        CHECK(vec[0].base_i_2() == doctest::Approx(base_i_2));
        CHECK(vec[0].base_i_3() == doctest::Approx(base_i_3));
    }

    SUBCASE("update - check changed") {
        SUBCASE("update tap") {
            auto changed = vec[0].update(ThreeWindingTransformerUpdate{
                .id = 1, .status_1 = na_IntS, .status_2 = na_IntS, .status_3 = na_IntS, .tap_pos = -2});
            CHECK(!changed.topo);
            CHECK(changed.param);
        }
        SUBCASE("update status_1") {
            auto changed = vec[0].update(ThreeWindingTransformerUpdate{
                .id = 1, .status_1 = 0, .status_2 = 1, .status_3 = 1, .tap_pos = na_IntS});
            CHECK(changed.topo);
            CHECK(changed.param);
        }
        SUBCASE("update status_2") {
            auto changed = vec[0].update(ThreeWindingTransformerUpdate{
                .id = 1, .status_1 = 1, .status_2 = 0, .status_3 = 1, .tap_pos = na_IntS});
            CHECK(changed.topo);
            CHECK(changed.param);
        }
        SUBCASE("update status_3") {
            auto changed = vec[0].update(ThreeWindingTransformerUpdate{
                .id = 1, .status_1 = 1, .status_2 = 1, .status_3 = 0, .tap_pos = na_IntS});
            CHECK(changed.topo);
            CHECK(changed.param);
        }
        SUBCASE("update status") {
            auto changed = vec[0].update(ThreeWindingTransformerUpdate{
                .id = 1, .status_1 = 0, .status_2 = 0, .status_3 = 0, .tap_pos = na_IntS});
            CHECK(changed.topo);
            CHECK(changed.param);
        }
        SUBCASE("update status & tap") {
            auto changed = vec[0].update(
                ThreeWindingTransformerUpdate{.id = 1, .status_1 = 0, .status_2 = 0, .status_3 = 0, .tap_pos = -2});
            CHECK(changed.topo);
            CHECK(changed.param);
        }
        SUBCASE("update none") {
            auto changed = vec[0].update(ThreeWindingTransformerUpdate{
                .id = 1, .status_1 = na_IntS, .status_2 = na_IntS, .status_3 = na_IntS, .tap_pos = na_IntS});
            CHECK(!changed.topo);
            CHECK(!changed.param);
        }
    }

    SUBCASE("Update inverse") {
        ThreeWindingTransformerUpdate three_winding_transformer_update{
            .id = 1, .status_1 = na_IntS, .status_2 = na_IntS, .status_3 = na_IntS, .tap_pos = na_IntS};
        auto expected = three_winding_transformer_update;

        auto const& transformer = vec.front();

        SUBCASE("Identical") {
            // default values
        }

        SUBCASE("status_1") {
            SUBCASE("same") { three_winding_transformer_update.status_1 = status_to_int(transformer.status_1()); }
            SUBCASE("different") { three_winding_transformer_update.status_1 = IntS{0}; }
            expected.status_1 = status_to_int(transformer.status_1());
        }

        SUBCASE("status_2") {
            SUBCASE("same") { three_winding_transformer_update.status_2 = status_to_int(transformer.status_2()); }
            SUBCASE("different") { three_winding_transformer_update.status_2 = IntS{0}; }
            expected.status_2 = status_to_int(transformer.status_2());
        }

        SUBCASE("status_3") {
            SUBCASE("same") { three_winding_transformer_update.status_3 = status_to_int(transformer.status_3()); }
            SUBCASE("different") { three_winding_transformer_update.status_3 = IntS{0}; }
            expected.status_3 = status_to_int(transformer.status_3());
        }

        SUBCASE("Tap pos") {
            SUBCASE("same") { three_winding_transformer_update.tap_pos = IntS{0}; }
            SUBCASE("different") { three_winding_transformer_update.tap_pos = IntS{0}; }
            expected.tap_pos = IntS{2};
        }

        SUBCASE("multiple") {
            three_winding_transformer_update.status_1 = IntS{0};
            three_winding_transformer_update.status_2 = IntS{0};
            three_winding_transformer_update.status_3 = IntS{0};
            three_winding_transformer_update.tap_pos = IntS{0};
            expected.status_1 = status_to_int(transformer.status_1());
            expected.status_2 = status_to_int(transformer.status_2());
            expected.status_3 = status_to_int(transformer.status_3());
            expected.tap_pos = IntS{2};
        }

        auto const inv = transformer.inverse(three_winding_transformer_update);

        CHECK(inv.id == expected.id);
        CHECK(inv.status_1 == expected.status_1);
        CHECK(inv.status_2 == expected.status_2);
        CHECK(inv.status_3 == expected.status_3);
        CHECK(inv.tap_pos == expected.tap_pos);
    }
    SUBCASE("Test optional tap pos/nom") {
        input.tap_nom = 1;
        input.tap_pos = na_IntS;
        std::vector<ThreeWindingTransformer> trafo_vec;
        trafo_vec.emplace_back(input, 138e3, 69e3, 13.8e3);
        input.tap_nom = na_IntS;
        trafo_vec.emplace_back(input, 138e3, 69e3, 13.8e3);
        CHECK(trafo_vec[0].tap_pos() == 1);
        CHECK(trafo_vec[1].tap_pos() == 0);
        CHECK(trafo_vec[1].tap_nom() == 0);
    }
}

} // namespace power_grid_model
