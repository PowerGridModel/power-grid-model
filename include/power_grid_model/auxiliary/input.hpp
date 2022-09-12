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

POWER_GRID_MODEL_DATA_STRUCT_DEF(Branch3Input, 1, BaseInput, 
    ID, node_1, ID, node_2, ID, node_3, IntS, status_1, IntS, status_2, IntS, status_3);

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

// Due to the maximum entries in the POWER_GRID_MODEL_DATA_STRUCT_DEF macro, ThreeWindingTransformerInput is created in two steps
// TODO: generate input data in a different way
POWER_GRID_MODEL_DATA_STRUCT_DEF(ThreeWindingTransformerInputBasics, 1, Branch3Input, 
    double, u1, double, u2, double, u3, double, sn_1, double, sn_2, double, sn_3, 
    double, uk_12, double, uk_13, double, uk_23, double, pk_12, double, pk_13, double, pk_23, double, i0, double, p0, 
    WindingType, winding_1, WindingType, winding_2, WindingType, winding_3,
    IntS, clock_12, IntS, clock_13,
    Branch3Side, tap_side, IntS, tap_pos, IntS, tap_min, IntS, tap_max, IntS, tap_nom, double, tap_size);

POWER_GRID_MODEL_DATA_STRUCT_DEF(ThreeWindingTransformerInput, 1, ThreeWindingTransformerInputBasics, 
    double, uk_12_min, double, uk_12_max, double, uk_13_min, double, uk_13_max, double, uk_23_min, double, uk_23_max, 
    double, pk_12_min, double, pk_12_max, double, pk_13_min, double, pk_13_max, double, pk_23_min, double, pk_23_max, 
    double, r_grounding_1, double, x_grounding_1, double, r_grounding_2, double, x_grounding_2, 
    double, r_grounding_3, double, x_grounding_3);

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

POWER_GRID_MODEL_DATA_STRUCT_DEF(Branch3Update, 1, BaseUpdate,
    IntS, status_1, IntS, status_2, IntS, status_3);

POWER_GRID_MODEL_DATA_STRUCT_DEF(TransformerUpdate, 1, BranchUpdate,
    IntS, tap_pos);

POWER_GRID_MODEL_DATA_STRUCT_DEF(ThreeWindingTransformerUpdate, 1, Branch3Update,
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
