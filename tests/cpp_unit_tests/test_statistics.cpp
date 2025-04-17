// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/common/statistics.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {
namespace {
using AsymRealValue = RealValue<asymmetric_t>;

using std::numbers::sqrt2;
constexpr auto inv_sqrt2 = sqrt2 / 2;

constexpr auto sqrt3_2 = std::numbers::sqrt3 * 0.5;
constexpr auto deg_90 = std::numbers::pi / 2.0;
constexpr auto deg_45 = deg_90 / 2.0;
constexpr auto deg_60 = deg_30 * 2.0;
const ComplexValue<asymmetric_t> unit_sym_phasor{1.0};

} // namespace

TEST_SUITE_BEGIN("Statistics module tests");

TEST_CASE("Test statistics") {
    SUBCASE("UniformRealRandVar<symmetric_t>") {
        for (auto const& [value, variance] :
             std::array{std::tuple{1.0, 1.0}, std::tuple{2.0, 3.0}, std::tuple{0.0, 1.0}, std::tuple{2.0, 3.0}}) {
            CAPTURE(value);
            CAPTURE(variance);

            UniformRealRandVar<symmetric_t> const uniform{.value = value, .variance = variance};

            SUBCASE("Constructor") {
                CHECK(uniform.value == value);
                CHECK(uniform.variance == variance);
            }
            SUBCASE("Conversion to UniformRealRandVar<asymmetric_t>") {
                auto const asymmetric = static_cast<UniformRealRandVar<asymmetric_t>>(uniform);

                CHECK(asymmetric.value(0) == doctest::Approx(uniform.value));
                CHECK(asymmetric.value(1) == doctest::Approx(uniform.value));
                CHECK(asymmetric.value(2) == doctest::Approx(uniform.value));
                CHECK(asymmetric.variance == doctest::Approx(variance));
            }
        }
    }
    SUBCASE("UniformRealRandVar<asymmetric_t>") {
        for (auto const& [value_a, value_b, value_c, variance] :
             std::array{std::tuple{1.0, 2.0, 3.0, 1.0}, std::tuple{2.0, 2.1, 2.2, 3.0}, std::tuple{0.0, 0.1, 0.2, 1.0},
                        std::tuple{2.0, 0.0, 0.0, 3.0}}) {
            CAPTURE(value_a);
            CAPTURE(value_b);
            CAPTURE(value_c);
            CAPTURE(variance);

            UniformRealRandVar<asymmetric_t> const uniform{.value = {value_a, value_b, value_c}, .variance = variance};

            SUBCASE("Constructor") {
                CHECK(uniform.value(0) == value_a);
                CHECK(uniform.value(1) == value_b);
                CHECK(uniform.value(2) == value_c);
                CHECK(uniform.variance == variance);
            }
            SUBCASE("Conversion to UniformRealRandVar<symmetric_t>") {
                auto const symmetric = static_cast<UniformRealRandVar<symmetric_t>>(uniform);

                CHECK(symmetric.value == doctest::Approx(mean_val(uniform.value)));
                CHECK(symmetric.variance == doctest::Approx(variance / 3));
            }
        }
    }

    SUBCASE("IndependentRealRandVar<symmetric_t>") {
        for (auto const& [value, variance] :
             std::array{std::tuple{1.0, 1.0}, std::tuple{2.0, 3.0}, std::tuple{0.0, 1.0}, std::tuple{2.0, 3.0}}) {
            CAPTURE(value);
            CAPTURE(variance);

            IndependentRealRandVar<symmetric_t> const independent{.value = value, .variance = variance};

            SUBCASE("Constructor") {
                CHECK(independent.value == value);
                CHECK(independent.variance == variance);
            }
            SUBCASE("Conversion to IndependentRealRandVar<asymmetric_t>") {
                auto const asymmetric = static_cast<IndependentRealRandVar<asymmetric_t>>(independent);

                CHECK(asymmetric.value(0) == doctest::Approx(independent.value));
                CHECK(asymmetric.value(1) == doctest::Approx(independent.value));
                CHECK(asymmetric.value(2) == doctest::Approx(independent.value));
                CHECK(asymmetric.variance(0) == doctest::Approx(independent.variance));
                CHECK(asymmetric.variance(1) == doctest::Approx(independent.variance));
                CHECK(asymmetric.variance(2) == doctest::Approx(independent.variance));
            }
        }
    }
    SUBCASE("IndependentRealRandVar<asymmetric_t>") {
        for (auto const& [value_a, value_b, value_c, variance_a, variance_b, variance_c] :
             std::array{std::tuple{1.0, 2.0, 3.0, 1.0, 2.0, 3.0}, std::tuple{2.0, 2.1, 2.2, 3.0, 1.0, 2.0},
                        std::tuple{0.0, 0.1, 0.2, 1.0, 1.0, 1.0}, std::tuple{2.0, 0.0, 0.0, 3.0, 3.0, 3.0}}) {
            CAPTURE(value_a);
            CAPTURE(value_b);
            CAPTURE(value_c);
            CAPTURE(variance_a);
            CAPTURE(variance_b);
            CAPTURE(variance_c);

            IndependentRealRandVar<asymmetric_t> const independent{.value = {value_a, value_b, value_c},
                                                                   .variance = {variance_a, variance_b, variance_c}};

            SUBCASE("Constructor") {
                CHECK(independent.value(0) == value_a);
                CHECK(independent.value(1) == value_b);
                CHECK(independent.value(2) == value_c);
                CHECK(independent.variance(0) == variance_a);
                CHECK(independent.variance(1) == variance_b);
                CHECK(independent.variance(2) == variance_c);
            }
            SUBCASE("Conversion to IndependentRealRandVar<symmetric_t>") {
                auto const symmetric = static_cast<IndependentRealRandVar<symmetric_t>>(independent);

                CHECK(symmetric.value == doctest::Approx(mean_val(independent.value)));
                CHECK(symmetric.variance == doctest::Approx(mean_val(independent.variance) / 3.0));
            }
            SUBCASE("Conversion to UniformRealRandVar<symmetric_t>") {
                auto const uniform = static_cast<UniformRealRandVar<symmetric_t>>(independent);
                auto const via_asym_uniform = static_cast<UniformRealRandVar<symmetric_t>>(
                    static_cast<UniformRealRandVar<asymmetric_t>>(independent));
                auto const via_sym_independent = static_cast<UniformRealRandVar<symmetric_t>>(
                    static_cast<IndependentRealRandVar<symmetric_t>>(independent));

                CHECK(uniform.value == doctest::Approx(via_asym_uniform.value));
                CHECK(uniform.variance == doctest::Approx(via_asym_uniform.variance));
                CHECK(uniform.value == doctest::Approx(via_sym_independent.value));
                CHECK(uniform.variance == doctest::Approx(via_sym_independent.variance));
            }
        }
    }

    SUBCASE("UniformComplexRandVar<symmetric_t>") {
        for (auto const& [real_value, imag_value, variance] :
             std::array{std::tuple{1.0, 0.0, 1.0}, std::tuple{2.0, 0.0, 3.0}, std::tuple{0.0, 1.0, 1.0},
                        std::tuple{0.0, 2.0, 1.0}, std::tuple{1.0, 1.0, 1.0}, std::tuple{2.0, 2.0, 3.0}}) {
            CAPTURE(real_value);
            CAPTURE(imag_value);
            CAPTURE(variance);

            UniformComplexRandVar<symmetric_t> const uniform{.value = {real_value, imag_value}, .variance = variance};

            SUBCASE("Constructor") {
                CHECK(real(uniform.value) == real_value);
                CHECK(imag(uniform.value) == imag_value);
                CHECK(uniform.variance == variance);
            }
            SUBCASE("To three-phase") {
                auto const asymmetric = three_phase(uniform);

                CHECK(real(asymmetric.value(0)) == doctest::Approx(real(uniform.value)));
                CHECK(imag(asymmetric.value(0)) == doctest::Approx(imag(uniform.value)));
                CHECK(real(asymmetric.value(1)) == doctest::Approx(real(uniform.value * a2)));
                CHECK(imag(asymmetric.value(1)) == doctest::Approx(imag(uniform.value * a2)));
                CHECK(real(asymmetric.value(2)) == doctest::Approx(real(uniform.value * a)));
                CHECK(imag(asymmetric.value(2)) == doctest::Approx(imag(uniform.value * a)));
                CHECK(asymmetric.variance == variance);
            }
        }
    }

    SUBCASE("UniformComplexRandVar<asymmetric_t>") {
        for (auto const& [real_value, imag_value, variance] :
             std::array{std::tuple{AsymRealValue{1.0, 2.0, 3.0}, AsymRealValue{0.0, 0.0, 0.0}, 1.0},
                        std::tuple{AsymRealValue{2.0, 0.0, 0.0}, AsymRealValue{0.0, 3.0, 3.0}, 3.0},
                        std::tuple{AsymRealValue{0.0, 0.0, 0.0}, AsymRealValue{1.0, 1.0, 1.0}, 1.0},
                        std::tuple{AsymRealValue{0.0, -1.0, -2.0}, AsymRealValue{2.0, -1.0, -2.0}, 1.0},
                        std::tuple{AsymRealValue{1.0, 1.0, 1.0}, AsymRealValue{1.0, 1.0, 1.0}, 1.0},
                        std::tuple{AsymRealValue{2.0, 2.0, 2.0}, AsymRealValue{2.0, 2.0, 2.0}, 3.0}}) {
            CAPTURE(real_value);
            CAPTURE(imag_value);
            CAPTURE(variance);

            UniformComplexRandVar<asymmetric_t> const uniform{.value = {real_value, imag_value}, .variance = variance};

            SUBCASE("Constructor") {
                CHECK(real(uniform.value(0)) == real_value(0));
                CHECK(imag(uniform.value(0)) == imag_value(0));
                CHECK(real(uniform.value(1)) == real_value(1));
                CHECK(imag(uniform.value(1)) == imag_value(1));
                CHECK(real(uniform.value(2)) == real_value(2));
                CHECK(imag(uniform.value(2)) == imag_value(2));
                CHECK(uniform.variance == variance);
            }
            SUBCASE("Positive sequence") {
                auto const positive_sequence = pos_seq(uniform);

                CHECK(real(positive_sequence.value) == doctest::Approx(real(pos_seq(uniform.value))));
                CHECK(imag(positive_sequence.value) == doctest::Approx(imag(pos_seq(uniform.value))));
                CHECK(positive_sequence.variance == doctest::Approx(variance / 3.0));
            }
        }
    }
    SUBCASE("IndependentComplexRandVar<symmetric_t>") {
        for (auto const& [real_value, imag_value, variance] :
             std::array{std::tuple{1.0, 0.0, 1.0}, std::tuple{2.0, 0.0, 3.0}, std::tuple{0.0, 1.0, 1.0},
                        std::tuple{0.0, 2.0, 1.0}, std::tuple{1.0, 1.0, 1.0}, std::tuple{2.0, 2.0, 3.0}}) {
            CAPTURE(real_value);
            CAPTURE(imag_value);
            CAPTURE(variance);

            IndependentComplexRandVar<symmetric_t> const independent{.value = {real_value, imag_value},
                                                                     .variance = variance};

            SUBCASE("Constructor") {
                CHECK(real(independent.value) == real_value);
                CHECK(imag(independent.value) == imag_value);
                CHECK(independent.variance == variance);
            }
            SUBCASE("Conversion to UniformComplexRandVar<symmetric_t>") {
                auto const uniform = static_cast<UniformComplexRandVar<symmetric_t>>(independent);

                CHECK(real(uniform.value) == doctest::Approx(real(independent.value)));
                CHECK(imag(uniform.value) == doctest::Approx(imag(independent.value)));
                CHECK(uniform.variance == doctest::Approx(variance));
            }
        }
    }
    SUBCASE("DecomposedComplexRandVar<symmetric_t>") {
        for (auto const& [real_value, real_variance, imag_value, imag_variance] : std::array{
                 std::tuple{1.0, 1.0, 0.0, 0.2}, std::tuple{2.0, 3.0, 0.0, 0.2}, std::tuple{0.0, 1.0, 1.0, 0.2},
                 std::tuple{0.0, 1.0, 2.0, 0.2}, std::tuple{1.0, 1.0, 1.0, 0.2}, std::tuple{2.0, 1.0, 2.0, 0.2}}) {
            CAPTURE(real_value);
            CAPTURE(real_variance);
            CAPTURE(imag_value);
            CAPTURE(imag_variance);

            DecomposedComplexRandVar<symmetric_t> const decomposed{
                .real_component = {.value = real_value, .variance = real_variance},
                .imag_component = {.value = imag_value, .variance = imag_variance}};

            SUBCASE("Constructor") {
                CHECK(decomposed.real_component.value == real_value);
                CHECK(decomposed.real_component.variance == real_variance);
                CHECK(decomposed.imag_component.value == imag_value);
                CHECK(decomposed.imag_component.variance == imag_variance);
            }
            SUBCASE("Aggregate value") {
                CHECK(real(decomposed.value()) == doctest::Approx(real_value));
                CHECK(imag(decomposed.value()) == doctest::Approx(imag_value));
            }
            SUBCASE("Conversion to UniformComplexRandVar<symmetric_t>") {
                auto const uniform = static_cast<UniformComplexRandVar<symmetric_t>>(decomposed);

                CHECK(real(uniform.value) == doctest::Approx(real(decomposed.value())));
                CHECK(imag(uniform.value) == doctest::Approx(imag(decomposed.value())));
                CHECK(uniform.variance == doctest::Approx(real_variance + imag_variance));
            }
            SUBCASE("Conversion to IndependentComplexRandVar<symmetric_t>") {
                auto const independent = static_cast<IndependentComplexRandVar<symmetric_t>>(decomposed);

                CHECK(real(independent.value) == doctest::Approx(real(decomposed.value())));
                CHECK(imag(independent.value) == doctest::Approx(imag(decomposed.value())));
                CHECK(independent.variance == doctest::Approx(real_variance + imag_variance));
            }
        }
    }

    SUBCASE("DecomposedComplexRandVar<asymmetric_t>") {
        for (auto const& [real_value_a, real_value_b, real_value_c, real_variance_a, real_variance_b, real_variance_c,
                          imag_value_a, imag_value_b, imag_value_c, imag_variance_a, imag_variance_b, imag_variance_c] :
             std::array{std::tuple{1.0, 2.0, 3.0, 0.2, 0.3, 0.4, 0.0, 0.0, 0.0, 0.2, 0.3, 0.4},
                        std::tuple{2.0, 3.0, 4.0, 0.3, 0.4, 0.5, 0.0, 1.0, 1.0, 0.3, 0.4, 0.5}}) {
            CAPTURE(real_value_a);
            CAPTURE(real_value_b);
            CAPTURE(real_value_c);
            CAPTURE(real_variance_a);
            CAPTURE(real_variance_b);
            CAPTURE(real_variance_c);
            CAPTURE(imag_value_a);
            CAPTURE(imag_value_b);
            CAPTURE(imag_value_c);
            CAPTURE(imag_variance_a);
            CAPTURE(imag_variance_b);
            CAPTURE(imag_variance_c);

            DecomposedComplexRandVar<asymmetric_t> const decomposed{
                .real_component = {.value = {real_value_a, real_value_b, real_value_c},
                                   .variance = {real_variance_a, real_variance_b, real_variance_c}},
                .imag_component = {.value = {imag_value_a, imag_value_b, imag_value_c},
                                   .variance = {imag_variance_a, imag_variance_b, imag_variance_c}}};

            SUBCASE("Constructor") {
                CHECK(decomposed.real_component.value(0) == real_value_a);
                CHECK(decomposed.real_component.value(1) == real_value_b);
                CHECK(decomposed.real_component.value(2) == real_value_c);
                CHECK(decomposed.real_component.variance(0) == real_variance_a);
                CHECK(decomposed.real_component.variance(1) == real_variance_b);
                CHECK(decomposed.real_component.variance(2) == real_variance_c);
                CHECK(decomposed.imag_component.value(0) == imag_value_a);
                CHECK(decomposed.imag_component.value(1) == imag_value_b);
                CHECK(decomposed.imag_component.value(2) == imag_value_c);
                CHECK(decomposed.imag_component.variance(0) == imag_variance_a);
                CHECK(decomposed.imag_component.variance(1) == imag_variance_b);
                CHECK(decomposed.imag_component.variance(2) == imag_variance_c);
            }
            SUBCASE("Aggregate value") {
                CHECK(real(decomposed.value()(0)) == doctest::Approx(real_value_a));
                CHECK(imag(decomposed.value()(0)) == doctest::Approx(imag_value_a));
                CHECK(real(decomposed.value()(1)) == doctest::Approx(real_value_b));
                CHECK(imag(decomposed.value()(1)) == doctest::Approx(imag_value_b));
                CHECK(real(decomposed.value()(2)) == doctest::Approx(real_value_c));
                CHECK(imag(decomposed.value()(2)) == doctest::Approx(imag_value_c));
            }
            SUBCASE("Conversion to UniformComplexRandVar<asymmetric_t>") {
                auto const uniform = static_cast<UniformComplexRandVar<asymmetric_t>>(decomposed);

                CHECK(real(uniform.value(0)) == doctest::Approx(real(decomposed.value()(0))));
                CHECK(imag(uniform.value(0)) == doctest::Approx(imag(decomposed.value()(0))));
                CHECK(real(uniform.value(1)) == doctest::Approx(real(decomposed.value()(1))));
                CHECK(imag(uniform.value(1)) == doctest::Approx(imag(decomposed.value()(1))));
                CHECK(real(uniform.value(2)) == doctest::Approx(real(decomposed.value()(2))));
                CHECK(imag(uniform.value(2)) == doctest::Approx(imag(decomposed.value()(2))));
                CHECK(uniform.variance == doctest::Approx(real_variance_a + real_variance_b + real_variance_c +
                                                          imag_variance_a + imag_variance_b + imag_variance_c));
            }
            SUBCASE("Conversion to IndependentComplexRandVar<asymmetric_t>") {
                auto const independent = static_cast<IndependentComplexRandVar<asymmetric_t>>(decomposed);

                CHECK(real(independent.value(0)) == doctest::Approx(real(decomposed.value()(0))));
                CHECK(imag(independent.value(0)) == doctest::Approx(imag(decomposed.value()(0))));
                CHECK(real(independent.value(1)) == doctest::Approx(real(decomposed.value()(1))));
                CHECK(imag(independent.value(1)) == doctest::Approx(imag(decomposed.value()(1))));
                CHECK(real(independent.value(2)) == doctest::Approx(real(decomposed.value()(2))));
                CHECK(imag(independent.value(2)) == doctest::Approx(imag(decomposed.value()(2))));
                CHECK(independent.variance(0) == doctest::Approx(real_variance_a + imag_variance_a));
                CHECK(independent.variance(1) == doctest::Approx(real_variance_b + imag_variance_b));
                CHECK(independent.variance(2) == doctest::Approx(real_variance_c + imag_variance_c));
            }
        }
    }

    SUBCASE("PolarComplexRandVar<symmetric_t>") {
        SUBCASE("Constructor") {
            for (auto const& [magnitude, magnitude_variance, angle, angle_variance] :
                 std::array{std::tuple{1.0, 1.0, 0.0, 0.2}, std::tuple{2.0, 3.0, 0.0, 0.2},
                            std::tuple{1.0, 1.0, deg_90, 0.2}, std::tuple{1.0, 1.0, deg_45, 0.2}}) {
                CAPTURE(magnitude);
                CAPTURE(magnitude_variance);
                CAPTURE(angle);
                CAPTURE(angle_variance);

                PolarComplexRandVar<symmetric_t> const polar{
                    .magnitude = {.value = magnitude, .variance = magnitude_variance},
                    .angle = {.value = angle, .variance = angle_variance}};

                CHECK(polar.magnitude.value == magnitude);
                CHECK(polar.magnitude.variance == magnitude_variance);
                CHECK(polar.angle.value == angle);
                CHECK(polar.angle.variance == angle_variance);
            }
        }
        SUBCASE("Aggregate value") {
            for (auto const& [magnitude, magnitude_variance, angle_variance] :
                 std::array{std::tuple{1.0, 1.0, 0.2}, std::tuple{2.0, 1.0, 0.2}, std::tuple{1.0, 3.0, 0.2},
                            std::tuple{1.0, 2.0, 0.4}}) {
                CAPTURE(magnitude);
                CAPTURE(magnitude_variance);
                CAPTURE(angle_variance);

                SUBCASE("No phase shift") {
                    PolarComplexRandVar<symmetric_t> const polar{
                        .magnitude = {.value = magnitude, .variance = magnitude_variance},
                        .angle = {.value = 0.0, .variance = angle_variance}};

                    CHECK(real(polar.value()) == doctest::Approx(polar.magnitude.value));
                    CHECK(imag(polar.value()) == doctest::Approx(0.0));
                }
                SUBCASE("90deg phase shift") {
                    PolarComplexRandVar<symmetric_t> const polar{
                        .magnitude = {.value = magnitude, .variance = magnitude_variance},
                        .angle = {.value = deg_90, .variance = angle_variance}};

                    CHECK(real(polar.value()) == doctest::Approx(0.0));
                    CHECK(imag(polar.value()) == doctest::Approx(polar.magnitude.value));
                }
                SUBCASE("45deg phase shift") {
                    PolarComplexRandVar<symmetric_t> const polar{
                        .magnitude = {.value = magnitude, .variance = magnitude_variance},
                        .angle = {.value = deg_45, .variance = angle_variance}};

                    CHECK(real(polar.value()) == doctest::Approx(polar.magnitude.value * inv_sqrt2));
                    CHECK(imag(polar.value()) == doctest::Approx(real(polar.value())));
                }
            }
        }

        SUBCASE("Conversion to DecomposedComplexRandVar<symmetric_t>") {
            for (auto const& [magnitude, magnitude_variance, angle_variance] :
                 std::array{std::tuple{1.0, 1.0, 0.2}, std::tuple{2.0, 1.0, 0.2}, std::tuple{1.0, 3.0, 0.2},
                            std::tuple{1.0, 2.0, 0.4}}) {
                CAPTURE(magnitude);
                CAPTURE(magnitude_variance);
                CAPTURE(angle_variance);

                SUBCASE("No phase shift") {
                    PolarComplexRandVar<symmetric_t> const polar{
                        .magnitude = {.value = magnitude, .variance = magnitude_variance},
                        .angle = {.value = 0.0, .variance = angle_variance}};

                    auto const decomposed = static_cast<DecomposedComplexRandVar<symmetric_t>>(polar);

                    CHECK(decomposed.real_component.value == doctest::Approx(polar.magnitude.value));
                    CHECK(decomposed.imag_component.value == doctest::Approx(0.0));
                    CHECK(decomposed.real_component.variance == doctest::Approx(polar.magnitude.variance));
                    CHECK(decomposed.imag_component.variance ==
                          doctest::Approx(magnitude * magnitude * polar.angle.variance));
                    CHECK(real(decomposed.value()) == doctest::Approx(real(polar.value())));
                    CHECK(imag(decomposed.value()) == doctest::Approx(imag(polar.value())));
                }

                SUBCASE("90deg phase shift") {
                    PolarComplexRandVar<symmetric_t> const polar{
                        .magnitude = {.value = magnitude, .variance = magnitude_variance},
                        .angle = {.value = deg_90, .variance = angle_variance}};

                    auto const decomposed = static_cast<DecomposedComplexRandVar<symmetric_t>>(polar);

                    CHECK(decomposed.real_component.value == doctest::Approx(0.0));
                    CHECK(decomposed.imag_component.value == doctest::Approx(polar.magnitude.value));
                    CHECK(decomposed.real_component.variance ==
                          doctest::Approx(magnitude * magnitude * polar.angle.variance));
                    CHECK(decomposed.imag_component.variance == doctest::Approx(polar.magnitude.variance));
                    CHECK(real(decomposed.value()) == doctest::Approx(real(polar.value())));
                    CHECK(imag(decomposed.value()) == doctest::Approx(imag(polar.value())));
                }

                SUBCASE("45deg phase shift") {
                    PolarComplexRandVar<symmetric_t> const polar{
                        .magnitude = {.value = magnitude, .variance = magnitude_variance},
                        .angle = {.value = deg_45, .variance = angle_variance}};

                    auto const decomposed = static_cast<DecomposedComplexRandVar<symmetric_t>>(polar);
                    auto const uniform = static_cast<UniformComplexRandVar<symmetric_t>>(polar);

                    CHECK(decomposed.real_component.value == doctest::Approx(real(uniform.value)));
                    CHECK(decomposed.imag_component.value == doctest::Approx(imag(uniform.value)));
                    CHECK(decomposed.real_component.variance == doctest::Approx(uniform.variance / 2));
                    CHECK(decomposed.imag_component.variance == doctest::Approx(decomposed.real_component.variance));
                    CHECK(real(decomposed.value()) == doctest::Approx(real(polar.value())));
                    CHECK(imag(decomposed.value()) == doctest::Approx(imag(polar.value())));
                }
            }
        }
        SUBCASE("Conversion to DecomposedComplexRandVar<asymmetric_t>") {
            for (auto const& [magnitude, magnitude_variance, angle_variance] :
                 std::array{std::tuple{1.0, 1.0, 0.2}, std::tuple{2.0, 1.0, 0.2}, std::tuple{1.0, 3.0, 0.2},
                            std::tuple{1.0, 2.0, 0.4}}) {
                CAPTURE(magnitude);
                CAPTURE(magnitude_variance);
                CAPTURE(angle_variance);

                for (auto const& shift : {0.0, deg_90, deg_45}) {
                    CAPTURE(shift);

                    PolarComplexRandVar<symmetric_t> const polar{
                        .magnitude = {.value = magnitude, .variance = magnitude_variance},
                        .angle = {.value = shift, .variance = angle_variance}};

                    auto const decomposed = static_cast<DecomposedComplexRandVar<asymmetric_t>>(polar);

                    ComplexValue<asymmetric_t> const three_phase_value{polar.value()};
                    CHECK(decomposed.real_component.value(0) == doctest::Approx(real(three_phase_value(0))));
                    CHECK(decomposed.imag_component.value(0) == doctest::Approx(imag(three_phase_value(0))));
                    CHECK(decomposed.real_component.value(1) == doctest::Approx(real(three_phase_value(1))));
                    CHECK(decomposed.imag_component.value(1) == doctest::Approx(imag(three_phase_value(1))));
                    CHECK(decomposed.real_component.value(2) == doctest::Approx(real(three_phase_value(2))));
                    CHECK(decomposed.imag_component.value(2) == doctest::Approx(imag(three_phase_value(2))));

                    CHECK(real(decomposed.value()(0)) == doctest::Approx(real(three_phase_value(0))));
                    CHECK(imag(decomposed.value()(0)) == doctest::Approx(imag(three_phase_value(0))));
                    CHECK(real(decomposed.value()(1)) == doctest::Approx(real(three_phase_value(1))));
                    CHECK(imag(decomposed.value()(1)) == doctest::Approx(imag(three_phase_value(1))));
                    CHECK(real(decomposed.value()(2)) == doctest::Approx(real(three_phase_value(2))));
                    CHECK(imag(decomposed.value()(2)) == doctest::Approx(imag(three_phase_value(2))));

                    // One value of variance to 3 phase
                    auto const real_variance_a = magnitude_variance * cos(shift) * cos(shift) +
                                                 magnitude * magnitude * sin(shift) * sin(shift) * angle_variance;
                    CHECK(decomposed.real_component.variance(0) == doctest::Approx(real_variance_a));
                    auto const real_variance_b =
                        magnitude_variance * cos(shift - deg_120) * cos(shift - deg_120) +
                        magnitude * magnitude * sin(shift - deg_120) * sin(shift - deg_120) * angle_variance;
                    CHECK(decomposed.real_component.variance(1) == doctest::Approx(real_variance_b));
                    auto const real_variance_c =
                        magnitude_variance * cos(shift - deg_240) * cos(shift - deg_240) +
                        magnitude * magnitude * sin(shift - deg_240) * sin(shift - deg_240) * angle_variance;
                    CHECK(decomposed.real_component.variance(2) == doctest::Approx(real_variance_c));

                    auto const imag_variance_a = magnitude_variance * sin(shift) * sin(shift) +
                                                 magnitude * magnitude * cos(shift) * cos(shift) * angle_variance;
                    CHECK(decomposed.imag_component.variance(0) == doctest::Approx(imag_variance_a));
                    auto const imag_variance_b =
                        magnitude_variance * sin(shift - deg_120) * sin(shift - deg_120) +
                        magnitude * magnitude * cos(shift - deg_120) * cos(shift - deg_120) * angle_variance;
                    CHECK(decomposed.imag_component.variance(1) == doctest::Approx(imag_variance_b));
                    auto const imag_variance_c =
                        magnitude_variance * sin(shift - deg_240) * sin(shift - deg_240) +
                        magnitude * magnitude * cos(shift - deg_240) * cos(shift - deg_240) * angle_variance;
                    CHECK(decomposed.imag_component.variance(2) == doctest::Approx(imag_variance_c));
                }
            }
        }

        SUBCASE("Conversion to IndependentComplexRandVar<symmetric_t>") {
            for (auto const& [magnitude, magnitude_variance, angle, angle_variance] :
                 std::array{std::tuple{1.0, 1.0, 0.0, 0.2}, std::tuple{2.0, 3.0, 0.0, 0.2},
                            std::tuple{1.0, 1.0, deg_90, 0.2}, std::tuple{1.0, 1.0, deg_45, 0.2}}) {
                CAPTURE(magnitude);
                CAPTURE(magnitude_variance);
                CAPTURE(angle);
                CAPTURE(angle_variance);

                PolarComplexRandVar<symmetric_t> const polar{
                    .magnitude = {.value = magnitude, .variance = magnitude_variance},
                    .angle = {.value = angle, .variance = angle_variance}};

                auto const independent = static_cast<IndependentComplexRandVar<symmetric_t>>(polar);

                CHECK(real(independent.value) == doctest::Approx(real(polar.value())));
                CHECK(imag(independent.value) == doctest::Approx(imag(polar.value())));
                CHECK(independent.variance ==
                      doctest::Approx(polar.magnitude.variance + magnitude * magnitude * polar.angle.variance));
            }
        }

        SUBCASE("Conversion to UniformComplexRandVar<symmetric_t>") {
            for (auto const& [magnitude, magnitude_variance, angle, angle_variance] :
                 std::array{std::tuple{1.0, 1.0, 0.0, 0.2}, std::tuple{2.0, 3.0, 0.0, 0.2},
                            std::tuple{1.0, 1.0, deg_90, 0.2}, std::tuple{1.0, 1.0, deg_45, 0.2}}) {
                CAPTURE(magnitude);
                CAPTURE(magnitude_variance);
                CAPTURE(angle);
                CAPTURE(angle_variance);

                PolarComplexRandVar<symmetric_t> const polar{
                    .magnitude = {.value = magnitude, .variance = magnitude_variance},
                    .angle = {.value = angle, .variance = angle_variance}};

                auto const uniform = static_cast<UniformComplexRandVar<symmetric_t>>(polar);

                CHECK(real(uniform.value) == doctest::Approx(real(polar.value())));
                CHECK(imag(uniform.value) == doctest::Approx(imag(polar.value())));
                CHECK(uniform.variance ==
                      doctest::Approx(polar.magnitude.variance + magnitude * magnitude * polar.angle.variance));
            }
        }
    }

    SUBCASE("PolarComplexRandVar<asymmetric_t>") {
        SUBCASE("Constructor") {
            for (auto const& [magnitude_a, magnitude_b, magnitude_c, magnitude_variance, angle_a, angle_b, angle_c,
                              angle_variance] : std::array{std::tuple{1.0, 2.0, 3.0, 0.2, 0.0, deg_45, deg_90, 0.2},
                                                           std::tuple{2.0, 3.0, 4.0, 0.3, 0.0, deg_30, deg_60, 0.3}}) {
                CAPTURE(magnitude_a);
                CAPTURE(magnitude_b);
                CAPTURE(magnitude_c);
                CAPTURE(magnitude_variance);
                CAPTURE(angle_a);
                CAPTURE(angle_b);
                CAPTURE(angle_c);
                CAPTURE(angle_variance);

                PolarComplexRandVar<asymmetric_t> const polar{
                    .magnitude = {.value = {magnitude_a, magnitude_b, magnitude_c}, .variance = magnitude_variance},
                    .angle = {.value = {angle_a, angle_b, angle_c}, .variance = angle_variance}};

                CHECK(polar.magnitude.value(0) == magnitude_a);
                CHECK(polar.magnitude.value(1) == magnitude_b);
                CHECK(polar.magnitude.value(2) == magnitude_c);
                CHECK(polar.magnitude.variance == magnitude_variance);
                CHECK(polar.angle.value(0) == angle_a);
                CHECK(polar.angle.value(1) == angle_b);
                CHECK(polar.angle.value(2) == angle_c);
                CHECK(polar.angle.variance == angle_variance);
            }
        }
        SUBCASE("Aggregate value") {
            for (auto const& [magnitude_a, magnitude_b, magnitude_c, magnitude_variance, angle_variance] :
                 std::array{std::tuple{1.0, 1.0, 1.0, 1.0, 0.2}, std::tuple{2.0, 2.0, 2.0, 1.0, 0.2},
                            std::tuple{1.0, 1.0, 1.0, 3.0, 0.2}, std::tuple{1.0, 1.0, 1.0, 2.0, 0.4}}) {
                CAPTURE(magnitude_a);
                CAPTURE(magnitude_b);
                CAPTURE(magnitude_c);
                CAPTURE(magnitude_variance);
                CAPTURE(angle_variance);

                SUBCASE("No phase shift") {
                    PolarComplexRandVar<asymmetric_t> const polar{
                        .magnitude = {.value = {magnitude_a, magnitude_b, magnitude_c}, .variance = magnitude_variance},
                        .angle = {.value = {0.0, deg_240, deg_120}, .variance = angle_variance}};

                    CHECK(real(polar.value()(0)) == doctest::Approx(polar.magnitude.value(0)));
                    CHECK(imag(polar.value()(0)) == doctest::Approx(0.0));
                    CHECK(real(polar.value()(1)) == doctest::Approx(polar.magnitude.value(1) * -0.5));
                    CHECK(imag(polar.value()(1)) == doctest::Approx(polar.magnitude.value(1) * -sqrt3_2));
                    CHECK(real(polar.value()(2)) == doctest::Approx(polar.magnitude.value(2) * -0.5));
                    CHECK(imag(polar.value()(2)) == doctest::Approx(polar.magnitude.value(2) * sqrt3_2));
                }
                SUBCASE("90deg phase shift") {
                    PolarComplexRandVar<asymmetric_t> const polar{
                        .magnitude = {.value = {magnitude_a, magnitude_b, magnitude_c}, .variance = magnitude_variance},
                        .angle = {.value = {deg_90, deg_240 + deg_90, deg_120 + deg_90}, .variance = angle_variance}};

                    CHECK(real(polar.value()(0)) == doctest::Approx(0.0));
                    CHECK(imag(polar.value()(0)) == doctest::Approx(polar.magnitude.value(0)));
                    CHECK(real(polar.value()(1)) == doctest::Approx(polar.magnitude.value(1) * sqrt3_2));
                    CHECK(imag(polar.value()(1)) == doctest::Approx(polar.magnitude.value(1) * -0.5));
                    CHECK(real(polar.value()(2)) == doctest::Approx(polar.magnitude.value(2) * -sqrt3_2));
                    CHECK(imag(polar.value()(2)) == doctest::Approx(polar.magnitude.value(2) * -0.5));
                }
                SUBCASE("45deg phase shift") {
                    PolarComplexRandVar<asymmetric_t> const polar{
                        .magnitude = {.value = {magnitude_a, magnitude_b, magnitude_c}, .variance = magnitude_variance},
                        .angle = {.value = {deg_45, deg_240 + deg_45, deg_120 + deg_45}, .variance = angle_variance}};

                    CHECK(real(polar.value()(0)) == doctest::Approx(polar.magnitude.value(0) * inv_sqrt2));
                    CHECK(imag(polar.value()(0)) == doctest::Approx(polar.magnitude.value(0) * inv_sqrt2));
                    CHECK(real(polar.value()(1)) == doctest::Approx(polar.magnitude.value(1) * 0.2588190451));
                    CHECK(imag(polar.value()(1)) == doctest::Approx(polar.magnitude.value(1) * -0.9659258263));
                    CHECK(real(polar.value()(2)) == doctest::Approx(polar.magnitude.value(2) * -0.9659258263));
                    CHECK(imag(polar.value()(2)) == doctest::Approx(polar.magnitude.value(2) * 0.2588190451));
                }
            }
        }

        SUBCASE("Conversion to DecomposedComplexRandVar<asymmetric_t>") {
            for (auto const& [magnitude_a, magnitude_b, magnitude_c, magnitude_variance, angle_variance] :
                 std::array{std::tuple{1.0, 2.0, 3.0, 0.2, 0.2}, std::tuple{2.0, 3.0, 4.0, 0.3, 0.3}}) {
                CAPTURE(magnitude_a);
                CAPTURE(magnitude_b);
                CAPTURE(magnitude_c);
                CAPTURE(magnitude_variance);
                CAPTURE(angle_variance);

                for (auto const& shift : {0.0, deg_90, deg_45}) {
                    CAPTURE(shift);

                    PolarComplexRandVar<asymmetric_t> const polar{
                        .magnitude = {.value = {magnitude_a, magnitude_b, magnitude_c}, .variance = magnitude_variance},
                        .angle = {.value = {shift, deg_240 + shift, deg_120 + shift}, .variance = angle_variance}};

                    auto const decomposed = static_cast<DecomposedComplexRandVar<asymmetric_t>>(polar);

                    CHECK(decomposed.real_component.value(0) == doctest::Approx(polar.magnitude.value(0) * cos(shift)));
                    CHECK(decomposed.imag_component.value(0) == doctest::Approx(polar.magnitude.value(0) * sin(shift)));
                    CHECK(decomposed.real_component.value(1) ==
                          doctest::Approx(polar.magnitude.value(1) * cos(shift + deg_240)));
                    CHECK(decomposed.imag_component.value(1) ==
                          doctest::Approx(polar.magnitude.value(1) * sin(shift + deg_240)));
                    CHECK(decomposed.real_component.value(2) ==
                          doctest::Approx(polar.magnitude.value(2) * cos(shift + deg_120)));
                    CHECK(decomposed.imag_component.value(2) ==
                          doctest::Approx(polar.magnitude.value(2) * sin(shift + deg_120)));

                    CHECK(real(decomposed.value()(0)) == doctest::Approx(real(polar.value()(0))));
                    CHECK(imag(decomposed.value()(0)) == doctest::Approx(imag(polar.value()(0))));
                    CHECK(real(decomposed.value()(1)) == doctest::Approx(real(polar.value()(1))));
                    CHECK(imag(decomposed.value()(1)) == doctest::Approx(imag(polar.value()(1))));
                    CHECK(real(decomposed.value()(2)) == doctest::Approx(real(polar.value()(2))));
                    CHECK(imag(decomposed.value()(2)) == doctest::Approx(imag(polar.value()(2))));

                    // One value of variance to 3 phase
                    auto const real_variance_a = magnitude_variance * cos(shift) * cos(shift) +
                                                 magnitude_a * magnitude_a * sin(shift) * sin(shift) * angle_variance;
                    CHECK(decomposed.real_component.variance(0) == doctest::Approx(real_variance_a));
                    auto const real_variance_b =
                        magnitude_variance * cos(deg_240 + shift) * cos(deg_240 + shift) +
                        magnitude_b * magnitude_b * sin(deg_240 + shift) * sin(deg_240 + shift) * angle_variance;
                    CHECK(decomposed.real_component.variance(1) == doctest::Approx(real_variance_b));
                    auto const real_variance_c =
                        magnitude_variance * cos(deg_120 + shift) * cos(deg_120 + shift) +
                        magnitude_c * magnitude_c * sin(deg_120 + shift) * sin(deg_120 + shift) * angle_variance;
                    CHECK(decomposed.real_component.variance(2) == doctest::Approx(real_variance_c));

                    auto const imag_variance_a = magnitude_variance * sin(shift) * sin(shift) +
                                                 magnitude_a * magnitude_a * cos(shift) * cos(shift) * angle_variance;
                    CHECK(decomposed.imag_component.variance(0) == doctest::Approx(imag_variance_a));
                    auto const imag_variance_b =
                        magnitude_variance * sin(deg_240 + shift) * sin(deg_240 + shift) +
                        magnitude_b * magnitude_b * cos(deg_240 + shift) * cos(deg_240 + shift) * angle_variance;
                    CHECK(decomposed.imag_component.variance(1) == doctest::Approx(imag_variance_b));
                    auto const imag_variance_c =
                        magnitude_variance * sin(deg_120 + shift) * sin(deg_120 + shift) +
                        magnitude_c * magnitude_c * cos(deg_120 + shift) * cos(deg_120 + shift) * angle_variance;
                    CHECK(decomposed.imag_component.variance(2) == doctest::Approx(imag_variance_c));
                }
            }
        }

        // TODO: Add Conversion to DecomposedComplexRandVar<symmetric_t>
        SUBCASE("Conversion to DecomposedComplexRandVar<symmetric_t>") {
            for (auto const& [magnitude_a, magnitude_b, magnitude_c, magnitude_variance, angle_variance] :
                 std::array{std::tuple{1.0, 2.0, 3.0, 0.2, 0.2}, std::tuple{2.0, 3.0, 4.0, 0.3, 0.3}}) {
                CAPTURE(magnitude_a);
                CAPTURE(magnitude_b);
                CAPTURE(magnitude_c);
                CAPTURE(magnitude_variance);
                CAPTURE(angle_variance);

                for (auto const& shift : {0.0, deg_90, deg_45}) {
                    CAPTURE(shift);

                    PolarComplexRandVar<asymmetric_t> const polar{
                        .magnitude = {.value = {magnitude_a, magnitude_b, magnitude_c}, .variance = magnitude_variance},
                        .angle = {.value = {shift, deg_240 + shift, deg_120 + shift}, .variance = angle_variance}};

                    auto const decomposed = static_cast<DecomposedComplexRandVar<symmetric_t>>(polar);

                    auto const pos_seq_value = pos_seq(polar.value());

                    CHECK(decomposed.real_component.value == doctest::Approx(real(pos_seq_value)));
                    CHECK(decomposed.imag_component.value == doctest::Approx(imag(pos_seq_value)));

                    CHECK(real(decomposed.value()) == doctest::Approx(real(pos_seq_value)));
                    CHECK(imag(decomposed.value()) == doctest::Approx(imag(pos_seq_value)));

                    // One value of variance to 3 phase
                    auto const real_variance = (magnitude_variance * cos(shift) * cos(shift) +
                                                magnitude_a * magnitude_a * sin(shift) * sin(shift) * angle_variance +
                                                magnitude_variance * cos(shift) * cos(shift) +
                                                magnitude_b * magnitude_b * sin(shift) * sin(shift) * angle_variance +
                                                magnitude_variance * cos(shift) * cos(shift) +
                                                magnitude_c * magnitude_c * sin(shift) * sin(shift) * angle_variance) /
                                               9.0;
                    CHECK(decomposed.real_component.variance == doctest::Approx(real_variance));

                    auto const imag_variance = (magnitude_variance * sin(shift) * sin(shift) +
                                                magnitude_a * magnitude_a * cos(shift) * cos(shift) * angle_variance +
                                                magnitude_variance * sin(shift) * sin(shift) +
                                                magnitude_b * magnitude_b * cos(shift) * cos(shift) * angle_variance +
                                                magnitude_variance * sin(shift) * sin(shift) +
                                                magnitude_c * magnitude_c * cos(shift) * cos(shift) * angle_variance) /
                                               9.0;
                    CHECK(decomposed.imag_component.variance == doctest::Approx(imag_variance));
                }
            }
        }

        SUBCASE("Conversion to IndependentComplexRandVar<asymmetric_t>") {
            for (auto const& [magnitude_a, magnitude_b, magnitude_c, magnitude_variance, angle_a, angle_b, angle_c,
                              angle_variance] : std::array{std::tuple{1.0, 2.0, 3.0, 0.2, 0.0, deg_45, deg_90, 0.2},
                                                           std::tuple{2.0, 3.0, 4.0, 0.3, 0.0, deg_30, deg_60, 0.3}}) {
                CAPTURE(magnitude_a);
                CAPTURE(magnitude_b);
                CAPTURE(magnitude_c);
                CAPTURE(magnitude_variance);
                CAPTURE(angle_a);
                CAPTURE(angle_b);
                CAPTURE(angle_c);
                CAPTURE(angle_variance);

                PolarComplexRandVar<asymmetric_t> const polar{
                    .magnitude = {.value = {magnitude_a, magnitude_b, magnitude_c}, .variance = magnitude_variance},
                    .angle = {.value = {angle_a, angle_b, angle_c}, .variance = angle_variance}};

                auto const independent = static_cast<IndependentComplexRandVar<asymmetric_t>>(polar);

                CHECK(real(independent.value(0)) == doctest::Approx(real(polar.value()(0))));
                CHECK(imag(independent.value(0)) == doctest::Approx(imag(polar.value()(0))));
                CHECK(real(independent.value(1)) == doctest::Approx(real(polar.value()(1))));
                CHECK(imag(independent.value(1)) == doctest::Approx(imag(polar.value()(1))));
                CHECK(real(independent.value(2)) == doctest::Approx(real(polar.value()(2))));
                CHECK(imag(independent.value(2)) == doctest::Approx(imag(polar.value()(2))));
                CHECK(independent.variance(0) ==
                      doctest::Approx(magnitude_variance + magnitude_a * magnitude_a * angle_variance));
                CHECK(independent.variance(1) ==
                      doctest::Approx(magnitude_variance + magnitude_b * magnitude_b * angle_variance));
                CHECK(independent.variance(2) ==
                      doctest::Approx(magnitude_variance + magnitude_c * magnitude_c * angle_variance));
            }
        }

        SUBCASE("Conversion to UniformComplexRandVar<asymmetric_t>") {
            for (auto const& [magnitude_a, magnitude_b, magnitude_c, magnitude_variance, angle_a, angle_b, angle_c,
                              angle_variance] : std::array{std::tuple{1.0, 2.0, 3.0, 0.2, 0.0, deg_45, deg_90, 0.2},
                                                           std::tuple{2.0, 3.0, 4.0, 0.3, 0.0, deg_30, deg_60, 0.3}}) {
                CAPTURE(magnitude_a);
                CAPTURE(magnitude_b);
                CAPTURE(magnitude_c);
                CAPTURE(magnitude_variance);
                CAPTURE(angle_a);
                CAPTURE(angle_b);
                CAPTURE(angle_c);
                CAPTURE(angle_variance);

                PolarComplexRandVar<asymmetric_t> const polar{
                    .magnitude = {.value = {magnitude_a, magnitude_b, magnitude_c}, .variance = magnitude_variance},
                    .angle = {.value = {angle_a, angle_b, angle_c}, .variance = angle_variance}};

                auto const uniform = static_cast<UniformComplexRandVar<asymmetric_t>>(polar);

                CHECK(real(uniform.value(0)) == doctest::Approx(real(polar.value()(0))));
                CHECK(imag(uniform.value(0)) == doctest::Approx(imag(polar.value()(0))));
                CHECK(real(uniform.value(1)) == doctest::Approx(real(polar.value()(1))));
                CHECK(imag(uniform.value(1)) == doctest::Approx(imag(polar.value()(1))));
                CHECK(real(uniform.value(2)) == doctest::Approx(real(polar.value()(2))));
                CHECK(imag(uniform.value(2)) == doctest::Approx(imag(polar.value()(2))));

                CHECK(uniform.variance ==
                      doctest::Approx(magnitude_variance + magnitude_a * magnitude_a * angle_variance +
                                      magnitude_variance + magnitude_b * magnitude_b * angle_variance +
                                      magnitude_variance + magnitude_c * magnitude_c * angle_variance));
            }
        }
    }
}

TEST_CASE("Test statistics - conjugate") {
    SUBCASE("UniformComplexRandVar<symmetric_t> | IndependentComplexRandVar<symmetric_t>") {
        auto const check = [](auto const& var) {
            auto const conjugated = conj(var);
            CHECK(real(conjugated.value) == real(conj(var.value)));
            CHECK(imag(conjugated.value) == imag(conj(var.value)));
            CHECK(conjugated.variance == var.variance);
        };

        SUBCASE("UniformComplexRandVar<symmetric_t>") {
            check(UniformComplexRandVar<symmetric_t>{.value = {1.0, 2.0}, .variance = 2.0});
        }
        SUBCASE("IndependentComplexRandVar<symmetric_t>") {
            check(IndependentComplexRandVar<symmetric_t>{.value = {1.0, 2.0}, .variance = 2.0});
        }
    }
    SUBCASE("UniformComplexRandVar<asymmetric_t>") {
        auto const var = UniformComplexRandVar<asymmetric_t>{
            .value = {RealValue<asymmetric_t>{1.0, 2.0, 3.0}, RealValue<asymmetric_t>{4.0, 5.0, 6.0}}, .variance = 2.0};
        auto const conjugated = conj(var);
        CHECK(real(conjugated.value(0)) == real(conj(var.value(0))));
        CHECK(imag(conjugated.value(0)) == imag(conj(var.value(0))));
        CHECK(real(conjugated.value(1)) == real(conj(var.value(1))));
        CHECK(imag(conjugated.value(1)) == imag(conj(var.value(1))));
        CHECK(real(conjugated.value(2)) == real(conj(var.value(2))));
        CHECK(imag(conjugated.value(2)) == imag(conj(var.value(2))));
        CHECK(conjugated.variance == var.variance);
    }
    SUBCASE("IndependentComplexRandVar<asymmetric_t>") {
        auto const var = IndependentComplexRandVar<asymmetric_t>{
            .value = {RealValue<asymmetric_t>{1.0, 2.0, 3.0}, RealValue<asymmetric_t>{4.0, 5.0, 6.0}},
            .variance = {2.0, 3.0, 4.0}};
        auto const conjugated = conj(var);
        CHECK(real(conjugated.value(0)) == real(conj(var.value(0))));
        CHECK(imag(conjugated.value(0)) == imag(conj(var.value(0))));
        CHECK(real(conjugated.value(1)) == real(conj(var.value(1))));
        CHECK(imag(conjugated.value(1)) == imag(conj(var.value(1))));
        CHECK(real(conjugated.value(2)) == real(conj(var.value(2))));
        CHECK(imag(conjugated.value(2)) == imag(conj(var.value(2))));
        CHECK(conjugated.variance(0) == var.variance(0));
        CHECK(conjugated.variance(1) == var.variance(1));
        CHECK(conjugated.variance(2) == var.variance(2));
    }
    SUBCASE("DecomposedComplexRandVar<symmetric_t>") {
        auto const var = DecomposedComplexRandVar<symmetric_t>{.real_component = {.value = 1.0, .variance = 2.0},
                                                               .imag_component = {.value = 3.0, .variance = 4.0}};
        auto const conjugated = conj(var);
        CHECK(real(conjugated.value()) == real(conj(var.value())));
        CHECK(imag(conjugated.value()) == imag(conj(var.value())));
        CHECK(conjugated.real_component.variance == var.real_component.variance);
        CHECK(conjugated.imag_component.variance == var.imag_component.variance);
    }
    SUBCASE("DecomposedComplexRandVar<asymmetric_t>") {
        auto const var = DecomposedComplexRandVar<asymmetric_t>{
            .real_component = {.value = RealValue<asymmetric_t>{1.0, 2.0, 3.0},
                               .variance = RealValue<asymmetric_t>{2.0, 3.0, 4.0}},
            .imag_component = {.value = RealValue<asymmetric_t>{4.0, 5.0, 6.0},
                               .variance = RealValue<asymmetric_t>{3.0, 4.0, 5.0}}};
        auto const conjugated = conj(var);
        CHECK(real(conjugated.value()(0)) == real(conj(var.value())(0)));
        CHECK(imag(conjugated.value()(0)) == imag(conj(var.value())(0)));
        CHECK(real(conjugated.value()(1)) == real(conj(var.value())(1)));
        CHECK(imag(conjugated.value()(1)) == imag(conj(var.value())(1)));
        CHECK(real(conjugated.value()(2)) == real(conj(var.value())(2)));
        CHECK(imag(conjugated.value()(2)) == imag(conj(var.value())(2)));
        CHECK(conjugated.real_component.variance(0) == var.real_component.variance(0));
        CHECK(conjugated.imag_component.variance(0) == var.imag_component.variance(0));
        CHECK(conjugated.real_component.variance(1) == var.real_component.variance(1));
        CHECK(conjugated.imag_component.variance(1) == var.imag_component.variance(1));
        CHECK(conjugated.real_component.variance(2) == var.real_component.variance(2));
        CHECK(conjugated.imag_component.variance(2) == var.imag_component.variance(2));
    }

    SUBCASE("PolarComplexRandVar<symmetric_t>") {
        auto const var = PolarComplexRandVar<symmetric_t>{.magnitude = {.value = 1.0, .variance = 2.0},
                                                          .angle = {.value = 3.0, .variance = 4.0}};
        auto const conjugated = conj(var);
        CHECK(real(conjugated.value()) == real(conj(var.value())));
        CHECK(imag(conjugated.value()) == imag(conj(var.value())));
        CHECK(conjugated.magnitude.variance == var.magnitude.variance);
        CHECK(conjugated.angle.variance == var.angle.variance);
    }
    SUBCASE("PolarComplexRandVar<asymmetric_t>") {
        auto const var = PolarComplexRandVar<asymmetric_t>{
            .magnitude = {.value = RealValue<asymmetric_t>{1.0, 2.0, 3.0}, .variance = 2.0},
            .angle = {.value = RealValue<asymmetric_t>{4.0, 5.0, 6.0}, .variance = 4.0}};
        auto const conjugated = conj(var);
        CHECK(real(conjugated.value()(0)) == real(conj(var.value()(0))));
        CHECK(imag(conjugated.value()(0)) == imag(conj(var.value()(0))));
        CHECK(real(conjugated.value()(1)) == real(conj(var.value()(1))));
        CHECK(imag(conjugated.value()(1)) == imag(conj(var.value()(1))));
        CHECK(real(conjugated.value()(2)) == real(conj(var.value()(2))));
        CHECK(imag(conjugated.value()(2)) == imag(conj(var.value()(2))));
        CHECK(conjugated.magnitude.variance == var.magnitude.variance);
        CHECK(conjugated.angle.variance == var.angle.variance);
    }
}

TEST_CASE("Test statistics - scale") {
    using statistics::scale;

    SUBCASE("UniformRealRandVar<symmetric_t> | IndependentRealRandVar<symmetric_t>") {
        auto const check = []<typename RandVar> {
            RandVar const var{.value = 1.0, .variance = 2.0};
            auto const scaled = scale(var, 3.0);
            CHECK(scaled.value == doctest::Approx(3.0));
            CHECK(scaled.variance == doctest::Approx(18.0));
        };

        SUBCASE("UniformRealRandVar<symmetric_t>") { check.template operator()<UniformRealRandVar<symmetric_t>>(); }
        SUBCASE("IndependentRealRandVar<symmetric_t>") {
            check.template operator()<IndependentRealRandVar<symmetric_t>>();
        }
    }
    SUBCASE("UniformRealRandVar<asymmetric_t>") {
        UniformRealRandVar<asymmetric_t> const var{.value = {1.0, 2.0, 3.0}, .variance = 2.0};
        SUBCASE("Scalar scale") {
            auto const scaled = scale(var, 3.0);
            CHECK(scaled.value(0) == doctest::Approx(3.0));
            CHECK(scaled.value(1) == doctest::Approx(6.0));
            CHECK(scaled.value(2) == doctest::Approx(9.0));
            CHECK(scaled.variance == doctest::Approx(18.0));
        }
        // scaling asymmetrically would promote the UniformRealRandVar to an IndependentRealRandVar, because the
        // individual phases scale differently
    }
    SUBCASE("IndependentRealRandVar<asymmetric_t>") {
        auto const var = IndependentRealRandVar<asymmetric_t>{.value = {1.0, 2.0, 3.0}, .variance = {2.0, 3.0, 4.0}};
        SUBCASE("Scalar scale") {
            auto const scaled = scale(var, 3.0);
            CHECK(scaled.value(0) == doctest::Approx(3.0));
            CHECK(scaled.value(1) == doctest::Approx(6.0));
            CHECK(scaled.value(2) == doctest::Approx(9.0));
            CHECK(scaled.variance(0) == doctest::Approx(18.0));
            CHECK(scaled.variance(1) == doctest::Approx(27.0));
            CHECK(scaled.variance(2) == doctest::Approx(36.0));
        }
        SUBCASE("Asymmetric scale") {
            auto const scaled = scale(var, RealValue<asymmetric_t>{1.0, 2.0, 3.0});
            CHECK(scaled.value(0) == doctest::Approx(1.0));
            CHECK(scaled.value(1) == doctest::Approx(4.0));
            CHECK(scaled.value(2) == doctest::Approx(9.0));
            CHECK(scaled.variance(0) == doctest::Approx(2.0));
            CHECK(scaled.variance(1) == doctest::Approx(12.0));
            CHECK(scaled.variance(2) == doctest::Approx(36.0));
        }
    }
    SUBCASE("UniformComplexRandVar<symmetric_t> | IndependentComplexRandVar<symmetric_t>") {
        auto const check = []<typename RandVar> {
            RandVar const var{.value = {1.0, 2.0}, .variance = 2.0};

            SUBCASE("Real scalar scale") {
                auto const scaled = scale(var, 3.0);
                CHECK(real(scaled.value) == doctest::Approx(3.0));
                CHECK(imag(scaled.value) == doctest::Approx(6.0));
                CHECK(scaled.variance == doctest::Approx(18.0));
            }
            SUBCASE("Complex scalar scale") {
                auto const scaled = scale(var, DoubleComplex{3.0, 4.0});
                CHECK(real(scaled.value) == doctest::Approx(3.0 * 1.0 - 4.0 * 2.0));
                CHECK(imag(scaled.value) == doctest::Approx(3.0 * 2.0 + 4.0 * 1.0));
                CHECK(scaled.variance == doctest::Approx(2.0 * (3.0 * 3.0 + 4.0 * 4.0)));
            }
        };
        SUBCASE("UniformComplexRandVar<symmetric_t>") {
            check.template operator()<UniformComplexRandVar<symmetric_t>>();
        }
        SUBCASE("IndependentComplexRandVar<symmetric_t>") {
            check.template operator()<IndependentComplexRandVar<symmetric_t>>();
        }
    }
    SUBCASE("UniformComplexRandVar<asymmetric_t>") {
        UniformComplexRandVar<asymmetric_t> const var{
            .value = {RealValue<asymmetric_t>{1.0, 2.0, 3.0}, RealValue<asymmetric_t>{4.0, 5.0, 6.0}}, .variance = 2.0};
        SUBCASE("Real scalar scale") {
            auto const scaled = scale(var, 3.0);
            CHECK(real(scaled.value(0)) == doctest::Approx(3.0));
            CHECK(real(scaled.value(1)) == doctest::Approx(6.0));
            CHECK(real(scaled.value(2)) == doctest::Approx(9.0));
            CHECK(imag(scaled.value(0)) == doctest::Approx(12.0));
            CHECK(imag(scaled.value(1)) == doctest::Approx(15.0));
            CHECK(imag(scaled.value(2)) == doctest::Approx(18.0));
            CHECK(scaled.variance == doctest::Approx(18.0));
        }
        SUBCASE("Complex scalar scale") {
            DoubleComplex const scale_factor{3.0, 4.0};
            auto const scaled = scale(var, scale_factor);
            for (auto i = 0; i < 3; ++i) {
                CHECK(real(scaled.value(i)) ==
                      real(scale(UniformComplexRandVar<symmetric_t>{.value = var.value(i), .variance = var.variance},
                                 scale_factor)
                               .value));
                CHECK(real(scaled.value(i)) ==
                      real(scale(UniformComplexRandVar<symmetric_t>{.value = var.value(i), .variance = var.variance},
                                 scale_factor)
                               .value));
            }
            CHECK(scaled.variance == doctest::Approx(2.0 * (3.0 * 3.0 + 4.0 * 4.0)));
        }
        // scaling asymmetrically would promote the UniformComplexRandVar to an IndependentComplexRandVar, because the
        // individual phases scale differently
    }
    SUBCASE("IndependentComplexRandVar<asymmetric_t>") {
        auto const var = IndependentComplexRandVar<asymmetric_t>{
            .value = {RealValue<asymmetric_t>{1.0, 2.0, 3.0}, RealValue<asymmetric_t>{4.0, 5.0, 6.0}},
            .variance = {2.0, 3.0, 4.0}};
        auto const individual_phases = [&var] {
            std::array<UniformComplexRandVar<symmetric_t>, 3> result;
            for (auto i = 0; i < 3; ++i) {
                result[i] = {.value = var.value(i), .variance = var.variance(i)};
            }
            return result;
        }();
        SUBCASE("Real scalar scale") {
            auto const scale_factor = 3.0;
            auto const scaled = scale(var, scale_factor);
            for (auto i = 0; i < 3; ++i) {
                auto const expected = scale(individual_phases[i], scale_factor);
                CHECK(real(scaled.value(i)) == doctest::Approx(real(expected.value)));
                CHECK(imag(scaled.value(i)) == doctest::Approx(imag(expected.value)));
                CHECK(scaled.variance(i) == doctest::Approx(expected.variance));
            }
        }
        SUBCASE("Complex scalar scale") {
            DoubleComplex const scale_factor{3.0, 4.0};
            auto const scaled = scale(var, scale_factor);
            for (auto i = 0; i < 3; ++i) {
                auto const expected = scale(individual_phases[i], scale_factor);
                CHECK(real(scaled.value(i)) == doctest::Approx(real(expected.value)));
                CHECK(imag(scaled.value(i)) == doctest::Approx(imag(expected.value)));
                CHECK(scaled.variance(i) == doctest::Approx(expected.variance));
            }
        }
        SUBCASE("Real asymmetric scale") {
            RealValue<asymmetric_t> const scale_factor{3.0, 4.0, 5.0};
            auto const scaled = scale(var, scale_factor);
            for (auto i = 0; i < 3; ++i) {
                auto const expected = scale(individual_phases[i], scale_factor[i]);
                CHECK(real(scaled.value(i)) == doctest::Approx(real(expected.value)));
                CHECK(imag(scaled.value(i)) == doctest::Approx(imag(expected.value)));
                CHECK(scaled.variance(i) == doctest::Approx(expected.variance));
            }
        }
        SUBCASE("Complex asymmetric scale") {
            ComplexValue<asymmetric_t> const scale_factor{RealValue<asymmetric_t>{3.0, 4.0, 5.0},
                                                          RealValue<asymmetric_t>{6.0, 7.0, 8.0}};
            auto const scaled = scale(var, scale_factor);
            for (auto i = 0; i < 3; ++i) {
                auto const expected = scale(individual_phases[i], scale_factor(i));
                CHECK(real(scaled.value(i)) == doctest::Approx(real(expected.value)));
                CHECK(imag(scaled.value(i)) == doctest::Approx(imag(expected.value)));
                CHECK(scaled.variance(i) == doctest::Approx(expected.variance));
            }
        }
    }
    SUBCASE("DecomposedComplexRandVar<symmetric_t>") {
        DecomposedComplexRandVar<symmetric_t> const var{.real_component = {.value = 1.0, .variance = 2.0},
                                                        .imag_component = {.value = 4.0, .variance = 5.0}};

        SUBCASE("Real scalar scale") {
            auto const scale_factor = 3.0;
            auto const scaled = scale(var, scale_factor);
            CHECK(scaled.real_component.value == scale(var.real_component, scale_factor).value);
            CHECK(scaled.imag_component.value == scale(var.imag_component, scale_factor).value);
            CHECK(scaled.real_component.variance == scale(var.real_component, scale_factor).variance);
            CHECK(scaled.imag_component.variance == scale(var.imag_component, scale_factor).variance);
        }
        SUBCASE("Complex scalar scale") {
            auto const scale_factor = DoubleComplex{3.0, 4.0};
            auto const scaled = scale(var, scale_factor);
            CHECK(scaled.real_component.value == real(var.value() * scale_factor));
            CHECK(scaled.imag_component.value == imag(var.value() * scale_factor));
            CHECK(scaled.real_component.variance ==
                  real(scale_factor) * real(scale_factor) * var.real_component.variance +
                      imag(scale_factor) * imag(scale_factor) * var.imag_component.variance);
            CHECK(scaled.imag_component.variance ==
                  real(scale_factor) * real(scale_factor) * var.imag_component.variance +
                      imag(scale_factor) * imag(scale_factor) * var.real_component.variance);
        }
    }
    SUBCASE("DecomposedComplexRandVar<asymmetric_t>") {
        DecomposedComplexRandVar<asymmetric_t> const var{
            .real_component = {.value = {1.0, 2.0, 3.0}, .variance = {2.0, 3.0, 4.0}},
            .imag_component = {.value = {4.0, 5.0, 6.0}, .variance = {5.0, 6.0, 7.0}}};

        SUBCASE("Real scalar scale") {
            auto const scale_factor = 3.0;
            auto const scaled = scale(var, scale_factor);

            for (int i = 0; i < 3; ++i) {
                CHECK(scaled.real_component.value(i) == scale(var.real_component, scale_factor).value(i));
                CHECK(scaled.imag_component.value(i) == scale(var.imag_component, scale_factor).value(i));
                CHECK(scaled.real_component.variance(i) == scale(var.real_component, scale_factor).variance(i));
                CHECK(scaled.imag_component.variance(i) == scale(var.imag_component, scale_factor).variance(i));
            }
        }
        SUBCASE("Real asymmetric scale") {
            auto const scale_factor = RealValue<asymmetric_t>{3.0, 4.0, 5.0};
            auto const scaled = scale(var, scale_factor);

            for (int i = 0; i < 3; ++i) {
                CHECK(scaled.real_component.value(i) == scale(var.real_component, scale_factor).value(i));
                CHECK(scaled.imag_component.value(i) == scale(var.imag_component, scale_factor).value(i));
                CHECK(scaled.real_component.variance(i) == scale(var.real_component, scale_factor).variance(i));
                CHECK(scaled.imag_component.variance(i) == scale(var.imag_component, scale_factor).variance(i));
            }
        }
        SUBCASE("Complex scalar scale") {
            auto const scale_factor = DoubleComplex{3.0, 4.0};
            auto const scaled = scale(var, scale_factor);

            for (int i = 0; i < 3; ++i) {
                CHECK(scaled.real_component.value(i) == real(var.value()(i) * scale_factor));
                CHECK(scaled.imag_component.value(i) == imag(var.value()(i) * scale_factor));
                CHECK(scaled.real_component.variance(i) ==
                      real(scale_factor) * real(scale_factor) * var.real_component.variance(i) +
                          imag(scale_factor) * imag(scale_factor) * var.imag_component.variance(i));
                CHECK(scaled.imag_component.variance(i) ==
                      real(scale_factor) * real(scale_factor) * var.imag_component.variance(i) +
                          imag(scale_factor) * imag(scale_factor) * var.real_component.variance(i));
            }
        }
        SUBCASE("Complex asymmetric scale") {
            auto const scale_factor = ComplexValue<asymmetric_t>{RealValue<asymmetric_t>{3.0, 4.0, 5.0},
                                                                 RealValue<asymmetric_t>{6.0, 7.0, 8.0}};
            auto const scaled = scale(var, scale_factor);

            for (int i = 0; i < 3; ++i) {
                CHECK(scaled.real_component.value(i) == real(var.value()(i) * scale_factor(i)));
                CHECK(scaled.imag_component.value(i) == imag(var.value()(i) * scale_factor(i)));
                CHECK(scaled.real_component.variance(i) ==
                      real(scale_factor(i)) * real(scale_factor(i)) * var.real_component.variance(i) +
                          imag(scale_factor(i)) * imag(scale_factor(i)) * var.imag_component.variance(i));
                CHECK(scaled.imag_component.variance(i) ==
                      real(scale_factor(i)) * real(scale_factor(i)) * var.imag_component.variance(i) +
                          imag(scale_factor(i)) * imag(scale_factor(i)) * var.real_component.variance(i));
            }
        }
    }
}

TEST_CASE("Test statistics - combine") {
    using statistics::combine;
    using std::views::take;

    SUBCASE("UniformRealRandVar<symmetric_t> | IndependentRealRandVar<symmetric_t>") {
        // using a template lambda to avoid code duplication and to avoid having to create a separate test case
        auto const check = []<typename T>() {
            std::vector<T> const measurements{
                {.value = 1.0, .variance = 0.2}, {.value = 2.0, .variance = 0.3}, {.value = 3.0, .variance = 0.6}};

            CHECK(combine(measurements | take(0)).value == 0.0);
            CHECK(is_inf(combine(measurements | take(0)).variance));

            CHECK(combine(measurements | take(1)).value == measurements.front().value);
            CHECK(combine(measurements | take(1)).variance == measurements.front().variance);

            CHECK(combine(measurements | take(2)).value == doctest::Approx(7.0 / 5.0));
            CHECK(combine(measurements | take(2)).variance == doctest::Approx(3.0 / 25.0));

            CHECK(combine(measurements | take(3)).value == doctest::Approx(5.0 / 3.0));
            CHECK(combine(measurements | take(3)).variance == doctest::Approx(1.0 / 10.0));
        };
        SUBCASE("UniformRealRandVar<symmetric_t>") { check.template operator()<UniformRealRandVar<symmetric_t>>(); }
        SUBCASE("IndependentRealRandVar<symmetric_t>") {
            check.template operator()<IndependentRealRandVar<symmetric_t>>();
        }
    }

    SUBCASE("UniformRealRandVar<asymmetric_t>") {
        std::vector<UniformRealRandVar<asymmetric_t>> const measurements{{.value = {1.0, 2.0, -1.0}, .variance = 0.2},
                                                                         {.value = {2.0, 4.0, 3.0}, .variance = 0.3},
                                                                         {.value = {4.0, 5.0, 6.0}, .variance = 0.6}};

        CHECK(combine(measurements | take(0)).value(0) == 0.0);
        CHECK(combine(measurements | take(0)).value(1) == 0.0);
        CHECK(combine(measurements | take(0)).value(2) == 0.0);
        CHECK(is_inf(combine(measurements | take(0)).variance));

        CHECK(combine(measurements | take(1)).value(0) == measurements.front().value(0));
        CHECK(combine(measurements | take(1)).value(1) == measurements.front().value(1));
        CHECK(combine(measurements | take(1)).value(2) == measurements.front().value(2));
        CHECK(combine(measurements | take(1)).variance == measurements.front().variance);

        CHECK(combine(measurements | take(2)).value(0) == doctest::Approx(7.0 / 5.0));
        CHECK(combine(measurements | take(2)).value(1) == doctest::Approx(14.0 / 5.0));
        CHECK(combine(measurements | take(2)).value(2) == doctest::Approx(3.0 / 5.0));
        CHECK(combine(measurements | take(2)).variance == doctest::Approx(3.0 / 25.0));

        CHECK(combine(measurements | take(3)).value(0) == doctest::Approx(11.0 / 6.0));
        CHECK(combine(measurements | take(3)).value(1) == doctest::Approx(19.0 / 6.0));
        CHECK(combine(measurements | take(3)).value(2) == doctest::Approx(3.0 / 2.0));
        CHECK(combine(measurements | take(3)).variance == doctest::Approx(1.0 / 10.0));
    }

    SUBCASE("IndependentRealRandVar<asymmetric_t>") {
        std::vector<IndependentRealRandVar<asymmetric_t>> const measurements{
            {.value = {1.0, 2.0, -1.0}, .variance = {0.2, 0.3, 0.4}},
            {.value = {2.0, 4.0, 3.0}, .variance = {0.3, 0.4, 0.5}},
            {.value = {4.0, 5.0, 6.0}, .variance = {0.6, 0.7, 0.8}}};

        CHECK(combine(measurements | take(0)).value(0) == 0.0);
        CHECK(combine(measurements | take(0)).value(1) == 0.0);
        CHECK(combine(measurements | take(0)).value(2) == 0.0);
        CHECK(is_inf(combine(measurements | take(0)).variance(0)));
        CHECK(is_inf(combine(measurements | take(0)).variance(1)));
        CHECK(is_inf(combine(measurements | take(0)).variance(2)));

        CHECK(combine(measurements | take(1)).value(0) == measurements.front().value(0));
        CHECK(combine(measurements | take(1)).value(1) == measurements.front().value(1));
        CHECK(combine(measurements | take(1)).value(2) == measurements.front().value(2));
        CHECK(combine(measurements | take(1)).variance(0) == measurements.front().variance(0));
        CHECK(combine(measurements | take(1)).variance(1) == measurements.front().variance(1));
        CHECK(combine(measurements | take(1)).variance(2) == measurements.front().variance(2));

        CHECK(combine(measurements | take(2)).value(0) == doctest::Approx(7.0 / 5.0));
        CHECK(combine(measurements | take(2)).value(1) == doctest::Approx(20.0 / 7.0));
        CHECK(combine(measurements | take(2)).value(2) == doctest::Approx(7.0 / 9.0));
        CHECK(combine(measurements | take(2)).variance(0) == doctest::Approx(3.0 / 25.0));
        CHECK(combine(measurements | take(2)).variance(1) == doctest::Approx(6.0 / 35.0));
        CHECK(combine(measurements | take(2)).variance(2) == doctest::Approx(2.0 / 9.0));

        CHECK(combine(measurements | take(3)).value(0) == doctest::Approx(11.0 / 6.0));
        CHECK(combine(measurements | take(3)).value(1) == doctest::Approx(200.0 / 61.0));
        CHECK(combine(measurements | take(3)).value(2) == doctest::Approx(44.0 / 23.0));
        CHECK(combine(measurements | take(3)).variance(0) == doctest::Approx(1.0 / 10.0));
        CHECK(combine(measurements | take(3)).variance(1) == doctest::Approx(42.0 / 305.0));
        CHECK(combine(measurements | take(3)).variance(2) == doctest::Approx(4.0 / 23.0));
    }

    SUBCASE("UniformComplexRandVar<symmetric_t> | IndependentComplexRandVar<symmetric_t>") {
        // using a template lambda to avoid code duplication and to avoid having to create a separate test case
        auto const check = []<typename T>() {
            std::vector<T> const measurements{T{.value = 1.0 + 5.0i, .variance = 0.2},
                                              T{.value = 2.0 + 6.0i, .variance = 0.3},
                                              T{.value = 4.0 + 3.0i, .variance = 0.6}};

            CHECK(combine(measurements | take(0)).value.real() == 0.0);
            CHECK(combine(measurements | take(0)).value.imag() == 0.0);
            CHECK(is_inf(combine(measurements | take(0)).variance));

            CHECK(combine(measurements | take(1)).value.real() == measurements.front().value.real());
            CHECK(combine(measurements | take(1)).value.imag() == measurements.front().value.imag());
            CHECK(combine(measurements | take(1)).variance == measurements.front().variance);

            CHECK(combine(measurements | take(2)).value.real() == doctest::Approx(7.0 / 5.0));
            CHECK(combine(measurements | take(2)).value.imag() == doctest::Approx(27.0 / 5.0));
            CHECK(combine(measurements | take(2)).variance == doctest::Approx(3.0 / 25.0));

            CHECK(combine(measurements | take(3)).value.real() == doctest::Approx(11.0 / 6.0));
            CHECK(combine(measurements | take(3)).value.imag() == doctest::Approx(30.0 / 6.0));
            CHECK(combine(measurements | take(3)).variance == doctest::Approx(1.0 / 10.0));
        };
        SUBCASE("UniformComplexRandVar<symmetric_t>") {
            check.template operator()<UniformComplexRandVar<symmetric_t>>();
        }
        SUBCASE("IndependentComplexRandVar<symmetric_t>") {
            check.template operator()<IndependentComplexRandVar<symmetric_t>>();
        }
    }

    SUBCASE("UniformComplexRandVar<asymmetric_t>") {
        std::vector<UniformComplexRandVar<asymmetric_t>> const measurements{
            {.value = {RealValue<asymmetric_t>{1.0, 2.0, -1.0}, RealValue<asymmetric_t>{5.0, 6.0, 7.0}},
             .variance = 0.2},
            {.value = {RealValue<asymmetric_t>{2.0, 4.0, 3.0}, RealValue<asymmetric_t>{6.0, -7.0, 2.0}},
             .variance = 0.3},
            {.value = {RealValue<asymmetric_t>{4.0, 5.0, 6.0}, RealValue<asymmetric_t>{3.0, 1.0, 2.0}},
             .variance = 0.6}};

        CHECK(combine(measurements | take(0)).value(0).real() == 0.0);
        CHECK(combine(measurements | take(0)).value(1).real() == 0.0);
        CHECK(combine(measurements | take(0)).value(2).real() == 0.0);
        CHECK(combine(measurements | take(0)).value(0).imag() == 0.0);
        CHECK(combine(measurements | take(0)).value(1).imag() == 0.0);
        CHECK(combine(measurements | take(0)).value(2).imag() == 0.0);
        CHECK(is_inf(combine(measurements | take(0)).variance));

        CHECK(combine(measurements | take(1)).value(0).real() == measurements.front().value(0).real());
        CHECK(combine(measurements | take(1)).value(1).real() == measurements.front().value(1).real());
        CHECK(combine(measurements | take(1)).value(2).real() == measurements.front().value(2).real());
        CHECK(combine(measurements | take(1)).value(0).imag() == measurements.front().value(0).imag());
        CHECK(combine(measurements | take(1)).value(1).imag() == measurements.front().value(1).imag());
        CHECK(combine(measurements | take(1)).value(2).imag() == measurements.front().value(2).imag());
        CHECK(combine(measurements | take(1)).variance == measurements.front().variance);

        CHECK(combine(measurements | take(2)).value(0).real() == doctest::Approx(7.0 / 5.0));
        CHECK(combine(measurements | take(2)).value(1).real() == doctest::Approx(14.0 / 5.0));
        CHECK(combine(measurements | take(2)).value(2).real() == doctest::Approx(3.0 / 5.0));
        CHECK(combine(measurements | take(2)).value(0).imag() == doctest::Approx(27.0 / 5.0));
        CHECK(combine(measurements | take(2)).value(1).imag() == doctest::Approx(4.0 / 5.0));
        CHECK(combine(measurements | take(2)).value(2).imag() == doctest::Approx(25.0 / 5.0));
        CHECK(combine(measurements | take(2)).variance == doctest::Approx(3.0 / 25.0));

        CHECK(combine(measurements | take(3)).value(0).real() == doctest::Approx(11.0 / 6.0));
        CHECK(combine(measurements | take(3)).value(1).real() == doctest::Approx(19.0 / 6.0));
        CHECK(combine(measurements | take(3)).value(2).real() == doctest::Approx(9.0 / 6.0));
        CHECK(combine(measurements | take(3)).value(0).imag() == doctest::Approx(30.0 / 6.0));
        CHECK(combine(measurements | take(3)).value(1).imag() == doctest::Approx(5.0 / 6.0));
        CHECK(combine(measurements | take(3)).value(2).imag() == doctest::Approx(27.0 / 6.0));
        CHECK(combine(measurements | take(3)).variance == doctest::Approx(1.0 / 10.0));
    }

    SUBCASE("IndependentComplexRandVar<asymmetric_t>") {
        std::vector<IndependentComplexRandVar<asymmetric_t>> const measurements{
            {.value = {RealValue<asymmetric_t>{1.0, 2.0, -1.0}, RealValue<asymmetric_t>{5.0, 6.0, 7.0}},
             .variance = {0.2, 0.3, 0.4}},
            {.value = {RealValue<asymmetric_t>{2.0, 4.0, 3.0}, RealValue<asymmetric_t>{6.0, -7.0, 2.0}},
             .variance = {0.3, 0.4, 0.5}},
            {.value = {RealValue<asymmetric_t>{4.0, 5.0, 6.0}, RealValue<asymmetric_t>{3.0, 1.0, 2.0}},
             .variance = {0.6, 0.7, 0.8}}};

        CHECK(combine(measurements | take(0)).value(0).real() == 0.0);
        CHECK(combine(measurements | take(0)).value(1).real() == 0.0);
        CHECK(combine(measurements | take(0)).value(2).real() == 0.0);
        CHECK(combine(measurements | take(0)).value(0).imag() == 0.0);
        CHECK(combine(measurements | take(0)).value(1).imag() == 0.0);
        CHECK(combine(measurements | take(0)).value(2).imag() == 0.0);
        CHECK(is_inf(combine(measurements | take(0)).variance(0)));
        CHECK(is_inf(combine(measurements | take(0)).variance(1)));
        CHECK(is_inf(combine(measurements | take(0)).variance(2)));

        CHECK(combine(measurements | take(1)).value(0).real() == measurements.front().value(0).real());
        CHECK(combine(measurements | take(1)).value(1).real() == measurements.front().value(1).real());
        CHECK(combine(measurements | take(1)).value(2).real() == measurements.front().value(2).real());
        CHECK(combine(measurements | take(1)).value(0).imag() == measurements.front().value(0).imag());
        CHECK(combine(measurements | take(1)).value(1).imag() == measurements.front().value(1).imag());
        CHECK(combine(measurements | take(1)).value(2).imag() == measurements.front().value(2).imag());
        CHECK(combine(measurements | take(1)).variance(0) == measurements.front().variance(0));
        CHECK(combine(measurements | take(1)).variance(1) == measurements.front().variance(1));
        CHECK(combine(measurements | take(1)).variance(2) == measurements.front().variance(2));

        CHECK(combine(measurements | take(2)).value(0).real() == doctest::Approx(7.0 / 5.0));
        CHECK(combine(measurements | take(2)).value(1).real() == doctest::Approx(20.0 / 7.0));
        CHECK(combine(measurements | take(2)).value(2).real() == doctest::Approx(7.0 / 9.0));
        CHECK(combine(measurements | take(2)).value(0).imag() == doctest::Approx(27.0 / 5.0));
        CHECK(combine(measurements | take(2)).value(1).imag() == doctest::Approx(3.0 / 7.0));
        CHECK(combine(measurements | take(2)).value(2).imag() == doctest::Approx(43.0 / 9.0));
        CHECK(combine(measurements | take(2)).variance(0) == doctest::Approx(3.0 / 25.0));
        CHECK(combine(measurements | take(2)).variance(1) == doctest::Approx(6.0 / 35.0));
        CHECK(combine(measurements | take(2)).variance(2) == doctest::Approx(2.0 / 9.0));

        CHECK(combine(measurements | take(3)).value(0).real() == doctest::Approx(11.0 / 6.0));
        CHECK(combine(measurements | take(3)).value(1).real() == doctest::Approx(200.0 / 61.0));
        CHECK(combine(measurements | take(3)).value(2).real() == doctest::Approx(44.0 / 23.0));
        CHECK(combine(measurements | take(3)).value(0).imag() == doctest::Approx(30.0 / 6.0));
        CHECK(combine(measurements | take(3)).value(1).imag() == doctest::Approx(33.0 / 61.0));
        CHECK(combine(measurements | take(3)).value(2).imag() == doctest::Approx(96.0 / 23.0));
        CHECK(combine(measurements | take(3)).variance(0) == doctest::Approx(1.0 / 10.0));
        CHECK(combine(measurements | take(3)).variance(1) == doctest::Approx(42.0 / 305.0));
        CHECK(combine(measurements | take(3)).variance(2) == doctest::Approx(4.0 / 23.0));
    }

    SUBCASE("DecomposedComplexRandVar<symmetric_t>") {
        std::vector<DecomposedComplexRandVar<symmetric_t>> const measurements{
            DecomposedComplexRandVar<symmetric_t>{.real_component = {.value = 1.0, .variance = 0.2},
                                                  .imag_component = {.value = 5.0, .variance = 0.1}},
            DecomposedComplexRandVar<symmetric_t>{.real_component = {.value = 2.0, .variance = 0.3},
                                                  .imag_component = {.value = 6.0, .variance = 0.2}},
            DecomposedComplexRandVar<symmetric_t>{.real_component = {.value = 4.0, .variance = 0.6},
                                                  .imag_component = {.value = 3.0, .variance = 0.3}}};

        CHECK(combine(measurements | take(0)).real_component.value == 0.0);
        CHECK(combine(measurements | take(0)).imag_component.value == 0.0);
        CHECK(is_inf(combine(measurements | take(0)).real_component.variance));
        CHECK(is_inf(combine(measurements | take(0)).imag_component.variance));

        CHECK(combine(measurements | take(1)).real_component.value == measurements.front().real_component.value);
        CHECK(combine(measurements | take(1)).imag_component.value == measurements.front().imag_component.value);
        CHECK(combine(measurements | take(1)).real_component.variance == measurements.front().real_component.variance);
        CHECK(combine(measurements | take(1)).imag_component.variance == measurements.front().imag_component.variance);

        CHECK(combine(measurements | take(2)).real_component.value == doctest::Approx(7.0 / 5.0));
        CHECK(combine(measurements | take(2)).imag_component.value == doctest::Approx(80.0 / 15.0));
        CHECK(combine(measurements | take(2)).real_component.variance == doctest::Approx(3.0 / 25.0));
        CHECK(combine(measurements | take(2)).imag_component.variance == doctest::Approx(1.0 / 15.0));

        CHECK(combine(measurements | take(3)).real_component.value == doctest::Approx(11.0 / 6.0));
        CHECK(combine(measurements | take(3)).imag_component.value == doctest::Approx(270.0 / 55.0));
        CHECK(combine(measurements | take(3)).real_component.variance == doctest::Approx(1.0 / 10.0));
        CHECK(combine(measurements | take(3)).imag_component.variance == doctest::Approx(3.0 / 55.0));
    }

    SUBCASE("DecomposedComplexRandVar<asymmetric_t>") {
        std::vector<DecomposedComplexRandVar<asymmetric_t>> const measurements{
            DecomposedComplexRandVar<asymmetric_t>{
                .real_component = {.value = RealValue<asymmetric_t>{1.0, 2.0, -1.0}, .variance = {0.2, 0.3, 0.4}},
                .imag_component = {.value = RealValue<asymmetric_t>{5.0, 6.0, 7.0}, .variance = {0.1, 0.2, 0.3}}},
            DecomposedComplexRandVar<asymmetric_t>{
                .real_component = {.value = RealValue<asymmetric_t>{2.0, 4.0, 3.0}, .variance = {0.3, 0.4, 0.5}},
                .imag_component = {.value = RealValue<asymmetric_t>{6.0, -7.0, 2.0}, .variance = {0.2, 0.3, 0.4}}},
            DecomposedComplexRandVar<asymmetric_t>{
                .real_component = {.value = RealValue<asymmetric_t>{4.0, 5.0, 6.0}, .variance = {0.6, 0.7, 0.8}},
                .imag_component = {.value = RealValue<asymmetric_t>{3.0, 1.0, 2.0}, .variance = {0.3, 0.4, 0.5}}}};

        CHECK(combine(measurements | take(0)).real_component.value(0) == 0.0);
        CHECK(combine(measurements | take(0)).real_component.value(2) == 0.0);
        CHECK(combine(measurements | take(0)).real_component.value(1) == 0.0);
        CHECK(combine(measurements | take(0)).imag_component.value(0) == 0.0);
        CHECK(combine(measurements | take(0)).imag_component.value(2) == 0.0);
        CHECK(combine(measurements | take(0)).imag_component.value(1) == 0.0);
        CHECK(is_inf(combine(measurements | take(0)).real_component.variance(0)));
        CHECK(is_inf(combine(measurements | take(0)).real_component.variance(1)));
        CHECK(is_inf(combine(measurements | take(0)).real_component.variance(2)));
        CHECK(is_inf(combine(measurements | take(0)).imag_component.variance(0)));
        CHECK(is_inf(combine(measurements | take(0)).imag_component.variance(1)));
        CHECK(is_inf(combine(measurements | take(0)).imag_component.variance(2)));

        CHECK(combine(measurements | take(1)).real_component.value(0) == measurements.front().real_component.value(0));
        CHECK(combine(measurements | take(1)).real_component.value(1) == measurements.front().real_component.value(1));
        CHECK(combine(measurements | take(1)).real_component.value(2) == measurements.front().real_component.value(2));
        CHECK(combine(measurements | take(1)).imag_component.value(0) == measurements.front().imag_component.value(0));
        CHECK(combine(measurements | take(1)).imag_component.value(1) == measurements.front().imag_component.value(1));
        CHECK(combine(measurements | take(1)).imag_component.value(2) == measurements.front().imag_component.value(2));
        CHECK(combine(measurements | take(1)).real_component.variance(0) ==
              measurements.front().real_component.variance(0));
        CHECK(combine(measurements | take(1)).real_component.variance(1) ==
              measurements.front().real_component.variance(1));
        CHECK(combine(measurements | take(1)).real_component.variance(2) ==
              measurements.front().real_component.variance(2));
        CHECK(combine(measurements | take(1)).imag_component.variance(0) ==
              measurements.front().imag_component.variance(0));
        CHECK(combine(measurements | take(1)).imag_component.variance(1) ==
              measurements.front().imag_component.variance(1));
        CHECK(combine(measurements | take(1)).imag_component.variance(2) ==
              measurements.front().imag_component.variance(2));

        CHECK(combine(measurements | take(2)).real_component.value(0) == doctest::Approx(7.0 / 5.0));
        CHECK(combine(measurements | take(2)).real_component.value(1) == doctest::Approx(20.0 / 7.0));
        CHECK(combine(measurements | take(2)).real_component.value(2) == doctest::Approx(7.0 / 9.0));
        CHECK(combine(measurements | take(2)).imag_component.value(0) == doctest::Approx(80.0 / 15.0));
        CHECK(combine(measurements | take(2)).imag_component.value(1) == doctest::Approx(20 / 25.0));
        CHECK(combine(measurements | take(2)).imag_component.value(2) == doctest::Approx(170.0 / 35.0));
        CHECK(combine(measurements | take(2)).real_component.variance(0) == doctest::Approx(3.0 / 25.0));
        CHECK(combine(measurements | take(2)).real_component.variance(1) == doctest::Approx(6.0 / 35.0));
        CHECK(combine(measurements | take(2)).real_component.variance(2) == doctest::Approx(2.0 / 9.0));
        CHECK(combine(measurements | take(2)).imag_component.variance(0) == doctest::Approx(1.0 / 15.0));
        CHECK(combine(measurements | take(2)).imag_component.variance(1) == doctest::Approx(3.0 / 25.0));
        CHECK(combine(measurements | take(2)).imag_component.variance(2) == doctest::Approx(6.0 / 35.0));

        CHECK(combine(measurements | take(3)).real_component.value(0) == doctest::Approx(11.0 / 6.0));
        CHECK(combine(measurements | take(3)).real_component.value(1) == doctest::Approx(200.0 / 61.0));
        CHECK(combine(measurements | take(3)).real_component.value(2) == doctest::Approx(44.0 / 23.0));
        CHECK(combine(measurements | take(3)).imag_component.value(0) == doctest::Approx(270.0 / 55.0));
        CHECK(combine(measurements | take(3)).imag_component.value(1) == doctest::Approx(55.0 / 65.0));
        CHECK(combine(measurements | take(3)).imag_component.value(2) == doctest::Approx(194.0 / 47.0));
        CHECK(combine(measurements | take(3)).real_component.variance(0) == doctest::Approx(1.0 / 10.0));
        CHECK(combine(measurements | take(3)).real_component.variance(1) == doctest::Approx(42.0 / 305.0));
        CHECK(combine(measurements | take(3)).real_component.variance(2) == doctest::Approx(4.0 / 23.0));
        CHECK(combine(measurements | take(3)).imag_component.variance(0) == doctest::Approx(3.0 / 55.0));
        CHECK(combine(measurements | take(3)).imag_component.variance(1) == doctest::Approx(6.0 / 65));
        CHECK(combine(measurements | take(3)).imag_component.variance(2) == doctest::Approx(6.0 / 47.0));
    }
}

TEST_CASE("Test statistics - combine_magnitude") {
    using statistics::combine_magnitude;
    using std::views::take;

    SUBCASE("UniformComplexRandVar<symmetric_t>") {
        // using a template lambda to avoid code duplication and to avoid having to create a separate test case
        std::vector<UniformComplexRandVar<symmetric_t>> const measurements{
            {.value = ComplexValue<symmetric_t>{1.0, 5.0}, .variance = 0.2},
            {.value = ComplexValue<symmetric_t>{2.0, nan}, .variance = 0.3},
            {.value = ComplexValue<symmetric_t>{4.0, nan}, .variance = 0.6}};

        CHECK(combine_magnitude(measurements | take(0)).value.real() == 0.0);
        CHECK(is_nan(combine_magnitude(measurements | take(0)).value.imag()));
        CHECK(is_inf(combine_magnitude(measurements | take(0)).variance));

        CHECK(combine_magnitude(measurements | take(1)).value.real() == cabs(measurements.front().value));
        CHECK(is_nan(combine_magnitude(measurements | take(1)).value.imag()));
        CHECK(combine_magnitude(measurements | take(1)).variance == measurements.front().variance);

        CHECK(combine_magnitude(measurements | take(2)).value.real() ==
              doctest::Approx((3.0 * std::sqrt(26.0) + 4.0) / 5.0));
        CHECK(is_nan(combine_magnitude(measurements | take(2)).value.imag()));
        CHECK(combine_magnitude(measurements | take(2)).variance == doctest::Approx(3.0 / 25.0));

        CHECK(combine_magnitude(measurements | take(3)).value.real() ==
              doctest::Approx((8.0 + 3.0 * std::sqrt(26.0)) / 6.0));
        CHECK(is_nan(combine_magnitude(measurements | take(3)).value.imag()));
        CHECK(combine_magnitude(measurements | take(3)).variance == doctest::Approx(1.0 / 10.0));
    }

    SUBCASE("UniformComplexRandVar<asymmetric_t>") {
        std::vector<UniformComplexRandVar<asymmetric_t>> const measurements{
            {.value = {RealValue<asymmetric_t>{1.0, 2.0, -1.0}, RealValue<asymmetric_t>{5.0, 6.0, 7.0}},
             .variance = 0.2},
            {.value = {RealValue<asymmetric_t>{2.0, 4.0, 3.0}, RealValue<asymmetric_t>{nan}}, .variance = 0.3},
            {.value = {RealValue<asymmetric_t>{4.0, 5.0, 6.0}, RealValue<asymmetric_t>{nan}}, .variance = 0.6}};

        CHECK(combine_magnitude(measurements | take(0)).value(0).real() == 0.0);
        CHECK(combine_magnitude(measurements | take(0)).value(1).real() == 0.0);
        CHECK(combine_magnitude(measurements | take(0)).value(2).real() == 0.0);
        CHECK(is_nan(combine_magnitude(measurements | take(0)).value(0).imag()));
        CHECK(is_nan(combine_magnitude(measurements | take(0)).value(0).imag()));
        CHECK(is_nan(combine_magnitude(measurements | take(0)).value(0).imag()));
        CHECK(is_inf(combine_magnitude(measurements | take(0)).variance));

        CHECK(combine_magnitude(measurements | take(1)).value(0).real() ==
              doctest::Approx(cabs(measurements.front().value(0))));
        CHECK(combine_magnitude(measurements | take(1)).value(1).real() ==
              doctest::Approx(cabs(measurements.front().value(1))));
        CHECK(combine_magnitude(measurements | take(1)).value(2).real() ==
              doctest::Approx(cabs(measurements.front().value(2))));
        CHECK(is_nan(combine_magnitude(measurements | take(1)).value(0).imag()));
        CHECK(is_nan(combine_magnitude(measurements | take(1)).value(1).imag()));
        CHECK(is_nan(combine_magnitude(measurements | take(1)).value(2).imag()));
        CHECK(combine_magnitude(measurements | take(1)).variance == measurements.front().variance);

        CHECK(combine_magnitude(measurements | take(2)).value(0).real() ==
              doctest::Approx((3.0 * std::sqrt(26.0) + 4.0) / 5.0));
        CHECK(combine_magnitude(measurements | take(2)).value(1).real() ==
              doctest::Approx((8.0 + 6.0 * std::sqrt(10.0)) / 5.0));
        CHECK(combine_magnitude(measurements | take(2)).value(2).real() ==
              doctest::Approx((6.0 + 15.0 * std::sqrt(2.0)) / 5.0));
        CHECK(is_nan(combine_magnitude(measurements | take(2)).value(0).imag()));
        CHECK(is_nan(combine_magnitude(measurements | take(2)).value(1).imag()));
        CHECK(is_nan(combine_magnitude(measurements | take(2)).value(2).imag()));
        CHECK(combine_magnitude(measurements | take(2)).variance == doctest::Approx(3.0 / 25.0));

        CHECK(combine_magnitude(measurements | take(3)).value(0).real() ==
              doctest::Approx((8.0 + 3.0 * std::sqrt(26.0)) / 6.0));
        CHECK(combine_magnitude(measurements | take(3)).value(1).real() ==
              doctest::Approx((13.0 + 6.0 * std::sqrt(10.0)) / 6.0));
        CHECK(combine_magnitude(measurements | take(3)).value(2).real() ==
              doctest::Approx((4.0 + 5.0 * std::sqrt(2.0)) / 2.0));
        CHECK(is_nan(combine_magnitude(measurements | take(3)).value(0).imag()));
        CHECK(is_nan(combine_magnitude(measurements | take(3)).value(1).imag()));
        CHECK(is_nan(combine_magnitude(measurements | take(3)).value(2).imag()));
        CHECK(combine_magnitude(measurements | take(3)).variance == doctest::Approx(1.0 / 10.0));
    }
}

TEST_SUITE_END();

} // namespace power_grid_model
