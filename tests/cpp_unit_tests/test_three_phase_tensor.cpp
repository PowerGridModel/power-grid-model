// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include "doctest/doctest.h"
#include "power_grid_model/auxiliary/output.hpp"
#include "power_grid_model/three_phase_tensor.hpp"

namespace power_grid_model {

using namespace std::complex_literals;

TEST_CASE("Three phase tensor") {
    SUBCASE("Test vectors") {
        RealValue<false> vec1;
        vec1 << 1, 2, 3;
        RealValue<false> vec2;
        vec2 << 4, 5, 6;
        RealValue<false> vec3 = vec1 * vec2;
        CHECK(vec3(0) == 4);
        RealValue<false> vec4 = cos(vec1);
        CHECK(vec4(0) == doctest::Approx(cos(1)));
        RealValue<false> vec5 = vec1 / vec2;
        CHECK(vec5(1) == doctest::Approx(0.4));
        ComplexValue<false> vec6 = vec1 * exp(1.0i * vec2);
        CHECK(cabs(vec6(1) - DoubleComplex(2.0 * cos(5.0), 2.0 * sin(5.0))) < 1e-8);
        CHECK(max_val(vec1) == 3.0);
        CHECK(max_val(3.0) == 3.0);
        ComplexValue<false> vec7;
        vec7 << 1.0, 2.0, 3.0;
        CHECK(real(sum_val(vec7)) == 6.0);
        CHECK(real(mean_val(vec7)) == 2.0);
        CHECK(real(sum_val(DoubleComplex{1.0, 0.0})) == 1.0);
        CHECK(real(mean_val(DoubleComplex{1.0, 0.0})) == 1.0);
    }

    SUBCASE("Test vector initialization with single value") {
        RealValue<false> vec6{5.0};
        CHECK(vec6(0) == doctest::Approx(5.0));
        CHECK(vec6(1) == doctest::Approx(5.0));
        CHECK(vec6(2) == doctest::Approx(5.0));
        ComplexValue<false> vec7{1.0};
        CHECK(cabs(vec7(0) - 1.0) < 1e-8);
        CHECK(cabs(vec7(1) - a2) < 1e-8);
        CHECK(cabs(vec7(2) - a) < 1e-8);
        CHECK(RealValue<true>{1.0} == 1.0);
        CHECK(ComplexValue<true>{1.0} == 1.0);
        CHECK(real(vec7)(0) == 1.0);
    }

    SUBCASE("Test complex vector piecewise initialization with single value") {
        ComplexValue<false> vec7 = piecewise_complex_value<false>(1.0);
        CHECK(cabs(vec7(0) - 1.0) < 1e-8);
        CHECK(cabs(vec7(1) - 1.0) < 1e-8);
        CHECK(cabs(vec7(2) - 1.0) < 1e-8);
    }

    SUBCASE("Test tensors") {
        RealValue<false> vec1;
        vec1 << 1, 2, 3;
        RealValue<false> vec2;
        vec2 << 4, 5, 6;
        RealTensor<false> mat = vector_outer_product(vec1, vec2);
        CHECK(mat(0, 0) == 4.0);
        CHECK(mat(2, 0) == 12.0);
        CHECK(mat(0, 2) == 6.0);
        CHECK(vector_outer_product(2.0, 3.0) == 6.0);
        RealTensor<false> mat1;
        mat1 << 1, 2, 3, 4, 5, 6, 7, 8, 9;
        RealValue<false> vec3 = dot(mat1, vec1 + vec2);
        CHECK(vec3(0) == 46.0);
        CHECK(vec3(1) == 109.0);
        CHECK(vec3(2) == 172.0);
        CHECK(dot(2.0, 3.0) == 6.0);
        CHECK(dot(2.0i, 3.0i) == -6.0);
        RealValue<false> vec4 = sum_row(mat1);
        CHECK(vec4(0) == 6.0);
        CHECK(vec4(1) == 15.0);
        CHECK(vec4(2) == 24.0);
        CHECK(sum_row(2.0) == 2.0);
        RealTensor<false> mat3;
        mat3 << 1, 1, 1, 1, 1, 1, 1, 1, 1;
        RealTensor<false> mat4 = diag_mult(vec1, mat3, vec2);
        CHECK((mat4 == mat).all());
        CHECK(diag_mult(1.0, 2.0, 3.0) == 6.0);
        // test layout
        double* arr = (double*)&mat1;
        CHECK(arr[0] == 1);
        CHECK(arr[2] == 7);
        CHECK(arr[6] == 3);
    }

    SUBCASE("Test tensor initialization and inverse") {
        ComplexTensor<false> mat{1.0 + 1.0i};
        ComplexTensor<false> mat2;
        mat2 << (1.0 + 1.0i), 0.0, 0.0, 0.0, (1.0 + 1.0i), 0.0, 0.0, 0.0, (1.0 + 1.0i);
        CHECK((mat == mat2).all());
        CHECK(ComplexTensor<true>{1.0 + 1.0i} == (1.0 + 1.0i));
        ComplexTensor<false> mat3 = inv(mat2);
        CHECK(cabs(mat3(0, 0) - 1.0 / (1.0 + 1.0i)) < 1e-8);
        CHECK(cabs(inv((1.0 + 1.0i)) - 1.0 / (1.0 + 1.0i)) < 1e-8);
    }

    SUBCASE("Test value initialization") {
        NodeOutput<true> sym{};
        CHECK(sym.id == 0);
        CHECK(sym.energized == 0);
        CHECK(sym.u_pu == 0.0);
        CHECK(sym.u == 0.0);
        CHECK(sym.u_angle == 0.0);
        NodeOutput<false> asym{};
        CHECK(asym.id == 0);
        CHECK(asym.energized == 0);
        CHECK(asym.u_pu(0) == doctest::Approx(0.0));
        CHECK(asym.u(1) == doctest::Approx(0.0));
        CHECK(asym.u_angle(2) == doctest::Approx(0.0));
    }

    SUBCASE("Test symmetrical matrix") {
        ComplexTensor<false> sym = get_sym_matrix();
        ComplexTensor<false> sym1 = get_sym_matrix_inv();
        ComplexValue<false> uabc{1.0};
        ComplexValue<false> u012 = dot(sym1, uabc);
        CHECK(cabs(u012(0)) < numerical_tolerance);
        CHECK(cabs(u012(1)) == doctest::Approx(1.0));
        CHECK(cabs(u012(2)) < numerical_tolerance);
        ComplexValue<false> uabc1 = dot(sym, u012);
        CHECK((cabs(uabc1 - uabc) < numerical_tolerance).all());
    }

    SUBCASE("Test diagonal add") {
        RealTensor<false> mat1;
        mat1 << 1, 2, 3, 4, 5, 6, 7, 8, 9;
        RealValue<false> vec1;
        vec1 << 1, 2, 3;
        RealValue<false> vec2;
        vec2 << 4, 5, 6;
        add_diag(mat1, -vec1 * vec2);
        RealTensor<false> mat2;
        mat2 << -3, 2, 3, 4, -5, 6, 7, 8, -9;
        CHECK((mat1 == mat2).all());
        double x = 5, y = 10;
        add_diag(x, -y);
        CHECK(x == -5);
    }

    SUBCASE("Test Hermitian transpose") {
        double const x = 1.0;
        DoubleComplex const y{1.0, 5.0};
        RealTensor<false> const z1{1.0, 2.0};
        ComplexTensor<false> z2;
        z2 << 1.0 + 5.0i, 3.0 - 4.0i, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0;
        ComplexTensor<false> z2ht;
        z2ht << 1.0 - 5.0i, 0.0, 0.0, 3.0 + 4.0i, 0.0, 0.0, 0.0, 0.0, 0.0;

        CHECK(hermitian_transpose(x) == 1.0);
        CHECK(hermitian_transpose(y) == (1.0 - 5.0i));
        CHECK((hermitian_transpose(z1) == z1).all());
        CHECK((hermitian_transpose(z2) == z2ht).all());
    }

    SUBCASE("Test average of nan") {
        DoubleComplex const x{1.0, nan};
        DoubleComplex const y{2.0, 2.0};
        DoubleComplex const z{3.0, 5.0};
        DoubleComplex const avg = (x + y + z) / 3.0;
        CHECK(real(avg) == 2.0);
        CHECK(is_nan(imag(avg)));

        ComplexValue<false> const v1{x, x, x};
        ComplexValue<false> const v2{y, y, y};
        ComplexValue<false> const v3{z, z, z};
        ComplexValue<false> const va = (v1 + v2 + v3) / 3.0;
        CHECK((real(va) == 2.0).all());
        CHECK(is_nan(imag(va)));
    }

    SUBCASE("Test RealValue update - sym") {
        RealValue<true> value = 1.0;
        RealValue<true> update_1 = nan;
        RealValue<true> update_2 = 2.0;
        double scalar = 3.0;

        update_real_value<true>(update_1, value, scalar);
        CHECK(value == 1.0);

        update_real_value<true>(update_2, value, scalar);
        CHECK(value == 6.0);
    }

    SUBCASE("Test RealValue update - asym") {
        RealValue<false> vec{1.0, nan, nan};
        RealValue<false> vec_update_1{nan, nan, nan};
        RealValue<false> vec_update_2{nan, nan, 2.0};
        double scalar = 3.0;

        update_real_value<false>(vec_update_1, vec, scalar);
        CHECK(vec(0) == 1.0);
        CHECK(is_nan(vec(1)));
        CHECK(is_nan(vec(2)));

        update_real_value<false>(vec_update_2, vec, scalar);
        CHECK(vec(0) == 1.0);
        CHECK(is_nan(vec(1)));
        CHECK(vec(2) == 6.0);
    }
}

}  // namespace power_grid_model