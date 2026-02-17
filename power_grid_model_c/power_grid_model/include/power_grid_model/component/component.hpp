// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../common/common.hpp"
#include "../common/enum.hpp"

#include <concepts>
#include <string_view>

namespace power_grid_model {

// change of update cause topology and param change, or just param change
struct UpdateChange {
    bool topo{};
    bool param{};

    friend constexpr UpdateChange operator||(UpdateChange const& x, UpdateChange const& y) {
        return UpdateChange{.topo = x.topo || y.topo, .param = x.param || y.param};
    }
};

template <typename T>
concept component_c = requires(T t, T const& ct, typename T::UpdateType u, typename T::UpdateType const& cu) {
    typename T::InputType;
    typename T::UpdateType;

    { T::name } -> std::convertible_to<std::string_view>;
    { ct.math_model_type() } -> std::convertible_to<ComponentType>;

    { ct.id() } -> std::same_as<ID>;

    { t.update(cu) } -> std::same_as<UpdateChange>;
    { ct.inverse(u) } -> std::same_as<typename T::UpdateType>;
};

struct load_appliance_t {};
struct gen_appliance_t {};

template <typename T>
concept appliance_type_tag = std::same_as<T, load_appliance_t> || std::same_as<T, gen_appliance_t>;
template <appliance_type_tag T> constexpr bool is_generator_v = std::same_as<T, gen_appliance_t>;

static_assert(appliance_type_tag<load_appliance_t>);
static_assert(appliance_type_tag<gen_appliance_t>);
static_assert(!is_generator_v<load_appliance_t>);
static_assert(is_generator_v<gen_appliance_t>);

// Forward declarations of all components
class Base;
class Node;
class Branch;
class Branch3;
class Appliance;
class GenericLoadGen;
class GenericLoad;
class GenericGenerator;
class GenericPowerSensor;
class GenericVoltageSensor;
class GenericCurrentSensor;
class Regulator;
class Line;
class AsymLine;
class Link;
class GenericBranch;
class Transformer;
class ThreeWindingTransformer;
class Shunt;
class Source;

template <symmetry_tag loadgen_symmetry_, appliance_type_tag appliance_type_> class LoadGen;
using SymGenerator = LoadGen<symmetric_t, gen_appliance_t>;
using AsymGenerator = LoadGen<asymmetric_t, gen_appliance_t>;
using SymLoad = LoadGen<symmetric_t, load_appliance_t>;
using AsymLoad = LoadGen<asymmetric_t, load_appliance_t>;

template <symmetry_tag power_sensor_symmetry_> class PowerSensor;
using SymPowerSensor = PowerSensor<symmetric_t>;
using AsymPowerSensor = PowerSensor<asymmetric_t>;

template <symmetry_tag sym> class VoltageSensor;
using SymVoltageSensor = VoltageSensor<symmetric_t>;
using AsymVoltageSensor = VoltageSensor<asymmetric_t>;
template <symmetry_tag sym> class CurrentSensor;
using SymCurrentSensor = CurrentSensor<symmetric_t>;
using AsymCurrentSensor = CurrentSensor<asymmetric_t>;

class Fault;
class TransformerTapRegulator;
class VoltageRegulator;

} // namespace power_grid_model
