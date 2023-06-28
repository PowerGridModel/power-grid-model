// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_ALL_COMPONENTS_HPP
#define POWER_GRID_MODEL_ALL_COMPONENTS_HPP

// define all components
#include "power_grid_model.hpp"
// component include
#include "component/appliance.hpp"
#include "component/fault.hpp"
#include "component/line.hpp"
#include "component/link.hpp"
#include "component/load_gen.hpp"
#include "component/node.hpp"
#include "component/power_sensor.hpp"
#include "component/sensor.hpp"
#include "component/shunt.hpp"
#include "component/source.hpp"
#include "component/three_winding_transformer.hpp"
#include "component/transformer.hpp"
#include "component/voltage_sensor.hpp"

namespace power_grid_model {

using AllComponents =
    ComponentList<Node, Line, Link, Transformer, ThreeWindingTransformer, Shunt, Source, SymGenerator, AsymGenerator,
                  SymLoad, AsymLoad, SymPowerSensor, AsymPowerSensor, SymVoltageSensor, AsymVoltageSensor, Fault>;

}  // namespace power_grid_model

#endif  // POWER_GRID_MODEL_ALL_COMPONENTS_HPP
