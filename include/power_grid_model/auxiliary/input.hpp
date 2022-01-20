// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_INPUT_HPP
#define POWER_GRID_MODEL_AUXILIARY_INPUT_HPP

#include "../enum.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"

namespace power_grid_model {

struct BaseInput {
    ID id;
};

struct NodeInput : BaseInput {
    double u_rated;
};

struct BranchInput : BaseInput {
    ID from_node;
    ID to_node;
    IntS from_status;
    IntS to_status;
};

struct ApplianceInput : BaseInput {
    ID node;
    IntS status;
};

struct LineInput : BranchInput {
    double r1, x1, c1, tan1;
    double r0, x0, c0, tan0;
    double i_n;
};

struct LinkInput : BranchInput {};

struct TransformerInput : BranchInput {
    double u1, u2;
    double sn;
    double uk, pk, i0, p0;
    WindingType winding_from, winding_to;
    IntS clock;
    BranchSide tap_side;
    IntS tap_pos, tap_min, tap_max, tap_nom;
    double tap_size;
    double uk_min, uk_max, pk_min, pk_max;
    double r_grounding_from, x_grounding_from;
    double r_grounding_to, x_grounding_to;
};

struct GenericLoadGenInput : ApplianceInput {
    LoadGenType type;
};

template <bool sym>
struct LoadGenInput : GenericLoadGenInput {
    RealValue<sym> p_specified;
    RealValue<sym> q_specified;
};

struct ShuntInput : ApplianceInput {
    double g1, b1;
    double g0, b0;
};

struct SourceInput : ApplianceInput {
    double u_ref;
    double sk, rx_ratio, z01_ratio;
};

using SymLoadGenInput = LoadGenInput<true>;
using AsymLoadGenInput = LoadGenInput<false>;

struct SensorInput : BaseInput {
    ID measured_object;
};

struct GenericVoltageSensorInput : SensorInput {
    double u_sigma;
};

template <bool sym>
struct VoltageSensorInput : GenericVoltageSensorInput {
    RealValue<sym> u_measured;
    RealValue<sym> u_angle_measured;
};

using SymVoltageSensorInput = VoltageSensorInput<true>;
using AsymVoltageSensorInput = VoltageSensorInput<false>;

struct GenericPowerSensorInput : SensorInput {
    MeasuredTerminalType measured_terminal_type;
    double power_sigma;
};

template <bool sym>
struct PowerSensorInput : GenericPowerSensorInput {
    RealValue<sym> p_measured;
    RealValue<sym> q_measured;
};

using SymPowerSensorInput = PowerSensorInput<true>;
using AsymPowerSensorInput = PowerSensorInput<false>;

// update
using BaseUpdate = BaseInput;

struct BranchUpdate : BaseUpdate {
    IntS from_status, to_status;
};
struct TransformerUpdate : BranchUpdate {
    IntS tap_pos;
};
struct ApplianceUpdate : BaseUpdate {
    IntS status;
};
struct SourceUpdate : ApplianceUpdate {
    double u_ref;
};
template <bool sym>
struct LoadGenUpdate : ApplianceUpdate {
    RealValue<sym> p_specified, q_specified;
};

using SymLoadGenUpdate = LoadGenUpdate<true>;
using AsymLoadGenUpdate = LoadGenUpdate<false>;

template <bool sym>
struct VoltageSensorUpdate : BaseUpdate {
    double u_sigma;
    RealValue<sym> u_measured;
    RealValue<sym> u_angle_measured;
};

using SymVoltageSensorUpdate = VoltageSensorUpdate<true>;
using AsymVoltageSensorUpdate = VoltageSensorUpdate<false>;

template <bool sym>
struct PowerSensorUpdate : BaseUpdate {
    double power_sigma;
    RealValue<sym> p_measured;
    RealValue<sym> q_measured;
};

using SymPowerSensorUpdate = PowerSensorUpdate<true>;
using AsymPowerSensorUpdate = PowerSensorUpdate<false>;

}  // namespace power_grid_model

#endif
