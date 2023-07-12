// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

// This header file is automatically generated. DO NOT modify it manually!

// clang-format off
#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_UPDATE_HPP
#define POWER_GRID_MODEL_AUXILIARY_UPDATE_HPP

#include "meta_data.hpp"

#include "../enum.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"

namespace power_grid_model {

struct BaseUpdate {
    ID id;  // ID of the object
};

struct BranchUpdate : BaseUpdate {
    IntS from_status;  // whether the branch is connected at each side
    IntS to_status;  // whether the branch is connected at each side
};

struct Branch3Update : BaseUpdate {
    IntS status_1;  // whether the branch is connected at each side
    IntS status_2;  // whether the branch is connected at each side
    IntS status_3;  // whether the branch is connected at each side
};

struct ApplianceUpdate : BaseUpdate {
    IntS status;  // whether the appliance is connected
};

struct TransformerUpdate : BranchUpdate {
    IntS tap_pos;  // tap changer parameters
};

struct ThreeWindingTransformerUpdate : Branch3Update {
    IntS tap_pos;  // tap changer parameters
};

template <bool sym>
struct LoadGenUpdate : ApplianceUpdate {
    RealValue<sym> p_specified;  // specified active/reactive power
    RealValue<sym> q_specified;  // specified active/reactive power
};
using SymLoadGenUpdate = LoadGenUpdate<true>;
using AsymLoadGenUpdate = LoadGenUpdate<false>;

struct SourceUpdate : ApplianceUpdate {
    double u_ref;  // reference voltage
    double u_ref_angle;  // reference voltage
};

template <bool sym>
struct VoltageSensorUpdate : BaseUpdate {
    double u_sigma;  // sigma of error margin of voltage measurement
    RealValue<sym> u_measured;  // measured voltage magnitude and angle
    RealValue<sym> u_angle_measured;  // measured voltage magnitude and angle
};
using SymVoltageSensorUpdate = VoltageSensorUpdate<true>;
using AsymVoltageSensorUpdate = VoltageSensorUpdate<false>;

template <bool sym>
struct PowerSensorUpdate : BaseUpdate {
    double power_sigma;  // sigma of error margin of power measurement
    RealValue<sym> p_measured;  // measured active/reactive power
    RealValue<sym> q_measured;  // measured active/reactive power
};
using SymPowerSensorUpdate = PowerSensorUpdate<true>;
using AsymPowerSensorUpdate = PowerSensorUpdate<false>;

struct FaultUpdate : BaseUpdate {
    IntS status;  // whether the fault is connected
    FaultType fault_type;  // type of the fault
    FaultPhase fault_phase;  // phase(s) of the fault
    ID fault_object;  // ID of the faulted object
};



} // namespace power_grid_model

#endif
// clang-format on