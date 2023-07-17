// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MAIN_CORE_STATE_HPP
#define POWER_GRID_MODEL_MAIN_CORE_STATE_HPP

#include "../calculation_parameters.hpp"
#include "../container.hpp"

#include <concepts>

namespace power_grid_model::main_core {

template <class CompContainer>
struct MainModelState {
    using ComponentContainer = CompContainer;

    ComponentContainer components;
    // calculation parameters
    std::shared_ptr<ComponentTopology const> comp_topo;
    std::shared_ptr<ComponentToMathCoupling const> comp_coup;
};

template <typename ContainerType, typename ComponentType>
concept component_container = requires(ContainerType const& c) {
    { c.template citer<ComponentType>().begin() } -> std::forward_iterator;
    { c.template citer<ComponentType>().end() } -> std::forward_iterator;
    { *(c.template citer<ComponentType>().begin()) } -> std::same_as<ComponentType const&>;
    { *(c.template citer<ComponentType>().end()) } -> std::same_as<ComponentType const&>;
};

template <template <typename T> class StateType, typename ContainerType, typename ComponentType>
concept model_component_state =
    component_container<typename StateType<ContainerType>::ComponentContainer, ComponentType> &&
    std::same_as<StateType<ContainerType>, MainModelState<ContainerType>>;

}  // namespace power_grid_model::main_core

#endif
