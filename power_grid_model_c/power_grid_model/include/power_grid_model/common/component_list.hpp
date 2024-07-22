// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"

namespace power_grid_model {

// component list
template <class... T> struct ComponentList {};

template <typename... Ts> struct IsInList : std::false_type {};
template <typename T, typename... Ts>
    requires is_in_list_c<T, Ts...>
struct IsInList<T, ComponentList<Ts...>> : std::true_type {};

} // namespace power_grid_model
