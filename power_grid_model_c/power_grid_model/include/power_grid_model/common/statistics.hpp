// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"
#include "three_phase_tensor.hpp"

/**
 * @file statistics.hpp
 * @brief This file contains various structures and functions for handling statistical representations of
 * random variables (RandVar) used in the Power Grid Model, like in the State Estimation algorithms to
 * handle measurements.
 *
 * The structures provided in this file are used to represent measured values of sensors
 * with different types of variances. These structures support both symmetric and asymmetric representations and
 * provide conversion operators to transform between these representations.
 *
 * A random variable in PGM can have following characteristics:
 *  - Uniform: Single total variance for all phases
 *  - Independent: all phases are independent from each other
 *  - Scalar: Named as `Real` in this file, a scalar value `RealValue`, eg. real axis: RealValue (* 1), imaginary axis:
 * RealValue (* i).
 *  - Complex: a complex value with real and imaginary parts.
 *
 * Based on these, we use combined variables in Decomposed/Polar forms:
 * - Decomposed: treat random variables individually as in cartesian co-ordinates with separated variances for both real
 * and imaginary part.
 * - Polar: random variables are in polar co-ordinates, with magnitudes and angles.
 *
 */

namespace power_grid_model {
template <symmetry_tag sym_type> struct UniformRealRandVar {
    using sym = sym_type;

    RealValue<sym> value{};
    double variance{}; // variance (sigma^2) of the error range

    explicit operator UniformRealRandVar<asymmetric_t>() const
        requires(is_symmetric_v<sym>)
    {
        return {.value = RealValue<asymmetric_t>{std::piecewise_construct, value}, .variance = variance};
    }
    explicit operator UniformRealRandVar<symmetric_t>() const
        requires(is_asymmetric_v<sym>)
    {
        return {.value = mean_val(value), .variance = variance / 3.0};
    }
};

template <symmetry_tag sym_type> struct IndependentRealRandVar {
    using sym = sym_type;

    RealValue<sym> value{};
    RealValue<sym> variance{}; // variance (sigma^2) of the error range

    explicit operator UniformRealRandVar<symmetric_t>() const {
        constexpr auto scale = is_asymmetric_v<sym> ? 3.0 : 1.0;
        return {.value = mean_val(value), .variance = mean_val(variance) / scale};
    }
    explicit operator UniformRealRandVar<asymmetric_t>() const {
        return {.value = value, .variance = mean_val(variance)};
    }
    explicit operator IndependentRealRandVar<asymmetric_t>() const
        requires(is_symmetric_v<sym>)
    {
        return {.value = RealValue<asymmetric_t>{std::piecewise_construct, value},
                .variance = RealValue<asymmetric_t>{std::piecewise_construct, variance}};
    }
    explicit operator IndependentRealRandVar<symmetric_t>() const
        requires(is_asymmetric_v<sym>)
    {
        return {.value = mean_val(value), .variance = mean_val(variance) / 3.0};
    }
};

// Complex measured value of a sensor with a uniform variance across all phases and axes of the complex plane
// (rotationally symmetric)
template <symmetry_tag sym_type> struct UniformComplexRandVar {
    using sym = sym_type;

    ComplexValue<sym> value{};
    double variance{}; // variance (sigma^2) of the error range
};

inline UniformComplexRandVar<symmetric_t> pos_seq(UniformComplexRandVar<asymmetric_t> const& var) {
    return {.value = pos_seq(var.value), .variance = var.variance / 3.0};
}
inline UniformComplexRandVar<asymmetric_t> three_phase(UniformComplexRandVar<symmetric_t> const& var) {
    return {.value = ComplexValue<asymmetric_t>{var.value}, .variance = var.variance};
}

// Complex measured value of a sensor with separate variances per phase (but rotationally symmetric in the
// complex plane)
template <symmetry_tag sym_type> struct IndependentComplexRandVar {
    using sym = sym_type;

    ComplexValue<sym> value{};
    RealValue<sym> variance{}; // variance (sigma^2) of the error range

    explicit operator UniformComplexRandVar<sym>() const {
        return UniformComplexRandVar<sym>{.value = value, .variance = sum_val(variance)};
    }
};

// Complex measured value of a sensor modeled as separate real and imaginary components with independent
// variances (rotationally symmetric)
template <symmetry_tag sym_type> struct DecomposedComplexRandVar {
    using sym = sym_type;

    IndependentRealRandVar<sym> real_component;
    IndependentRealRandVar<sym> imag_component;

    ComplexValue<sym> value() const { return {real_component.value, imag_component.value}; }

    explicit operator UniformComplexRandVar<sym>() const {
        return static_cast<UniformComplexRandVar<sym>>(static_cast<IndependentComplexRandVar<sym>>(*this));
    }
    explicit operator IndependentComplexRandVar<sym>() const {
        return IndependentComplexRandVar<sym>{.value = value(),
                                              .variance = real_component.variance + imag_component.variance};
    }
};

// Complex measured value of a sensor in polar coordinates (magnitude and angle)
// (rotationally symmetric)
template <symmetry_tag sym_type> struct PolarComplexRandVar {
    using sym = sym_type;

    UniformRealRandVar<sym> magnitude;
    UniformRealRandVar<sym> angle;

    ComplexValue<sym> value() const { return magnitude.value * exp(1.0i * angle.value); }

    explicit operator UniformComplexRandVar<sym>() const {
        return static_cast<UniformComplexRandVar<sym>>(static_cast<IndependentComplexRandVar<sym>>(*this));
    }
    explicit operator IndependentComplexRandVar<sym>() const {
        return IndependentComplexRandVar<sym>{
            .value = value(), .variance = magnitude.variance + magnitude.value * magnitude.value * angle.variance};
    }

    // For sym to sym conversion:
    // Var(I_Re) ≈ Var(I) * cos^2(θ) + Var(θ) * I^2 * sin^2(θ)
    // Var(I_Im) ≈ Var(I) * sin^2(θ) + Var(θ) * I^2 * cos^2(θ)
    // For asym to asym conversion:
    // Var(I_Re,p) ≈ Var(I_p) * cos^2(θ_p) + Var(θ_p) * I_p^2 * sin^2(θ_p)
    // Var(I_Im,p) ≈ Var(I_p) * sin^2(θ_p) + Var(θ_p) * I_p^2 * cos^2(θ_p)
    explicit operator DecomposedComplexRandVar<sym>() const {
        auto const cos_theta = cos(angle.value);
        auto const sin_theta = sin(angle.value);
        auto const real_component = magnitude.value * cos_theta;
        auto const imag_component = magnitude.value * sin_theta;
        return DecomposedComplexRandVar<sym>{
            .real_component = {.value = real_component,
                               .variance = magnitude.variance * cos_theta * cos_theta +
                                           imag_component * imag_component * angle.variance},
            .imag_component = {.value = imag_component,
                               .variance = magnitude.variance * sin_theta * sin_theta +
                                           real_component * real_component * angle.variance}};
    }

    // Var(I_Re,p) ≈ Var(I) * cos^2(θ - 2πp/3) + Var(θ) * I^2 * sin^2(θ - 2πp/3)
    // Var(I_Im,p) ≈ Var(I) * sin^2(θ - 2πp/3) + Var(θ) * I^2 * cos^2(θ - 2πp/3)
    explicit operator DecomposedComplexRandVar<asymmetric_t>() const
        requires(is_symmetric_v<sym>)
    {
        ComplexValue<asymmetric_t> const unit_complex{exp(1.0i * angle.value)};
        ComplexValue<asymmetric_t> const complex = magnitude.value * unit_complex;
        return DecomposedComplexRandVar<asymmetric_t>{
            .real_component = {.value = real(complex),
                               .variance = magnitude.variance * real(unit_complex) * real(unit_complex) +
                                           imag(complex) * imag(complex) * angle.variance},
            .imag_component = {.value = imag(complex),
                               .variance = magnitude.variance * imag(unit_complex) * imag(unit_complex) +
                                           real(complex) * real(complex) * angle.variance}};
    }

    // Var(I_Re) ≈ (1 / 9) * sum_p(Var(I_p) * cos^2(theta_p + 2 * pi * p / 3) + Var(theta_p) * I_p^2 * sin^2(theta_p + 2
    // * pi * p / 3))
    // Var(I_Im) ≈ (1 / 9) * sum_p(Var(I_p) * sin^2(theta_p + 2 * pi * p / 3) + Var(theta_p) * I_p^2 *
    // cos^2(theta_p + 2 * pi * p / 3))
    explicit operator DecomposedComplexRandVar<symmetric_t>() const
        requires(is_asymmetric_v<sym>)
    {
        ComplexValue<asymmetric_t> const unit_complex{exp(1.0i * angle.value)};
        ComplexValue<asymmetric_t> const unit_pos_seq_per_phase{unit_complex(0), a * unit_complex(1),
                                                                a2 * unit_complex(2)};
        DoubleComplex const pos_seq_value = pos_seq(magnitude.value * unit_complex);
        return DecomposedComplexRandVar<symmetric_t>{
            .real_component = {.value = real(pos_seq_value),
                               .variance = sum_val(magnitude.variance * real(unit_pos_seq_per_phase) *
                                                       real(unit_pos_seq_per_phase) +
                                                   imag(unit_pos_seq_per_phase) * imag(unit_pos_seq_per_phase) *
                                                       magnitude.value * magnitude.value * angle.variance) /
                                           9.0},
            .imag_component = {
                .value = imag(pos_seq_value),
                .variance = sum_val(magnitude.variance * imag(unit_pos_seq_per_phase) * imag(unit_pos_seq_per_phase) +
                                    real(unit_pos_seq_per_phase) * real(unit_pos_seq_per_phase) * magnitude.value *
                                        magnitude.value * angle.variance) /
                            9.0}};
    }
};
} // namespace power_grid_model
