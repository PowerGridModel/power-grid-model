// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <power_grid_model/common/small_vector.hpp>

export module power_grid_model.common.small_vector;

export namespace power_grid_model {
using power_grid_model::SmallVector;
} // namespace power_grid_model

// boost::container::small_vector constructs its elements with placement new (from <new>) at instantiation. That
// overload is not named by the export above, so it would be discarded from the global module fragment. Naming it
// here (without exporting) keeps it reachable for importers.
namespace power_grid_model::detail::module_reachable {
using ::operator new;
} // namespace power_grid_model::detail::module_reachable
