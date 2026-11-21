// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

// Request and result types of the uncertainty quantification (UQ) of state estimation, and the classification of a
// calculated variance.

#include "../common/common.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <limits>
#include <vector>

namespace power_grid_model::math_solver {

// The voltage uncertainties that are requested for one bus.
// The bus is identified by its index in the math model of one island.
struct VoltageUQRequest {
    Idx bus{};
    bool magnitude{};
    bool angle{};
};
struct StateEstimationUQRequest {
    std::vector<VoltageUQRequest> voltage;
    constexpr bool empty() const { return voltage.empty(); }
};

// Classification of a calculated variance.
enum class UQVarianceStatus : IntS {
    not_requested = 0,    // the quantity was not requested
    valid = 1,            // the variance is finite and not negative
    clipped_roundoff = 2, // the variance is negative within the roundoff of its terms and is reported as zero
    invalid = 3,          // the variance is not finite or negative beyond roundoff, or it could not be calculated
};

// The uncertainty of one phase of an output quantity, in the per-unit and radian convention of the math solver:
// - voltage magnitude: raw_variance and absolute_scale in p.u.^2, sigma in p.u.
// - voltage angle:     raw_variance and absolute_scale in rad^2, sigma in rad
// A voltage angle has the reference of the state estimation output:
// - the absolute angle, if the island has a voltage angle measurement
// - otherwise the angle relative to phase a of the slack bus. That angle itself has zero variance;
//   phases b and c of the slack bus do not.
struct UQPhaseValue {
    double raw_variance{nan};   // signed variance as calculated, which is kept when it is clipped or invalid
    double absolute_scale{nan}; // sum of the magnitudes of the terms that add up to raw_variance
    double sigma{nan};          // standard deviation, only for the statuses valid and clipped_roundoff
    UQVarianceStatus raw_status{UQVarianceStatus::not_requested};
};
template <symmetry_tag sym> struct UQValue {
    std::array<UQPhaseValue, is_symmetric_v<sym> ? 1 : 3> phase{};
};
template <symmetry_tag sym> struct VoltageUQResult {
    UQValue<sym> magnitude;
    UQValue<sym> angle;
};

// The uncertainties of one island.
// voltage[k] answers the request row voltage[k].
template <symmetry_tag sym> struct StateEstimationUQResult {
    std::vector<VoltageUQResult<sym>> voltage;
};

// The request and the result of one island, as run_state_estimation() takes them.
// The caller owns both and keeps them alive during the call.
// The result is written after the estimate has converged. An empty request is valid and gives an empty result.
template <symmetry_tag sym> struct StateEstimationUQContext {
    std::reference_wrapper<StateEstimationUQRequest const> request;
    std::reference_wrapper<StateEstimationUQResult<sym>> result;
};

namespace state_estimation_uq {
// A negative variance is attributed to the roundoff of cancelling terms, if its magnitude is at most this factor
// times the sum of the magnitudes of these terms.
// The factor is a heuristic margin above the machine precision: the terms carry the rounding errors of the
// factorization and the selective inverse.
inline constexpr double roundoff_tolerance = 1000.0 * std::numeric_limits<double>::epsilon();

// A variance that is the sum of several terms, together with the sum of the magnitudes of these terms.
// That sum is the scale against which a negative variance is judged.
class RealSum {
  public:
    void add(double term) {
        value_ += term;
        absolute_scale_ += std::abs(term);
    }
    constexpr double value() const { return value_; }
    constexpr double absolute_scale() const { return absolute_scale_; }

  private:
    double value_{};
    double absolute_scale_{};
};

// Classify a variance as described at UQVarianceStatus.
// The variance and its scale are stored for every status.
inline UQPhaseValue finalize_variance(double variance, double absolute_scale) {
    using enum UQVarianceStatus;

    if (!std::isfinite(variance) || !std::isfinite(absolute_scale) || variance < -roundoff_tolerance * absolute_scale) {
        return {.raw_variance = variance, .absolute_scale = absolute_scale, .sigma = nan, .raw_status = invalid};
    }
    return {.raw_variance = variance,
            .absolute_scale = absolute_scale,
            .sigma = std::sqrt(std::max(0.0, variance)),
            .raw_status = variance < 0.0 ? clipped_roundoff : valid};
}
// Classify the variance that a sum of terms adds up to, against the scale of these terms.
inline UQPhaseValue finalize_variance(RealSum const& sum) {
    return finalize_variance(sum.value(), sum.absolute_scale());
}
} // namespace state_estimation_uq
} // namespace power_grid_model::math_solver
