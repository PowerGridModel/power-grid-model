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
    SUBCASE("UniformRealRDV<symmetric_t>") {
        for (auto const [value, variance] :
             std::array{std::tuple{1.0, 1.0}, std::tuple{2.0, 3.0}, std::tuple{0.0, 1.0}, std::tuple{2.0, 3.0}}) {
            CAPTURE(value);
            CAPTURE(variance);

            UniformRealRDV<symmetric_t> const uniform{.value = value, .variance = variance};

            SUBCASE("Constructor") {
                CHECK(uniform.value == value);
                CHECK(uniform.variance == variance);
            }
            SUBCASE("Conversion to UniformRealRDV<asymmetric_t>") {
                auto const asymmetric = static_cast<UniformRealRDV<asymmetric_t>>(uniform);

                CHECK(asymmetric.value(0) == doctest::Approx(uniform.value));
                CHECK(asymmetric.value(1) == doctest::Approx(uniform.value));
                CHECK(asymmetric.value(2) == doctest::Approx(uniform.value));
                CHECK(asymmetric.variance == doctest::Approx(variance));
            }
        }
    }
    SUBCASE("UniformRealRDV<asymmetric_t>") {
        for (auto const [value_a, value_b, value_c, variance] :
             std::array{std::tuple{1.0, 2.0, 3.0, 1.0}, std::tuple{2.0, 2.1, 2.2, 3.0}, std::tuple{0.0, 0.1, 0.2, 1.0},
                        std::tuple{2.0, 0.0, 0.0, 3.0}}) {
            CAPTURE(value_a);
            CAPTURE(value_b);
            CAPTURE(value_c);
            CAPTURE(variance);

            UniformRealRDV<asymmetric_t> const uniform{.value = {value_a, value_b, value_c}, .variance = variance};

            SUBCASE("Constructor") {
                CHECK(uniform.value(0) == value_a);
                CHECK(uniform.value(1) == value_b);
                CHECK(uniform.value(2) == value_c);
                CHECK(uniform.variance == variance);
            }
            SUBCASE("Conversion to UniformRealRDV<symmetric_t>") {
                auto const symmetric = static_cast<UniformRealRDV<symmetric_t>>(uniform);

                CHECK(symmetric.value == doctest::Approx(mean_val(uniform.value)));
                CHECK(symmetric.variance == doctest::Approx(variance / 3));
            }
        }
    }

    SUBCASE("IndependentRealRDV<symmetric_t>") {
        for (auto const [value, variance] :
             std::array{std::tuple{1.0, 1.0}, std::tuple{2.0, 3.0}, std::tuple{0.0, 1.0}, std::tuple{2.0, 3.0}}) {
            CAPTURE(value);
            CAPTURE(variance);

            IndependentRealRDV<symmetric_t> const independent{.value = value, .variance = variance};

            SUBCASE("Constructor") {
                CHECK(independent.value == value);
                CHECK(independent.variance == variance);
            }
            SUBCASE("Conversion to IndependentRealRDV<asymmetric_t>") {
                auto const asymmetric = static_cast<IndependentRealRDV<asymmetric_t>>(independent);

                CHECK(asymmetric.value(0) == doctest::Approx(independent.value));
                CHECK(asymmetric.value(1) == doctest::Approx(independent.value));
                CHECK(asymmetric.value(2) == doctest::Approx(independent.value));
                CHECK(asymmetric.variance(0) == doctest::Approx(independent.variance));
                CHECK(asymmetric.variance(1) == doctest::Approx(independent.variance));
                CHECK(asymmetric.variance(2) == doctest::Approx(independent.variance));
            }
        }
    }
    SUBCASE("IndependentRealRDV<asymmetric_t>") {
        for (auto const [value_a, value_b, value_c, variance_a, variance_b, variance_c] :
             std::array{std::tuple{1.0, 2.0, 3.0, 1.0, 2.0, 3.0}, std::tuple{2.0, 2.1, 2.2, 3.0, 1.0, 2.0},
                        std::tuple{0.0, 0.1, 0.2, 1.0, 1.0, 1.0}, std::tuple{2.0, 0.0, 0.0, 3.0, 3.0, 3.0}}) {
            CAPTURE(value_a);
            CAPTURE(value_b);
            CAPTURE(value_c);
            CAPTURE(variance_a);
            CAPTURE(variance_b);
            CAPTURE(variance_c);

            IndependentRealRDV<asymmetric_t> const independent{.value = {value_a, value_b, value_c},
                                                               .variance = {variance_a, variance_b, variance_c}};

            SUBCASE("Constructor") {
                CHECK(independent.value(0) == value_a);
                CHECK(independent.value(1) == value_b);
                CHECK(independent.value(2) == value_c);
                CHECK(independent.variance(0) == variance_a);
                CHECK(independent.variance(1) == variance_b);
                CHECK(independent.variance(2) == variance_c);
            }
            SUBCASE("Conversion to IndependentRealRDV<symmetric_t>") {
                auto const symmetric = static_cast<IndependentRealRDV<symmetric_t>>(independent);

                CHECK(symmetric.value == doctest::Approx(mean_val(independent.value)));
                CHECK(symmetric.variance == doctest::Approx(mean_val(independent.variance) / 3.0));
            }
            SUBCASE("Conversion to UniformRealRDV<symmetric_t>") {
                auto const uniform = static_cast<UniformRealRDV<symmetric_t>>(independent);
                auto const via_asym_uniform =
                    static_cast<UniformRealRDV<symmetric_t>>(static_cast<UniformRealRDV<asymmetric_t>>(independent));
                auto const via_sym_independent =
                    static_cast<UniformRealRDV<symmetric_t>>(static_cast<IndependentRealRDV<symmetric_t>>(independent));

                CHECK(uniform.value == doctest::Approx(via_asym_uniform.value));
                CHECK(uniform.variance == doctest::Approx(via_asym_uniform.variance));
                CHECK(uniform.value == doctest::Approx(via_sym_independent.value));
                CHECK(uniform.variance == doctest::Approx(via_sym_independent.variance));
            }
        }
    }

    SUBCASE("UniformComplexRDV<symmetric_t>") {
        for (auto const [real_value, imag_value, variance] :
             std::array{std::tuple{1.0, 0.0, 1.0}, std::tuple{2.0, 0.0, 3.0}, std::tuple{0.0, 1.0, 1.0},
                        std::tuple{0.0, 2.0, 1.0}, std::tuple{1.0, 1.0, 1.0}, std::tuple{2.0, 2.0, 3.0}}) {
            CAPTURE(real_value);
            CAPTURE(imag_value);
            CAPTURE(variance);

            UniformComplexRDV<symmetric_t> const uniform{.value = {real_value, imag_value}, .variance = variance};

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

    SUBCASE("UniformComplexRDV<asymmetric_t>") {
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

            UniformComplexRDV<asymmetric_t> const uniform{.value = {real_value, imag_value}, .variance = variance};

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
    SUBCASE("IndependentComplexRDV") {
        for (auto const [real_value, imag_value, variance] :
             std::array{std::tuple{1.0, 0.0, 1.0}, std::tuple{2.0, 0.0, 3.0}, std::tuple{0.0, 1.0, 1.0},
                        std::tuple{0.0, 2.0, 1.0}, std::tuple{1.0, 1.0, 1.0}, std::tuple{2.0, 2.0, 3.0}}) {
            CAPTURE(real_value);
            CAPTURE(imag_value);
            CAPTURE(variance);

            IndependentComplexRDV<symmetric_t> const independent{.value = {real_value, imag_value},
                                                                 .variance = variance};

            SUBCASE("Constructor") {
                CHECK(real(independent.value) == real_value);
                CHECK(imag(independent.value) == imag_value);
                CHECK(independent.variance == variance);
            }
            SUBCASE("Conversion to UniformComplexRDV") {
                auto const uniform = static_cast<UniformComplexRDV<symmetric_t>>(independent);

                CHECK(real(uniform.value) == doctest::Approx(real(independent.value)));
                CHECK(imag(uniform.value) == doctest::Approx(imag(independent.value)));
                CHECK(uniform.variance == doctest::Approx(variance));
            }
        }
    }
    SUBCASE("DecomposedComplexRDV") {
        for (auto const [real_value, real_variance, imag_value, imag_variance] : std::array{
                 std::tuple{1.0, 1.0, 0.0, 0.2}, std::tuple{2.0, 3.0, 0.0, 0.2}, std::tuple{0.0, 1.0, 1.0, 0.2},
                 std::tuple{0.0, 1.0, 2.0, 0.2}, std::tuple{1.0, 1.0, 1.0, 0.2}, std::tuple{2.0, 1.0, 2.0, 0.2}}) {
            CAPTURE(real_value);
            CAPTURE(real_variance);
            CAPTURE(imag_value);
            CAPTURE(imag_variance);

            DecomposedComplexRDV<symmetric_t> const decomposed{
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
            SUBCASE("Conversion to UniformComplexRDV") {
                auto const uniform = static_cast<UniformComplexRDV<symmetric_t>>(decomposed);

                CHECK(real(uniform.value) == doctest::Approx(real(decomposed.value())));
                CHECK(imag(uniform.value) == doctest::Approx(imag(decomposed.value())));
                CHECK(uniform.variance == doctest::Approx(real_variance + imag_variance));
            }
            SUBCASE("Conversion to IndependentComplexRDV") {
                auto const independent = static_cast<IndependentComplexRDV<symmetric_t>>(decomposed);

                CHECK(real(independent.value) == doctest::Approx(real(decomposed.value())));
                CHECK(imag(independent.value) == doctest::Approx(imag(decomposed.value())));
                CHECK(independent.variance == doctest::Approx(real_variance + imag_variance));
            }
        }
    }

    SUBCASE("PolarComplexRDV") {
        SUBCASE("Constructor") {
            for (auto const [magnitude, magnitude_variance, angle, angle_variance] :
                 std::array{std::tuple{1.0, 1.0, 0.0, 0.2}, std::tuple{2.0, 3.0, 0.0, 0.2},
                            std::tuple{1.0, 1.0, pi / 2, 0.2}, std::tuple{1.0, 1.0, pi / 4, 0.2}}) {
                CAPTURE(magnitude);
                CAPTURE(magnitude_variance);
                CAPTURE(angle);
                CAPTURE(angle_variance);

                PolarComplexRDV<symmetric_t> const polar{
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
                    PolarComplexRDV<symmetric_t> const polar{
                        .magnitude = {.value = magnitude, .variance = magnitude_variance},
                        .angle = {.value = 0.0, .variance = angle_variance}};

                    CHECK(real(polar.value()) == doctest::Approx(polar.magnitude.value));
                    CHECK(imag(polar.value()) == doctest::Approx(0.0));
                }
                SUBCASE("Perpendicular phase shift") {
                    PolarComplexRDV<symmetric_t> const polar{
                        .magnitude = {.value = magnitude, .variance = magnitude_variance},
                        .angle = {.value = pi / 2, .variance = angle_variance}};

                    CHECK(real(polar.value()) == doctest::Approx(0.0));
                    CHECK(imag(polar.value()) == doctest::Approx(polar.magnitude.value));
                }
                SUBCASE("45deg phase shift") {
                    PolarComplexRDV<symmetric_t> const polar{
                        .magnitude = {.value = magnitude, .variance = magnitude_variance},
                        .angle = {.value = pi / 4, .variance = angle_variance}};

                    CHECK(real(polar.value()) == doctest::Approx(polar.magnitude.value * inv_sqrt2));
                    CHECK(imag(polar.value()) == doctest::Approx(real(polar.value())));
                }
            }
        }

        SUBCASE("Conversion to DecomposedComplexRDV") {
            for (auto const [magnitude, magnitude_variance, angle_variance] :
                 std::array{std::tuple{1.0, 1.0, 0.2}, std::tuple{2.0, 1.0, 0.2}, std::tuple{1.0, 3.0, 0.2},
                            std::tuple{1.0, 2.0, 0.4}}) {
                CAPTURE(magnitude);
                CAPTURE(magnitude_variance);
                CAPTURE(angle_variance);

                SUBCASE("No phase shift") {
                    PolarComplexRDV<symmetric_t> const polar{.magnitude = {.value = magnitude, .variance = magnitude},
                                                             .angle = {.value = 0.0, .variance = angle_variance}};

                    auto const decomposed = static_cast<DecomposedComplexRDV<symmetric_t>>(polar);

                    CHECK(decomposed.real_component.value == doctest::Approx(polar.magnitude.value));
                    CHECK(decomposed.imag_component.value == doctest::Approx(0.0));
                    CHECK(decomposed.real_component.variance == doctest::Approx(polar.magnitude.variance));
                    CHECK(decomposed.imag_component.variance ==
                          doctest::Approx(magnitude * magnitude * polar.angle.variance));
                    CHECK(real(decomposed.value()) == doctest::Approx(real(polar.value())));
                    CHECK(imag(decomposed.value()) == doctest::Approx(imag(polar.value())));
                }

                SUBCASE("Perpendicular phase shift") {
                    PolarComplexRDV<symmetric_t> const polar{.magnitude = {.value = magnitude, .variance = magnitude},
                                                             .angle = {.value = pi / 2, .variance = angle_variance}};

                    auto const decomposed = static_cast<DecomposedComplexRDV<symmetric_t>>(polar);

                    CHECK(decomposed.real_component.value == doctest::Approx(0.0));
                    CHECK(decomposed.imag_component.value == doctest::Approx(polar.magnitude.value));
                    CHECK(decomposed.real_component.variance ==
                          doctest::Approx(magnitude * magnitude * polar.angle.variance));
                    CHECK(decomposed.imag_component.variance == doctest::Approx(polar.magnitude.variance));
                    CHECK(real(decomposed.value()) == doctest::Approx(real(polar.value())));
                    CHECK(imag(decomposed.value()) == doctest::Approx(imag(polar.value())));
                }

                SUBCASE("45deg phase shift") {
                    PolarComplexRDV<symmetric_t> const polar{.magnitude = {.value = magnitude, .variance = magnitude},
                                                             .angle = {.value = pi / 4, .variance = angle_variance}};

                    auto const decomposed = static_cast<DecomposedComplexRDV<symmetric_t>>(polar);
                    auto const uniform = static_cast<UniformComplexRDV<symmetric_t>>(polar);

                    CHECK(decomposed.real_component.value == doctest::Approx(real(uniform.value)));
                    CHECK(decomposed.imag_component.value == doctest::Approx(imag(uniform.value)));
                    CHECK(decomposed.real_component.variance == doctest::Approx(uniform.variance / 2));
                    CHECK(decomposed.imag_component.variance == doctest::Approx(decomposed.real_component.variance));
                    CHECK(real(decomposed.value()) == doctest::Approx(real(polar.value())));
                    CHECK(imag(decomposed.value()) == doctest::Approx(imag(polar.value())));
                }
            }
        }

        SUBCASE("Conversion to IndependentComplexRDV") {
            for (auto const [magnitude, magnitude_variance, angle, angle_variance] :
                 std::array{std::tuple{1.0, 1.0, 0.0, 0.2}, std::tuple{2.0, 3.0, 0.0, 0.2},
                            std::tuple{1.0, 1.0, pi / 2, 0.2}, std::tuple{1.0, 1.0, pi / 4, 0.2}}) {
                CAPTURE(magnitude);
                CAPTURE(magnitude_variance);
                CAPTURE(angle);
                CAPTURE(angle_variance);

                PolarComplexRDV<symmetric_t> const polar{
                    .magnitude = {.value = magnitude, .variance = magnitude_variance},
                    .angle = {.value = angle, .variance = angle_variance}};

                auto const independent = static_cast<IndependentComplexRDV<symmetric_t>>(polar);

                CHECK(real(independent.value) == doctest::Approx(real(polar.value())));
                CHECK(imag(independent.value) == doctest::Approx(imag(polar.value())));
                CHECK(independent.variance ==
                      doctest::Approx(polar.magnitude.variance + magnitude * magnitude * polar.angle.variance));
            }
        }

        SUBCASE("Conversion to UniformComplexRDV") {
            for (auto const [magnitude, magnitude_variance, angle, angle_variance] :
                 std::array{std::tuple{1.0, 1.0, 0.0, 0.2}, std::tuple{2.0, 3.0, 0.0, 0.2},
                            std::tuple{1.0, 1.0, pi / 2, 0.2}, std::tuple{1.0, 1.0, pi / 4, 0.2}}) {
                CAPTURE(magnitude);
                CAPTURE(magnitude_variance);
                CAPTURE(angle);
                CAPTURE(angle_variance);

                PolarComplexRDV<symmetric_t> const polar{
                    .magnitude = {.value = magnitude, .variance = magnitude_variance},
                    .angle = {.value = angle, .variance = angle_variance}};

                auto const uniform = static_cast<UniformComplexRDV<symmetric_t>>(polar);

                CHECK(real(uniform.value) == doctest::Approx(real(polar.value())));
                CHECK(imag(uniform.value) == doctest::Approx(imag(polar.value())));
                CHECK(uniform.variance ==
                      doctest::Approx(polar.magnitude.variance + magnitude * magnitude * polar.angle.variance));
            }
        }
    }

    SUBCASE("PolarComplexRDV Asym") {
        SUBCASE("Constructor") {
            for (auto const [magnitude_a, magnitude_b, magnitude_c, magnitude_variance, angle_a, angle_b, angle_c,
                             angle_variance] : std::array{std::tuple{1.0, 2.0, 3.0, 0.2, 0.0, pi / 4, pi / 2, 0.2},
                                                          std::tuple{2.0, 3.0, 4.0, 0.3, 0.0, pi / 6, pi / 3, 0.3}}) {
                CAPTURE(magnitude_a);
                CAPTURE(magnitude_b);
                CAPTURE(magnitude_c);
                CAPTURE(magnitude_variance);
                CAPTURE(angle_a);
                CAPTURE(angle_b);
                CAPTURE(angle_c);
                CAPTURE(angle_variance);

                PolarComplexRDV<asymmetric_t> const polar{
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
            for (auto const [magnitude_a, magnitude_b, magnitude_c, magnitude_variance, angle_variance] :
                 std::array{std::tuple{1.0, 1.0, 1.0, 1.0, 0.2}, std::tuple{2.0, 2.0, 2.0, 1.0, 0.2},
                            std::tuple{1.0, 1.0, 1.0, 3.0, 0.2}, std::tuple{1.0, 1.0, 1.0, 2.0, 0.4}}) {
                CAPTURE(magnitude_a);
                CAPTURE(magnitude_b);
                CAPTURE(magnitude_c);
                CAPTURE(magnitude_variance);
                CAPTURE(angle_variance);

                SUBCASE("No phase shift") {
                    PolarComplexRDV<asymmetric_t> const polar{
                        .magnitude = {.value = {magnitude_a, magnitude_b, magnitude_c}, .variance = magnitude_variance},
                        .angle = {.value = {0.0, deg_240, deg_120}, .variance = angle_variance}};

                    CHECK(real(polar.value()(0)) == doctest::Approx(polar.magnitude.value(0)));
                    CHECK(imag(polar.value()(0)) == doctest::Approx(0.0));
                    CHECK(real(polar.value()(1)) == doctest::Approx(polar.magnitude.value(1) * -0.5));
                    CHECK(imag(polar.value()(1)) == doctest::Approx(polar.magnitude.value(1) * -sqrt3 / 2));
                    CHECK(real(polar.value()(2)) == doctest::Approx(polar.magnitude.value(2) * -0.5));
                    CHECK(imag(polar.value()(2)) == doctest::Approx(polar.magnitude.value(2) * sqrt3 / 2));
                }
                SUBCASE("Perpendicular phase shift") {
                    PolarComplexRDV<asymmetric_t> const polar{
                        .magnitude = {.value = {magnitude_a, magnitude_b, magnitude_c}, .variance = magnitude_variance},
                        .angle = {.value = {pi / 2, deg_240 + pi / 2, deg_120 + pi / 2}, .variance = angle_variance}};

                    CHECK(real(polar.value()(0)) == doctest::Approx(0.0));
                    CHECK(imag(polar.value()(0)) == doctest::Approx(polar.magnitude.value(0)));
                    CHECK(real(polar.value()(1)) == doctest::Approx(polar.magnitude.value(1) * sqrt3 / 2));
                    CHECK(imag(polar.value()(1)) == doctest::Approx(polar.magnitude.value(1) * -0.5));
                    CHECK(real(polar.value()(2)) == doctest::Approx(polar.magnitude.value(2) * -sqrt3 / 2));
                    CHECK(imag(polar.value()(2)) == doctest::Approx(polar.magnitude.value(2) * -0.5));
                }
                SUBCASE("45deg phase shift") {
                    PolarComplexRDV<asymmetric_t> const polar{
                        .magnitude = {.value = {magnitude_a, magnitude_b, magnitude_c}, .variance = magnitude_variance},
                        .angle = {.value = {pi / 4, deg_240 + pi / 4, deg_120 + pi / 4}, .variance = angle_variance}};

                    CHECK(real(polar.value()(0)) == doctest::Approx(polar.magnitude.value(0) * inv_sqrt2));
                    CHECK(imag(polar.value()(0)) == doctest::Approx(polar.magnitude.value(0) * inv_sqrt2));
                    CHECK(real(polar.value()(1)) == doctest::Approx(polar.magnitude.value(1) * 0.2588190451));
                    CHECK(imag(polar.value()(1)) == doctest::Approx(polar.magnitude.value(1) * -0.9659258263));
                    CHECK(real(polar.value()(2)) == doctest::Approx(polar.magnitude.value(2) * -0.9659258263));
                    CHECK(imag(polar.value()(2)) == doctest::Approx(polar.magnitude.value(2) * 0.2588190451));
                }
            }
        }
    }

    SUBCASE("DecomposedComplexRDV Asym") {
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

            DecomposedComplexRDV<asymmetric_t> const decomposed{
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
            SUBCASE("Conversion to UniformComplexRDV") {
                auto const uniform = static_cast<UniformComplexRDV<asymmetric_t>>(decomposed);

                CHECK(real(uniform.value(0)) == doctest::Approx(real(decomposed.value()(0))));
                CHECK(imag(uniform.value(0)) == doctest::Approx(imag(decomposed.value()(0))));
                CHECK(real(uniform.value(1)) == doctest::Approx(real(decomposed.value()(1))));
                CHECK(imag(uniform.value(1)) == doctest::Approx(imag(decomposed.value()(1))));
                CHECK(real(uniform.value(2)) == doctest::Approx(real(decomposed.value()(2))));
                CHECK(imag(uniform.value(2)) == doctest::Approx(imag(decomposed.value()(2))));
                CHECK(uniform.variance == doctest::Approx((real_variance_a + real_variance_b + real_variance_c +
                                                           imag_variance_a + imag_variance_b + imag_variance_c) /
                                                          3.0));
            }
            SUBCASE("Conversion to IndependentComplexRDV") {
                auto const independent = static_cast<IndependentComplexRDV<asymmetric_t>>(decomposed);

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
}

} // namespace power_grid_model
