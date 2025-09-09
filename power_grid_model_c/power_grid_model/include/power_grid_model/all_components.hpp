// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

// define all components
#include "common/common.hpp"
#include "common/component_list.hpp"
// component include
#include "component/appliance.hpp"
#include "component/asym_line.hpp"
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
namespace detail {
template <typename List, typename T, typename... NeedsTypes>
concept dependent_type_check = !IsInList<T, List>::value || (IsInList<NeedsTypes, List>::value && ...);

template <typename CompList>
concept validate_component_types_c =
    dependent_type_check<CompList, Source, Node> &&                  //
    dependent_type_check<CompList, Line, Node> &&                    //
    dependent_type_check<CompList, Link, Node> &&                    //
    dependent_type_check<CompList, Transformer, Node> &&             //
    dependent_type_check<CompList, GenericBranch, Node> &&           //
    dependent_type_check<CompList, AsymLine, Node> &&                //
    dependent_type_check<CompList, ThreeWindingTransformer, Node> && //
    dependent_type_check<CompList, Shunt, Node> &&                   //
    dependent_type_check<CompList, SymGenerator, Node> &&            //
    dependent_type_check<CompList, AsymGenerator, Node> &&           //
    dependent_type_check<CompList, SymLoad, Node> &&                 //
    dependent_type_check<CompList, AsymLoad, Node> &&                //
    dependent_type_check<CompList, SymVoltageSensor, Node> &&        //
    dependent_type_check<CompList, AsymVoltageSensor, Node> &&       //
    dependent_type_check<CompList, SymPowerSensor, Node, Line, AsymLine, Link, GenericBranch, Transformer,
                         ThreeWindingTransformer, SymGenerator, AsymGenerator, SymLoad, AsymLoad> && //
    dependent_type_check<CompList, AsymPowerSensor, Node, Line, AsymLine, Link, GenericBranch, Transformer,
                         ThreeWindingTransformer, SymGenerator, AsymGenerator, SymLoad, AsymLoad> && //
    dependent_type_check<CompList, SymCurrentSensor, Line, AsymLine, Link, GenericBranch, Transformer,
                         ThreeWindingTransformer> && //
    dependent_type_check<CompList, AsymCurrentSensor, Line, AsymLine, Link, GenericBranch, Transformer,
                         ThreeWindingTransformer> &&                                                       //
    dependent_type_check<CompList, TransformerTapRegulator, Node, Transformer, ThreeWindingTransformer> && //
    dependent_type_check<CompList, Fault, Node>;

} // namespace detail

using AllComponents =
    ComponentList<Node, Line, AsymLine, Link, GenericBranch, Transformer, ThreeWindingTransformer, Shunt, Source,
                  SymGenerator, AsymGenerator, SymLoad, AsymLoad, SymPowerSensor, AsymPowerSensor, SymVoltageSensor,
                  AsymVoltageSensor, SymCurrentSensor, AsymCurrentSensor, Fault, TransformerTapRegulator>;

static_assert(detail::validate_component_types_c<AllComponents>);
static_assert(detail::validate_component_types_c<ComponentList<Node, Source>>);
static_assert(detail::validate_component_types_c<ComponentList<Source, Node>>);
static_assert(detail::validate_component_types_c<ComponentList<Node, Line>>);
static_assert(!detail::validate_component_types_c<ComponentList<Line>>);
static_assert(!detail::validate_component_types_c<ComponentList<Source, Line>>);

// TODO We can also collect all Abstract types here
// using AllAbstractTypes =
//     ExtraRetrievableTypes<Base, Node, Branch, Branch3, Appliance, GenericLoadGen, GenericLoad, GenericGenerator,
//                   GenericPowerSensor, GenericVoltageSensor, GenericCurrentSensor, Regulator>;

// TODO Shall we make similar rules for Abstract types?

} // namespace power_grid_model
