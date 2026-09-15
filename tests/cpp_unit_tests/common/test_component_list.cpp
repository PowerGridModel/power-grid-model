// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/common/component_list.hpp>

namespace power_grid_model {
namespace {
struct A {};
struct B {};
struct C {};
struct D {};
} // namespace

static_assert(IsInList<A, ComponentList<A, B>>::value);
static_assert(IsInList<B, ComponentList<A, B>>::value);
static_assert(!IsInList<C, ComponentList<A, B>>::value);

static_assert(before_in_list_c<ComponentList<A, B, C, D>, A, B>);
static_assert(before_in_list_c<ComponentList<A, B, C, D>, A, C>);
static_assert(!before_in_list_c<ComponentList<A, B, C, D>, C, A>);
static_assert(!before_in_list_c<ComponentList<A, B, C, D>, B, A>);
static_assert(before_in_list_c<ComponentList<A, B>, A, C>);
} // namespace power_grid_model
