// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

// define all components
#include "common/common.hpp"
#include "common/component_list.hpp"
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
#include "component/transformer_tap_regulator.hpp"
#include "component/voltage_sensor.hpp"

namespace power_grid_model {

using AllComponents = ComponentList<Node, Line, Link, Transformer, ThreeWindingTransformer, Shunt, Source, SymGenerator,
                                    AsymGenerator, SymLoad, AsymLoad, SymPowerSensor, AsymPowerSensor, SymVoltageSensor,
                                    AsymVoltageSensor, Fault, TransformerTapRegulator>;

} // namespace power_grid_model
