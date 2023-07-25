// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/component/appliance.hpp>
#include <power_grid_model/component/branch.hpp>
#include <power_grid_model/component/branch3.hpp>
#include <power_grid_model/component/fault.hpp>
#include <power_grid_model/component/line.hpp>
#include <power_grid_model/component/link.hpp>
#include <power_grid_model/component/load_gen.hpp>
#include <power_grid_model/component/node.hpp>
#include <power_grid_model/component/power_sensor.hpp>
#include <power_grid_model/component/sensor.hpp>
#include <power_grid_model/component/shunt.hpp>
#include <power_grid_model/component/source.hpp>
#include <power_grid_model/component/three_winding_transformer.hpp>
#include <power_grid_model/component/transformer.hpp>
#include <power_grid_model/component/voltage_sensor.hpp>

#include <doctest/doctest.h>

#include <iostream>

namespace power_grid_model {

// Test whether it is possible to copy a class to its base class
// (This would mean that we lose private member variables or overloads)
template <typename T, typename U>
concept is_copyable_to = std::derived_from<T, U> && requires(T const t, U u) {
    {
        U {
            t
        }
        } -> std::same_as<U>;       // copy
    { u = t } -> std::same_as<U&>;  // copy assignment
};

static_assert(is_copyable_to<Fault, Fault>);
static_assert(!is_copyable_to<Fault, Base>);

static_assert(is_copyable_to<Line, Line>);
static_assert(!is_copyable_to<Line, Branch>);
static_assert(!is_copyable_to<Line, Base>);

static_assert(is_copyable_to<Link, Link>);
static_assert(!is_copyable_to<Link, Branch>);
static_assert(!is_copyable_to<Link, Base>);

static_assert(is_copyable_to<Node, Node>);
static_assert(!is_copyable_to<Node, Base>);

static_assert(is_copyable_to<Shunt, Shunt>);
static_assert(!is_copyable_to<Shunt, Appliance>);
static_assert(!is_copyable_to<Shunt, Base>);

static_assert(is_copyable_to<Source, Source>);
static_assert(!is_copyable_to<Source, Appliance>);
static_assert(!is_copyable_to<Source, Base>);

static_assert(is_copyable_to<ThreeWindingTransformer, ThreeWindingTransformer>);
static_assert(!is_copyable_to<ThreeWindingTransformer, Branch3>);
static_assert(!is_copyable_to<ThreeWindingTransformer, Base>);

static_assert(is_copyable_to<Transformer, Transformer>);
static_assert(!is_copyable_to<Transformer, Branch>);
static_assert(!is_copyable_to<Transformer, Base>);

// abstract classes (no constructors)
static_assert(std::is_abstract_v<Appliance>);
static_assert(!is_copyable_to<Appliance, Appliance>);

static_assert(std::is_abstract_v<Branch>);
static_assert(!is_copyable_to<Branch, Branch>);

static_assert(std::is_abstract_v<Branch3>);
static_assert(!is_copyable_to<Branch3, Branch3>);

static_assert(std::is_abstract_v<GenericGenerator>);
static_assert(!is_copyable_to<GenericGenerator, GenericGenerator>);

static_assert(std::is_abstract_v<GenericLoad>);
static_assert(!is_copyable_to<GenericLoad, GenericLoad>);

static_assert(std::is_abstract_v<GenericLoadGen>);
static_assert(!is_copyable_to<GenericLoadGen, GenericLoadGen>);

static_assert(std::is_abstract_v<GenericPowerSensor>);
static_assert(!is_copyable_to<GenericPowerSensor, GenericPowerSensor>);

static_assert(std::is_abstract_v<GenericVoltageSensor>);
static_assert(!is_copyable_to<GenericVoltageSensor, GenericVoltageSensor>);

static_assert(std::is_abstract_v<Sensor>);
static_assert(!is_copyable_to<Sensor, Sensor>);

}  // namespace power_grid_model
