// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common/common.hpp"

#include <concepts>

namespace power_grid_model::common {

namespace detail {
template <typename ContainerType, typename RetrievableType>
concept single_component_container_c = requires(ContainerType const& c, ID id, Idx2D idx2d) {
    { c.template citer<RetrievableType>().begin() } -> std::forward_iterator;
    { c.template citer<RetrievableType>().end() } -> std::forward_iterator;
    { *(c.template citer<RetrievableType>().begin()) } -> std::same_as<RetrievableType const&>;
    {
        c.template citer<RetrievableType>().end()
    } -> std::same_as<decltype(c.template citer<RetrievableType>().begin())>;
    { c.template get_item<RetrievableType>(id) } -> std::convertible_to<RetrievableType const&>;
    { c.template size<RetrievableType>() } -> std::same_as<Idx>;
    { c.template get_seq<RetrievableType>(idx2d) } -> std::same_as<Idx>;
};
} // namespace detail

template <typename ContainerType, typename... RetrievableType>
concept component_container_c = (detail::single_component_container_c<ContainerType, RetrievableType> && ...);

} // namespace power_grid_model::common
