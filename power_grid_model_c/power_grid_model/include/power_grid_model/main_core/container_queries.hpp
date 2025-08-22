// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../container.hpp"

namespace power_grid_model::main_core {

// TODO Reconfirm if there is duplication with state_queries and if we can remove either one of them

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

} // namespace power_grid_model::main_core
