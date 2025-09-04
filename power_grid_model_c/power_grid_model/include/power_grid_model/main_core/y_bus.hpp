// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../common/common.hpp"

namespace power_grid_model::main_core {

template <symmetry_tag sym, typename ComponentContainer, typename... ComponentType>
void prepare_y_bus(MainModelState<ComponentContainer> const& state_, Idx n_math_solvers_, MathState& math_state_) {
    std::vector<YBus<sym>>& y_bus_vec = main_core::get_y_bus<sym>(math_state_);
    // also get the vector of other Y_bus (sym -> asym, or asym -> sym)
    std::vector<YBus<other_symmetry_t<sym>>>& other_y_bus_vec =
        main_core::get_y_bus<other_symmetry_t<sym>>(math_state_);
    // If no Ybus exists, build them
    if (y_bus_vec.empty()) {
        bool const other_y_bus_exist = (!other_y_bus_vec.empty());
        y_bus_vec.reserve(n_math_solvers_);
        auto math_params = get_math_param<sym>(state_, n_math_solvers_);

        // Check the branch and shunt indices
        constexpr auto branch_param_in_seq_map =
            std::array{main_core::utils::index_of_component<Line, ComponentType...>,
                       main_core::utils::index_of_component<Link, ComponentType...>,
                       main_core::utils::index_of_component<Transformer, ComponentType...>};
        constexpr auto shunt_param_in_seq_map =
            std::array{main_core::utils::index_of_component<Shunt, ComponentType...>};

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

            y_bus_vec.back().set_branch_param_idx(
                IdxVector{branch_param_in_seq_map.begin(), branch_param_in_seq_map.end()});
            y_bus_vec.back().set_shunt_param_idx(
                IdxVector{shunt_param_in_seq_map.begin(), shunt_param_in_seq_map.end()});
        }
    }
}

template <symmetry_tag sym, typename ComponentContainer, typename... ComponentType>
static std::vector<MathModelParamIncrement> get_math_param_increment(
    MainModelState<ComponentContainer>& received_state, Idx n_math_solvers_,
    std::array<std::vector<Idx2D>, main_core::utils::n_types<ComponentType...>> const& parameter_changed_components_) {
    using AddToIncrement =
        void (*)(std::vector<MathModelParamIncrement>&, MainModelState<ComponentContainer> const&, Idx2D const&);

    static constexpr std::array<AddToIncrement, main_core::utils::n_types<ComponentType...>> add_to_increments{
        [](std::vector<MathModelParamIncrement>& increments, MainModelState<ComponentContainer> const& state,
           Idx2D const& changed_component_idx) {
            if constexpr (std::derived_from<ComponentType, Branch>) {
                Idx2D const math_idx =
                    state.topo_comp_coup
                        ->branch[main_core::get_component_sequence_idx<Branch>(state, changed_component_idx)];
                if (math_idx.group == isolated_component) {
                    return;
                }
                // assign parameters
                increments[math_idx.group].branch_param_to_change.push_back(math_idx.pos);
            } else if constexpr (std::derived_from<ComponentType, Branch3>) {
                Idx2DBranch3 const math_idx =
                    state.topo_comp_coup
                        ->branch3[main_core::get_component_sequence_idx<Branch3>(state, changed_component_idx)];
                if (math_idx.group == isolated_component) {
                    return;
                }
                // assign parameters, branch3 param consists of three branch parameters
                for (size_t branch2 = 0; branch2 < 3; ++branch2) {
                    increments[math_idx.group].branch_param_to_change.push_back(math_idx.pos[branch2]);
                }
            } else if constexpr (std::same_as<ComponentType, Shunt>) {
                Idx2D const math_idx =
                    state.topo_comp_coup
                        ->shunt[main_core::get_component_sequence_idx<Shunt>(state, changed_component_idx)];
                if (math_idx.group == isolated_component) {
                    return;
                }
                // assign parameters
                increments[math_idx.group].shunt_param_to_change.push_back(math_idx.pos);
            }
        }...};

    std::vector<MathModelParamIncrement> math_param_increment(n_math_solvers_);

    for (size_t i = 0; i < main_core::utils::n_types<ComponentType...>; ++i) {
        auto const& changed_type_components = parameter_changed_components_[i];
        auto const& add_type_to_increment = add_to_increments[i];
        for (auto const& changed_component : changed_type_components) {
            add_type_to_increment(math_param_increment, received_state, changed_component);
        }
    }

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
