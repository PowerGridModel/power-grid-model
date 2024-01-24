// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MAIN_CORE_UPDATE_HPP
#define POWER_GRID_MODEL_MAIN_CORE_UPDATE_HPP

#include "state.hpp"

#include "../all_components.hpp"

#include <boost/range/adaptors.hpp>
#include <boost/range/irange.hpp>

#include <ranges>

namespace power_grid_model::main_core {

namespace detail {
template <std::derived_from<Base> Component, class ComponentContainer, typename UpdateType>
    requires model_component_state<MainModelState, ComponentContainer, Component> &&
             std::same_as<std::remove_cvref_t<typename Component::UpdateType>, std::remove_cvref_t<UpdateType>>
inline Idx2D get_idx_by_id(MainModelState<ComponentContainer> const& state, UpdateType const& update) {
    return state.components.template get_idx_by_id<Component>(update.id);
}

template <std::derived_from<Base> Component, std::forward_iterator ForwardIterator, typename Func>
    requires std::invocable<std::remove_cvref_t<Func>, typename Component::UpdateType, Idx2D const&>
inline void iterate_component_sequence(Func&& func, ForwardIterator begin, ForwardIterator end,
                                       std::vector<Idx2D> const& sequence_idx) {
    Idx seq = 0;

    // loop to to update component
    for (auto it = begin; it != end; ++it, ++seq) {
        // get component directly using sequence id
        func(*it, sequence_idx[seq]);
    }
}
} // namespace detail

template <std::derived_from<Base> Component, class ComponentContainer, std::forward_iterator ForwardIterator,
          std::output_iterator<Idx2D> OutputIterator>
    requires model_component_state<MainModelState, ComponentContainer, Component>
inline void get_component_sequence(MainModelState<ComponentContainer> const& state, ForwardIterator begin,
                                   ForwardIterator end, OutputIterator destination) {
    using UpdateType = typename Component::UpdateType;

    std::transform(begin, end, destination,
                   [&state](UpdateType const& update) { return detail::get_idx_by_id<Component>(state, update); });
}

template <std::derived_from<Base> Component, class ComponentContainer, std::forward_iterator ForwardIterator>
    requires model_component_state<MainModelState, ComponentContainer, Component>
inline std::vector<Idx2D> get_component_sequence(MainModelState<ComponentContainer> const& state, ForwardIterator begin,
                                                 ForwardIterator end) {
    std::vector<Idx2D> result;
    result.reserve(std::distance(begin, end));
    get_component_sequence<Component>(state, begin, end, std::back_inserter(result));
    return result;
}

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
        begin, end, sequence_idx);

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
        begin, end, sequence_idx);
}

// mark changed components if not **exactly** equal; diffs with rounding errors are considered not equal
template <bool sym> inline bool cmplx_neq(ComplexTensor<sym> const& lhs, ComplexTensor<sym> const& rhs) {
    if constexpr (sym) {
        return lhs != rhs;
    } else {
        return lhs(0, 0) != rhs(0, 0) || lhs(1, 1) != rhs(1, 1) || lhs(2, 2) != rhs(2, 2);
    }
};

template <bool sym>
inline void update_y_bus(YBus<sym>& y_bus, std::shared_ptr<MathModelParam<sym> const> const& math_model_param,
                         std::shared_ptr<std::vector<std::vector<Idx2D>> const> const& seq_idx_map) {
    // verify that the number of branches, shunts and source is the same
    MathModelParam<sym> const& y_bus_param = y_bus.math_model_param();
    (void)y_bus_param; // suppress compiler unused variable warning
    assert(y_bus_param.branch_param.size() == math_model_param->branch_param.size());
    assert(y_bus_param.shunt_param.size() == math_model_param->shunt_param.size());
    assert(y_bus_param.source_param.size() == math_model_param->source_param.size());

    if (seq_idx_map == nullptr) {
        return;
    }

    // when sequence_idx_map available: loop through all branches and shunts of math_model_param, check changes
    auto query_param_in_seq_map = [&seq_idx_map](IdxVector const& param_idx_vec) {
        std::vector<Idx2D> result;
        for (auto const& param_idd : param_idx_vec) {
            auto const& seq_idx = (*seq_idx_map)[param_idd];
            result.insert(result.end(), seq_idx.begin(), seq_idx.end());
        }
        return result;
    };

    auto branch_params = query_param_in_seq_map(y_bus.get_branch_param_idx());
    auto shunt_params = query_param_in_seq_map(y_bus.get_shunt_param_idx());

    MathModelParamIncrement<sym> math_model_param_incrmt;
    math_model_param_incrmt.branch_param_to_change.reserve(branch_params.size());
    math_model_param_incrmt.shunt_param_to_change.reserve(shunt_params.size());
    std::ranges::transform(branch_params, std::back_inserter(math_model_param_incrmt.branch_param_to_change),
                           [](const Idx2D& val) { return val.pos; });
    std::ranges::transform(shunt_params, std::back_inserter(math_model_param_incrmt.shunt_param_to_change),
                           [](const Idx2D& val) { return val.pos; });

    // check changes in case sequence_idx_map is available but indicates empty
    // this back-up check fixes the edge case where the sequence_idx_map is available but indicates empty
    if (branch_params.size() == 0) {
        auto branch_param_to_change_views =
            boost::irange(y_bus_param.branch_param.size()) |
            boost::adaptors::filtered([&math_model_param, &y_bus_param](Idx i) {
                return cmplx_neq<sym>(math_model_param->branch_param[i].yff(), y_bus_param.branch_param[i].yff()) ||
                       cmplx_neq<sym>(math_model_param->branch_param[i].yft(), y_bus_param.branch_param[i].yft()) ||
                       cmplx_neq<sym>(math_model_param->branch_param[i].ytf(), y_bus_param.branch_param[i].ytf()) ||
                       cmplx_neq<sym>(math_model_param->branch_param[i].ytt(), y_bus_param.branch_param[i].ytt());
            });
        math_model_param_incrmt.branch_param_to_change.reserve(
            std::distance(branch_param_to_change_views.begin(), branch_param_to_change_views.end()));
        math_model_param_incrmt.branch_param_to_change = {branch_param_to_change_views.begin(),
                                                          branch_param_to_change_views.end()};
    }

    if (shunt_params.size() == 0) {
        auto shunt_param_to_change_views =
            boost::irange(y_bus_param.shunt_param.size()) |
            boost::adaptors::filtered([&math_model_param, &y_bus_param](Idx i) {
                return cmplx_neq<sym>(math_model_param->shunt_param[i], y_bus_param.shunt_param[i]);
            });
        math_model_param_incrmt.shunt_param_to_change.reserve(
            std::distance(shunt_param_to_change_views.begin(), shunt_param_to_change_views.end()));
        math_model_param_incrmt.shunt_param_to_change = {shunt_param_to_change_views.begin(),
                                                         shunt_param_to_change_views.end()};
    }

    auto param_incrmt_ptr = std::make_shared<MathModelParamIncrement<sym> const>(std::move(math_model_param_incrmt));

    y_bus.update_admittance_increment(math_model_param, param_incrmt_ptr,
                                      false); /* param, changed_param, is_decrement */
}

template <bool sym>
inline void update_y_bus(MathState& math_state, std::vector<MathModelParam<sym>> const& math_model_params,
                         Idx n_math_solvers, std::vector<std::vector<Idx2D>> const& seq_idx_map) {
    for (Idx i = 0; i != n_math_solvers; ++i) {
        auto seq_idx_map_copy = seq_idx_map;
        if constexpr (sym) {
            update_y_bus(math_state.y_bus_vec_sym[i],
                         std::make_shared<MathModelParam<sym> const>(std::move(math_model_params[i])),
                         std::make_shared<std::vector<std::vector<Idx2D>> const>(std::move(seq_idx_map_copy)));

        } else {
            update_y_bus(math_state.y_bus_vec_asym[i],
                         std::make_shared<MathModelParam<sym> const>(std::move(math_model_params[i])),
                         std::make_shared<std::vector<std::vector<Idx2D>> const>(std::move(seq_idx_map_copy)));
        }
    }
}

} // namespace power_grid_model::main_core

#endif
