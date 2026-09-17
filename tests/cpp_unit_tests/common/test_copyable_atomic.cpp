// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/common/copyable_atomic.hpp>

#include <atomic>
#include <concepts>

namespace power_grid_model {
namespace {
static_assert(std::derived_from<power_grid_model::common::atomic::CopyableAtomic<int>, std::atomic<int>>);
static_assert(std::is_nothrow_move_constructible_v<power_grid_model::common::atomic::CopyableAtomic<int>>);
static_assert(std::is_nothrow_move_assignable_v<power_grid_model::common::atomic::CopyableAtomic<int>>);
static_assert(std::is_copy_constructible_v<power_grid_model::common::atomic::CopyableAtomic<int>>);
static_assert(std::is_copy_assignable_v<power_grid_model::common::atomic::CopyableAtomic<int>>);
} // namespace
} // namespace power_grid_model
