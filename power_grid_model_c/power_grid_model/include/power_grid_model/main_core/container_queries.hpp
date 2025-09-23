// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../all_components.hpp"
#include "../container.hpp"

namespace power_grid_model::main_core {

template <typename ComponentType, class ComponentContainer>
    requires common::storagable_component_container_c<ComponentContainer, ComponentType>
inline Idx get_component_type_index(ComponentContainer const& components) {
    return components.template get_type_idx<ComponentType>();
}

template <typename ComponentType, class ComponentContainer>
    requires common::component_container_c<ComponentContainer, ComponentType>
constexpr auto get_component_size(ComponentContainer const& components) {
    return components.template size<ComponentType>();
}

template <typename ComponentType, class ComponentContainer>
    requires common::component_container_c<ComponentContainer, ComponentType>
inline Idx get_component_sequence_idx(ComponentContainer const& components, auto const& id_or_index) {
    return components.template get_seq<ComponentType>(id_or_index);
}

template <class ComponentContainer> inline Idx2D get_component_idx_by_id(ComponentContainer const& components, ID id) {
    return components.get_idx_by_id(id);
}

template <typename ComponentType, class ComponentContainer>
    requires common::component_container_c<ComponentContainer, ComponentType>
inline Idx2D get_component_idx_by_id(ComponentContainer const& components, ID id) {
    return components.template get_idx_by_id<ComponentType>(id);
}

template <typename ComponentType, class ComponentContainer>
    requires common::storagable_component_container_c<ComponentContainer, ComponentType>
constexpr Idx get_component_group_idx(ComponentContainer const& components) {
    return components.template get_group_idx<ComponentType>();
}

template <std::derived_from<Base> BaseComponent, std::derived_from<Base> Component, class ComponentContainer>
    requires std::derived_from<Component, BaseComponent> &&
             common::component_container_c<ComponentContainer, BaseComponent> &&
             common::storagable_component_container_c<ComponentContainer, Component>
constexpr auto get_component_sequence_offset(ComponentContainer const& components) {
    return components.template get_start_idx<BaseComponent, Component>();
}

template <typename ComponentType, class ComponentContainer>
    requires common::component_container_c<ComponentContainer, ComponentType>
constexpr auto& get_component(ComponentContainer const& components, auto const& id_or_index) {
    return components.template get_item<ComponentType>(id_or_index);
}

template <typename ComponentType, class ComponentContainer>
    requires common::component_container_c<ComponentContainer, ComponentType>
constexpr auto& get_component(ComponentContainer& components, auto const& id_or_index) {
    return components.template get_item<ComponentType>(id_or_index);
}

template <typename ComponentType, class ComponentContainer>
    requires common::component_container_c<ComponentContainer, ComponentType>
constexpr auto& get_component_by_sequence(ComponentContainer const& components, Idx sequence) {
    return components.template get_item_by_seq<ComponentType>(sequence);
}

template <typename ComponentType, class ComponentContainer>
    requires common::component_container_c<ComponentContainer, ComponentType>
constexpr auto& get_component_by_sequence(ComponentContainer& components, Idx sequence) {
    return components.template get_item_by_seq<ComponentType>(sequence);
}

template <typename ComponentType, class ComponentContainer, typename... Args>
    requires common::storagable_component_container_c<ComponentContainer, ComponentType>
constexpr auto emplace_component(ComponentContainer& components, ID id, Args&&... args) {
    return components.template emplace<ComponentType>(id, std::forward<Args>(args)...);
}

template <typename ComponentType, class ComponentContainer, typename... Args>
    requires common::storagable_component_container_c<ComponentContainer, ComponentType>
constexpr void reserve_component(ComponentContainer& components, std::integral auto size) {
    components.template reserve<ComponentType>(size);
}

template <typename ComponentType, class ComponentContainer>
    requires common::component_container_c<ComponentContainer, ComponentType>
constexpr auto get_component_citer(ComponentContainer const& components) {
    return components.template citer<ComponentType>();
}

template <std::derived_from<Branch> ComponentType, class ComponentContainer>
    requires common::component_container_c<ComponentContainer, ComponentType>
constexpr auto get_topology_index(ComponentContainer const& components, auto const& id_or_index) {
    return get_component_sequence_idx<Branch>(components, id_or_index);
}

template <std::derived_from<Branch3> ComponentType, class ComponentContainer>
    requires common::component_container_c<ComponentContainer, ComponentType>
constexpr auto get_topology_index(ComponentContainer const& components, auto const& id_or_index) {
    return get_component_sequence_idx<Branch3>(components, id_or_index);
}

template <std::derived_from<Regulator> ComponentType, class ComponentContainer>
    requires common::component_container_c<ComponentContainer, ComponentType>
constexpr auto get_topology_index(ComponentContainer const& components, auto const& id_or_index) {
    return get_component_sequence_idx<Regulator>(components, id_or_index);
}

} // namespace power_grid_model::main_core
