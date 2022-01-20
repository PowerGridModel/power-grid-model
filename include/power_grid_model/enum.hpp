// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_ENUM_HPP
#define POWER_GRID_MODEL_ENUM_HPP

#include "power_grid_model.hpp"

namespace power_grid_model {

enum class LoadGenType : IntS {
    const_pq = 0,  // constant power
    const_y = 1,   // constant element_admittance (impedance)
    const_i = 2,   // constant current
};

enum class WindingType : IntS { wye = 0, wye_n = 1, delta = 2 };

enum class BranchSide : IntS { from = 0, to = 1 };

enum class CalculationMethod : IntS { linear = 0, newton_raphson = 1, iterative_linear = 2 };

enum class MeasuredTerminalType : IntS {
    branch_from = 0,
    branch_to = 1,
    source = 2,
    shunt = 3,
    load = 4,
    generator = 5
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
    source = 8
};

// DO NOT change the order of enumerations
// this has special meaning
// for 0b00 - 0b11
//     0bXY, where X, Y means the from(0)/to(1) side of branch
//        i.e. 0b01 is the branch element for Yft
enum class YBusElementType : IntS { bff = 0b00, bft = 0b01, btf = 0b10, btt = 0b11, shunt = 0b100 };

}  // namespace power_grid_model

#endif