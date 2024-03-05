// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/common/common.hpp>

namespace power_grid_model {
namespace {
static_assert(symmetry_tag<symmetric_t>);
static_assert(symmetry_tag<symmetric_t>);
static_assert(is_symmetric_v<symmetric_t>);
static_assert(!is_symmetric_v<asymmetric_t>);
} // namespace
} // namespace power_grid_model
