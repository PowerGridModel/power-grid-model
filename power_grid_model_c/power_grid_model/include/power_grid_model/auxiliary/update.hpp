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

struct BranchUpdate {
    ID id;  // ID of the object
    IntS from_status;  // whether the branch is connected at each side
    IntS to_status;  // whether the branch is connected at each side

    // implicit conversions to BaseUpdate
    operator BaseUpdate&() { return reinterpret_cast<BaseUpdate&>(*this); }
    operator BaseUpdate const&() const { return reinterpret_cast<BaseUpdate const&>(*this); }
};

struct Branch3Update {
    ID id;  // ID of the object
    IntS status_1;  // whether the branch is connected at each side
    IntS status_2;  // whether the branch is connected at each side
    IntS status_3;  // whether the branch is connected at each side

    // implicit conversions to BaseUpdate
    operator BaseUpdate&() { return reinterpret_cast<BaseUpdate&>(*this); }
    operator BaseUpdate const&() const { return reinterpret_cast<BaseUpdate const&>(*this); }
};

struct ApplianceUpdate {
    ID id;  // ID of the object
    IntS status;  // whether the appliance is connected

    // implicit conversions to BaseUpdate
    operator BaseUpdate&() { return reinterpret_cast<BaseUpdate&>(*this); }
    operator BaseUpdate const&() const { return reinterpret_cast<BaseUpdate const&>(*this); }
};

struct TransformerUpdate {
    ID id;  // ID of the object
    IntS from_status;  // whether the branch is connected at each side
    IntS to_status;  // whether the branch is connected at each side
    IntS tap_pos;  // tap changer parameters

    // implicit conversions to BaseUpdate
    operator BaseUpdate&() { return reinterpret_cast<BaseUpdate&>(*this); }
    operator BaseUpdate const&() const { return reinterpret_cast<BaseUpdate const&>(*this); }

    // implicit conversions to BranchUpdate
    operator BranchUpdate&() { return reinterpret_cast<BranchUpdate&>(*this); }
    operator BranchUpdate const&() const { return reinterpret_cast<BranchUpdate const&>(*this); }
};

struct ThreeWindingTransformerUpdate {
    ID id;  // ID of the object
    IntS status_1;  // whether the branch is connected at each side
    IntS status_2;  // whether the branch is connected at each side
    IntS status_3;  // whether the branch is connected at each side
    IntS tap_pos;  // tap changer parameters

    // implicit conversions to BaseUpdate
    operator BaseUpdate&() { return reinterpret_cast<BaseUpdate&>(*this); }
    operator BaseUpdate const&() const { return reinterpret_cast<BaseUpdate const&>(*this); }

    // implicit conversions to Branch3Update
    operator Branch3Update&() { return reinterpret_cast<Branch3Update&>(*this); }
    operator Branch3Update const&() const { return reinterpret_cast<Branch3Update const&>(*this); }
};

template <bool sym>
struct LoadGenUpdate {
    ID id;  // ID of the object
    IntS status;  // whether the appliance is connected
    RealValue<sym> p_specified;  // specified active/reactive power
    RealValue<sym> q_specified;  // specified active/reactive power

    // implicit conversions to BaseUpdate
    operator BaseUpdate&() { return reinterpret_cast<BaseUpdate&>(*this); }
    operator BaseUpdate const&() const { return reinterpret_cast<BaseUpdate const&>(*this); }

    // implicit conversions to ApplianceUpdate
    operator ApplianceUpdate&() { return reinterpret_cast<ApplianceUpdate&>(*this); }
    operator ApplianceUpdate const&() const { return reinterpret_cast<ApplianceUpdate const&>(*this); }
};

using SymLoadGenUpdate = LoadGenUpdate<true>;
using AsymLoadGenUpdate = LoadGenUpdate<false>;

struct SourceUpdate {
    ID id;  // ID of the object
    IntS status;  // whether the appliance is connected
    double u_ref;  // reference voltage
    double u_ref_angle;  // reference voltage

    // implicit conversions to BaseUpdate
    operator BaseUpdate&() { return reinterpret_cast<BaseUpdate&>(*this); }
    operator BaseUpdate const&() const { return reinterpret_cast<BaseUpdate const&>(*this); }

    // implicit conversions to ApplianceUpdate
    operator ApplianceUpdate&() { return reinterpret_cast<ApplianceUpdate&>(*this); }
    operator ApplianceUpdate const&() const { return reinterpret_cast<ApplianceUpdate const&>(*this); }
};

struct ShuntUpdate {
    ID id;  // ID of the object
    IntS status;  // whether the appliance is connected
    double g1;  // positive sequence admittance
    double b1;  // positive sequence admittance
    double g0;  // zero sequence admittance
    double b0;  // zero sequence admittance

    // implicit conversions to BaseUpdate
    operator BaseUpdate&() { return reinterpret_cast<BaseUpdate&>(*this); }
    operator BaseUpdate const&() const { return reinterpret_cast<BaseUpdate const&>(*this); }

    // implicit conversions to ApplianceUpdate
    operator ApplianceUpdate&() { return reinterpret_cast<ApplianceUpdate&>(*this); }
    operator ApplianceUpdate const&() const { return reinterpret_cast<ApplianceUpdate const&>(*this); }
};

template <bool sym>
struct VoltageSensorUpdate {
    ID id;  // ID of the object
    double u_sigma;  // sigma of error margin of voltage measurement
    RealValue<sym> u_measured;  // measured voltage magnitude and angle
    RealValue<sym> u_angle_measured;  // measured voltage magnitude and angle

    // implicit conversions to BaseUpdate
    operator BaseUpdate&() { return reinterpret_cast<BaseUpdate&>(*this); }
    operator BaseUpdate const&() const { return reinterpret_cast<BaseUpdate const&>(*this); }
};

using SymVoltageSensorUpdate = VoltageSensorUpdate<true>;
using AsymVoltageSensorUpdate = VoltageSensorUpdate<false>;

template <bool sym>
struct PowerSensorUpdate {
    ID id;  // ID of the object
    double power_sigma;  // sigma of error margin of power measurement
    RealValue<sym> p_measured;  // measured active/reactive power
    RealValue<sym> q_measured;  // measured active/reactive power
    RealValue<sym> p_sigma;  // sigma of error margin of active/reactive power measurement
    RealValue<sym> q_sigma;  // sigma of error margin of active/reactive power measurement

    // implicit conversions to BaseUpdate
    operator BaseUpdate&() { return reinterpret_cast<BaseUpdate&>(*this); }
    operator BaseUpdate const&() const { return reinterpret_cast<BaseUpdate const&>(*this); }
};

using SymPowerSensorUpdate = PowerSensorUpdate<true>;
using AsymPowerSensorUpdate = PowerSensorUpdate<false>;

struct FaultUpdate {
    ID id;  // ID of the object
    IntS status;  // whether the fault is connected
    FaultType fault_type;  // type of the fault
    FaultPhase fault_phase;  // phase(s) of the fault
    ID fault_object;  // ID of the faulted object
    double r_f;  // short circuit impedance
    double x_f;  // short circuit impedance

    // implicit conversions to BaseUpdate
    operator BaseUpdate&() { return reinterpret_cast<BaseUpdate&>(*this); }
    operator BaseUpdate const&() const { return reinterpret_cast<BaseUpdate const&>(*this); }
};



} // namespace power_grid_model

#endif
// clang-format on