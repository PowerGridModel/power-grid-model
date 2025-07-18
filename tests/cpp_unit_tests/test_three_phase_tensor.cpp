// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/auxiliary/output.hpp>
#include <power_grid_model/common/three_phase_tensor.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {

using namespace std::complex_literals;

TEST_CASE("Three phase tensor") {
    SUBCASE("Test vectors") {
        RealValue<asymmetric_t> vec1;
        vec1 << 1, 2, 3;
        RealValue<asymmetric_t> vec2;
        vec2 << 4, 5, 6;
        RealValue<asymmetric_t> vec3 = vec1 * vec2;
        CHECK(vec3(0) == 4);
        RealValue<asymmetric_t> vec4 = cos(vec1);
        CHECK(vec4(0) == doctest::Approx(cos(1)));
        RealValue<asymmetric_t> vec5 = vec1 / vec2;
        CHECK(vec5(1) == doctest::Approx(0.4));
        ComplexValue<asymmetric_t> vec6 = vec1 * exp(1.0i * vec2);
        CHECK(cabs(vec6(1) - DoubleComplex(2.0 * cos(5.0), 2.0 * sin(5.0))) < 1e-8);
        CHECK(max_val(vec1) == 3.0);
        CHECK(max_val(3.0) == 3.0);
        ComplexValue<asymmetric_t> vec7;
        vec7 << 1.0, 2.0, 3.0;
        CHECK(real(sum_val(vec7)) == 6.0);
        CHECK(real(mean_val(vec7)) == 2.0);
        CHECK(real(sum_val(DoubleComplex{1.0, 0.0})) == 1.0);
        CHECK(real(mean_val(DoubleComplex{1.0, 0.0})) == 1.0);
    }

    SUBCASE("Test vector initialization with single value") {
        RealValue<asymmetric_t> vec6{5.0};
        CHECK(vec6(0) == doctest::Approx(5.0));
        CHECK(vec6(1) == doctest::Approx(5.0));
        CHECK(vec6(2) == doctest::Approx(5.0));
        ComplexValue<asymmetric_t> vec7{1.0};
        CHECK(cabs(vec7(0) - 1.0) < 1e-8);
        CHECK(cabs(vec7(1) - a2) < 1e-8);
        CHECK(cabs(vec7(2) - a) < 1e-8);
        static_assert(RealValue<symmetric_t>{1.0} == 1.0);
        static_assert(ComplexValue<symmetric_t>{1.0} == 1.0);
        CHECK(real(vec7)(0) == 1.0);
    }

    SUBCASE("Test complex vector piecewise initialization with single value") {
        ComplexValue<asymmetric_t> vec7 = piecewise_complex_value<asymmetric_t>(1.0);
        CHECK(cabs(vec7(0) - 1.0) < 1e-8);
        CHECK(cabs(vec7(1) - 1.0) < 1e-8);
        CHECK(cabs(vec7(2) - 1.0) < 1e-8);
    }

    SUBCASE("Test tensors") {
        RealValue<asymmetric_t> vec1;
        vec1 << 1, 2, 3;
        RealValue<asymmetric_t> vec2;
        vec2 << 4, 5, 6;
        RealTensor<asymmetric_t> mat = vector_outer_product(vec1, vec2);
        CHECK(mat(0, 0) == 4.0);
        CHECK(mat(2, 0) == 12.0);
        CHECK(mat(0, 2) == 6.0);
        CHECK(vector_outer_product(2.0, 3.0) == 6.0);
        RealTensor<asymmetric_t> mat1;
        mat1 << 1, 2, 3, 4, 5, 6, 7, 8, 9;
        RealValue<asymmetric_t> vec3 = dot(mat1, vec1 + vec2);
        CHECK(vec3(0) == 46.0);
        CHECK(vec3(1) == 109.0);
        CHECK(vec3(2) == 172.0);
        CHECK(dot(2.0, 3.0) == 6.0);
        CHECK(dot(2.0i, 3.0i) == -6.0);
        RealValue<asymmetric_t> vec4 = sum_row(mat1);
        CHECK(vec4(0) == 6.0);
        CHECK(vec4(1) == 15.0);
        CHECK(vec4(2) == 24.0);
        CHECK(sum_row(2.0) == 2.0);
        RealTensor<asymmetric_t> mat3;
        mat3 << 1, 1, 1, 1, 1, 1, 1, 1, 1;
        RealTensor<asymmetric_t> const mat4 = diag_mult(vec1, mat3, vec2);
        CHECK((mat4 == mat).all());
        CHECK(diag_mult(1.0, 2.0, 3.0) == 6.0);
        // test layout
        auto* arr = reinterpret_cast<double*>(&mat1);
        CHECK(arr[0] == 1);
        CHECK(arr[2] == 7);
        CHECK(arr[6] == 3);
    }

    SUBCASE("Test tensor initialization and inverse") {
        ComplexTensor<asymmetric_t> const mat{1.0 + 1.0i};
        ComplexTensor<asymmetric_t> mat2;
        mat2 << (1.0 + 1.0i), 0.0, 0.0, 0.0, (1.0 + 1.0i), 0.0, 0.0, 0.0, (1.0 + 1.0i);
        CHECK((mat == mat2).all());
        static_assert(ComplexTensor<symmetric_t>{DoubleComplex{1.0, 1.0}} == DoubleComplex{1.0, 1.0});
        ComplexTensor<asymmetric_t> mat3 = inv(mat2);
        CHECK(cabs(mat3(0, 0) - 1.0 / (1.0 + 1.0i)) < 1e-8);
        CHECK(cabs(inv((1.0 + 1.0i)) - 1.0 / (1.0 + 1.0i)) < 1e-8);
    }

    SUBCASE("Test value initialization") {
        constexpr NodeOutput<symmetric_t> sym{};
        static_assert(sym.id == na_IntID);
        static_assert(sym.energized == na_IntS);
        static_assert(is_nan(sym.id));
        static_assert(is_nan(sym.energized));
        CHECK(is_nan(sym.u_pu));
        CHECK(is_nan(sym.u));
        CHECK(is_nan(sym.u_angle));
        NodeOutput<asymmetric_t> const asym{};
        CHECK(asym.id == na_IntID);
        CHECK(asym.energized == na_IntS);
        CHECK(is_nan(asym.id));
        CHECK(is_nan(asym.energized));
        CHECK(is_nan(asym.u_pu(0)));
        CHECK(is_nan(asym.u(1)));
        CHECK(is_nan(asym.u_angle(2)));
    }

    SUBCASE("Test symmetrical matrix") {
        ComplexTensor<asymmetric_t> const sym = get_sym_matrix();
        ComplexTensor<asymmetric_t> const sym1 = get_sym_matrix_inv();
        ComplexValue<asymmetric_t> const uabc{1.0};
        ComplexValue<asymmetric_t> u012 = dot(sym1, uabc);
        CHECK(cabs(u012(0)) < numerical_tolerance);
        CHECK(cabs(u012(1)) == doctest::Approx(1.0));
        CHECK(cabs(u012(2)) < numerical_tolerance);
        ComplexValue<asymmetric_t> const uabc1 = dot(sym, u012);
        CHECK((cabs(uabc1 - uabc) < numerical_tolerance).all());
    }

    SUBCASE("Test diagonal add") {
        RealTensor<asymmetric_t> mat1;
        mat1 << 1, 2, 3, 4, 5, 6, 7, 8, 9;
        RealValue<asymmetric_t> vec1;
        vec1 << 1, 2, 3;
        RealValue<asymmetric_t> vec2;
        vec2 << 4, 5, 6;
        add_diag(mat1, -vec1 * vec2);
        RealTensor<asymmetric_t> mat2;
        mat2 << -3, 2, 3, 4, -5, 6, 7, 8, -9;
        CHECK((mat1 == mat2).all());
        static_assert([] {
            double x = 5;
            double const y = 10;
            add_diag(x, -y);
            return x;
        }() == -5);
    }

    SUBCASE("Test Hermitian transpose") {
        constexpr double x = 1.0;
        constexpr DoubleComplex y{1.0, 5.0};
        RealTensor<asymmetric_t> const z1{1.0, 2.0};
        ComplexTensor<asymmetric_t> z2;
        z2 << 1.0 + 5.0i, 3.0 - 4.0i, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0;
        ComplexTensor<asymmetric_t> z2ht;
        z2ht << 1.0 - 5.0i, 0.0, 0.0, 3.0 + 4.0i, 0.0, 0.0, 0.0, 0.0, 0.0;

        static_assert(hermitian_transpose(x) == 1.0);
        static_assert(hermitian_transpose(y) == DoubleComplex{1.0, -5.0});
        CHECK((hermitian_transpose(z1) == z1).all());
        CHECK((hermitian_transpose(z2) == z2ht).all());
    }

    SUBCASE("Test average of nan") {
        constexpr DoubleComplex x{1.0, nan};
        constexpr DoubleComplex y{2.0, 2.0};
        constexpr DoubleComplex z{3.0, 5.0};
        DoubleComplex const avg{(x + y + z) / 3.0};
        CHECK(real(avg) == 2.0);
        CHECK(is_nan(imag(avg)));

        ComplexValue<asymmetric_t> const v1{x, x, x};
        ComplexValue<asymmetric_t> const v2{y, y, y};
        ComplexValue<asymmetric_t> const v3{z, z, z};
        ComplexValue<asymmetric_t> const va = (v1 + v2 + v3) / 3.0;
        CHECK((real(va) == 2.0).all());
        CHECK(is_nan(imag(va)));
    }

    SUBCASE("Test RealValue update - sym") {
        constexpr RealValue<symmetric_t> update_1 = nan;
        constexpr RealValue<symmetric_t> update_2 = 2.0;
        constexpr double scalar = 3.0;

        RealValue<symmetric_t> value = 1.0;
        CHECK(value == 1.0);

        update_real_value<symmetric_t>(update_1, value, scalar);
        CHECK(value == 1.0);

        update_real_value<symmetric_t>(update_2, value, scalar);
        CHECK(value == 6.0);
    }

    SUBCASE("Test RealValue update - asym") {
        RealValue<asymmetric_t> const vec_update_1{nan, nan, nan};
        RealValue<asymmetric_t> const vec_update_2{nan, nan, 2.0};
        constexpr double scalar = 3.0;

        RealValue<asymmetric_t> vec{1.0, nan, nan};
        CHECK(vec(0) == 1.0);
        CHECK(is_nan(vec(1)));
        CHECK(is_nan(vec(2)));

        update_real_value<asymmetric_t>(vec_update_1, vec, scalar);
        CHECK(vec(0) == 1.0);
        CHECK(is_nan(vec(1)));
        CHECK(is_nan(vec(2)));

        update_real_value<asymmetric_t>(vec_update_2, vec, scalar);
        CHECK(vec(0) == 1.0);
        CHECK(is_nan(vec(1)));
        CHECK(vec(2) == 6.0);
    }
    SUBCASE("phase_mod_2pi") {
        auto const check = [](double value) {
            CAPTURE(value);
            CHECK_GE(value, -pi);
            CHECK_LE(value, pi);
            if (value != pi && value != -pi) {
                CHECK(phase_mod_2pi(value) == doctest::Approx(value));
            }
        };
        auto const check_asym = [&check](RealValue<asymmetric_t> const& value) {
            for (Idx i : {0, 1, 2}) {
                CAPTURE(i);
                check(value(i));
            }
        };
        check(phase_mod_2pi(0.0));
        check(phase_mod_2pi(2.0 * pi));
        check(phase_mod_2pi(2.0 * pi + 1.0));
        check(phase_mod_2pi(-1.0));
        check(phase_mod_2pi(-pi));
        check(phase_mod_2pi(pi));
        check(phase_mod_2pi(-3.0 * pi));
        check(phase_mod_2pi(3.0 * pi));
        check(phase_mod_2pi(pi * (1.0 + std::numeric_limits<double>::epsilon())));
        check(phase_mod_2pi(pi * (1.0 - std::numeric_limits<double>::epsilon())));
        check(phase_mod_2pi(-pi * (1.0 + std::numeric_limits<double>::epsilon())));
        check(phase_mod_2pi(-pi * (1.0 - std::numeric_limits<double>::epsilon())));

        check_asym(phase_mod_2pi(RealValue<asymmetric_t>{0.0, 2.0 * pi, 2.0 * pi + 1.0}));
        check_asym(phase_mod_2pi(RealValue<asymmetric_t>{0.0, 2.0 * pi, 2.0 * pi + 1.0}));
        check_asym(phase_mod_2pi(RealValue<asymmetric_t>{0.0, 2.0 * pi, 2.0 * pi + 1.0}));
    }
}

} // namespace power_grid_model
