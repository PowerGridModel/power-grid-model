// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/maybe_owning_view.hpp>

export module power_grid_model.common.maybe_owning_view;

export namespace power_grid_model {
using power_grid_model::maybe_owning_view;
using power_grid_model::MaybeOwningConstVector;
using power_grid_model::MaybeOwningVector;
} // namespace power_grid_model

// The detail::is_owning_view<maybe_owning_view<...>> partial specialization is not named by any of the exports above,
// so it would be discarded from the global module fragment. Referring to it keeps it reachable for importers.
namespace power_grid_model {
static_assert(detail::is_owning_view<MaybeOwningVector<Idx>>::value);
} // namespace power_grid_model
