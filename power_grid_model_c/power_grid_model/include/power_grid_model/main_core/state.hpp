// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MAIN_CORE_STATE_HPP
#define POWER_GRID_MODEL_MAIN_CORE_STATE_HPP

#include "../all_components.hpp"
#include "../calculation_parameters.hpp"

namespace power_grid_model::main_core {

template <class CompContainer>
struct MainModelState {
    using ComponentContainer = CompContainer;

    ComponentContainer components;
    // calculation parameters
    std::shared_ptr<ComponentTopology const> comp_topo;
    std::shared_ptr<ComponentToMathCoupling const> comp_coup;
};

}  // namespace power_grid_model::main_core

#endif
