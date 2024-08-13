// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/common/component_list.hpp>

namespace power_grid_model {
namespace {
struct A {};
struct B {};
struct C {};
} // namespace

static_assert(IsInList<A, ComponentList<A, B>>::value);
static_assert(IsInList<B, ComponentList<A, B>>::value);
static_assert(!IsInList<C, ComponentList<A, B>>::value);
} // namespace power_grid_model
