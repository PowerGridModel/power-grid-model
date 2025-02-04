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
};

template <symmetry_tag sym_type> struct IndependentRealRandomVariable {
    using sym = sym_type;

    static constexpr bool symmetric{is_symmetric_v<sym>};

    RealValue<sym> value{};
    RealValue<sym> variance{}; // variance (sigma^2) of the error range, in p.u.
};

// Complex measured value of a sensor in p.u. with a uniform variance across all phases and axes of the complex plane
// (circularly symmetric)
template <symmetry_tag sym_type> struct UniformComplexRandomVariable {
    using sym = sym_type;

    static constexpr bool symmetric{is_symmetric_v<sym>};

    ComplexValue<sym> value{};
    double variance{}; // variance (sigma^2) of the error range, in p.u.
};

// Complex measured value of a sensor in p.u. modeled as separate real and imaginary components with uniform variances
// (circularly symmetric)
template <symmetry_tag sym_type> struct DecomposedUniformComplexRandomVariable {
    using sym = sym_type;

    static constexpr bool symmetric{is_symmetric_v<sym>};

    UniformRealRandomVariable<sym> real_component;
    UniformRealRandomVariable<sym> imag_component;

    ComplexValue<sym> value() const { return {real_component.value, imag_component.value}; }
};

// Complex measured value of a sensor in p.u. modeled as separate real and imaginary components with independent
// variances (circularly symmetric)
template <symmetry_tag sym_type> struct DecomposedIndependentComplexRandomVariable {
    using sym = sym_type;

    static constexpr bool symmetric{is_symmetric_v<sym>};

    IndependentRealRandomVariable<sym> real_component;
    IndependentRealRandomVariable<sym> imag_component;

    ComplexValue<sym> value() const { return {real_component.value, imag_component.value}; }
};

// Complex measured value of a sensor in p.u. in polar coordinates (magnitude and angle)
// (circularly symmetric)
template <symmetry_tag sym_type> struct PolarComplexRandomVariable {
    using sym = sym_type;

    static constexpr bool symmetric{is_symmetric_v<sym>};

    UniformRealRandomVariable<sym> magnitude;
    UniformRealRandomVariable<sym> angle;

    ComplexValue<sym> value() const { return magnitude.value * exp(1.0i * angle.value); }

    explicit operator UniformComplexRandomVariable<sym>() const {
        return UniformComplexRandomVariable<sym>{
            .value = value(), .variance = magnitude.variance + magnitude.value * magnitude.value * angle.variance};
    }
    explicit operator DecomposedIndependentComplexRandomVariable<sym>() const {
        auto const cos_theta = cos(angle.value);
        auto const sin_theta = sin(angle.value);
        auto const real_component = magnitude.value * cos_theta;
        auto const imag_component = magnitude.value * sin_theta;
        return DecomposedIndependentComplexRandomVariable<sym>{
            .real_component = {.value = real_component,
                               .variance = magnitude.variance * cos_theta * cos_theta +
                                           imag_component * imag_component * angle.variance},
            .imag_component = {.value = imag_component,
                               .variance = magnitude.variance * sin_theta * sin_theta +
                                           real_component * real_component * angle.variance}};
    }
};
} // namespace power_grid_model
