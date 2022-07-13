// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_COMPONENT_BRANCH3_HPP
#define POWER_GRID_MODEL_COMPONENT_BRANCH3_HPP

#include "../auxiliary/input.hpp"
#include "base.hpp"

namespace power_grid_model {

class Branch3 : public Base {
   public:
	   using InputType = Branch3Input;

};

}  // namespace power_grid_model

#endif