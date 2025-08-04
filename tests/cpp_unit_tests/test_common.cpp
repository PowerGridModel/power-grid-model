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
static_assert(std::same_as<other_symmetry_t<symmetric_t>, asymmetric_t>);
static_assert(std::same_as<other_symmetry_t<asymmetric_t>, symmetric_t>);

static_assert(std::invocable<IncludeAll>);
static_assert(std::invocable<IncludeAll, Idx>);
static_assert(std::invocable<IncludeAll, Idx, Idx>);
static_assert(include_all());
static_assert(include_all(1));

// NOLINTNEXTLINE(performance-move-const-arg,hicpp-move-const-arg) // to test that rvalues work
static_assert(include_all(Idx{2}, std::move(Idx{3}))); // NOSONAR // to test that rvalues work

// periodic mapping
static_assert(map_to_cyclic_range(5, 3) == 2);
static_assert(map_to_cyclic_range(5.0, 3.0) == 2.0);
static_assert(map_to_cyclic_range(-1, 3) == 2);
static_assert(map_to_cyclic_range(-1.0, 3.0) == 2.0);
static_assert(map_to_cyclic_range(12, 12) == 0);
static_assert(map_to_cyclic_range(13, 12) == 1);
static_assert(map_to_cyclic_range(11, 12) == 11);
static_assert(map_to_cyclic_range(-1, -3) == -1);
} // namespace
} // namespace power_grid_model
