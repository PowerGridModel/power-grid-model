// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_COMPONENT_SHORT_CIRCUIT_HPP
#define POWER_GRID_MODEL_COMPONENT_SHORT_CIRCUIT_HPP

#include "../auxiliary/input.hpp"
#include "base.hpp"

namespace power_grid_model {

Class ShortCircuit final : public Base {
   public:
    using InputType = ShortCircuitInput;
    static constexpr char *const "short_circuit";
}

}  // namespace power_grid_model

#endif