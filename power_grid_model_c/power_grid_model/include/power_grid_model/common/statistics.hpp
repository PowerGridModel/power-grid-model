// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"
#include "three_phase_tensor.hpp"

namespace power_grid_model {
template <symmetry_tag sym_type> struct UniformRealRandomVariable {
    using sym = sym_type;

    static constexpr bool symmetric{is_symmetric_v<sym>};

    RealValue<sym> value{};
    double variance{}; // variance (sigma^2) of the error range, in p.u.

    explicit operator UniformRealRandomVariable<asymmetric_t>() const
        requires(is_symmetric_v<sym>)
    {
        return {.value = RealValue<asymmetric_t>{std::piecewise_construct, value}, .variance = variance};
    }
    explicit operator UniformRealRandomVariable<symmetric_t>() const
        requires(is_asymmetric_v<sym>)
    {
        return {.value = mean_val(value), .variance = variance / 3.0};
    }
};

template <symmetry_tag sym_type> struct IndependentRealRandomVariable {
    using sym = sym_type;

    static constexpr bool symmetric{is_symmetric_v<sym>};

    RealValue<sym> value{};
    RealValue<sym> variance{}; // variance (sigma^2) of the error range, in p.u.

    explicit operator UniformRealRandomVariable<symmetric_t>() const {
        constexpr auto scale = is_asymmetric_v<sym> ? 3.0 : 1.0;
        return {.value = mean_val(value), .variance = mean_val(variance) / scale};
    }
    explicit operator UniformRealRandomVariable<asymmetric_t>() const {
        return {.value = value, .variance = mean_val(variance)};
    }
    explicit operator IndependentRealRandomVariable<asymmetric_t>() const
        requires(is_symmetric_v<sym>)
    {
        return {.value = RealValue<asymmetric_t>{std::piecewise_construct, value},
                .variance = RealValue<asymmetric_t>{std::piecewise_construct, variance}};
    }
    explicit operator IndependentRealRandomVariable<symmetric_t>() const
        requires(is_asymmetric_v<sym>)
    {
        return {.value = mean_val(value), .variance = mean_val(variance) / 3.0};
    }
};

// Complex measured value of a sensor in p.u. with a uniform variance across all phases and axes of the complex plane
// (rotationally symmetric)
template <symmetry_tag sym_type> struct UniformComplexRandomVariable {
    using sym = sym_type;

    static constexpr bool symmetric{is_symmetric_v<sym>};

    ComplexValue<sym> value{};
    double variance{}; // variance (sigma^2) of the error range, in p.u.
};

inline UniformComplexRandomVariable<symmetric_t> pos_seq(UniformComplexRandomVariable<asymmetric_t> const& var) {
    return {.value = pos_seq(var.value), .variance = var.variance / 3.0};
}
inline UniformComplexRandomVariable<asymmetric_t> three_phase(UniformComplexRandomVariable<symmetric_t> const& var) {
    return {.value = ComplexValue<asymmetric_t>{var.value}, .variance = var.variance};
}

// Complex measured value of a sensor in p.u. with separate variances per phase (but rotationally symmetric in the
// complex plane)
template <symmetry_tag sym_type> struct IndependentComplexRandomVariable {
    using sym = sym_type;

    static constexpr bool symmetric{is_symmetric_v<sym>};

    ComplexValue<sym> value{};
    RealValue<sym> variance{}; // variance (sigma^2) of the error range, in p.u.

    explicit operator UniformComplexRandomVariable<sym>() const {
        return UniformComplexRandomVariable<sym>{.value = value, .variance = sum_val(variance)};
    }
};

// Complex measured value of a sensor in p.u. modeled as separate real and imaginary components with independent
// variances (rotationally symmetric)
template <symmetry_tag sym_type> struct DecomposedComplexRandomVariable {
    using sym = sym_type;

    static constexpr bool symmetric{is_symmetric_v<sym>};

    IndependentRealRandomVariable<sym> real_component;
    IndependentRealRandomVariable<sym> imag_component;

    ComplexValue<sym> value() const { return {real_component.value, imag_component.value}; }

    explicit operator UniformComplexRandomVariable<sym>() const {
        return static_cast<UniformComplexRandomVariable<sym>>(
            static_cast<IndependentComplexRandomVariable<sym>>(*this));
    }
    explicit operator IndependentComplexRandomVariable<sym>() const {
        return IndependentComplexRandomVariable<sym>{.value = value(),
                                                     .variance = real_component.variance + imag_component.variance};
    }
};

// Complex measured value of a sensor in p.u. in polar coordinates (magnitude and angle)
// (rotationally symmetric)
template <symmetry_tag sym_type> struct PolarComplexRandomVariable {
    using sym = sym_type;

    static constexpr bool symmetric{is_symmetric_v<sym>};

    UniformRealRandomVariable<sym> magnitude;
    UniformRealRandomVariable<sym> angle;

    ComplexValue<sym> value() const { return magnitude.value * exp(1.0i * angle.value); }

    explicit operator UniformComplexRandomVariable<sym>() const {
        return static_cast<UniformComplexRandomVariable<sym>>(
            static_cast<IndependentComplexRandomVariable<sym>>(*this));
    }
    explicit operator IndependentComplexRandomVariable<sym>() const {
        return IndependentComplexRandomVariable<sym>{
            .value = value(), .variance = magnitude.variance + magnitude.value * magnitude.value * angle.variance};
    }
    explicit operator DecomposedComplexRandomVariable<sym>() const {
        auto const cos_theta = cos(angle.value);
        auto const sin_theta = sin(angle.value);
        auto const real_component = magnitude.value * cos_theta;
        auto const imag_component = magnitude.value * sin_theta;
        return DecomposedComplexRandomVariable<sym>{
            .real_component = {.value = real_component,
                               .variance = magnitude.variance * cos_theta * cos_theta +
                                           imag_component * imag_component * angle.variance},
            .imag_component = {.value = imag_component,
                               .variance = magnitude.variance * sin_theta * sin_theta +
                                           real_component * real_component * angle.variance}};
    }
};
} // namespace power_grid_model
