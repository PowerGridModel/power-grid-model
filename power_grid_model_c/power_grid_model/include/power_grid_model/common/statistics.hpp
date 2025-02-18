// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"
#include "three_phase_tensor.hpp"

/**
 * @file statistics.hpp
 * @brief This file contains various structures and functions for handling statistical representations of
 * randomly distributed variables(RDV) used in the Power Grid Model, like in the State Estimation algorithms to
 * handle measurements.
 *
 * The structures provided in this file are used to represent measured values of sensors
 * with different types of variances. These structures support both symmetric and asymmetric representations and
 * provide conversion operators to transform between these representations.
 *
 * A Randomly distributed variable in PGM can have following characteristics:
 *  - Uniform: Single Variance for all phases
 *  - Independent: Unique Variance for each phase
 *  - Real: The Real value without direction, eg. real axis: RealValue (* 1), imaginary axis: RealValue (* 1i).
 *  - Complex: A combined complex value in `a + bi` notation.
 *
 * Based on these, we use combine variables in Polar/Decomposed forms:
 * - Decomposed: Treat RDV individually as in cartesian co-ordinates with a separate variance for both real and
 * complex component.
 * - Polar: RDV is in polar co-ordinates, with magnitude and angle.
 *
 */

namespace power_grid_model {
template <symmetry_tag sym_type> struct UniformRealRDV {
    using sym = sym_type;

    static constexpr bool symmetric{is_symmetric_v<sym>};

    RealValue<sym> value{};
    double variance{}; // variance (sigma^2) of the error range, in p.u.

    explicit operator UniformRealRDV<asymmetric_t>() const
        requires(is_symmetric_v<sym>)
    {
        return {.value = RealValue<asymmetric_t>{std::piecewise_construct, value}, .variance = variance};
    }
    explicit operator UniformRealRDV<symmetric_t>() const
        requires(is_asymmetric_v<sym>)
    {
        return {.value = mean_val(value), .variance = variance / 3.0};
    }
};

template <symmetry_tag sym_type> struct IndependentRealRDV {
    using sym = sym_type;

    static constexpr bool symmetric{is_symmetric_v<sym>};

    RealValue<sym> value{};
    RealValue<sym> variance{}; // variance (sigma^2) of the error range, in p.u.

    explicit operator UniformRealRDV<symmetric_t>() const {
        constexpr auto scale = is_asymmetric_v<sym> ? 3.0 : 1.0;
        return {.value = mean_val(value), .variance = mean_val(variance) / scale};
    }
    explicit operator UniformRealRDV<asymmetric_t>() const { return {.value = value, .variance = mean_val(variance)}; }
    explicit operator IndependentRealRDV<asymmetric_t>() const
        requires(is_symmetric_v<sym>)
    {
        return {.value = RealValue<asymmetric_t>{std::piecewise_construct, value},
                .variance = RealValue<asymmetric_t>{std::piecewise_construct, variance}};
    }
    explicit operator IndependentRealRDV<symmetric_t>() const
        requires(is_asymmetric_v<sym>)
    {
        return {.value = mean_val(value), .variance = mean_val(variance) / 3.0};
    }
};

// Complex measured value of a sensor in p.u. with a uniform variance across all phases and axes of the complex plane
// (rotationally symmetric)
template <symmetry_tag sym_type> struct UniformComplexRDV {
    using sym = sym_type;

    static constexpr bool symmetric{is_symmetric_v<sym>};

    ComplexValue<sym> value{};
    double variance{}; // variance (sigma^2) of the error range, in p.u.
};

inline UniformComplexRDV<symmetric_t> pos_seq(UniformComplexRDV<asymmetric_t> const& var) {
    return {.value = pos_seq(var.value), .variance = var.variance / 3.0};
}
inline UniformComplexRDV<asymmetric_t> three_phase(UniformComplexRDV<symmetric_t> const& var) {
    return {.value = ComplexValue<asymmetric_t>{var.value}, .variance = var.variance};
}

// Complex measured value of a sensor in p.u. with separate variances per phase (but rotationally symmetric in the
// complex plane)
template <symmetry_tag sym_type> struct IndependentComplexRDV {
    using sym = sym_type;

    static constexpr bool symmetric{is_symmetric_v<sym>};

    ComplexValue<sym> value{};
    RealValue<sym> variance{}; // variance (sigma^2) of the error range, in p.u.

    explicit operator UniformComplexRDV<sym>() const {
        return UniformComplexRDV<sym>{.value = value, .variance = sum_val(variance)};
    }
};

// Complex measured value of a sensor in p.u. modeled as separate real and imaginary components with independent
// variances (rotationally symmetric)
template <symmetry_tag sym_type> struct DecomposedComplexRDV {
    using sym = sym_type;

    static constexpr bool symmetric{is_symmetric_v<sym>};

    IndependentRealRDV<sym> real_component;
    IndependentRealRDV<sym> imag_component;

    ComplexValue<sym> value() const { return {real_component.value, imag_component.value}; }

    explicit operator UniformComplexRDV<sym>() const {
        return static_cast<UniformComplexRDV<sym>>(static_cast<IndependentComplexRDV<sym>>(*this));
    }
    explicit operator IndependentComplexRDV<sym>() const {
        return IndependentComplexRDV<sym>{.value = value(),
                                          .variance = real_component.variance + imag_component.variance};
    }
    explicit operator DecomposedComplexRDV<asymmetric_t>() const
        requires(is_symmetric_v<sym>)
    {
        return DecomposedComplexRDV<asymmetric_t>{
            .real_component = static_cast<IndependentRealRDV<asymmetric_t>>(real_component),
            .imag_component = static_cast<IndependentRealRDV<asymmetric_t>>(imag_component)};
    }
    explicit operator DecomposedComplexRDV<symmetric_t>() const
        requires(is_asymmetric_v<sym>)
    {
        return DecomposedComplexRDV<symmetric_t>{
            .real_component = static_cast<IndependentRealRDV<symmetric_t>>(real_component),
            .imag_component = static_cast<IndependentRealRDV<symmetric_t>>(imag_component)};
    }
};

// Complex measured value of a sensor in p.u. in polar coordinates (magnitude and angle)
// (rotationally symmetric)
template <symmetry_tag sym_type> struct PolarComplexRDV {
    using sym = sym_type;

    static constexpr bool symmetric{is_symmetric_v<sym>};

    UniformRealRDV<sym> magnitude;
    UniformRealRDV<sym> angle;

    ComplexValue<sym> value() const { return magnitude.value * exp(1.0i * angle.value); }

    explicit operator UniformComplexRDV<sym>() const {
        return static_cast<UniformComplexRDV<sym>>(static_cast<IndependentComplexRDV<sym>>(*this));
    }
    explicit operator IndependentComplexRDV<sym>() const {
        return IndependentComplexRDV<sym>{
            .value = value(), .variance = magnitude.variance + magnitude.value * magnitude.value * angle.variance};
    }
    explicit operator DecomposedComplexRDV<sym>() const {
        auto const cos_theta = cos(angle.value);
        auto const sin_theta = sin(angle.value);
        auto const real_component = magnitude.value * cos_theta;
        auto const imag_component = magnitude.value * sin_theta;
        return DecomposedComplexRDV<sym>{
            .real_component = {.value = real_component,
                               .variance = magnitude.variance * cos_theta * cos_theta +
                                           imag_component * imag_component * angle.variance},
            .imag_component = {.value = imag_component,
                               .variance = magnitude.variance * sin_theta * sin_theta +
                                           real_component * real_component * angle.variance}};
    }
};
} // namespace power_grid_model
