// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/common/statistics.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {
namespace {
using std::numbers::sqrt2;
constexpr auto inv_sqrt2 = sqrt2 / 2;
} // namespace

TEST_CASE("Test statistics") {
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

        SUBCASE("Conversion to DecomposedIndependentComplexRandomVariable") {
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

                    auto const independent =
                        static_cast<DecomposedIndependentComplexRandomVariable<symmetric_t>>(polar);

                    CHECK(independent.real_component.value == doctest::Approx(polar.magnitude.value));
                    CHECK(independent.imag_component.value == doctest::Approx(0.0));
                    CHECK(independent.real_component.variance == doctest::Approx(polar.magnitude.variance));
                    CHECK(independent.imag_component.variance ==
                          doctest::Approx(magnitude * magnitude * polar.angle.variance));
                    CHECK(real(independent.value()) == doctest::Approx(real(polar.value())));
                    CHECK(imag(independent.value()) == doctest::Approx(imag(polar.value())));
                }

                SUBCASE("Perpendicular phase shift") {
                    PolarComplexRandomVariable<symmetric_t> const polar{
                        .magnitude = {.value = magnitude, .variance = magnitude},
                        .angle = {.value = pi / 2, .variance = angle_variance}};

                    auto const independent =
                        static_cast<DecomposedIndependentComplexRandomVariable<symmetric_t>>(polar);

                    CHECK(independent.real_component.value == doctest::Approx(0.0));
                    CHECK(independent.imag_component.value == doctest::Approx(polar.magnitude.value));
                    CHECK(independent.real_component.variance ==
                          doctest::Approx(magnitude * magnitude * polar.angle.variance));
                    CHECK(independent.imag_component.variance == doctest::Approx(polar.magnitude.variance));
                    CHECK(real(independent.value()) == doctest::Approx(real(polar.value())));
                    CHECK(imag(independent.value()) == doctest::Approx(imag(polar.value())));
                }

                SUBCASE("45deg phase shift") {
                    PolarComplexRandomVariable<symmetric_t> const polar{
                        .magnitude = {.value = magnitude, .variance = magnitude},
                        .angle = {.value = pi / 4, .variance = angle_variance}};

                    auto const independent =
                        static_cast<DecomposedIndependentComplexRandomVariable<symmetric_t>>(polar);
                    auto const uniform = static_cast<UniformComplexRandomVariable<symmetric_t>>(polar);

                    CHECK(independent.real_component.value == doctest::Approx(real(uniform.value)));
                    CHECK(independent.imag_component.value == doctest::Approx(imag(uniform.value)));
                    CHECK(independent.real_component.variance == doctest::Approx(uniform.variance / 2));
                    CHECK(independent.imag_component.variance == doctest::Approx(independent.real_component.variance));
                    CHECK(real(independent.value()) == doctest::Approx(real(polar.value())));
                    CHECK(imag(independent.value()) == doctest::Approx(imag(polar.value())));
                }
            }
        }
    }
}
} // namespace power_grid_model
