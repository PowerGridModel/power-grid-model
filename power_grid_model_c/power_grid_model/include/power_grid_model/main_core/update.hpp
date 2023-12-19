// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MAIN_CORE_UPDATE_HPP
#define POWER_GRID_MODEL_MAIN_CORE_UPDATE_HPP

#include "state.hpp"

#include "../all_components.hpp"

#include <ranges>

namespace power_grid_model::main_core {

namespace detail {
template <std::derived_from<Base> Component, class ComponentContainer, std::forward_iterator ForwardIterator,
          typename Func>
    requires model_component_state<MainModelState, ComponentContainer, Component> &&
             std::invocable<std::remove_cvref_t<Func>, typename Component::UpdateType, Idx2D const&>
inline void iterate_component_sequence(Func&& func, MainModelState<ComponentContainer> const& state,
                                       ForwardIterator begin, ForwardIterator end,
                                       std::vector<Idx2D> const& sequence_idx) {
    bool const has_sequence_id = !sequence_idx.empty();
    Idx seq = 0;

    // loop to to update component
    for (auto it = begin; it != end; ++it, ++seq) {
        // get component
        // either using ID via hash map
        // either directly using sequence id
        Idx2D const sequence_single =
            has_sequence_id ? sequence_idx[seq] : state.components.template get_idx_by_id<Component>(it->id);

        func(*it, sequence_single);
    }
}
} // namespace detail

// template to update components
// using forward interators
// different selection based on component type
// if sequence_idx is given, it will be used to load the object instead of using IDs via hash map.
template <std::derived_from<Base> Component, class ComponentContainer, std::forward_iterator ForwardIterator>
    requires model_component_state<MainModelState, ComponentContainer, Component>
inline UpdateChange update_component(MainModelState<ComponentContainer>& state, ForwardIterator begin,
                                     ForwardIterator end, std::vector<Idx2D> const& sequence_idx = {}) {
    using UpdateType = typename Component::UpdateType;

    UpdateChange changed;

    detail::iterate_component_sequence<Component>(
        [&changed, &state](UpdateType const& update_data, Idx2D const& sequence_single) {
            auto& comp = state.components.template get_item<Component>(sequence_single);
            changed = changed || comp.update(update_data);
        },
        state, begin, end, sequence_idx);

    return changed;
}

// template to get the inverse update for components
// using forward interators
// different selection based on component type
// if sequence_idx is given, it will be used to load the object instead of using IDs via hash map.
template <std::derived_from<Base> Component, class ComponentContainer, std::forward_iterator ForwardIterator,
          std::output_iterator<typename Component::UpdateType> OutputIterator>
    requires model_component_state<MainModelState, ComponentContainer, Component>
inline void update_inverse(MainModelState<ComponentContainer> const& state, ForwardIterator begin, ForwardIterator end,
                           OutputIterator destination, std::vector<Idx2D> const& sequence_idx = {}) {
    using UpdateType = typename Component::UpdateType;

    detail::iterate_component_sequence<Component>(
        [&destination, &state](UpdateType const& update_data, Idx2D const& sequence_single) {
            auto const& comp = state.components.template get_item<Component>(sequence_single);
            *destination++ = comp.inverse(update_data);
        },
        state, begin, end, sequence_idx);
}

template <bool sym>
inline void update_y_bus(YBus<sym>& y_bus, std::shared_ptr<MathModelParam<sym> const> const& math_model_param) {
    y_bus.update_admittance(math_model_param);
}

template <bool sym>
inline void update_y_bus(YBus<sym>& y_bus, std::shared_ptr<MathModelParam<sym> const> const& math_model_param,
                         bool increment) {
    auto branch_param_to_change_views =
        std::views::iota(0, math_model_param->branch_param.size()) | std::views::filter([&math_model_param](Idx i) {
            return math_model_param->branch_param[i].yff() != ComplexTensor<sym>{0.0} ||
                   math_model_param->branch_param[i].yft() != ComplexTensor<sym>{0.0} ||
                   math_model_param->branch_param[i].ytf() != ComplexTensor<sym>{0.0} ||
                   math_model_param->branch_param[i].ytt() != ComplexTensor<sym>{0.0};
        });
    auto shunt_param_to_change_views =
        std::views::iota(0, math_model_param->shunt_param.size()) | std::views::filter([&math_model_param](Idx i) {
            return math_model_param->shunt_param[i] != ComplexTensor<sym>{0.0};
        });

    MathModelParamIncrement<sym> math_model_param_incrmt;
    math_model_param_incrmt.branch_param = math_model_param->branch_param;
    math_model_param_incrmt.shunt_param = math_model_param->shunt_param;
    math_model_param_incrmt.source_param = math_model_param->source_param; // not sure if we actually need this
    math_model_param_incrmt.branch_param_to_change = branch_param_to_change_views;
    math_model_param_incrmt.shunt_param_to_change = shunt_param_to_change_views;

    auto math_model_param_incrmt_ptr = std::make_shared<MathModelParamIncrement<sym> const>(math_model_param_incrmt);

    y_bus.update_admittance_increment(math_model_param_incrmt_ptr, !increment);
}

template <bool sym>
inline void update_y_bus(MathState& math_state, std::vector<MathModelParam<sym>> const& math_model_params,
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
