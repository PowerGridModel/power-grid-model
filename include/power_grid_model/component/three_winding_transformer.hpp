// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_COMPONENT_THREE_WINDING_TRANSFORMER_HPP
#define POWER_GRID_MODEL_COMPONENT_THREE_WINDING_TRANSFORMER_HPP

#include "branch3.hpp"

namespace power_grid_model {

class ThreeWindingTransformer : public Branch3 {
   public:
    using InputType = ThreeWindingTransformerInput;
    using UpdateType = ThreeWindingTransformerUpdate;
    static constexpr char const* name = "three_winding_transformer";
};

}  // namespace power_grid_model

#endif