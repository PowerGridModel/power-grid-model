// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "state.hpp"
#include "state_queries.hpp"

#include "../all_components.hpp"
#include "../common/iterator_like_concepts.hpp"

#include <unordered_set>

namespace power_grid_model::main_core {

// template to construct components
// using forward interators
// different selection based on component type
template <std::derived_from<Base> Component, class ComponentContainer,
          forward_iterator_like<typename Component::InputType> ForwardIterator>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
inline void add_component(MainModelState<ComponentContainer>& state, ForwardIterator begin, ForwardIterator end,
                          double system_frequency) {
    using ComponentView = std::conditional_t<std::same_as<decltype(*begin), typename Component::InputType const&>,
                                             typename Component::InputType const&, typename Component::InputType>;

    reserve_component<Component>(state, std::distance(begin, end));
    // do sanity check on the transformer tap regulator
    std::vector<Idx2D> regulated_objects;
    // loop to add component
    for (auto it = begin; it != end; ++it) {
        ComponentView const input = *it;
        ID const id = input.id;
        // construct based on type of component
        if constexpr (std::derived_from<Component, Node>) {
            emplace_component<Component>(state, id, input);
        } else if constexpr (std::derived_from<Component, Branch>) {
            double const u1 = get_component<Node>(state, input.from_node).u_rated();
            double const u2 = get_component<Node>(state, input.to_node).u_rated();
            // set system frequency for line
            if constexpr (std::same_as<Component, Line>) {
                emplace_component<Component>(state, id, input, system_frequency, u1, u2);
            } else {
                emplace_component<Component>(state, id, input, u1, u2);
            }
        } else if constexpr (std::derived_from<Component, Appliance>) {
            double const u = get_component<Node>(state, input.node).u_rated();
            emplace_component<Component>(state, id, input, u);
        }
    }
    // Make sure that each regulated object has at most one regulator
    const std::unordered_set<Idx2D, Idx2DHash> unique_regulated_objects(regulated_objects.begin(),
                                                                        regulated_objects.end());
    if (unique_regulated_objects.size() != regulated_objects.size()) {
        // There are duplicates
        throw DuplicativelyRegulatedObject{};
    }
}

} // namespace power_grid_model::main_core
