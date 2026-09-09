// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"

#include <type_traits>

namespace power_grid_model {

// component list
template <class... T> struct ComponentList {};

template <typename... Ts> struct IsInList : std::false_type {};
template <typename T, typename... Ts>
    requires is_in_list_c<T, Ts...>
struct IsInList<T, ComponentList<Ts...>> : std::true_type {};

namespace detail {
template <typename U, typename V, typename List> struct BeforeInList : std::false_type {};

template <typename U, typename V, typename T, typename... Ts>
struct BeforeInList<U, V, ComponentList<T, Ts...>>
    : std::conditional_t<std::is_same_v<T, std::remove_const_t<U>>, std::bool_constant<is_in_list_c<V, Ts...>>,
                         BeforeInList<U, V, ComponentList<Ts...>>> {};
} // namespace detail

template <typename ListT, typename U, typename V>
concept before_in_list_c = !std::is_same_v<U, V> && (detail::BeforeInList<U, V, ListT>::value ||
                                                     !IsInList<U, ListT>::value || !IsInList<V, ListT>::value);

// type traits associated with the container
template <class... T> struct ExtraRetrievableTypes {};

} // namespace power_grid_model
