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

#define INCREMENT_UPDATE_Y_BUS

// mark changed components if not **exactly** equal; diffs with rounding errors are considered not equal
template <bool sym> inline bool cmplx_neq(ComplexTensor<sym> const& lhs, ComplexTensor<sym> const& rhs) {
    if constexpr (sym) {
        return static_cast<bool>(lhs.real() != rhs.real() || lhs.imag() != rhs.imag());
    } else {
        return static_cast<bool>((lhs(0, 0).real() != rhs(0, 0).real() || lhs(0, 0).imag() != rhs(0, 0).imag()) ||
                                 (lhs(1, 1).real() != rhs(1, 1).real() || lhs(1, 1).imag() != rhs(1, 1).imag()) ||
                                 (lhs(2, 2).real() != rhs(2, 2).real() || lhs(2, 2).imag() != rhs(2, 2).imag()));
    }
};

template <bool sym>
inline void update_y_bus(YBus<sym>& y_bus, std::shared_ptr<MathModelParam<sym> const> const& math_model_param) {
#ifdef INCREMENT_UPDATE_Y_BUS
    // verify that the number of branches, shunts and source is the same
    MathModelParam<sym> const& y_bus_param = y_bus.math_model_param();
    assert(y_bus_param.branch_param.size() == math_model_param->branch_param.size());
    assert(y_bus_param.shunt_param.size() == math_model_param->shunt_param.size());
    assert(y_bus_param.source_param.size() == math_model_param->source_param.size());

    // when sequence_idx_map not available: loop through all branches and shunts of math_model_param, check changes
    auto branch_param_to_change_views =
        boost::irange(y_bus_param.branch_param.size()) |
        boost::adaptors::filtered([&math_model_param, &y_bus_param](Idx i) {
            return cmplx_neq<sym>(math_model_param->branch_param[i].yff(), y_bus_param.branch_param[i].yff()) ||
                   cmplx_neq<sym>(math_model_param->branch_param[i].yft(), y_bus_param.branch_param[i].yft()) ||
                   cmplx_neq<sym>(math_model_param->branch_param[i].ytf(), y_bus_param.branch_param[i].ytf()) ||
                   cmplx_neq<sym>(math_model_param->branch_param[i].ytt(), y_bus_param.branch_param[i].ytt());
        });
    auto shunt_param_to_change_views =
        boost::irange(y_bus_param.shunt_param.size()) |
        boost::adaptors::filtered([&math_model_param, &y_bus_param](Idx i) {
            return cmplx_neq<sym>(math_model_param->shunt_param[i], y_bus_param.shunt_param[i]);
        });

    // Placeholder for extracting to_change_indices for both branches and shunts from sequence_idx_map
    // -
    // -

    MathModelParamIncrement<sym> math_model_param_incrmt;
    math_model_param_incrmt.branch_param_to_change = {branch_param_to_change_views.begin(),
                                                      branch_param_to_change_views.end()};
    math_model_param_incrmt.shunt_param_to_change = {shunt_param_to_change_views.begin(),
                                                     shunt_param_to_change_views.end()};

    auto param_incrmt_ptr = std::make_shared<MathModelParamIncrement<sym> const>(math_model_param_incrmt);

    y_bus.update_admittance_increment(math_model_param, param_incrmt_ptr,
                                      false); /* param, changed_param, is_decrement */
#else
    y_bus.update_admittance(math_model_param);
#endif // INCREMENT_UPDATE_Y_BUS
}

// Delta based progressive update for y_bus
template <bool sym>
inline void update_y_bus_increment(YBus<sym>& y_bus, std::shared_ptr<MathModelParam<sym> const> const& math_model_param,
                                   bool increment) {
    auto branch_param_to_change_views =
        boost::irange(0, math_model_param->branch_param.size()) | boost::adaptors::filtered([&math_model_param](Idx i) {
            return math_model_param->branch_param[i].yff() != ComplexTensor<sym>{0.0} ||
                   math_model_param->branch_param[i].yft() != ComplexTensor<sym>{0.0} ||
                   math_model_param->branch_param[i].ytf() != ComplexTensor<sym>{0.0} ||
                   math_model_param->branch_param[i].ytt() != ComplexTensor<sym>{0.0};
        });
    auto shunt_param_to_change_views =
        boost::irange(0, math_model_param->branch_param.size()) | boost::adaptors::filtered([&math_model_param](Idx i) {
            return math_model_param->shunt_param[i] != ComplexTensor<sym>{0.0};
        });

    MathModelParamIncrement<sym> math_model_param_incrmt;
    math_model_param_incrmt.branch_param_to_change = {branch_param_to_change_views.begin(),
                                                      branch_param_to_change_views.end()};
    math_model_param_incrmt.shunt_param_to_change = {shunt_param_to_change_views.begin(),
                                                     shunt_param_to_change_views.end()};

    auto param_incrmt_ptr = std::make_shared<MathModelParamIncrement<sym> const>(math_model_param_incrmt);

    y_bus.update_admittance_increment(math_model_param, param_incrmt_ptr,
                                      !increment); /* param, changed_param, is_decrement */
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
