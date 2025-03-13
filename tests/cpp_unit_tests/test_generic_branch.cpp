// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/component/generic_branch.hpp>
#include <power_grid_model/component/transformer.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {

using namespace std::complex_literals;

TEST_CASE("Test generic_branch") {
    constexpr double u1 = 150e3;
    constexpr double u2 = 10.0e3;
    constexpr double base_i_from = base_power_3p / u1 / sqrt3;
    constexpr double base_i_to = base_power_3p / u2 / sqrt3;

    GenericBranchInput const input{.id = 1,
                                   .from_node = 2,
                                   .to_node = 3,
                                   .from_status = 1,
                                   .to_status = 1,
                                   .r1 = 0.016,
                                   .x1 = 0.16,
                                   .g1 = 0.0,
                                   .b1 = 0.0,
                                   .k = 1.0,
                                   .theta = 0.0,
                                   .sn = 30e6};

    GenericBranch const branch{input, u1, u2};
    double const base_y = base_i_to * base_i_to / base_power_1p;

    DoubleComplex const y1_series = 1.0 / (input.r1 + 1.0i * input.x1) / base_y;
    DoubleComplex const y1_shunt = (input.g1 + 1.0i * input.b1) / base_y;

    // symmetric
    DoubleComplex const yff1 = y1_series + 0.5 * y1_shunt;
    DoubleComplex const yft1 = -y1_series;

    SUBCASE("General") {
        CHECK(branch.from_node() == 2);
        CHECK(branch.to_node() == 3);
        CHECK(branch.from_status() == true);
        CHECK(branch.to_status() == true);
        CHECK(branch.branch_status() == true);
        CHECK(branch.status(BranchSide::from) == branch.from_status());
        CHECK(branch.status(BranchSide::to) == branch.to_status());
        CHECK(branch.base_i_from() == doctest::Approx(base_i_from));
        CHECK(branch.base_i_to() == doctest::Approx(base_i_to));
        CHECK(branch.phase_shift() == 0.0);
        CHECK(!branch.is_param_mutable());
    }

    SUBCASE("Symmetric parameters") {
        BranchCalcParam<symmetric_t> param = branch.calc_param<symmetric_t>();
        CHECK(cabs(param.yff() - yff1) < numerical_tolerance);
        CHECK(cabs(param.ytt() - yff1) < numerical_tolerance);
        CHECK(cabs(param.ytf() - yft1) < numerical_tolerance);
        CHECK(cabs(param.yft() - yft1) < numerical_tolerance);
    }

    SUBCASE("Test i base") {
        CHECK(base_i_from == doctest::Approx(base_i_from));
        CHECK(base_i_to == doctest::Approx(base_i_to));
    }

    SUBCASE("Test genericbranch phase shift") { CHECK(branch.phase_shift() == doctest::Approx(0.0)); }

    SUBCASE("Test genericbranch loading") { CHECK(branch.loading(60.0e6, 0.0) == doctest::Approx(2.0)); }
}

TEST_CASE("Test compare_generic_branch") {
    constexpr double ratio = 1.03;
    constexpr double u1 = 1e4;
    constexpr double u2 = 4e2;

    constexpr double u1_rated = u1;
    constexpr double u2_rated = ratio * 4e2; // ensures that the transformer ratio matches the generic branch ratio

    constexpr double base_i_to = base_power_3p / u2_rated / sqrt3;
    constexpr double base_y = base_i_to * base_i_to / base_power_1p;

    GenericBranchInput const genb_input{.id = 1,
                                        .from_node = 2,
                                        .to_node = 3,
                                        .from_status = 1,
                                        .to_status = 1,
                                        .r1 = 0.016,
                                        .x1 = 0.159198,
                                        .g1 = 6.25e-08,
                                        .b1 = -6.21867e-07,
                                        .k = 1.03,
                                        .theta = 0.0,
                                        .sn = 1e5};

    GenericBranch const gen_branch{genb_input, u1_rated, u2_rated};

    double const theta{gen_branch.phase_shift()};

    DoubleComplex const genb_ratio = genb_input.k * std::exp(1.0i * theta);
    DoubleComplex const genb_abs = std::abs(genb_ratio);
    DoubleComplex const z1_series = genb_input.r1 + 1.0i * genb_input.x1;
    DoubleComplex const y1_series = 1.0 / z1_series / base_y;
    DoubleComplex const y1_shunt = (genb_input.g1 + 1.0i * genb_input.b1) / base_y;

    SUBCASE("Symmetric genbranch_parameters") {
        BranchCalcParam<symmetric_t> param1 = gen_branch.calc_param<symmetric_t>();
        // symmetric
        DoubleComplex const ytt1 = y1_series + 0.5 * y1_shunt;
        DoubleComplex const yff1 = (1.0 / genb_abs / genb_abs) * ytt1;
        DoubleComplex const yft1 = (-1.0 / conj(genb_ratio)) * y1_series;
        DoubleComplex const ytf1 = (-1.0 / genb_ratio) * y1_series;

        CHECK(cabs(param1.yff() - yff1) < numerical_tolerance);
        CHECK(cabs(param1.ytt() - ytt1) < numerical_tolerance);
        CHECK(cabs(param1.ytf() - ytf1) < numerical_tolerance);
        CHECK(cabs(param1.yft() - yft1) < numerical_tolerance);
    }

    TransformerInput const trans_input{
        .id = 1,
        .from_node = 2,
        .to_node = 3,
        .from_status = 1,
        .to_status = 1,
        .u1 = 1e4,
        .u2 = 4e2,
        .sn = 1e5,
        .uk = 0.1,
        .pk = 1e3,
        .i0 = 1.0e-6,
        .p0 = 0.01,
        .winding_from = WindingType::wye_n,
        .winding_to = WindingType::wye_n,
        .clock = 12,
        .tap_side = BranchSide::from,
        .tap_pos = 0, // tap_pos influences uk and p0, which results in modified z_series and y_shunt values!
        .tap_min = -11,
        .tap_max = 9,
        .tap_nom = 0,
        .tap_size = 100.0,
        .uk_min = nan,
        .uk_max = nan,
        .pk_min = nan,
        .pk_max = nan,
        .r_grounding_from = nan,
        .x_grounding_from = nan,
        .r_grounding_to = nan,
        .x_grounding_to = nan};

    double const sn{trans_input.sn};
    double const pk{trans_input.pk};
    double const p0{trans_input.p0};
    double const uk{trans_input.uk};
    double const nominal_ratio{u1_rated / u2_rated};
    double const k{(trans_input.u1 / trans_input.u2) / nominal_ratio};
    double const i0{trans_input.i0};

    Transformer const transformer(trans_input, u1_rated, u2_rated);

    DoubleComplex const trafo_ratio = k * std::exp(1.0i * (transformer.phase_shift()));
    double const ratio_abs = std::abs(trafo_ratio);

    // y_series:
    double const z_series_abs = uk * u2 * u2 / sn;
    double const r_series = pk * u2 * u2 / sn / sn;
    double const z_series_imag_squared = z_series_abs * z_series_abs - r_series * r_series;
    double const z_series_imag = (z_series_imag_squared > 0.0 ? std::sqrt(z_series_imag_squared) : 0.0);
    DoubleComplex const z_series = r_series + 1.0i * z_series_imag;
    DoubleComplex const y_series = 1.0 / z_series / base_y;

    // y_shunt:
    DoubleComplex y_shunt;
    double const y_shunt_abs = i0 * sn / u2 / u2;
    y_shunt.real(p0 / u2 / u2);
    double const y_shunt_imag_squared = y_shunt_abs * y_shunt_abs - y_shunt.real() * y_shunt.real();
    y_shunt.imag(y_shunt_imag_squared > 0.0 ? -std::sqrt(y_shunt_imag_squared) : 0.0);
    y_shunt = y_shunt / base_y;

    SUBCASE("Tap position") { CHECK((trans_input.tap_pos - trans_input.tap_nom) == 0); }

    SUBCASE("Symmetric trafo_parameters") {
        BranchCalcParam<symmetric_t> param = transformer.calc_param<symmetric_t>();

        DoubleComplex const ytt = y_series + 0.5 * y_shunt;
        DoubleComplex const yff = (1.0 / ratio_abs / ratio_abs) * ytt;
        DoubleComplex const yft = (-1.0 / conj(trafo_ratio)) * y_series;
        DoubleComplex const ytf = (-1.0 / trafo_ratio) * y_series;

        CHECK(cabs(param.yff() - yff) < numerical_tolerance);
        CHECK(cabs(param.ytt() - ytt) < numerical_tolerance);
        CHECK(cabs(param.ytf() - ytf) < numerical_tolerance);
        CHECK(cabs(param.yft() - yft) < numerical_tolerance);
    }

    SUBCASE("Symmetric compare Y-Parameters") {
        BranchCalcParam<symmetric_t> param1 = gen_branch.calc_param<symmetric_t>();
        BranchCalcParam<symmetric_t> param = transformer.calc_param<symmetric_t>();

        CHECK(cabs((trafo_ratio - genb_ratio)) < numerical_tolerance);

        CHECK((y_shunt.real() - y1_shunt.real()) < 1e-6);
        CHECK((y_shunt.imag() - y1_shunt.imag()) < 1e-6);
        CHECK((y_series.real() - y1_series.real()) < 1e-6);
        CHECK((y_series.imag() - y1_series.imag()) < 1e-6);

        CHECK(cabs(param1.yff() - param.yff()) < 1e-6);
        CHECK(cabs(param1.ytt() - param.ytt()) < 1e-6);
        CHECK(cabs(param1.ytf() - param1.ytf()) < 1e-6);
        CHECK(cabs(param1.yft() - param1.yft()) < 1e-6);
    }

    SUBCASE("Test transformer phase shift") { CHECK(transformer.phase_shift() == doctest::Approx(0.0)); }
}

} // namespace power_grid_model
