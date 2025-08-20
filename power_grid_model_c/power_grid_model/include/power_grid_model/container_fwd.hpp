// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common/common.hpp"

#include <concepts>

namespace power_grid_model {

template <typename ContainerType, typename ComponentType>
concept component_container_c = requires(ContainerType const& c, ID id) {
    { c.template citer<ComponentType>().begin() } -> std::forward_iterator;
    { c.template citer<ComponentType>().end() } -> std::forward_iterator;
    { *(c.template citer<ComponentType>().begin()) } -> std::same_as<ComponentType const&>;
    { *(c.template citer<ComponentType>().end()) } -> std::same_as<ComponentType const&>;
    { c.template get_item<ComponentType>(id) } -> std::convertible_to<ComponentType const&>;
};

// TODO merge with component_container_c
template <typename ContainerType, typename ComponentType>
concept extended_component_container_c =
    component_container_c<ContainerType, ComponentType> && requires(ContainerType const& c, Idx2D const& idx2d) {
        { c.template size<ComponentType>() } -> std::same_as<Idx>;
        { c.template get_seq<ComponentType>(idx2d) } -> std::same_as<Idx>;
    };

template <typename ContainerType, typename... ComponentType>
concept multi_extended_component_container_c = (extended_component_container_c<ContainerType, ComponentType> && ...);

} // namespace power_grid_model