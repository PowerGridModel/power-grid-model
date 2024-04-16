// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"

namespace power_grid_model {

enum class LoadGenType : IntS {
    const_pq = 0, // constant power
    const_y = 1,  // constant element_admittance (impedance)
    const_i = 2,  // constant current
};

enum class WindingType : IntS { wye = 0, wye_n = 1, delta = 2, zigzag = 3, zigzag_n = 4 };

enum class BranchSide : IntS { from = 0, to = 1 };

enum class Branch3Side : IntS { side_1 = 0, side_2 = 1, side_3 = 2 };

enum class ControlSide : IntS { from = 0, to = 1, side_1 = 0, side_2 = 1, side_3 = 2 };

enum class CalculationMethod : IntS {
    default_method = -128,
    linear = 0,
    newton_raphson = 1,
    iterative_linear = 2,
    iterative_current = 3,
    linear_current = 4,
    iec60909 = 5,
};

enum class MeasuredTerminalType : IntS {
    branch_from = 0,
    branch_to = 1,
    source = 2,
    shunt = 3,
    load = 4,
    generator = 5,
    branch3_1 = 6,
    branch3_2 = 7,
    branch3_3 = 8,
    node = 9
};

enum class ComponentType : IntS {
    node = 0,
    branch = 1,
    appliance = 2,
    sensor = 3,
    generic_power_sensor = 4,
    generic_voltage_sensor = 5,
    generic_load_gen = 6,
    shunt = 7,
    source = 8,
    branch3 = 9,
    fault = 10,
    regulator = 11,
    transformer_tap_regulator = 12,
    test = -128 // any stub or mock may use this. do not use this in production
};

// DO NOT change the order of enumerations
// this has special meaning
// for 0b00 - 0b11
//     0bXY, where X, Y means the from(0)/to(1) side of branch
//        i.e. 0b01 is the branch element for Yft
enum class YBusElementType : IntS {
    bff = 0b00,
    bft = 0b01,
    btf = 0b10,
    btt = 0b11,
    shunt = 0b100,
    fill_in_ft = 0b101,
    fill_in_tf = 0b110
};

enum class FaultType : IntS {
    three_phase = 0,
    single_phase_to_ground = 1,
    two_phase = 2,
    two_phase_to_ground = 3,
    nan = na_IntS
};

enum class FaultPhase : IntS {
    abc = 0,
    a = 1,
    b = 2,
    c = 3,
    ab = 4,
    ac = 5,
    bc = 6,
    default_value = -1,
    nan = na_IntS
};

enum class ShortCircuitVoltageScaling : IntS { minimum = 0, maximum = 1 };

enum class CType : IntS { c_int32 = 0, c_int8 = 1, c_double = 2, c_double3 = 3 };

enum class SerializationFormat : IntS { json = 0, msgpack = 1 };

enum class OptimizerType : IntS {
    no_optimization = 0,          // do nothing
    automatic_tap_adjustment = 1, // power flow with automatic tap adjustment
};

enum class OptimizerStrategy : IntS { // Conventions for optimization strategies
    any = 0,                          // any = Any{f(x) \in Range} for x \in Domain
    global_minimum = 1,               // global_minimum = argmin{f(x) \in Range} for x in Domain
    global_maximum = 2,               // global_maximum = argmax{f(x) \in Range} for x in Domain
    local_minimum = 3,                // local_minimum = Any{argmin{f(x) \in Range}} for x in Domain
    local_maximum = 4,                // local_maximum = Any{argmax{f(x) \in Range}} for x in Domain
};

} // namespace power_grid_model
