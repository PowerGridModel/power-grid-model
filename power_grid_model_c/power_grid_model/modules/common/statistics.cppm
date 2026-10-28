// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <power_grid_model/common/statistics.hpp>

export module power_grid_model.common.statistics;

export namespace power_grid_model {
// random variables
using power_grid_model::DecomposedComplexRandVar;
using power_grid_model::IndependentComplexRandVar;
using power_grid_model::IndependentRealRandVar;
using power_grid_model::PolarComplexRandVar;
using power_grid_model::UniformComplexRandVar;
using power_grid_model::UniformRealRandVar;

// functions
using power_grid_model::conj;
using power_grid_model::pos_seq;
using power_grid_model::three_phase;

namespace statistics {
using power_grid_model::statistics::combine;
using power_grid_model::statistics::combine_magnitude;
using power_grid_model::statistics::scale;
} // namespace statistics
} // namespace power_grid_model

// The templates above call exp, cos and sin unqualified on Eigen arrays (found via ADL at instantiation). Those
// overloads are not named by any of the exports, so they would be discarded from the global module fragment.
// Naming them here (without exporting) keeps them reachable for importers.
namespace power_grid_model::statistics::detail::module_reachable {
using Eigen::cos;
using Eigen::exp;
using Eigen::sin;
} // namespace power_grid_model::statistics::detail::module_reachable
