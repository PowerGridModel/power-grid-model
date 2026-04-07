// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../calculation_parameters.hpp"
#include "../common/common.hpp"
#include "../common/counting_iterator.hpp"
#include "../component/branch.hpp"
#include "../component/branch3.hpp"
#include "../component/component.hpp"
#include "../math_solver/y_bus.hpp"
#include "container_queries.hpp"
#include "math_state.hpp"
#include "state.hpp"

#include <algorithm>
#include <concepts>
#include <vector>

namespace power_grid_model::main_core {
constexpr Idx isolated_component{-1};

namespace detail {

template <std::derived_from<Branch> ComponentType, math_model_param_c MathModelParamType, typename ComponentContainer>
constexpr void add_to_math_model_params(std::vector<MathModelParamType>& math_model_param,
                                        MainModelState<ComponentContainer> const& state,
                                        Idx const topology_sequence_idx) {
    Idx2D const math_idx = get_math_id<Branch>(state, topology_sequence_idx);
    if (math_idx.group == isolated_component) {
        return;
    }
    // assign parameters
    auto& model_params = math_model_param[math_idx.group];
    auto branch_params = state.components.template get_item_by_seq<Branch>(topology_sequence_idx)
                             .template calc_param<typename MathModelParamType::sym>();

    if constexpr (std::derived_from<MathModelParamType, MathModelParamIncrement<typename MathModelParamType::sym>>) {
        model_params.branch_param.push_back(std::move(branch_params));
        model_params.branch_param_to_change.push_back(math_idx.pos);
    } else {
        model_params.branch_param[math_idx.pos] = std::move(branch_params);
    }
}

template <std::derived_from<Branch3> ComponentType, math_model_param_c MathModelParamType, typename ComponentContainer>
constexpr void add_to_math_model_params(std::vector<MathModelParamType>& math_model_param,
                                        MainModelState<ComponentContainer> const& state,
                                        Idx const topology_sequence_idx) {
    Idx2DBranch3 const math_idx = get_math_id<Branch3>(state, topology_sequence_idx);
    if (math_idx.group == isolated_component) {
        return;
    }
    // assign parameters, branch3 param consists of three branch parameters
    auto branch3_param = state.components.template get_item_by_seq<Branch3>(topology_sequence_idx)
                             .template calc_param<typename MathModelParamType::sym>();

    auto& model_params = math_model_param[math_idx.group];
    for (Idx const branch2 : IdxRange{3}) {

        if constexpr (std::derived_from<MathModelParamType,
                                        MathModelParamIncrement<typename MathModelParamType::sym>>) {
            model_params.branch_param.push_back(std::move(branch3_param[branch2]));
            model_params.branch_param_to_change.push_back(math_idx.pos[branch2]);
        } else {
            model_params.branch_param[math_idx.pos[branch2]] = std::move(branch3_param[branch2]);
        }
    }
}

template <std::same_as<Shunt> ComponentType, math_model_param_c MathModelParamType, typename ComponentContainer>
constexpr void add_to_math_model_params(std::vector<MathModelParamType>& math_model_param,
                                        MainModelState<ComponentContainer> const& state,
                                        Idx const topology_sequence_idx) {
    Idx2D const math_idx = get_math_id<Shunt>(state, topology_sequence_idx);
    if (math_idx.group == isolated_component) {
        return;
    }

    // assign parameters
    auto shunt_params = state.components.template get_item_by_seq<Shunt>(topology_sequence_idx)
                            .template calc_param<typename MathModelParamType::sym>();

    auto& model_params = math_model_param[math_idx.group];

    if constexpr (std::derived_from<MathModelParamType, MathModelParamIncrement<typename MathModelParamType::sym>>) {
        model_params.shunt_param.push_back(std::move(shunt_params));
        model_params.shunt_param_to_change.push_back(math_idx.pos);
    } else {
        model_params.shunt_param[math_idx.pos] = std::move(shunt_params);
    }
}

template <std::same_as<Source> ComponentType, math_model_param_c MathModelParamType, typename ComponentContainer>
constexpr void add_to_math_model_params(std::vector<MathModelParamType>& math_model_param,
                                        MainModelState<ComponentContainer> const& state,
                                        Idx const topology_sequence_idx) {
    Idx2D const math_idx = get_math_id<Source>(state, topology_sequence_idx);

    if (math_idx.group == isolated_component) {
        return;
    }

    // assign parameters
    auto source_params = state.components.template get_item_by_seq<Source>(topology_sequence_idx)
                             .template math_param<typename MathModelParamType::sym>();
    if constexpr (std::derived_from<MathModelParamType, MathModelParamIncrement<typename MathModelParamType::sym>>) {
        math_model_param[math_idx.group].source_param.push_back(std::move(source_params));
        math_model_param[math_idx.group].source_param_to_change.push_back(math_idx.pos);
    } else {
        math_model_param[math_idx.group].source_param[math_idx.pos] = source_params;
    }
}

// default implementation for other components, does nothing
template <typename ComponentType, math_model_param_c MathModelParamType, typename ComponentContainer>
constexpr void add_to_math_model_params(std::vector<MathModelParamType> const& /* math_param */,
                                        MainModelState<ComponentContainer> const& /* state */,
                                        Idx const& /* topology_sequence_idx */) {
    // default implementation is no-op
}

// default implementation for other components, does nothing
template <typename ComponentType, symmetry_tag sym, typename ComponentContainer>
    requires std::derived_from<ComponentType, Branch> || std::derived_from<ComponentType, Branch3> ||
             std::derived_from<ComponentType, Shunt> || std::derived_from<ComponentType, Source>
constexpr void add_to_increment(std::vector<MathModelParamIncrement<sym>>& increments,
                                MainModelState<ComponentContainer> const& state, Idx2D const& changed_component_idx) {
    Idx const topo_sequence_idx = main_core::get_topology_index<ComponentType>(state.components, changed_component_idx);
    add_to_math_model_params<ComponentType>(increments, state, topo_sequence_idx);
}
// default implementation for other components, does nothing
template <typename ComponentType, symmetry_tag sym, typename ComponentContainer>
constexpr void add_to_increment(std::vector<MathModelParamIncrement<sym>> const& /* increments */,
                                MainModelState<ComponentContainer> const& /* state */,
                                Idx2D const& /* changed_component_idx */) {
    // default implementation is no-op
}
} // namespace detail

template <symmetry_tag sym, typename MainModelType>
inline void prepare_y_bus(typename MainModelType::MainModelState const& state_, Idx n_math_solvers_,
                          MathState& math_state_) {
    std::vector<YBus<sym>>& y_bus_vec = main_core::get_y_bus<sym>(math_state_);
    // also get the vector of other Y_bus (sym -> asym, or asym -> sym)
    std::vector<YBus<other_symmetry_t<sym>>>& other_y_bus_vec =
        main_core::get_y_bus<other_symmetry_t<sym>>(math_state_);
    // If no Ybus exists, build them
    if (y_bus_vec.empty()) {
        bool const other_y_bus_exist = (!other_y_bus_vec.empty());
        y_bus_vec.reserve(n_math_solvers_);
        auto math_params = get_math_param<sym>(state_, n_math_solvers_);

        for (Idx i = 0; i != n_math_solvers_; ++i) {
            // construct from existing Y_bus structure if possible
            if (other_y_bus_exist) {
                y_bus_vec.emplace_back(*state_.math_topology[i], std::move(math_params[i]),
                                       other_y_bus_vec[i].shared_y_bus_structure());
            } else {
                y_bus_vec.emplace_back(*state_.math_topology[i], std::move(math_params[i]));
            }
        }
    }
}

template <symmetry_tag sym, typename MainModelType>
inline std::vector<MathModelParamIncrement<sym>>
get_math_param_increment(typename MainModelType::MainModelState const& state, Idx n_math_solvers_,
                         typename MainModelType::SequenceIdx const& parameter_changed_components_) {

    std::vector<MathModelParamIncrement<sym>> math_param_increment(n_math_solvers_);

    MainModelType::run_functor_with_all_component_types_return_void(
        [&math_param_increment, &state, &parameter_changed_components_]<typename CompType>() {
            static constexpr auto comp_index = MainModelType::template index_of_component<CompType>;
            for (auto const& changed_component : std::get<comp_index>(parameter_changed_components_)) {
                detail::add_to_increment<CompType, sym, typename MainModelType::ComponentContainer>(
                    math_param_increment, state, changed_component);
            }
        });

    return math_param_increment;
}

template <symmetry_tag sym>
inline std::vector<MathModelParam<sym>> get_math_param(main_model_state_c auto const& state, Idx n_math_solvers) {
    std::vector<MathModelParam<sym>> math_param(n_math_solvers);
    for (Idx const i : IdxRange{n_math_solvers}) {
        math_param[i].branch_param.resize(state.math_topology[i]->n_branch());
        math_param[i].shunt_param.resize(state.math_topology[i]->n_shunt());
        math_param[i].source_param.resize(state.math_topology[i]->n_source());
    }
    // loop all branch
    for (Idx const i : IdxRange{std::ssize(state.comp_topo->branch_node_idx)}) {
        detail::add_to_math_model_params<Branch>(math_param, state, i);
    }
    // loop all branch3
    for (Idx const i : IdxRange{std::ssize(state.comp_topo->branch3_node_idx)}) {
        detail::add_to_math_model_params<Branch3>(math_param, state, i);
    }
    // loop all shunt
    for (Idx const i : IdxRange{std::ssize(state.comp_topo->shunt_node_idx)}) {
        detail::add_to_math_model_params<Shunt>(math_param, state, i);
    }
    // loop all source
    for (Idx const i : IdxRange{std::ssize(state.comp_topo->source_node_idx)}) {
        detail::add_to_math_model_params<Source>(math_param, state, i);
    }

    return math_param;
}

} // namespace power_grid_model::main_core
