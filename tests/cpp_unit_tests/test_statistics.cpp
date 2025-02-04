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
} // namespace

TEST_CASE("Test statistics") {
    SUBCASE("UniformRealRandomVariable<symmetric_t>") {
        for (auto const [value, variance] :
             std::array{std::tuple{1.0, 1.0}, std::tuple{2.0, 3.0}, std::tuple{0.0, 1.0}, std::tuple{2.0, 3.0}}) {
            CAPTURE(value);
            CAPTURE(variance);

            UniformRealRandomVariable<symmetric_t> const uniform{.value = value, .variance = variance};

            SUBCASE("Constructor") {
                CHECK(uniform.value == value);
                CHECK(uniform.variance == variance);
            }
            SUBCASE("Conversion to UniformRealRandomVariable<asymmetric_t>") {
                auto const asymmetric = static_cast<UniformRealRandomVariable<asymmetric_t>>(uniform);

                CHECK(asymmetric.value(0) == doctest::Approx(uniform.value));
                CHECK(asymmetric.value(1) == doctest::Approx(uniform.value));
                CHECK(asymmetric.value(2) == doctest::Approx(uniform.value));
                CHECK(asymmetric.variance == doctest::Approx(variance));
            }
        }
    }
    SUBCASE("UniformRealRandomVariable<asymmetric_t>") {
        for (auto const [value_a, value_b, value_c, variance] :
             std::array{std::tuple{1.0, 2.0, 3.0, 1.0}, std::tuple{2.0, 2.1, 2.2, 3.0}, std::tuple{0.0, 0.1, 0.2, 1.0},
                        std::tuple{2.0, 0.0, 0.0, 3.0}}) {
            CAPTURE(value_a);
            CAPTURE(value_b);
            CAPTURE(value_c);
            CAPTURE(variance);

            UniformRealRandomVariable<asymmetric_t> const uniform{.value = {value_a, value_b, value_c},
                                                                  .variance = variance};

            SUBCASE("Constructor") {
                CHECK(uniform.value(0) == value_a);
                CHECK(uniform.value(1) == value_b);
                CHECK(uniform.value(2) == value_c);
                CHECK(uniform.variance == variance);
            }
            SUBCASE("Conversion to UniformRealRandomVariable<symmetric_t>") {
                auto const symmetric = static_cast<UniformRealRandomVariable<symmetric_t>>(uniform);

                CHECK(symmetric.value == doctest::Approx(mean_val(uniform.value)));
                CHECK(symmetric.variance == doctest::Approx(variance / 3));
            }
        }
    }

    SUBCASE("IndependentRealRandomVariable<symmetric_t>") {
        for (auto const [value, variance] :
             std::array{std::tuple{1.0, 1.0}, std::tuple{2.0, 3.0}, std::tuple{0.0, 1.0}, std::tuple{2.0, 3.0}}) {
            CAPTURE(value);
            CAPTURE(variance);

            IndependentRealRandomVariable<symmetric_t> const independent{.value = value, .variance = variance};

            SUBCASE("Constructor") {
                CHECK(independent.value == value);
                CHECK(independent.variance == variance);
            }
            SUBCASE("Conversion to IndependentRealRandomVariable<asymmetric_t>") {
                auto const asymmetric = static_cast<IndependentRealRandomVariable<asymmetric_t>>(independent);
                auto const symmetric = static_cast<IndependentRealRandomVariable<symmetric_t>>(asymmetric);

                CHECK(asymmetric.value(0) == doctest::Approx(independent.value));
                CHECK(asymmetric.value(1) == doctest::Approx(independent.value));
                CHECK(asymmetric.value(2) == doctest::Approx(independent.value));
                CHECK(asymmetric.variance(0) == doctest::Approx(independent.variance));
                CHECK(asymmetric.variance(1) == doctest::Approx(independent.variance));
                CHECK(asymmetric.variance(2) == doctest::Approx(independent.variance));
            }
        }
    }
    SUBCASE("IndependentRealRandomVariable<asymmetric_t>") {
        for (auto const [value_a, value_b, value_c, variance_a, variance_b, variance_c] :
             std::array{std::tuple{1.0, 2.0, 3.0, 1.0, 2.0, 3.0}, std::tuple{2.0, 2.1, 2.2, 3.0, 1.0, 2.0},
                        std::tuple{0.0, 0.1, 0.2, 1.0, 1.0, 1.0}, std::tuple{2.0, 0.0, 0.0, 3.0, 3.0, 3.0}}) {
            CAPTURE(value_a);
            CAPTURE(value_b);
            CAPTURE(value_c);
            CAPTURE(variance_a);
            CAPTURE(variance_b);
            CAPTURE(variance_c);

            IndependentRealRandomVariable<asymmetric_t> const independent{
                .value = {value_a, value_b, value_c}, .variance = {variance_a, variance_b, variance_c}};

            SUBCASE("Constructor") {
                CHECK(independent.value(0) == value_a);
                CHECK(independent.value(1) == value_b);
                CHECK(independent.value(2) == value_c);
                CHECK(independent.variance(0) == variance_a);
                CHECK(independent.variance(1) == variance_b);
                CHECK(independent.variance(2) == variance_c);
            }
            SUBCASE("Conversion to IndependentRealRandomVariable<symmetric_t>") {
                auto const symmetric = static_cast<IndependentRealRandomVariable<symmetric_t>>(independent);
                auto const asymmetric = static_cast<IndependentRealRandomVariable<asymmetric_t>>(symmetric);

                CHECK(symmetric.value == doctest::Approx(mean_val(independent.value)));
                CHECK(symmetric.variance == doctest::Approx(mean_val(independent.variance) / 3.0));
            }
            SUBCASE("Conversion to UniformRealRandomVariable<symmetric_t>") {
                auto const uniform = static_cast<UniformRealRandomVariable<symmetric_t>>(independent);
                auto const via_asym_uniform = static_cast<UniformRealRandomVariable<symmetric_t>>(
                    static_cast<UniformRealRandomVariable<asymmetric_t>>(independent));
                auto const via_sym_independent = static_cast<UniformRealRandomVariable<symmetric_t>>(
                    static_cast<IndependentRealRandomVariable<symmetric_t>>(independent));

                CHECK(uniform.value == doctest::Approx(via_asym_uniform.value));
                CHECK(uniform.variance == doctest::Approx(via_asym_uniform.variance));
                CHECK(uniform.value == doctest::Approx(via_sym_independent.value));
                CHECK(uniform.variance == doctest::Approx(via_sym_independent.variance));
            }
        }
    }

    SUBCASE("UniformComplexRandomVariable<symmetric_t>") {
        for (auto const [real_value, imag_value, variance] :
             std::array{std::tuple{1.0, 0.0, 1.0}, std::tuple{2.0, 0.0, 3.0}, std::tuple{0.0, 1.0, 1.0},
                        std::tuple{0.0, 2.0, 1.0}, std::tuple{1.0, 1.0, 1.0}, std::tuple{2.0, 2.0, 3.0}}) {
            CAPTURE(real_value);
            CAPTURE(imag_value);
            CAPTURE(variance);

            UniformComplexRandomVariable<symmetric_t> const uniform{.value = {real_value, imag_value},
                                                                    .variance = variance};

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

    SUBCASE("UniformComplexRandomVariable<asymmetric_t>") {
        for (auto const [real_value, imag_value, variance] :
             std::array{std::tuple{AsymRealValue{1.0, 2.0, 3.0}, AsymRealValue{0.0, 0.0, 0.0}, 1.0},
                        std::tuple{AsymRealValue{2.0, 0.0, 0.0}, AsymRealValue{0.0, 3.0, 3.0}, 3.0},
                        std::tuple{AsymRealValue{0.0, 0.0, 0.0}, AsymRealValue{1.0, 1.0, 1.0}, 1.0},
                        std::tuple{AsymRealValue{0.0, -1.0, -2.0}, AsymRealValue{2.0, -1.0, -2.0}, 1.0},
                        std::tuple{AsymRealValue{1.0, 1.0, 1.0}, AsymRealValue{1.0, 1.0, 1.0}, 1.0},
                        std::tuple{AsymRealValue{2.0, 2.0, 2.0}, AsymRealValue{2.0, 2.0, 2.0}, 3.0}}) {
            CAPTURE(real_value);
            CAPTURE(imag_value);
            CAPTURE(variance);

            UniformComplexRandomVariable<asymmetric_t> const uniform{.value = {real_value, imag_value},
                                                                     .variance = variance};

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
    SUBCASE("IndependentComplexRandomVariable") {
        for (auto const [real_value, imag_value, variance] :
             std::array{std::tuple{1.0, 0.0, 1.0}, std::tuple{2.0, 0.0, 3.0}, std::tuple{0.0, 1.0, 1.0},
                        std::tuple{0.0, 2.0, 1.0}, std::tuple{1.0, 1.0, 1.0}, std::tuple{2.0, 2.0, 3.0}}) {
            CAPTURE(real_value);
            CAPTURE(imag_value);
            CAPTURE(variance);

            IndependentComplexRandomVariable<symmetric_t> const independent{.value = {real_value, imag_value},
                                                                            .variance = variance};

            SUBCASE("Constructor") {
                CHECK(real(independent.value) == real_value);
                CHECK(imag(independent.value) == imag_value);
                CHECK(independent.variance == variance);
            }
            SUBCASE("Conversion to UniformComplexRandomVariable") {
                auto const uniform = static_cast<UniformComplexRandomVariable<symmetric_t>>(independent);

                CHECK(real(uniform.value) == doctest::Approx(real(independent.value)));
                CHECK(imag(uniform.value) == doctest::Approx(imag(independent.value)));
                CHECK(uniform.variance == doctest::Approx(variance));
            }
        }
    }
    SUBCASE("DecomposedComplexRandomVariable") {
        for (auto const [real_value, real_variance, imag_value, imag_variance] : std::array{
                 std::tuple{1.0, 1.0, 0.0, 0.2}, std::tuple{2.0, 3.0, 0.0, 0.2}, std::tuple{0.0, 1.0, 1.0, 0.2},
                 std::tuple{0.0, 1.0, 2.0, 0.2}, std::tuple{1.0, 1.0, 1.0, 0.2}, std::tuple{2.0, 1.0, 2.0, 0.2}}) {
            CAPTURE(real_value);
            CAPTURE(real_variance);
            CAPTURE(imag_value);
            CAPTURE(imag_variance);

            DecomposedComplexRandomVariable<symmetric_t> const decomposed{
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
            SUBCASE("Conversion to UniformComplexRandomVariable") {
                auto const uniform = static_cast<UniformComplexRandomVariable<symmetric_t>>(decomposed);

                CHECK(real(uniform.value) == doctest::Approx(real(decomposed.value())));
                CHECK(imag(uniform.value) == doctest::Approx(imag(decomposed.value())));
                CHECK(uniform.variance == doctest::Approx(real_variance + imag_variance));
            }
            SUBCASE("Conversion to IndependentComplexRandomVariable") {
                auto const independent = static_cast<IndependentComplexRandomVariable<symmetric_t>>(decomposed);

                CHECK(real(independent.value) == doctest::Approx(real(decomposed.value())));
                CHECK(imag(independent.value) == doctest::Approx(imag(decomposed.value())));
                CHECK(independent.variance == doctest::Approx(real_variance + imag_variance));
            }
        }
    }

    SUBCASE("PolarComplexRandomVariable") {
        SUBCASE("Constructor") {
            for (auto const [magnitude, magnitude_variance, angle, angle_variance] :
                 std::array{std::tuple{1.0, 1.0, 0.0, 0.2}, std::tuple{2.0, 3.0, 0.0, 0.2},
                            std::tuple{1.0, 1.0, pi / 2, 0.2}, std::tuple{1.0, 1.0, pi / 4, 0.2}}) {
                CAPTURE(magnitude);
                CAPTURE(magnitude_variance);
                CAPTURE(angle);
                CAPTURE(angle_variance);

                PolarComplexRandomVariable<symmetric_t> const polar{
                    .magnitude = {.value = magnitude, .variance = magnitude_variance},
                    .angle = {.value = angle, .variance = angle_variance}};

                CHECK(polar.magnitude.value == magnitude);
                CHECK(polar.magnitude.variance == magnitude_variance);
                CHECK(polar.angle.value == angle);
                CHECK(polar.angle.variance == angle_variance);
            }
        }
        SUBCASE("Aggregate value") {
            for (auto const [magnitude, magnitude_variance, angle_variance] :
                 std::array{std::tuple{1.0, 1.0, 0.2}, std::tuple{2.0, 1.0, 0.2}, std::tuple{1.0, 3.0, 0.2},
                            std::tuple{1.0, 2.0, 0.4}}) {
                CAPTURE(magnitude);
                CAPTURE(magnitude_variance);
                CAPTURE(angle_variance);

                SUBCASE("No phase shift") {
                    PolarComplexRandomVariable<symmetric_t> const polar{
                        .magnitude = {.value = magnitude, .variance = magnitude_variance},
                        .angle = {.value = 0.0, .variance = angle_variance}};

                    CHECK(real(polar.value()) == doctest::Approx(polar.magnitude.value));
                    CHECK(imag(polar.value()) == doctest::Approx(0.0));
                }
                SUBCASE("Perpendicular phase shift") {
                    PolarComplexRandomVariable<symmetric_t> const polar{
                        .magnitude = {.value = magnitude, .variance = magnitude_variance},
                        .angle = {.value = pi / 2, .variance = angle_variance}};

                    CHECK(real(polar.value()) == doctest::Approx(0.0));
                    CHECK(imag(polar.value()) == doctest::Approx(polar.magnitude.value));
                }
                SUBCASE("45deg phase shift") {
                    PolarComplexRandomVariable<symmetric_t> const polar{
                        .magnitude = {.value = magnitude, .variance = magnitude_variance},
                        .angle = {.value = pi / 4, .variance = angle_variance}};

                    CHECK(real(polar.value()) == polar.magnitude.value * inv_sqrt2);
                    CHECK(imag(polar.value()) == real(polar.value()));
                }
            }
        }

        SUBCASE("Conversion to DecomposedComplexRandomVariable") {
            for (auto const [magnitude, magnitude_variance, angle_variance] :
                 std::array{std::tuple{1.0, 1.0, 0.2}, std::tuple{2.0, 1.0, 0.2}, std::tuple{1.0, 3.0, 0.2},
                            std::tuple{1.0, 2.0, 0.4}}) {
                CAPTURE(magnitude);
                CAPTURE(magnitude_variance);
                CAPTURE(angle_variance);

                SUBCASE("No phase shift") {
                    PolarComplexRandomVariable<symmetric_t> const polar{
                        .magnitude = {.value = magnitude, .variance = magnitude},
                        .angle = {.value = 0.0, .variance = angle_variance}};

                    auto const decomposed = static_cast<DecomposedComplexRandomVariable<symmetric_t>>(polar);

                    CHECK(decomposed.real_component.value == doctest::Approx(polar.magnitude.value));
                    CHECK(decomposed.imag_component.value == doctest::Approx(0.0));
                    CHECK(decomposed.real_component.variance == doctest::Approx(polar.magnitude.variance));
                    CHECK(decomposed.imag_component.variance ==
                          doctest::Approx(magnitude * magnitude * polar.angle.variance));
                    CHECK(real(decomposed.value()) == doctest::Approx(real(polar.value())));
                    CHECK(imag(decomposed.value()) == doctest::Approx(imag(polar.value())));
                }

                SUBCASE("Perpendicular phase shift") {
                    PolarComplexRandomVariable<symmetric_t> const polar{
                        .magnitude = {.value = magnitude, .variance = magnitude},
                        .angle = {.value = pi / 2, .variance = angle_variance}};

                    auto const decomposed = static_cast<DecomposedComplexRandomVariable<symmetric_t>>(polar);

                    CHECK(decomposed.real_component.value == doctest::Approx(0.0));
                    CHECK(decomposed.imag_component.value == doctest::Approx(polar.magnitude.value));
                    CHECK(decomposed.real_component.variance ==
                          doctest::Approx(magnitude * magnitude * polar.angle.variance));
                    CHECK(decomposed.imag_component.variance == doctest::Approx(polar.magnitude.variance));
                    CHECK(real(decomposed.value()) == doctest::Approx(real(polar.value())));
                    CHECK(imag(decomposed.value()) == doctest::Approx(imag(polar.value())));
                }

                SUBCASE("45deg phase shift") {
                    PolarComplexRandomVariable<symmetric_t> const polar{
                        .magnitude = {.value = magnitude, .variance = magnitude},
                        .angle = {.value = pi / 4, .variance = angle_variance}};

                    auto const decomposed = static_cast<DecomposedComplexRandomVariable<symmetric_t>>(polar);
                    auto const uniform = static_cast<UniformComplexRandomVariable<symmetric_t>>(polar);

                    CHECK(decomposed.real_component.value == doctest::Approx(real(uniform.value)));
                    CHECK(decomposed.imag_component.value == doctest::Approx(imag(uniform.value)));
                    CHECK(decomposed.real_component.variance == doctest::Approx(uniform.variance / 2));
                    CHECK(decomposed.imag_component.variance == doctest::Approx(decomposed.real_component.variance));
                    CHECK(real(decomposed.value()) == doctest::Approx(real(polar.value())));
                    CHECK(imag(decomposed.value()) == doctest::Approx(imag(polar.value())));
                }
            }
        }

        SUBCASE("Conversion to IndependentComplexRandomVariable") {
            for (auto const [magnitude, magnitude_variance, angle, angle_variance] :
                 std::array{std::tuple{1.0, 1.0, 0.0, 0.2}, std::tuple{2.0, 3.0, 0.0, 0.2},
                            std::tuple{1.0, 1.0, pi / 2, 0.2}, std::tuple{1.0, 1.0, pi / 4, 0.2}}) {
                CAPTURE(magnitude);
                CAPTURE(magnitude_variance);
                CAPTURE(angle);
                CAPTURE(angle_variance);

                PolarComplexRandomVariable<symmetric_t> const polar{
                    .magnitude = {.value = magnitude, .variance = magnitude_variance},
                    .angle = {.value = angle, .variance = angle_variance}};

                auto const independent = static_cast<IndependentComplexRandomVariable<symmetric_t>>(polar);

                CHECK(real(independent.value) == doctest::Approx(real(polar.value())));
                CHECK(imag(independent.value) == doctest::Approx(imag(polar.value())));
                CHECK(independent.variance ==
                      doctest::Approx(polar.magnitude.variance + magnitude * magnitude * polar.angle.variance));
            }
        }

        SUBCASE("Conversion to UniformComplexRandomVariable") {
            for (auto const [magnitude, magnitude_variance, angle, angle_variance] :
                 std::array{std::tuple{1.0, 1.0, 0.0, 0.2}, std::tuple{2.0, 3.0, 0.0, 0.2},
                            std::tuple{1.0, 1.0, pi / 2, 0.2}, std::tuple{1.0, 1.0, pi / 4, 0.2}}) {
                CAPTURE(magnitude);
                CAPTURE(magnitude_variance);
                CAPTURE(angle);
                CAPTURE(angle_variance);

                PolarComplexRandomVariable<symmetric_t> const polar{
                    .magnitude = {.value = magnitude, .variance = magnitude_variance},
                    .angle = {.value = angle, .variance = angle_variance}};

                auto const uniform = static_cast<UniformComplexRandomVariable<symmetric_t>>(polar);

                CHECK(real(uniform.value) == doctest::Approx(real(polar.value())));
                CHECK(imag(uniform.value) == doctest::Approx(imag(polar.value())));
                CHECK(uniform.variance ==
                      doctest::Approx(polar.magnitude.variance + magnitude * magnitude * polar.angle.variance));
            }
        }
    }
}
} // namespace power_grid_model
