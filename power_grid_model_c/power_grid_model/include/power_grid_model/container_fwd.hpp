// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common/common.hpp"

#include <concepts>

namespace power_grid_model::common {

namespace detail {
template <typename ContainerType, typename RetrievableType>
concept single_component_container_c =
    ContainerType::template is_gettable_v<RetrievableType> &&
    requires(ContainerType const& c, ContainerType& nc, ID id, Idx idx, Idx2D idx2d) {
        { c.template citer<RetrievableType>().begin() } -> std::forward_iterator;
        { c.template citer<RetrievableType>().end() } -> std::forward_iterator;
        { *(c.template citer<RetrievableType>().begin()) } -> std::same_as<RetrievableType const&>;
        {
            c.template citer<RetrievableType>().end()
        } -> std::same_as<decltype(c.template citer<RetrievableType>().begin())>;
        { c.template size<RetrievableType>() } -> std::same_as<Idx>;
        { c.template get_seq<RetrievableType>(idx2d) } -> std::same_as<Idx>;
        { c.template get_idx_by_id<RetrievableType>(id) } -> std::same_as<Idx2D>;
        { nc.template get_item<RetrievableType>(id) } -> std::same_as<RetrievableType&>;
        { c.template get_item<RetrievableType>(id) } -> std::same_as<RetrievableType const&>;
        { nc.template get_item_by_seq<RetrievableType>(idx) } -> std::same_as<RetrievableType&>;
        { c.template get_item_by_seq<RetrievableType>(idx) } -> std::same_as<RetrievableType const&>;
    };

template <typename ContainerType, typename StoragableType, typename... Args>
concept storagable_single_component_container_c =
    single_component_container_c<ContainerType, StoragableType> &&
    ContainerType::template is_storageable_v<StoragableType> &&
    requires(ContainerType const& c, ContainerType& nc, size_t size, ID id, Args... args) {
        { c.template get_group_idx<StoragableType>() } -> std::same_as<Idx>;
        { c.template get_type_idx<StoragableType>() } -> std::same_as<Idx>;
        { nc.template reserve<StoragableType>(size) } -> std::same_as<void>;
        { nc.template emplace<StoragableType>(id, args...) } -> std::same_as<void>;
    };

} // namespace detail

template <typename ContainerType, typename... RetrievableType>
concept component_container_c = (detail::single_component_container_c<ContainerType, RetrievableType> && ...);

template <typename ContainerType, typename... StoragableType>
concept storagable_component_container_c =
    (detail::storagable_single_component_container_c<ContainerType, StoragableType> && ...);

} // namespace power_grid_model::common
