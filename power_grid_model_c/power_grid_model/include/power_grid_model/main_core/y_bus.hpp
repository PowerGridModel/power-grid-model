// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../common/common.hpp"

namespace power_grid_model::main_core {

namespace detail {
template <std::derived_from<Branch> ComponentType, typename ComponentContainer>
constexpr void add_to_increment(std::vector<MathModelParamIncrement>& increments,
                                MainModelState<ComponentContainer> const& state, Idx2D const& changed_component_idx) {
    Idx2D const math_idx =
        state.topo_comp_coup
            ->branch[main_core::get_component_sequence_idx<Branch>(state.components, changed_component_idx)];
    if (math_idx.group == isolated_component) {
        return;
    }
    // assign parameters
    increments[math_idx.group].branch_param_to_change.push_back(math_idx.pos);
}

template <std::derived_from<Branch3> ComponentType, typename ComponentContainer>
constexpr void add_to_increment(std::vector<MathModelParamIncrement>& increments,
                                MainModelState<ComponentContainer> const& state, Idx2D const& changed_component_idx) {
    Idx2DBranch3 const math_idx =
        state.topo_comp_coup
            ->branch3[main_core::get_component_sequence_idx<Branch3>(state.components, changed_component_idx)];
    if (math_idx.group == isolated_component) {
        return;
    }
    // assign parameters, branch3 param consists of three branch parameters
    for (size_t branch2 = 0; branch2 < 3; ++branch2) {
        increments[math_idx.group].branch_param_to_change.push_back(math_idx.pos[branch2]);
    }
}

template <std::same_as<Shunt> ComponentType, typename ComponentContainer>
constexpr void add_to_increment(std::vector<MathModelParamIncrement>& increments,
                                MainModelState<ComponentContainer> const& state, Idx2D const& changed_component_idx) {
    Idx2D const math_idx =
        state.topo_comp_coup
            ->shunt[main_core::get_component_sequence_idx<Shunt>(state.components, changed_component_idx)];
    if (math_idx.group == isolated_component) {
        return;
    }
    // assign parameters
    increments[math_idx.group].shunt_param_to_change.push_back(math_idx.pos);
}

// default implementation for other components, does nothing
template <typename ComponentType, typename ComponentContainer>
constexpr void add_to_increment(std::vector<MathModelParamIncrement> const& /* increments */,
                                MainModelState<ComponentContainer> const& /* state */,
                                Idx2D const& /* changed_component_idx */) {
    // default implementation is no-op
}
} // namespace detail

template <symmetry_tag sym, typename MainModelType>
void prepare_y_bus(typename MainModelType::MainModelState const& state_, Idx n_math_solvers_, MathState& math_state_) {
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
                y_bus_vec.emplace_back(state_.math_topology[i],
                                       std::make_shared<MathModelParam<sym> const>(std::move(math_params[i])),
                                       other_y_bus_vec[i].get_y_bus_structure());
            } else {
                y_bus_vec.emplace_back(state_.math_topology[i],
                                       std::make_shared<MathModelParam<sym> const>(std::move(math_params[i])));
            }
        }
    }
}

template <typename MainModelType>
static std::vector<MathModelParamIncrement>
get_math_param_increment(typename MainModelType::MainModelState const& state, Idx n_math_solvers_,
                         typename MainModelType::SequenceIdx const& parameter_changed_components_) {

    std::vector<MathModelParamIncrement> math_param_increment(n_math_solvers_);

    MainModelType::run_functor_with_all_component_types_return_void(
        [&math_param_increment, &state, &parameter_changed_components_]<typename CompType>() {
            static constexpr auto comp_index = MainModelType::template index_of_component<CompType>;
            for (auto const& changed_component : std::get<comp_index>(parameter_changed_components_)) {
                detail::add_to_increment<CompType, typename MainModelType::ComponentContainer>(
                    math_param_increment, state, changed_component);
            }
        });

    return math_param_increment;
}

template <symmetry_tag sym>
std::vector<MathModelParam<sym>> get_math_param(main_model_state_c auto const& state, Idx n_math_solvers) {
    std::vector<MathModelParam<sym>> math_param(n_math_solvers);
    for (Idx i = 0; i != n_math_solvers; ++i) {
        math_param[i].branch_param.resize(state.math_topology[i]->n_branch());
        math_param[i].shunt_param.resize(state.math_topology[i]->n_shunt());
        math_param[i].source_param.resize(state.math_topology[i]->n_source());
    }
    // loop all branch
    for (Idx i = 0; i != static_cast<Idx>(state.comp_topo->branch_node_idx.size()); ++i) {
        Idx2D const math_idx = state.topo_comp_coup->branch[i];
        if (math_idx.group == isolated_component) {
            continue;
        }
        // assign parameters
        math_param[math_idx.group].branch_param[math_idx.pos] =
            state.components.template get_item_by_seq<Branch>(i).template calc_param<sym>();
    }
    // loop all branch3
    for (Idx i = 0; i != static_cast<Idx>(state.comp_topo->branch3_node_idx.size()); ++i) {
        Idx2DBranch3 const math_idx = state.topo_comp_coup->branch3[i];
        if (math_idx.group == isolated_component) {
            continue;
        }
        // assign parameters, branch3 param consists of three branch parameters
        auto const branch3_param = state.components.template get_item_by_seq<Branch3>(i).template calc_param<sym>();
        for (size_t branch2 = 0; branch2 < 3; ++branch2) {
            math_param[math_idx.group].branch_param[math_idx.pos[branch2]] = branch3_param[branch2];
        }
    }
    // loop all shunt
    for (Idx i = 0; i != static_cast<Idx>(state.comp_topo->shunt_node_idx.size()); ++i) {
        Idx2D const math_idx = state.topo_comp_coup->shunt[i];
        if (math_idx.group == isolated_component) {
            continue;
        }
        // assign parameters
        math_param[math_idx.group].shunt_param[math_idx.pos] =
            state.components.template get_item_by_seq<Shunt>(i).template calc_param<sym>();
    }
    // loop all source
    for (Idx i = 0; i != static_cast<Idx>(state.comp_topo->source_node_idx.size()); ++i) {
        Idx2D const math_idx = state.topo_comp_coup->source[i];
        if (math_idx.group == isolated_component) {
            continue;
        }
        // assign parameters
        math_param[math_idx.group].source_param[math_idx.pos] =
            state.components.template get_item_by_seq<Source>(i).template math_param<sym>();
    }
    return math_param;
}

} // namespace power_grid_model::main_core
