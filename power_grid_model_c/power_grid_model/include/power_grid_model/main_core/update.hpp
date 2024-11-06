// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "state.hpp"

#include "../all_components.hpp"
#include "../common/iterator_like_concepts.hpp"

namespace power_grid_model::main_core {

namespace detail {
template <component_c Component, forward_iterator_like<typename Component::UpdateType> ForwardIterator, typename Func>
    requires std::invocable<std::remove_cvref_t<Func>, typename Component::UpdateType, Idx2D const&>
inline void iterate_component_sequence(Func&& func, ForwardIterator begin, ForwardIterator end,
                                       std::span<Idx2D const> sequence_idx) {
    assert(std::distance(begin, end) >= static_cast<ptrdiff_t>(sequence_idx.size()));

    Idx seq = 0;

    // loop to to update component
    for (auto it = begin; it != end; ++it, ++seq) {
        // get component directly using sequence id
        func(*it, sequence_idx[seq]);
    }
}
} // namespace detail

template <component_c Component, class ComponentContainer,
          forward_iterator_like<typename Component::UpdateType> ForwardIterator,
          std::output_iterator<Idx2D> OutputIterator>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
inline void get_component_sequence(MainModelState<ComponentContainer> const& state, ForwardIterator begin,
                                   ForwardIterator end, OutputIterator destination, Idx n_comp_elements) {
    using UpdateType = typename Component::UpdateType;

    if (n_comp_elements < 0) {
        std::ranges::transform(begin, end, destination, [&state](UpdateType const& update) {
            return get_component_idx_by_id<Component>(state, update.id);
        });
    } else {
        assert(std::distance(begin, end) <= n_comp_elements);
        std::ranges::transform(
            begin, end, destination,
            [group = get_component_group_idx<Component>(state), index = 0](auto const& /*update*/) mutable {
                return Idx2D{group, index++}; // NOSONAR
            });
    }
}

template <component_c Component, class ComponentContainer,
          forward_iterator_like<typename Component::UpdateType> ForwardIterator>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
inline std::vector<Idx2D> get_component_sequence(MainModelState<ComponentContainer> const& state, ForwardIterator begin,
                                                 ForwardIterator end, Idx n_comp_elements = na_Idx) {
    std::vector<Idx2D> result;
    result.reserve(std::distance(begin, end));
    get_component_sequence<Component>(state, begin, end, std::back_inserter(result), n_comp_elements);
    return result;
}

// template to update components
// using forward interators
// different selection based on component type
// if sequence_idx is given, it will be used to load the object instead of using IDs via hash map.
template <component_c Component, class ComponentContainer,
          forward_iterator_like<typename Component::UpdateType> ForwardIterator,
          std::output_iterator<Idx2D> OutputIterator>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
inline UpdateChange update_component(MainModelState<ComponentContainer>& state, ForwardIterator begin,
                                     ForwardIterator end, OutputIterator changed_it,
                                     std::span<Idx2D const> sequence_idx) {
    using UpdateType = typename Component::UpdateType;

    UpdateChange state_changed;

    detail::iterate_component_sequence<Component>(
        [&state_changed, &changed_it, &state](UpdateType const& update_data, Idx2D const& sequence_single) {
            auto& comp = get_component<Component>(state, sequence_single);
            assert(state.components.get_id_by_idx(sequence_single) == comp.id());
            auto const comp_changed = comp.update(update_data);
            state_changed = state_changed || comp_changed;

            if (comp_changed.param || comp_changed.topo) {
                *changed_it++ = sequence_single;
            }
        },
        begin, end, sequence_idx);

    return state_changed;
}
template <component_c Component, class ComponentContainer,
          forward_iterator_like<typename Component::UpdateType> ForwardIterator,
          std::output_iterator<Idx2D> OutputIterator>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
inline UpdateChange update_component(MainModelState<ComponentContainer>& state, ForwardIterator begin,
                                     ForwardIterator end, OutputIterator changed_it) {
    return update_component<Component>(state, begin, end, changed_it,
                                       get_component_sequence<Component>(state, begin, end));
}

// template to get the inverse update for components
// using forward interators
// different selection based on component type
// if sequence_idx is given, it will be used to load the object instead of using IDs via hash map.
template <component_c Component, class ComponentContainer,
          forward_iterator_like<typename Component::UpdateType> ForwardIterator,
          std::output_iterator<typename Component::UpdateType> OutputIterator>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
inline void update_inverse(MainModelState<ComponentContainer> const& state, ForwardIterator begin, ForwardIterator end,
                           OutputIterator destination, std::span<Idx2D const> sequence_idx) {
    using UpdateType = typename Component::UpdateType;

    detail::iterate_component_sequence<Component>(
        [&destination, &state](UpdateType const& update_data, Idx2D const& sequence_single) {
            auto const& comp = get_component<Component>(state, sequence_single);
            *destination++ = comp.inverse(update_data);
        },
        begin, end, sequence_idx);
}
template <component_c Component, class ComponentContainer,
          forward_iterator_like<typename Component::UpdateType> ForwardIterator,
          std::output_iterator<typename Component::UpdateType> OutputIterator>
    requires model_component_state_c<MainModelState, ComponentContainer, Component>
inline void update_inverse(MainModelState<ComponentContainer> const& state, ForwardIterator begin, ForwardIterator end,
                           OutputIterator destination) {
    return update_inverse<Component>(state, begin, end, destination,
                                     get_component_sequence<Component>(state, begin, end));
}

} // namespace power_grid_model::main_core
