// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

// clang-format off

#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_INPUT_HPP
#define POWER_GRID_MODEL_AUXILIARY_INPUT_HPP

#include "../enum.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"
#include "meta_data.hpp"

namespace power_grid_model {

POWER_GRID_MODEL_DATA_STRUCT_DEF(BaseInput, 0, _, 
    ID, id);

POWER_GRID_MODEL_DATA_STRUCT_DEF(NodeInput, 1, BaseInput, 
    double, u_rated);

POWER_GRID_MODEL_DATA_STRUCT_DEF(BranchInput, 1, BaseInput, 
    ID, from_node, ID, to_node, IntS, from_status, IntS, to_status);

POWER_GRID_MODEL_DATA_STRUCT_DEF(ApplianceInput, 1, BaseInput, 
    ID, node, IntS, status);

POWER_GRID_MODEL_DATA_STRUCT_DEF(LineInput, 1, BranchInput, 
    double, r1, double, x1, double, c1, double, tan1, 
    double, r0, double, x0, double, c0, double, tan0, 
    double, i_n);

struct LinkInput : BranchInput {};

POWER_GRID_MODEL_DATA_STRUCT_DEF(TransformerInput, 1, BranchInput, 
    double, u1, double, u2, double, sn, 
    double, uk, double, pk, double, i0, double, p0, 
    WindingType, winding_from, WindingType, winding_to,
    IntS, clock, BranchSide, tap_side, IntS, tap_pos, IntS, tap_min, IntS, tap_max, IntS, tap_nom, double, tap_size, 
    double, uk_min, double, uk_max, double, pk_min, double, pk_max, 
    double, r_grounding_from, double, x_grounding_from, double, r_grounding_to, double, x_grounding_to);

POWER_GRID_MODEL_DATA_STRUCT_DEF(GenericLoadGenInput, 1, ApplianceInput,
    LoadGenType, type);

template <bool sym>
POWER_GRID_MODEL_DATA_STRUCT_DEF(LoadGenInput, 1, GenericLoadGenInput,
    RealValue<sym>, p_specified,
    RealValue<sym>, q_specified);


POWER_GRID_MODEL_DATA_STRUCT_DEF(ShuntInput, 1, ApplianceInput,
    double, g1, double, b1,
    double, g0, double, b0);

POWER_GRID_MODEL_DATA_STRUCT_DEF(SourceInput, 1, ApplianceInput,
    double, u_ref, double, u_ref_angle,
    double, sk, double, rx_ratio, double, z01_ratio);

using SymLoadGenInput = LoadGenInput<true>;
using AsymLoadGenInput = LoadGenInput<false>;


POWER_GRID_MODEL_DATA_STRUCT_DEF(SensorInput, 1, BaseInput,
    ID, measured_object);


POWER_GRID_MODEL_DATA_STRUCT_DEF(GenericVoltageSensorInput, 1, SensorInput,
    double, u_sigma);

template <bool sym>
POWER_GRID_MODEL_DATA_STRUCT_DEF(VoltageSensorInput, 1, GenericVoltageSensorInput,
    RealValue<sym>, u_measured,
    RealValue<sym>, u_angle_measured);

using SymVoltageSensorInput = VoltageSensorInput<true>;
using AsymVoltageSensorInput = VoltageSensorInput<false>;

POWER_GRID_MODEL_DATA_STRUCT_DEF(GenericPowerSensorInput, 1, SensorInput,
    MeasuredTerminalType, measured_terminal_type,
    double, power_sigma);

template <bool sym>
POWER_GRID_MODEL_DATA_STRUCT_DEF(PowerSensorInput, 1, GenericPowerSensorInput,
    RealValue<sym>, p_measured,
    RealValue<sym>, q_measured);

using SymPowerSensorInput = PowerSensorInput<true>;
using AsymPowerSensorInput = PowerSensorInput<false>;

// update
using BaseUpdate = BaseInput;

POWER_GRID_MODEL_DATA_STRUCT_DEF(BranchUpdate, 1, BaseUpdate,
    IntS, from_status, IntS, to_status);

POWER_GRID_MODEL_DATA_STRUCT_DEF(TransformerUpdate, 1, BranchUpdate,
    IntS, tap_pos);

POWER_GRID_MODEL_DATA_STRUCT_DEF(ApplianceUpdate, 1, BaseUpdate,
    IntS, status);

POWER_GRID_MODEL_DATA_STRUCT_DEF(SourceUpdate, 1, ApplianceUpdate,
    double, u_ref, double, u_ref_angle);

template <bool sym>
POWER_GRID_MODEL_DATA_STRUCT_DEF(LoadGenUpdate, 1, ApplianceUpdate,
    RealValue<sym>, p_specified, RealValue<sym>, q_specified);

using SymLoadGenUpdate = LoadGenUpdate<true>;
using AsymLoadGenUpdate = LoadGenUpdate<false>;

template <bool sym>
POWER_GRID_MODEL_DATA_STRUCT_DEF(VoltageSensorUpdate, 1, BaseUpdate,
    double, u_sigma,
    RealValue<sym>, u_measured,
    RealValue<sym>, u_angle_measured);

using SymVoltageSensorUpdate = VoltageSensorUpdate<true>;
using AsymVoltageSensorUpdate = VoltageSensorUpdate<false>;

template <bool sym>
POWER_GRID_MODEL_DATA_STRUCT_DEF(PowerSensorUpdate, 1, BaseUpdate,
    double, power_sigma,
    RealValue<sym>, p_measured,
    RealValue<sym>, q_measured);

using SymPowerSensorUpdate = PowerSensorUpdate<true>;
using AsymPowerSensorUpdate = PowerSensorUpdate<false>;

}  // namespace power_grid_model

#endif

// clang-format on
