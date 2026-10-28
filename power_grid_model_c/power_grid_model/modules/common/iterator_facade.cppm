// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <power_grid_model/common/iterator_facade.hpp>

export module power_grid_model.common.iterator_facade;

export namespace power_grid_model {
using power_grid_model::IteratorFacade;

namespace detail {
using power_grid_model::detail::iterator_facadeable_c;
} // namespace detail
} // namespace power_grid_model
