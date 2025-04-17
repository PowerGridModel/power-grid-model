// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

// define all components
#include "common/common.hpp"
#include "common/component_list.hpp"
// component include
#include "component/appliance.hpp"
#include "component/current_sensor.hpp"
#include "component/fault.hpp"
#include "component/generic_branch.hpp"
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
#include "component/transformer_tap_regulator.hpp"
#include "component/voltage_sensor.hpp"

namespace power_grid_model {

using AllComponents =
    ComponentList<Node, Line, Link, GenericBranch, Transformer, ThreeWindingTransformer, Shunt, Source, SymGenerator,
                  AsymGenerator, SymLoad, AsymLoad, SymPowerSensor, AsymPowerSensor, SymVoltageSensor,
                  AsymVoltageSensor, SymCurrentSensor, AsymCurrentSensor, Fault, TransformerTapRegulator>;

template <typename T>
concept power_or_current_sensor_c =
    std::derived_from<T, GenericPowerSensor> || std::derived_from<T, GenericCurrentSensor>;

static_assert(power_or_current_sensor_c<SymPowerSensor>);
static_assert(power_or_current_sensor_c<AsymPowerSensor>);
static_assert(power_or_current_sensor_c<SymCurrentSensor>);
static_assert(power_or_current_sensor_c<AsymCurrentSensor>);
static_assert(!power_or_current_sensor_c<SymVoltageSensor>);
static_assert(!power_or_current_sensor_c<AsymVoltageSensor>);

} // namespace power_grid_model
