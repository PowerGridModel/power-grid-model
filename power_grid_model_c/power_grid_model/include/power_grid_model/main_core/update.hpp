// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MAIN_CORE_UPDATE_HPP
#define POWER_GRID_MODEL_MAIN_CORE_UPDATE_HPP

#include "state.hpp"

#include "../all_components.hpp"

namespace power_grid_model::main_core {

// template to update components
// using forward interators
// different selection based on component type
// if sequence_idx is given, it will be used to load the object instead of using IDs via hash map.
template <std::derived_from<Base> Component, class CacheType, class ComponentContainer,
          std::forward_iterator ForwardIterator>
    requires model_component_state<MainModelState, ComponentContainer, Component>
UpdateChange update_component(MainModelState<ComponentContainer>& state, ForwardIterator begin, ForwardIterator end,
                              std::vector<Idx2D> const& sequence_idx = {}) {
    bool const has_sequence_id = !sequence_idx.empty();
    Idx seq = 0;

    UpdateChange changed;

    // loop to to update component
    for (auto it = begin; it != end; ++it, ++seq) {
        // get component
        // either using ID via hash map
        // either directly using sequence id
        Idx2D const sequence_single =
            has_sequence_id ? sequence_idx[seq] : state.components.template get_idx_by_id<Component>(it->id);

        if constexpr (CacheType::value) {
            state.components.template cache_item<Component>(sequence_single.pos);
        }

        auto& comp = state.components.template get_item<Component>(sequence_single);

        changed = changed || comp.update(*it);
    }

    return changed;
}

template <bool sym>
void update_y_bus(YBus<sym>& y_bus, std::shared_ptr<MathModelParam<sym> const> const& math_model_param) {
    y_bus.update_admittance(math_model_param);
}

template <bool sym>
void update_y_bus(MathState& math_state, std::vector<MathModelParam<sym>> const& math_model_params,
                  Idx n_math_solvers) {
    for (Idx i = 0; i != n_math_solvers; ++i) {
        if constexpr (sym) {
            update_y_bus(math_state.y_bus_vec_sym[i],
                         std::make_shared<MathModelParam<sym> const>(std::move(math_model_params[i])));

        } else {
            update_y_bus(math_state.y_bus_vec_asym[i],
                         std::make_shared<MathModelParam<sym> const>(std::move(math_model_params[i])));
        }
    }
}

} // namespace power_grid_model::main_core

#endif
