// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/component/transformer.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {
namespace {
using namespace std::complex_literals;

constexpr double base_i_from = base_power_3p / 150e3 / sqrt3;
constexpr double base_i_to = base_power_3p / 10e3 / sqrt3;

ComplexTensor<asymmetric_t> get_a() {
    ComplexTensor<asymmetric_t> A;
    A << 1.0, 1.0, 1.0, 1.0, a2, a, 1.0, a, a2;
    return A;
}

ComplexTensor<asymmetric_t> get_a_inv() {
    ComplexTensor<asymmetric_t> A_inv;
    A_inv << 1.0, 1.0, 1.0, 1.0, a, a2, 1.0, a2, a;
    A_inv = A_inv / 3.0;
    return A_inv;
}
} // namespace

TEST_CASE("Test transformer") {
    TransformerInput input{.id = 1,
                           .from_node = 2,
                           .to_node = 3,
                           .from_status = 1,
                           .to_status = 1,
                           .u1 = 155e3,
                           .u2 = 10.0e3,
                           .sn = 30e6,
                           .uk = 0.203,
                           .pk = 100e3,
                           .i0 = 0.0,
                           .p0 = 0.0,
                           .winding_from = WindingType::wye_n,
                           .winding_to = WindingType::wye_n,
                           .clock = 12,
                           .tap_side = BranchSide::from,
                           .tap_pos = 0,
                           .tap_min = -11,
                           .tap_max = 9,
                           .tap_nom = 0,
                           .tap_size = 2.5e3,
                           .uk_min = nan,
                           .uk_max = nan,
                           .pk_min = nan,
                           .pk_max = nan,
                           .r_grounding_from = nan,
                           .x_grounding_from = nan,
                           .r_grounding_to = nan,
                           .x_grounding_to = nan};

    std::vector<Transformer> vec;
    // 0 YNyn12
    vec.emplace_back(input, 150e3, 10e3);
    // 1 Dyn11
    input.winding_from = WindingType::delta;
    input.winding_to = WindingType::wye_n;
    input.clock = 11;
    vec.emplace_back(input, 150e3, 10e3);
    // 2 Yd1
    input.winding_from = WindingType::wye;
    input.winding_to = WindingType::delta;
    input.clock = 1;
    vec.emplace_back(input, 150e3, 10e3);
    // 3 Yy12
    input.winding_from = WindingType::wye;
    input.winding_to = WindingType::wye;
    input.clock = 12;
    vec.emplace_back(input, 150e3, 10e3);
    // 4 YNyn2
    input.winding_from = WindingType::wye_n;
    input.winding_to = WindingType::wye_n;
    input.clock = 2;
    vec.emplace_back(input, 150e3, 10e3);

    for (Transformer const& transformer : vec) {
        CHECK(transformer.math_model_type() == ComponentType::branch);
    }

    // calculation parameters
    double const base_y = base_i_to / (10e3 / sqrt3);
    double const z_series_abs = 0.203 * 10e3 * 10e3 / 30e6;
    double const r_series = 100e3 * 10e3 * 10e3 / 30e6 / 30e6;
    DoubleComplex const z_series = r_series + 1.0i * std::sqrt(z_series_abs * z_series_abs - r_series * r_series);
    DoubleComplex const y = 1.0 / z_series / base_y;

    // sym
    std::vector<BranchCalcParam<symmetric_t>> vec_sym;
    // 12
    vec_sym.push_back({{y, -y, -y, y}});
    // 11, -30
    vec_sym.push_back({{y, -y * std::exp(-1.0i * deg_30), -y * std::exp(1.0i * deg_30), y}});
    // 1, 30
    vec_sym.push_back({{y, -y * std::exp(1.0i * deg_30), -y * std::exp(-1.0i * deg_30), y}});
    // 12
    vec_sym.push_back({{y, -y, -y, y}});
    // 2 60
    vec_sym.push_back({{y, -y * std::exp(2.0i * deg_30), -y * std::exp(-2.0i * deg_30), y}});

    // asym
    std::vector<BranchCalcParam<asymmetric_t>> vec_asym;
    ComplexTensor<asymmetric_t> y1;
    y1 << y, 0.0, 0.0, 0.0, y, 0.0, 0.0, 0.0, y;
    ComplexTensor<asymmetric_t> y2;
    y2 << 2.0 * y, -y, -y, -y, 2.0 * y, -y, -y, -y, 2.0 * y;
    y2 = y2 / 3.0;
    ComplexTensor<asymmetric_t> y3;
    y3 << -y, y, 0.0, 0.0, -y, y, y, 0.0, -y;
    y3 = y3 / sqrt3;
    ComplexTensor<asymmetric_t> y3t = (y3.matrix().transpose()).array();
    ComplexTensor<asymmetric_t> y4;
    y4 << 0.0, y, 0.0, 0.0, 0.0, y, y, 0.0, 0.0;
    ComplexTensor<asymmetric_t> y4t = (y4.matrix().transpose()).array();

    // YNyn12
    vec_asym.push_back({{y1, -y1, -y1, y1}});
    // Dyn11
    vec_asym.push_back({{y2, y3t, y3, y1}});
    // Yd1
    vec_asym.push_back({{y2, y3, y3t, y2}});
    // Yy12
    vec_asym.push_back({{y2, -y2, -y2, y2}});
    // YNyn2
    vec_asym.push_back({{y1, y4, y4t, y1}});

    SUBCASE("Test getters") {
        CHECK(vec[0].tap_pos() == 0);
        CHECK(vec[0].tap_side() == BranchSide::from);
        CHECK(vec[0].tap_min() == -11);
        CHECK(vec[0].tap_max() == 9);
        CHECK(vec[0].tap_nom() == 0);
    }

    SUBCASE("Test i base") {
        CHECK(vec[0].base_i_from() == doctest::Approx(base_i_from));
        CHECK(vec[0].base_i_to() == doctest::Approx(base_i_to));
    }

    SUBCASE("invalid input") {
        input.winding_from = WindingType::delta;
        input.winding_to = WindingType::wye_n;
        input.clock = 12;
        CHECK_THROWS_AS(Transformer(input, 150.0e3, 10.0e3), InvalidTransformerClock);
        input.winding_from = WindingType::wye;
        input.winding_to = WindingType::wye_n;
        input.clock = 11;
        CHECK_THROWS_AS(Transformer(input, 150.0e3, 10.0e3), InvalidTransformerClock);
        // tap limit
        vec[0].set_tap(-100);
        CHECK(vec[0].tap_pos() == -11);
        vec[0].set_tap(100);
        CHECK(vec[0].tap_pos() == 9);
    }

    SUBCASE("periodic clock input") {
        input.clock = 24;
        Transformer const trafo_24(input, 150.0e3, 10.0e3);
        input.clock = 36;
        Transformer const trafo_36(input, 150.0e3, 10.0e3);
        input.clock = -2;
        Transformer const trafo_m2(input, 150.0e3, 10.0e3);
        CHECK(trafo_24.clock() == 0);
        CHECK(trafo_36.clock() == 0);
        CHECK(trafo_m2.clock() == 10);

        input.winding_to = WindingType::delta;
        input.clock = 25;
        Transformer const trafo_25(input, 150.0e3, 10.0e3);
        CHECK(trafo_25.clock() == 1);
    }

    SUBCASE("symmetric parameters") {
        for (size_t i = 0; i < 5; i++) {
            auto changed =
                vec[i].update(TransformerUpdate{.id = 1, .from_status = na_IntS, .to_status = na_IntS, .tap_pos = -2});
            CHECK(!changed.topo);
            CHECK(changed.param);
            BranchCalcParam<symmetric_t> const param = vec[i].calc_param<symmetric_t>();
            for (size_t j = 0; j < 4; j++) {
                CHECK(cabs(param.value[j] - vec_sym[i].value[j]) < numerical_tolerance);
            }
        }
    }

    SUBCASE("update - check changed") {
        SUBCASE("update tap") {
            auto changed =
                vec[0].update(TransformerUpdate{.id = 1, .from_status = na_IntS, .to_status = na_IntS, .tap_pos = -2});
            CHECK(!changed.topo);
            CHECK(changed.param);
        }
        SUBCASE("update from_status") {
            auto changed =
                vec[0].update(TransformerUpdate{.id = 1, .from_status = 0, .to_status = 1, .tap_pos = na_IntS});
            CHECK(changed.topo);
            CHECK(changed.param);
        }
        SUBCASE("update to_status") {
            auto changed =
                vec[0].update(TransformerUpdate{.id = 1, .from_status = 1, .to_status = 0, .tap_pos = na_IntS});
            CHECK(changed.topo);
            CHECK(changed.param);
        }
        SUBCASE("update status") {
            auto changed =
                vec[0].update(TransformerUpdate{.id = 1, .from_status = 0, .to_status = 0, .tap_pos = na_IntS});
            CHECK(changed.topo);
            CHECK(changed.param);
        }
        SUBCASE("update status & tap") {
            auto changed = vec[0].update(TransformerUpdate{.id = 1, .from_status = 0, .to_status = 0, .tap_pos = -2});
            CHECK(changed.topo);
            CHECK(changed.param);
        }
        SUBCASE("update none") {
            auto changed = vec[0].update(
                TransformerUpdate{.id = 1, .from_status = na_IntS, .to_status = na_IntS, .tap_pos = na_IntS});
            CHECK(!changed.topo);
            CHECK(!changed.param);
        }
    }

    SUBCASE("asymmetric paramters") {
        for (size_t i = 0; i < 5; i++) {
            vec[i].set_tap(-2);
            BranchCalcParam<asymmetric_t> const param = vec[i].calc_param<asymmetric_t>();
            for (size_t j = 0; j < 4; j++) {
                CHECK((cabs(param.value[j] - vec_asym[i].value[j]) < numerical_tolerance).all());
            }
        }
    }

    SUBCASE("Update inverse") {
        TransformerUpdate transformer_update{.id = 1, .from_status = na_IntS, .to_status = na_IntS, .tap_pos = na_IntS};
        auto expected = transformer_update;

        auto const& transformer = vec.front();

        SUBCASE("Identical") {
            // default values
        }

        SUBCASE("From status") {
            SUBCASE("same") { transformer_update.from_status = status_to_int(transformer.from_status()); }
            SUBCASE("different") { transformer_update.from_status = IntS{0}; }
            expected.from_status = status_to_int(transformer.from_status());
        }

        SUBCASE("To status") {
            SUBCASE("same") { transformer_update.to_status = status_to_int(transformer.to_status()); }
            SUBCASE("different") { transformer_update.to_status = IntS{0}; }
            expected.to_status = status_to_int(transformer.to_status());
        }

        SUBCASE("Tap pos") {
            SUBCASE("same") { transformer_update.tap_pos = transformer.tap_pos(); }
            SUBCASE("different") { transformer_update.tap_pos = IntS{1}; }
            expected.tap_pos = transformer.tap_pos();
        }

        SUBCASE("multiple") {
            transformer_update.from_status = IntS{0};
            transformer_update.to_status = IntS{0};
            transformer_update.tap_pos = IntS{0};
            expected.from_status = status_to_int(transformer.from_status());
            expected.to_status = status_to_int(transformer.to_status());
            expected.tap_pos = transformer.tap_pos();
        }

        auto const inv = transformer.inverse(transformer_update);

        CHECK(inv.id == expected.id);
        CHECK(inv.from_status == expected.from_status);
        CHECK(inv.to_status == expected.to_status);
        CHECK(inv.tap_pos == expected.tap_pos);
    }
    SUBCASE("Test optional tap pos/nom") {
        input.tap_nom = 1;
        input.tap_pos = na_IntS;
        std::vector<Transformer> trafo_vec;
        trafo_vec.emplace_back(input, 150e3, 10e3);
        input.tap_nom = na_IntS;
        trafo_vec.emplace_back(input, 150e3, 10e3);
        CHECK(trafo_vec[0].tap_pos() == 1);
        CHECK(trafo_vec[1].tap_pos() == 0);
        CHECK(trafo_vec[1].tap_nom() == 0);
    }
}

TEST_CASE("Test Transfomer - Test 0 YNyn12") {
    ComplexTensor<asymmetric_t> const A = get_a();
    ComplexTensor<asymmetric_t> const A_inv = get_a_inv();

    TransformerInput const input{.id = 1,
                                 .from_node = 2,
                                 .to_node = 3,
                                 .from_status = 1,
                                 .to_status = 1,
                                 .u1 = 155e3,
                                 .u2 = 10.0e3,
                                 .sn = 30e6,
                                 .uk = 0.203,
                                 .pk = 100e3,
                                 .i0 = 0.015,
                                 .p0 = 30.0e4,
                                 .winding_from = WindingType::wye_n,
                                 .winding_to = WindingType::wye_n,
                                 .clock = 12,
                                 .tap_side = BranchSide::from,
                                 .tap_pos = -2,
                                 .tap_min = -11,
                                 .tap_max = 9,
                                 .tap_nom = 0,
                                 .tap_size = 2.5e3,
                                 .uk_min = nan,
                                 .uk_max = nan,
                                 .pk_min = nan,
                                 .pk_max = nan,
                                 .r_grounding_from = 0.5,
                                 .x_grounding_from = 2.0,
                                 .r_grounding_to = 1.0,
                                 .x_grounding_to = 4.0};
    double const u1_rated{150.0e3};
    double const u2_rated{10.0e3};
    Transformer YNyn12{input, u1_rated, u2_rated};

    double const z_abs = input.uk * input.u2 * input.u2 / input.sn; // z_abs = uk * u2 * u2 / sn
    double const z_real = input.pk * input.u2 * input.u2 / input.sn / input.sn;
    double const z_imag = std::sqrt(z_abs * z_abs - z_real * z_real);

    double const u1 = input.u1 + (input.tap_pos - input.tap_nom) * input.tap_size;
    double const u2 = input.u2;                         // Tap is on the from side, not the to side
    double const k = (u1 / u2) / (u1_rated / u2_rated); // =1

    double const base_y_from = base_i_from / (u1_rated / sqrt3);
    double const base_y_to = base_i_to / (u2_rated / sqrt3);
    DoubleComplex const z_grounding_from = (input.r_grounding_from + 1i * input.x_grounding_from) * base_y_from;
    DoubleComplex const z_grounding_to = (input.r_grounding_to + 1i * input.x_grounding_to) * base_y_to;

    DoubleComplex const z_1_series = (z_real + 1i * z_imag) * base_y_to;
    DoubleComplex const z_2_series = z_1_series;
    DoubleComplex const z_0_series = z_1_series + 3.0 * (z_grounding_to + z_grounding_from / k / k);

    ComplexTensor<asymmetric_t> z_diagonal;
    z_diagonal << z_0_series, 0.0, 0.0, 0.0, z_1_series, 0.0, 0.0, 0.0, z_2_series;

    ComplexTensor<asymmetric_t> const z_series = dot(A, z_diagonal, A_inv);

    double const y_shunt_abs = input.i0 * input.sn / input.u2 / input.u2;
    double const y_shunt_real = input.p0 / input.u2 / input.u2;
    double y_shunt_imag;
    if (y_shunt_real > y_shunt_abs) {
        y_shunt_imag = 0.0;
    } else {
        y_shunt_imag = -std::sqrt(y_shunt_abs * y_shunt_abs - y_shunt_real * y_shunt_real);
    }
    DoubleComplex const y_1_shunt = (y_shunt_real + 1i * y_shunt_imag) / base_y_to;
    ComplexTensor<asymmetric_t> y_shunt_diagonal;
    y_shunt_diagonal << y_1_shunt, 0.0, 0.0, 0.0, y_1_shunt, 0.0, 0.0, 0.0, y_1_shunt;
    ComplexTensor<asymmetric_t> const y_shunt = dot(A, y_shunt_diagonal, A_inv);

    ComplexTensor<asymmetric_t> const y_ff = inv(z_series) + 0.5 * y_shunt;
    ComplexTensor<asymmetric_t> const y_ft = -inv(z_series);
    ComplexTensor<asymmetric_t> const y_tf = -inv(z_series);
    ComplexTensor<asymmetric_t> const y_tt = inv(z_series) + 0.5 * y_shunt;

    BranchCalcParam<asymmetric_t> const param = YNyn12.calc_param<asymmetric_t>();

    CHECK((cabs(param.value[0] - y_ff) < numerical_tolerance).all());
    CHECK((cabs(param.value[1] - y_ft) < numerical_tolerance).all());
    CHECK((cabs(param.value[2] - y_tf) < numerical_tolerance).all());
    CHECK((cabs(param.value[3] - y_tt) < numerical_tolerance).all());

    SUBCASE("Test transformer is_param_mutable") { CHECK(YNyn12.is_param_mutable() == true); }

    SUBCASE("Test transformer phase shift") { CHECK(YNyn12.phase_shift() == doctest::Approx(0.0)); }

    SUBCASE("Test transformer loading") { CHECK(YNyn12.loading(60.0e6, 0.0) == doctest::Approx(2.0)); }

    SUBCASE("Test transformer set_limit - false") {
        CHECK(YNyn12.set_tap(na_IntS) == false);
        CHECK(YNyn12.set_tap(input.tap_pos) == false);
    }
}

TEST_CASE("Test Transfomer - Test grounding - Dyn11") {
    ComplexTensor<asymmetric_t> const A = get_a();
    ComplexTensor<asymmetric_t> const A_inv = get_a_inv();

    TransformerInput const input{.id = 1,
                                 .from_node = 2,
                                 .to_node = 3,
                                 .from_status = 1,
                                 .to_status = 1,
                                 .u1 = 155e3,
                                 .u2 = 10.0e3,
                                 .sn = 30e6,
                                 .uk = 0.203,
                                 .pk = 100e3,
                                 .i0 = 0.015,
                                 .p0 = 30.0e4,
                                 .winding_from = WindingType::delta,
                                 .winding_to = WindingType::wye_n,
                                 .clock = 11,
                                 .tap_side = BranchSide::from,
                                 .tap_pos = -2,
                                 .tap_min = -11,
                                 .tap_max = 9,
                                 .tap_nom = 0,
                                 .tap_size = 2.5e3,
                                 .uk_min = nan,
                                 .uk_max = nan,
                                 .pk_min = nan,
                                 .pk_max = nan,
                                 .r_grounding_from = nan,
                                 .x_grounding_from = nan,
                                 .r_grounding_to = 1.0,
                                 .x_grounding_to = 4.0};
    double const u1_rated{150.0e3};
    double const u2_rated{10.0e3};
    Transformer const Dyn11{input, u1_rated, u2_rated};

    // Positive sequence
    double const z_1_abs = input.uk * input.u2 * input.u2 / input.sn;
    double const z_1_real = input.pk * input.u2 * input.u2 / input.sn / input.sn;
    double const z_1_imag = std::sqrt(z_1_abs * z_1_abs - z_1_real * z_1_real);

    double const u1 = input.u1 + (input.tap_pos - input.tap_nom) * input.tap_size;
    double const u2 = input.u2; // Tap is on the from side, not the to side
    double const k = (u1 / u2) / (u1_rated / u2_rated);

    double const base_y_to = base_i_to / (u2_rated / sqrt3);

    DoubleComplex const z_1_series = (z_1_real + 1i * z_1_imag) * base_y_to;

    double const y_shunt_abs = input.i0 * input.sn / input.u2 / input.u2;
    double const y_shunt_real = input.p0 / input.u2 / input.u2;
    double y_shunt_imag;
    if (y_shunt_real > y_shunt_abs) {
        y_shunt_imag = 0.0;
    } else {
        y_shunt_imag = -std::sqrt(y_shunt_abs * y_shunt_abs - y_shunt_real * y_shunt_real);
    }
    DoubleComplex const y_1_shunt = (y_shunt_real + 1i * y_shunt_imag) / base_y_to;

    DoubleComplex const tap_ratio_1 = k * std::exp(1.0i * (deg_30 * input.clock));

    DoubleComplex const y_1_tt = (1.0 / z_1_series) + 0.5 * y_1_shunt;
    DoubleComplex const y_1_ff = (1.0 / k / k) * y_1_tt;
    DoubleComplex const y_1_ft = (-1.0 / conj(tap_ratio_1)) * (1.0 / z_1_series);
    DoubleComplex const y_1_tf = (-1.0 / tap_ratio_1) * (1.0 / z_1_series);

    // Negative sequence
    DoubleComplex const tap_ratio_2 = k * std::exp(-1.0i * (deg_30 * input.clock));

    DoubleComplex const y_2_tt = (1.0 / z_1_series) + 0.5 * y_1_shunt;
    DoubleComplex const y_2_ff = (1.0 / k / k) * y_2_tt;
    DoubleComplex const y_2_ft = (-1.0 / conj(tap_ratio_2)) * (1.0 / z_1_series);
    DoubleComplex const y_2_tf = (-1.0 / tap_ratio_2) * (1.0 / z_1_series);

    // Zero sequence
    DoubleComplex const z_grounding_to = (input.r_grounding_to + 1i * input.x_grounding_to) * base_y_to;

    DoubleComplex const y_0_ff = 0.0;
    DoubleComplex const y_0_ft = 0.0;
    DoubleComplex const y_0_tf = 0.0;
    DoubleComplex const y_0_tt = (1.0 / (z_1_series + 3.0 * z_grounding_to)) + y_1_shunt;

    // Sequence admittances -> phase addmitance
    ComplexTensor<asymmetric_t> y_ff_diagonal;
    y_ff_diagonal << y_0_ff, 0.0, 0.0, 0.0, y_1_ff, 0.0, 0.0, 0.0, y_2_ff;

    ComplexTensor<asymmetric_t> y_ft_diagonal;
    y_ft_diagonal << y_0_ft, 0.0, 0.0, 0.0, y_1_ft, 0.0, 0.0, 0.0, y_2_ft;

    ComplexTensor<asymmetric_t> y_tf_diagonal;
    y_tf_diagonal << y_0_tf, 0.0, 0.0, 0.0, y_1_tf, 0.0, 0.0, 0.0, y_2_tf;

    ComplexTensor<asymmetric_t> y_tt_diagonal;
    y_tt_diagonal << y_0_tt, 0.0, 0.0, 0.0, y_1_tt, 0.0, 0.0, 0.0, y_2_tt;

    ComplexTensor<asymmetric_t> const y_ff = dot(A, y_ff_diagonal, A_inv);
    ComplexTensor<asymmetric_t> const y_ft = dot(A, y_ft_diagonal, A_inv);
    ComplexTensor<asymmetric_t> const y_tf = dot(A, y_tf_diagonal, A_inv);
    ComplexTensor<asymmetric_t> const y_tt = dot(A, y_tt_diagonal, A_inv);

    BranchCalcParam<asymmetric_t> const param = Dyn11.calc_param<asymmetric_t>();

    CHECK((cabs(param.value[0] - y_ff) < numerical_tolerance).all());
    CHECK((cabs(param.value[1] - y_ft) < numerical_tolerance).all());
    CHECK((cabs(param.value[2] - y_tf) < numerical_tolerance).all());
    CHECK((cabs(param.value[3] - y_tt) < numerical_tolerance).all());
}

TEST_CASE("Test Transfomer - Test grounding - Yzn11") {
    ComplexTensor<asymmetric_t> const A = get_a();
    ComplexTensor<asymmetric_t> const A_inv = get_a_inv();

    TransformerInput const input{.id = 1,
                                 .from_node = 2,
                                 .to_node = 3,
                                 .from_status = 1,
                                 .to_status = 1,
                                 .u1 = 155e3,
                                 .u2 = 10.0e3,
                                 .sn = 30e6,
                                 .uk = 0.203,
                                 .pk = 100e3,
                                 .i0 = 0.015,
                                 .p0 = 30.0e4,
                                 .winding_from = WindingType::wye,
                                 .winding_to = WindingType::zigzag_n,
                                 .clock = 11,
                                 .tap_side = BranchSide::from,
                                 .tap_pos = -2,
                                 .tap_min = -11,
                                 .tap_max = 9,
                                 .tap_nom = 0,
                                 .tap_size = 2.5e3,
                                 .uk_min = nan,
                                 .uk_max = nan,
                                 .pk_min = nan,
                                 .pk_max = nan,
                                 .r_grounding_from = nan,
                                 .x_grounding_from = nan,
                                 .r_grounding_to = 1.0,
                                 .x_grounding_to = 4.0};
    double const u1_rated{150.0e3};
    double const u2_rated{10.0e3};
    Transformer const Dyn11{input, u1_rated, u2_rated};

    // Positive sequence
    double const z_1_abs = input.uk * input.u2 * input.u2 / input.sn;
    double const z_1_real = input.pk * input.u2 * input.u2 / input.sn / input.sn;
    double const z_1_imag = std::sqrt(z_1_abs * z_1_abs - z_1_real * z_1_real);

    double const u1 = input.u1 + (input.tap_pos - input.tap_nom) * input.tap_size;
    double const u2 = input.u2; // Tap is on the from side, not the to side
    double const k = (u1 / u2) / (u1_rated / u2_rated);

    double const base_y_to = base_i_to / (u2_rated / sqrt3);

    DoubleComplex const z_1_series = (z_1_real + 1i * z_1_imag) * base_y_to;

    double const y_shunt_abs = input.i0 * input.sn / input.u2 / input.u2;
    double const y_shunt_real = input.p0 / input.u2 / input.u2;
    double y_shunt_imag;
    if (y_shunt_real > y_shunt_abs) {
        y_shunt_imag = 0.0;
    } else {
        y_shunt_imag = -std::sqrt(y_shunt_abs * y_shunt_abs - y_shunt_real * y_shunt_real);
    }
    DoubleComplex const y_1_shunt = (y_shunt_real + 1i * y_shunt_imag) / base_y_to;

    DoubleComplex const tap_ratio_1 = k * std::exp(1.0i * (deg_30 * input.clock));

    DoubleComplex const y_1_tt = (1.0 / z_1_series) + 0.5 * y_1_shunt;
    DoubleComplex const y_1_ff = (1.0 / k / k) * y_1_tt;
    DoubleComplex const y_1_ft = (-1.0 / conj(tap_ratio_1)) * (1.0 / z_1_series);
    DoubleComplex const y_1_tf = (-1.0 / tap_ratio_1) * (1.0 / z_1_series);

    // Negative sequence
    DoubleComplex const tap_ratio_2 = k * std::exp(-1.0i * (deg_30 * input.clock));

    DoubleComplex const y_2_tt = (1.0 / z_1_series) + 0.5 * y_1_shunt;
    DoubleComplex const y_2_ff = (1.0 / k / k) * y_2_tt;
    DoubleComplex const y_2_ft = (-1.0 / conj(tap_ratio_2)) * (1.0 / z_1_series);
    DoubleComplex const y_2_tf = (-1.0 / tap_ratio_2) * (1.0 / z_1_series);

    // Zero sequence
    DoubleComplex const z_grounding_to = (input.r_grounding_to + 1i * input.x_grounding_to) * base_y_to;

    DoubleComplex const y_0_ff = 0.0;
    DoubleComplex const y_0_ft = 0.0;
    DoubleComplex const y_0_tf = 0.0;
    DoubleComplex const y_0_tt = (1.0 / (z_1_series * 0.1 + 3.0 * z_grounding_to));

    // Sequence admittances -> phase addmitance
    ComplexTensor<asymmetric_t> y_ff_diagonal;
    y_ff_diagonal << y_0_ff, 0.0, 0.0, 0.0, y_1_ff, 0.0, 0.0, 0.0, y_2_ff;

    ComplexTensor<asymmetric_t> y_ft_diagonal;
    y_ft_diagonal << y_0_ft, 0.0, 0.0, 0.0, y_1_ft, 0.0, 0.0, 0.0, y_2_ft;

    ComplexTensor<asymmetric_t> y_tf_diagonal;
    y_tf_diagonal << y_0_tf, 0.0, 0.0, 0.0, y_1_tf, 0.0, 0.0, 0.0, y_2_tf;

    ComplexTensor<asymmetric_t> y_tt_diagonal;
    y_tt_diagonal << y_0_tt, 0.0, 0.0, 0.0, y_1_tt, 0.0, 0.0, 0.0, y_2_tt;

    ComplexTensor<asymmetric_t> const y_ff = dot(A, y_ff_diagonal, A_inv);
    ComplexTensor<asymmetric_t> const y_ft = dot(A, y_ft_diagonal, A_inv);
    ComplexTensor<asymmetric_t> const y_tf = dot(A, y_tf_diagonal, A_inv);
    ComplexTensor<asymmetric_t> const y_tt = dot(A, y_tt_diagonal, A_inv);

    BranchCalcParam<asymmetric_t> const param = Dyn11.calc_param<asymmetric_t>();

    CHECK((cabs(param.value[0] - y_ff) < numerical_tolerance).all());
    CHECK((cabs(param.value[1] - y_ft) < numerical_tolerance).all());
    CHECK((cabs(param.value[2] - y_tf) < numerical_tolerance).all());
    CHECK((cabs(param.value[3] - y_tt) < numerical_tolerance).all());
}

TEST_CASE("Test Transformer - Dyn11 - tap_max and tap_min flipped") {
    ComplexTensor<asymmetric_t> const A = get_a();
    ComplexTensor<asymmetric_t> const A_inv = get_a_inv();

    TransformerInput const input{.id = 1,
                                 .from_node = 2,
                                 .to_node = 3,
                                 .from_status = 1,
                                 .to_status = 1,
                                 .u1 = 155e3,
                                 .u2 = 10.0e3,
                                 .sn = 30e6,
                                 .uk = 0.203,
                                 .pk = 100e3,
                                 .i0 = 0.015,
                                 .p0 = 30.0e4,
                                 .winding_from = WindingType::delta,
                                 .winding_to = WindingType::wye_n,
                                 .clock = 11,
                                 .tap_side = BranchSide::from,
                                 .tap_pos = -2,
                                 .tap_min = 9,
                                 .tap_max = -11,
                                 .tap_nom = 0,
                                 .tap_size = 2.5e3,
                                 .uk_min = nan,
                                 .uk_max = nan,
                                 .pk_min = nan,
                                 .pk_max = nan,
                                 .r_grounding_from = nan,
                                 .x_grounding_from = nan,
                                 .r_grounding_to = 1.0,
                                 .x_grounding_to = 4.0};
    double const u1_rated{150.0e3};
    double const u2_rated{10.0e3};
    Transformer const Dyn11{input, u1_rated, u2_rated};

    // Positive sequence
    double const z_1_abs = input.uk * input.u2 * input.u2 / input.sn;
    double const z_1_real = input.pk * input.u2 * input.u2 / input.sn / input.sn;
    double const z_1_imag = std::sqrt(z_1_abs * z_1_abs - z_1_real * z_1_real);

    double const u1 = input.u1 - (input.tap_pos - input.tap_nom) * input.tap_size;
    double const u2 = input.u2; // Tap is on the from side, not the to side
    double const k = (u1 / u2) / (u1_rated / u2_rated);

    double const base_y_to = base_i_to / (u2_rated / sqrt3);

    DoubleComplex const z_1_series = (z_1_real + 1i * z_1_imag) * base_y_to;

    double const y_shunt_abs = input.i0 * input.sn / input.u2 / input.u2;
    double const y_shunt_real = input.p0 / input.u2 / input.u2;
    double y_shunt_imag;
    if (y_shunt_real > y_shunt_abs) {
        y_shunt_imag = 0.0;
    } else {
        y_shunt_imag = -std::sqrt(y_shunt_abs * y_shunt_abs - y_shunt_real * y_shunt_real);
    }
    DoubleComplex const y_1_shunt = (y_shunt_real + 1i * y_shunt_imag) / base_y_to;

    DoubleComplex const tap_ratio_1 = k * std::exp(1.0i * (deg_30 * input.clock));

    DoubleComplex const y_1_tt = (1.0 / z_1_series) + 0.5 * y_1_shunt;
    DoubleComplex const y_1_ff = (1.0 / k / k) * y_1_tt;
    DoubleComplex const y_1_ft = (-1.0 / conj(tap_ratio_1)) * (1.0 / z_1_series);
    DoubleComplex const y_1_tf = (-1.0 / tap_ratio_1) * (1.0 / z_1_series);

    // Negative sequence
    DoubleComplex const tap_ratio_2 = k * std::exp(-1.0i * (deg_30 * input.clock));

    DoubleComplex const y_2_tt = (1.0 / z_1_series) + 0.5 * y_1_shunt;
    DoubleComplex const y_2_ff = (1.0 / k / k) * y_2_tt;
    DoubleComplex const y_2_ft = (-1.0 / conj(tap_ratio_2)) * (1.0 / z_1_series);
    DoubleComplex const y_2_tf = (-1.0 / tap_ratio_2) * (1.0 / z_1_series);

    // Zero sequence
    DoubleComplex const z_grounding_to = (input.r_grounding_to + 1i * input.x_grounding_to) * base_y_to;

    DoubleComplex const y_0_ff = 0.0;
    DoubleComplex const y_0_ft = 0.0;
    DoubleComplex const y_0_tf = 0.0;
    DoubleComplex const y_0_tt = (1.0 / (z_1_series + 3.0 * z_grounding_to)) + y_1_shunt;

    // Sequence admittances -> phase addmitance
    ComplexTensor<asymmetric_t> y_ff_diagonal;
    y_ff_diagonal << y_0_ff, 0.0, 0.0, 0.0, y_1_ff, 0.0, 0.0, 0.0, y_2_ff;

    ComplexTensor<asymmetric_t> y_ft_diagonal;
    y_ft_diagonal << y_0_ft, 0.0, 0.0, 0.0, y_1_ft, 0.0, 0.0, 0.0, y_2_ft;

    ComplexTensor<asymmetric_t> y_tf_diagonal;
    y_tf_diagonal << y_0_tf, 0.0, 0.0, 0.0, y_1_tf, 0.0, 0.0, 0.0, y_2_tf;

    ComplexTensor<asymmetric_t> y_tt_diagonal;
    y_tt_diagonal << y_0_tt, 0.0, 0.0, 0.0, y_1_tt, 0.0, 0.0, 0.0, y_2_tt;

    ComplexTensor<asymmetric_t> const y_ff = dot(A, y_ff_diagonal, A_inv);
    ComplexTensor<asymmetric_t> const y_ft = dot(A, y_ft_diagonal, A_inv);
    ComplexTensor<asymmetric_t> const y_tf = dot(A, y_tf_diagonal, A_inv);
    ComplexTensor<asymmetric_t> const y_tt = dot(A, y_tt_diagonal, A_inv);

    BranchCalcParam<asymmetric_t> const param = Dyn11.calc_param<asymmetric_t>();

    CHECK((cabs(param.value[0] - y_ff) < numerical_tolerance).all());
    CHECK((cabs(param.value[1] - y_ft) < numerical_tolerance).all());
    CHECK((cabs(param.value[2] - y_tf) < numerical_tolerance).all());
    CHECK((cabs(param.value[3] - y_tt) < numerical_tolerance).all());
}

TEST_CASE("Test Transformer - Test uk_min, uk_max, pk_min, pk_max for tap_pos < tap_nom - Dyn11") {
    TransformerInput const input{.id = 1,
                                 .from_node = 2,
                                 .to_node = 3,
                                 .from_status = 1,
                                 .to_status = 1,
                                 .u1 = 155e3,
                                 .u2 = 10.0e3,
                                 .sn = 30e6,
                                 .uk = 0.203,
                                 .pk = 100e3,
                                 .i0 = 0.015,
                                 .p0 = 30.0e4,
                                 .winding_from = WindingType::delta,
                                 .winding_to = WindingType::wye_n,
                                 .clock = 11,
                                 .tap_side = BranchSide::from,
                                 .tap_pos = -2,
                                 .tap_min = -11,
                                 .tap_max = 9,
                                 .tap_nom = 0,
                                 .tap_size = 2.5e3,
                                 .uk_min = 0.1,
                                 .uk_max = 0.4,
                                 .pk_min = 50e3,
                                 .pk_max = 200e3,
                                 .r_grounding_from = nan,
                                 .x_grounding_from = nan,
                                 .r_grounding_to = nan,
                                 .x_grounding_to = nan};
    double const u1_rated{150.0e3};
    double const u2_rated{10.0e3};
    Transformer const Dyn11{input, u1_rated, u2_rated};

    double const uk_increment_per_tap = (input.uk_min - input.uk) / (input.tap_min - input.tap_nom);
    double const uk = input.uk + (input.tap_pos - input.tap_nom) * uk_increment_per_tap;

    double const pk_increment_per_tap = (input.pk_min - input.pk) / (input.tap_min - input.tap_nom);
    double const pk = input.pk + (input.tap_pos - input.tap_nom) * pk_increment_per_tap;

    double const z_abs = uk * input.u2 * input.u2 / input.sn;
    double const z_real = pk * input.u2 * input.u2 / input.sn / input.sn;
    double const z_imag = std::sqrt(z_abs * z_abs - z_real * z_real);

    double const u1 = input.u1 + (input.tap_pos - input.tap_nom) * input.tap_size;
    double const u2 = input.u2; // Tap is on the from side, not the to side
    double const k = (u1 / u2) / (u1_rated / u2_rated);

    double const base_y_to = base_i_to / (u2_rated / sqrt3);

    DoubleComplex const z_1_series = (z_real + 1i * z_imag) * base_y_to;

    double const y_1_shunt_abs = input.i0 * input.sn / input.u2 / input.u2;
    double const y_1_shunt_real = input.p0 / input.u2 / input.u2;
    double y_1_shunt_imag;
    if (y_1_shunt_real > y_1_shunt_abs) {
        y_1_shunt_imag = 0.0;
    } else {
        y_1_shunt_imag = -std::sqrt(y_1_shunt_abs * y_1_shunt_abs - y_1_shunt_real * y_1_shunt_real);
    }
    DoubleComplex const y_1_shunt = (y_1_shunt_real + 1i * y_1_shunt_imag) / base_y_to;

    DoubleComplex const tap_ratio = k * std::exp(1.0i * (deg_30 * input.clock));

    DoubleComplex const y_tt = (1.0 / z_1_series) + 0.5 * y_1_shunt;
    DoubleComplex const y_ff = (1.0 / k / k) * y_tt;
    DoubleComplex const y_ft = (-1.0 / conj(tap_ratio)) * (1.0 / z_1_series);
    DoubleComplex const y_tf = (-1.0 / tap_ratio) * (1.0 / z_1_series);

    BranchCalcParam<symmetric_t> const param = Dyn11.calc_param<symmetric_t>();

    CHECK(cabs(param.value[0] - y_ff) < numerical_tolerance);
    CHECK(cabs(param.value[1] - y_ft) < numerical_tolerance);
    CHECK(cabs(param.value[2] - y_tf) < numerical_tolerance);
    CHECK(cabs(param.value[3] - y_tt) < numerical_tolerance);
}

TEST_CASE("Test Transformer - Test uk_min, uk_max, pk_min, pk_max for tap_pos > tap_nom - Dyn11") {
    TransformerInput const input{.id = 1,
                                 .from_node = 2,
                                 .to_node = 3,
                                 .from_status = 1,
                                 .to_status = 1,
                                 .u1 = 155e3,
                                 .u2 = 10.0e3,
                                 .sn = 30e6,
                                 .uk = 0.203,
                                 .pk = 100e3,
                                 .i0 = 0.015,
                                 .p0 = 30.0e4,
                                 .winding_from = WindingType::delta,
                                 .winding_to = WindingType::wye_n,
                                 .clock = 11,
                                 .tap_side = BranchSide::from,
                                 .tap_pos = 2,
                                 .tap_min = -11,
                                 .tap_max = 9,
                                 .tap_nom = 0,
                                 .tap_size = 2.5e3,
                                 .uk_min = 0.1,
                                 .uk_max = 0.4,
                                 .pk_min = 50e3,
                                 .pk_max = 200e3,
                                 .r_grounding_from = nan,
                                 .x_grounding_from = nan,
                                 .r_grounding_to = nan,
                                 .x_grounding_to = nan};
    double const u1_rated{150.0e3};
    double const u2_rated{10.0e3};
    Transformer const Dyn11{input, u1_rated, u2_rated};

    double const uk_increment_per_tap = (input.uk_max - input.uk) / (input.tap_max - input.tap_nom);
    double const uk = input.uk + (input.tap_pos - input.tap_nom) * uk_increment_per_tap;

    double const pk_increment_per_tap = (input.pk_max - input.pk) / (input.tap_max - input.tap_nom);
    double const pk = input.pk + (input.tap_pos - input.tap_nom) * pk_increment_per_tap;

    double const z_abs = uk * input.u2 * input.u2 / input.sn;
    double const z_real = pk * input.u2 * input.u2 / input.sn / input.sn;
    double const z_imag = std::sqrt(z_abs * z_abs - z_real * z_real);

    double const u1 = input.u1 + (input.tap_pos - input.tap_nom) * input.tap_size;
    double const u2 = input.u2; // Tap is on the from side, not the to side
    double const k = (u1 / u2) / (u1_rated / u2_rated);

    double const base_y_to = base_i_to / (u2_rated / sqrt3);

    DoubleComplex const z_1_series = (z_real + 1i * z_imag) * base_y_to;

    double const y_1_shunt_abs = input.i0 * input.sn / input.u2 / input.u2;
    double const y_1_shunt_real = input.p0 / input.u2 / input.u2;
    double y_1_shunt_imag;
    if (y_1_shunt_real > y_1_shunt_abs) {
        y_1_shunt_imag = 0.0;
    } else {
        y_1_shunt_imag = -std::sqrt(y_1_shunt_abs * y_1_shunt_abs - y_1_shunt_real * y_1_shunt_real);
    }
    DoubleComplex const y_1_shunt = (y_1_shunt_real + 1i * y_1_shunt_imag) / base_y_to;

    DoubleComplex const tap_ratio = k * std::exp(1.0i * (deg_30 * input.clock));

    DoubleComplex const y_tt = (1.0 / z_1_series) + 0.5 * y_1_shunt;
    DoubleComplex const y_ff = (1.0 / k / k) * y_tt;
    DoubleComplex const y_ft = (-1.0 / conj(tap_ratio)) * (1.0 / z_1_series);
    DoubleComplex const y_tf = (-1.0 / tap_ratio) * (1.0 / z_1_series);

    BranchCalcParam<symmetric_t> const param = Dyn11.calc_param<symmetric_t>();

    CHECK(cabs(param.value[0] - y_ff) < numerical_tolerance);
    CHECK(cabs(param.value[1] - y_ft) < numerical_tolerance);
    CHECK(cabs(param.value[2] - y_tf) < numerical_tolerance);
    CHECK(cabs(param.value[3] - y_tt) < numerical_tolerance);
}

} // namespace power_grid_model
