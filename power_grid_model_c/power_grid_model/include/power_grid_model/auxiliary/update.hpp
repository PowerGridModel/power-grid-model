// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// This header file is automatically generated. DO NOT modify it manually!

// clang-format off
#pragma once

#include "meta_data.hpp"

#include "../common/common.hpp"
#include "../common/enum.hpp"
#include "../common/three_phase_tensor.hpp"

namespace power_grid_model {

struct BaseUpdate {
    ID id{na_IntID};  // ID of the object
};

struct BranchUpdate {
    ID id{na_IntID};  // ID of the object
    IntS from_status{na_IntS};  // whether the branch is connected at each side
    IntS to_status{na_IntS};  // whether the branch is connected at each side

    // implicit conversions to BaseUpdate
    operator BaseUpdate&() { return reinterpret_cast<BaseUpdate&>(*this); }
    operator BaseUpdate const&() const { return reinterpret_cast<BaseUpdate const&>(*this); }
};

struct Branch3Update {
    ID id{na_IntID};  // ID of the object
    IntS status_1{na_IntS};  // whether the branch is connected at each side
    IntS status_2{na_IntS};  // whether the branch is connected at each side
    IntS status_3{na_IntS};  // whether the branch is connected at each side

    // implicit conversions to BaseUpdate
    operator BaseUpdate&() { return reinterpret_cast<BaseUpdate&>(*this); }
    operator BaseUpdate const&() const { return reinterpret_cast<BaseUpdate const&>(*this); }
};

struct ApplianceUpdate {
    ID id{na_IntID};  // ID of the object
    IntS status{na_IntS};  // whether the appliance is connected

    // implicit conversions to BaseUpdate
    operator BaseUpdate&() { return reinterpret_cast<BaseUpdate&>(*this); }
    operator BaseUpdate const&() const { return reinterpret_cast<BaseUpdate const&>(*this); }
};

struct TransformerUpdate {
    ID id{na_IntID};  // ID of the object
    IntS from_status{na_IntS};  // whether the branch is connected at each side
    IntS to_status{na_IntS};  // whether the branch is connected at each side
    IntS tap_pos{na_IntS};  // tap changer parameters

    // implicit conversions to BaseUpdate
    operator BaseUpdate&() { return reinterpret_cast<BaseUpdate&>(*this); }
    operator BaseUpdate const&() const { return reinterpret_cast<BaseUpdate const&>(*this); }

    // implicit conversions to BranchUpdate
    operator BranchUpdate&() { return reinterpret_cast<BranchUpdate&>(*this); }
    operator BranchUpdate const&() const { return reinterpret_cast<BranchUpdate const&>(*this); }
};

struct ThreeWindingTransformerUpdate {
    ID id{na_IntID};  // ID of the object
    IntS status_1{na_IntS};  // whether the branch is connected at each side
    IntS status_2{na_IntS};  // whether the branch is connected at each side
    IntS status_3{na_IntS};  // whether the branch is connected at each side
    IntS tap_pos{na_IntS};  // tap changer parameters

    // implicit conversions to BaseUpdate
    operator BaseUpdate&() { return reinterpret_cast<BaseUpdate&>(*this); }
    operator BaseUpdate const&() const { return reinterpret_cast<BaseUpdate const&>(*this); }

    // implicit conversions to Branch3Update
    operator Branch3Update&() { return reinterpret_cast<Branch3Update&>(*this); }
    operator Branch3Update const&() const { return reinterpret_cast<Branch3Update const&>(*this); }
};

template <symmetry_tag sym_type>
struct LoadGenUpdate {
    using sym = sym_type;

    ID id{na_IntID};  // ID of the object
    IntS status{na_IntS};  // whether the appliance is connected
    RealValue<sym> p_specified{nan};  // specified active/reactive power
    RealValue<sym> q_specified{nan};  // specified active/reactive power

    // implicit conversions to BaseUpdate
    operator BaseUpdate&() { return reinterpret_cast<BaseUpdate&>(*this); }
    operator BaseUpdate const&() const { return reinterpret_cast<BaseUpdate const&>(*this); }

    // implicit conversions to ApplianceUpdate
    operator ApplianceUpdate&() { return reinterpret_cast<ApplianceUpdate&>(*this); }
    operator ApplianceUpdate const&() const { return reinterpret_cast<ApplianceUpdate const&>(*this); }
};

using SymLoadGenUpdate = LoadGenUpdate<symmetric_t>;
using AsymLoadGenUpdate = LoadGenUpdate<asymmetric_t>;

struct SourceUpdate {
    ID id{na_IntID};  // ID of the object
    IntS status{na_IntS};  // whether the appliance is connected
    double u_ref{nan};  // reference voltage
    double u_ref_angle{nan};  // reference voltage

    // implicit conversions to BaseUpdate
    operator BaseUpdate&() { return reinterpret_cast<BaseUpdate&>(*this); }
    operator BaseUpdate const&() const { return reinterpret_cast<BaseUpdate const&>(*this); }

    // implicit conversions to ApplianceUpdate
    operator ApplianceUpdate&() { return reinterpret_cast<ApplianceUpdate&>(*this); }
    operator ApplianceUpdate const&() const { return reinterpret_cast<ApplianceUpdate const&>(*this); }
};

struct ShuntUpdate {
    ID id{na_IntID};  // ID of the object
    IntS status{na_IntS};  // whether the appliance is connected
    double g1{nan};  // positive sequence admittance
    double b1{nan};  // positive sequence admittance
    double g0{nan};  // zero sequence admittance
    double b0{nan};  // zero sequence admittance

    // implicit conversions to BaseUpdate
    operator BaseUpdate&() { return reinterpret_cast<BaseUpdate&>(*this); }
    operator BaseUpdate const&() const { return reinterpret_cast<BaseUpdate const&>(*this); }

    // implicit conversions to ApplianceUpdate
    operator ApplianceUpdate&() { return reinterpret_cast<ApplianceUpdate&>(*this); }
    operator ApplianceUpdate const&() const { return reinterpret_cast<ApplianceUpdate const&>(*this); }
};

template <symmetry_tag sym_type>
struct VoltageSensorUpdate {
    using sym = sym_type;

    ID id{na_IntID};  // ID of the object
    double u_sigma{nan};  // sigma of error margin of voltage measurement
    RealValue<sym> u_measured{nan};  // measured voltage magnitude and angle
    RealValue<sym> u_angle_measured{nan};  // measured voltage magnitude and angle

    // implicit conversions to BaseUpdate
    operator BaseUpdate&() { return reinterpret_cast<BaseUpdate&>(*this); }
    operator BaseUpdate const&() const { return reinterpret_cast<BaseUpdate const&>(*this); }
};

using SymVoltageSensorUpdate = VoltageSensorUpdate<symmetric_t>;
using AsymVoltageSensorUpdate = VoltageSensorUpdate<asymmetric_t>;

template <symmetry_tag sym_type>
struct PowerSensorUpdate {
    using sym = sym_type;

    ID id{na_IntID};  // ID of the object
    double power_sigma{nan};  // sigma of error margin of power measurement
    RealValue<sym> p_measured{nan};  // measured active/reactive power
    RealValue<sym> q_measured{nan};  // measured active/reactive power
    RealValue<sym> p_sigma{nan};  // sigma of error margin of active/reactive power measurement
    RealValue<sym> q_sigma{nan};  // sigma of error margin of active/reactive power measurement

    // implicit conversions to BaseUpdate
    operator BaseUpdate&() { return reinterpret_cast<BaseUpdate&>(*this); }
    operator BaseUpdate const&() const { return reinterpret_cast<BaseUpdate const&>(*this); }
};

using SymPowerSensorUpdate = PowerSensorUpdate<symmetric_t>;
using AsymPowerSensorUpdate = PowerSensorUpdate<asymmetric_t>;

struct FaultUpdate {
    ID id{na_IntID};  // ID of the object
    IntS status{na_IntS};  // whether the fault is connected
    FaultType fault_type{static_cast<FaultType>(na_IntS)};  // type of the fault
    FaultPhase fault_phase{static_cast<FaultPhase>(na_IntS)};  // phase(s) of the fault
    ID fault_object{na_IntID};  // ID of the faulted object
    double r_f{nan};  // short circuit impedance
    double x_f{nan};  // short circuit impedance

    // implicit conversions to BaseUpdate
    operator BaseUpdate&() { return reinterpret_cast<BaseUpdate&>(*this); }
    operator BaseUpdate const&() const { return reinterpret_cast<BaseUpdate const&>(*this); }
};

struct RegulatorUpdate {
    ID id{na_IntID};  // ID of the object
    IntS status{na_IntS};  // regulator enables

    // implicit conversions to BaseUpdate
    operator BaseUpdate&() { return reinterpret_cast<BaseUpdate&>(*this); }
    operator BaseUpdate const&() const { return reinterpret_cast<BaseUpdate const&>(*this); }
};

struct TransformerTapRegulatorUpdate {
    ID id{na_IntID};  // ID of the object
    IntS status{na_IntS};  // regulator enables
    double u_set{nan};  // voltage setpoint
    double u_band{nan};  // voltage bandwidth
    double line_drop_compensation_r{nan};  // line drop compensation resistance
    double line_drop_compensation_x{nan};  // line drop compensation reactance

    // implicit conversions to BaseUpdate
    operator BaseUpdate&() { return reinterpret_cast<BaseUpdate&>(*this); }
    operator BaseUpdate const&() const { return reinterpret_cast<BaseUpdate const&>(*this); }

    // implicit conversions to RegulatorUpdate
    operator RegulatorUpdate&() { return reinterpret_cast<RegulatorUpdate&>(*this); }
    operator RegulatorUpdate const&() const { return reinterpret_cast<RegulatorUpdate const&>(*this); }
};

template <symmetry_tag sym_type>
struct CurrentSensorUpdate {
    using sym = sym_type;

    ID id{na_IntID};  // ID of the object
    double i_sigma{nan};  // sigma of error margin of current (angle) measurement
    double i_angle_sigma{nan};  // sigma of error margin of current (angle) measurement
    RealValue<sym> i_measured{nan};  // measured current and current angle
    RealValue<sym> i_angle_measured{nan};  // measured current and current angle

    // implicit conversions to BaseUpdate
    operator BaseUpdate&() { return reinterpret_cast<BaseUpdate&>(*this); }
    operator BaseUpdate const&() const { return reinterpret_cast<BaseUpdate const&>(*this); }
};

using SymCurrentSensorUpdate = CurrentSensorUpdate<symmetric_t>;
using AsymCurrentSensorUpdate = CurrentSensorUpdate<asymmetric_t>;



} // namespace power_grid_model

// clang-format on