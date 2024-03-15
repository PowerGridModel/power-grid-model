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

struct BaseInput {
    ID id;  // ID of the object
};

struct NodeInput {
    ID id;  // ID of the object
    double u_rated;  // rated line-line voltage

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }
};

struct BranchInput {
    ID id;  // ID of the object
    ID from_node;  // node IDs to which this branch is connected at both sides
    ID to_node;  // node IDs to which this branch is connected at both sides
    IntS from_status;  // whether the branch is connected at each side
    IntS to_status;  // whether the branch is connected at each side

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }
};

struct Branch3Input {
    ID id;  // ID of the object
    ID node_1;  // node IDs to which this branch3 is connected at three sides
    ID node_2;  // node IDs to which this branch3 is connected at three sides
    ID node_3;  // node IDs to which this branch3 is connected at three sides
    IntS status_1;  // whether the branch is connected at each side
    IntS status_2;  // whether the branch is connected at each side
    IntS status_3;  // whether the branch is connected at each side

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }
};

struct SensorInput {
    ID id;  // ID of the object
    ID measured_object;  // ID of the measured object

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }
};

struct ApplianceInput {
    ID id;  // ID of the object
    ID node;  // node ID to which this appliance is connected
    IntS status;  // whether the appliance is connected

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }
};

struct LineInput {
    ID id;  // ID of the object
    ID from_node;  // node IDs to which this branch is connected at both sides
    ID to_node;  // node IDs to which this branch is connected at both sides
    IntS from_status;  // whether the branch is connected at each side
    IntS to_status;  // whether the branch is connected at each side
    double r1;  // positive sequence parameters
    double x1;  // positive sequence parameters
    double c1;  // positive sequence parameters
    double tan1;  // positive sequence parameters
    double r0;  // zero sequence parameters
    double x0;  // zero sequence parameters
    double c0;  // zero sequence parameters
    double tan0;  // zero sequence parameters
    double i_n;  // rated current

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }

    // implicit conversions to BranchInput
    operator BranchInput&() { return reinterpret_cast<BranchInput&>(*this); }
    operator BranchInput const&() const { return reinterpret_cast<BranchInput const&>(*this); }
};

struct LinkInput {
    ID id;  // ID of the object
    ID from_node;  // node IDs to which this branch is connected at both sides
    ID to_node;  // node IDs to which this branch is connected at both sides
    IntS from_status;  // whether the branch is connected at each side
    IntS to_status;  // whether the branch is connected at each side

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }

    // implicit conversions to BranchInput
    operator BranchInput&() { return reinterpret_cast<BranchInput&>(*this); }
    operator BranchInput const&() const { return reinterpret_cast<BranchInput const&>(*this); }
};

struct TransformerInput {
    ID id;  // ID of the object
    ID from_node;  // node IDs to which this branch is connected at both sides
    ID to_node;  // node IDs to which this branch is connected at both sides
    IntS from_status;  // whether the branch is connected at each side
    IntS to_status;  // whether the branch is connected at each side
    double u1;  // rated voltage at both side
    double u2;  // rated voltage at both side
    double sn;  // rated power
    double uk;  // short circuit and open testing parameters
    double pk;  // short circuit and open testing parameters
    double i0;  // short circuit and open testing parameters
    double p0;  // short circuit and open testing parameters
    WindingType winding_from;  // winding type at each side
    WindingType winding_to;  // winding type at each side
    IntS clock;  // clock number
    BranchSide tap_side;  // side of tap changer
    IntS tap_pos;  // tap changer parameters
    IntS tap_min;  // tap changer parameters
    IntS tap_max;  // tap changer parameters
    IntS tap_nom;  // tap changer parameters
    double tap_size;  // size of each tap
    double uk_min;  // tap dependent short circuit parameters
    double uk_max;  // tap dependent short circuit parameters
    double pk_min;  // tap dependent short circuit parameters
    double pk_max;  // tap dependent short circuit parameters
    double r_grounding_from;  // grounding information
    double x_grounding_from;  // grounding information
    double r_grounding_to;  // grounding information
    double x_grounding_to;  // grounding information

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }

    // implicit conversions to BranchInput
    operator BranchInput&() { return reinterpret_cast<BranchInput&>(*this); }
    operator BranchInput const&() const { return reinterpret_cast<BranchInput const&>(*this); }
};

struct ThreeWindingTransformerInput {
    ID id;  // ID of the object
    ID node_1;  // node IDs to which this branch3 is connected at three sides
    ID node_2;  // node IDs to which this branch3 is connected at three sides
    ID node_3;  // node IDs to which this branch3 is connected at three sides
    IntS status_1;  // whether the branch is connected at each side
    IntS status_2;  // whether the branch is connected at each side
    IntS status_3;  // whether the branch is connected at each side
    double u1;  // rated voltage at three sides
    double u2;  // rated voltage at three sides
    double u3;  // rated voltage at three sides
    double sn_1;  // rated power at each side
    double sn_2;  // rated power at each side
    double sn_3;  // rated power at each side
    double uk_12;  // short circuit and open testing parameters
    double uk_13;  // short circuit and open testing parameters
    double uk_23;  // short circuit and open testing parameters
    double pk_12;  // short circuit and open testing parameters
    double pk_13;  // short circuit and open testing parameters
    double pk_23;  // short circuit and open testing parameters
    double i0;  // short circuit and open testing parameters
    double p0;  // short circuit and open testing parameters
    WindingType winding_1;  // winding type at each side
    WindingType winding_2;  // winding type at each side
    WindingType winding_3;  // winding type at each side
    IntS clock_12;  // clock numbers
    IntS clock_13;  // clock numbers
    Branch3Side tap_side;  // side of tap changer
    IntS tap_pos;  // tap changer parameters
    IntS tap_min;  // tap changer parameters
    IntS tap_max;  // tap changer parameters
    IntS tap_nom;  // tap changer parameters
    double tap_size;  // size of each tap
    double uk_12_min;  // tap dependent short circuit parameters
    double uk_12_max;  // tap dependent short circuit parameters
    double uk_13_min;  // tap dependent short circuit parameters
    double uk_13_max;  // tap dependent short circuit parameters
    double uk_23_min;  // tap dependent short circuit parameters
    double uk_23_max;  // tap dependent short circuit parameters
    double pk_12_min;  // tap dependent short circuit parameters
    double pk_12_max;  // tap dependent short circuit parameters
    double pk_13_min;  // tap dependent short circuit parameters
    double pk_13_max;  // tap dependent short circuit parameters
    double pk_23_min;  // tap dependent short circuit parameters
    double pk_23_max;  // tap dependent short circuit parameters
    double r_grounding_1;  // grounding information
    double x_grounding_1;  // grounding information
    double r_grounding_2;  // grounding information
    double x_grounding_2;  // grounding information
    double r_grounding_3;  // grounding information
    double x_grounding_3;  // grounding information

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }

    // implicit conversions to Branch3Input
    operator Branch3Input&() { return reinterpret_cast<Branch3Input&>(*this); }
    operator Branch3Input const&() const { return reinterpret_cast<Branch3Input const&>(*this); }
};

struct GenericLoadGenInput {
    ID id;  // ID of the object
    ID node;  // node ID to which this appliance is connected
    IntS status;  // whether the appliance is connected
    LoadGenType type;  // type of the load_gen

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }

    // implicit conversions to ApplianceInput
    operator ApplianceInput&() { return reinterpret_cast<ApplianceInput&>(*this); }
    operator ApplianceInput const&() const { return reinterpret_cast<ApplianceInput const&>(*this); }
};

template <symmetry_tag sym_type>
struct LoadGenInput {
    using sym = sym_type;

    ID id;  // ID of the object
    ID node;  // node ID to which this appliance is connected
    IntS status;  // whether the appliance is connected
    LoadGenType type;  // type of the load_gen
    RealValue<sym> p_specified;  // specified active/reactive power
    RealValue<sym> q_specified;  // specified active/reactive power

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }

    // implicit conversions to ApplianceInput
    operator ApplianceInput&() { return reinterpret_cast<ApplianceInput&>(*this); }
    operator ApplianceInput const&() const { return reinterpret_cast<ApplianceInput const&>(*this); }

    // implicit conversions to GenericLoadGenInput
    operator GenericLoadGenInput&() { return reinterpret_cast<GenericLoadGenInput&>(*this); }
    operator GenericLoadGenInput const&() const { return reinterpret_cast<GenericLoadGenInput const&>(*this); }
};

using SymLoadGenInput = LoadGenInput<symmetric_t>;
using AsymLoadGenInput = LoadGenInput<asymmetric_t>;

struct ShuntInput {
    ID id;  // ID of the object
    ID node;  // node ID to which this appliance is connected
    IntS status;  // whether the appliance is connected
    double g1;  // positive sequence admittance
    double b1;  // positive sequence admittance
    double g0;  // zero sequence admittance
    double b0;  // zero sequence admittance

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }

    // implicit conversions to ApplianceInput
    operator ApplianceInput&() { return reinterpret_cast<ApplianceInput&>(*this); }
    operator ApplianceInput const&() const { return reinterpret_cast<ApplianceInput const&>(*this); }
};

struct SourceInput {
    ID id;  // ID of the object
    ID node;  // node ID to which this appliance is connected
    IntS status;  // whether the appliance is connected
    double u_ref;  // reference voltage
    double u_ref_angle;  // reference voltage
    double sk;  // short circuit capacity
    double rx_ratio;  // short circuit capacity
    double z01_ratio;  // short circuit capacity

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }

    // implicit conversions to ApplianceInput
    operator ApplianceInput&() { return reinterpret_cast<ApplianceInput&>(*this); }
    operator ApplianceInput const&() const { return reinterpret_cast<ApplianceInput const&>(*this); }
};

struct GenericVoltageSensorInput {
    ID id;  // ID of the object
    ID measured_object;  // ID of the measured object
    double u_sigma;  // sigma of error margin of voltage measurement

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }

    // implicit conversions to SensorInput
    operator SensorInput&() { return reinterpret_cast<SensorInput&>(*this); }
    operator SensorInput const&() const { return reinterpret_cast<SensorInput const&>(*this); }
};

template <symmetry_tag sym_type>
struct VoltageSensorInput {
    using sym = sym_type;

    ID id;  // ID of the object
    ID measured_object;  // ID of the measured object
    double u_sigma;  // sigma of error margin of voltage measurement
    RealValue<sym> u_measured;  // measured voltage magnitude and angle
    RealValue<sym> u_angle_measured;  // measured voltage magnitude and angle

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }

    // implicit conversions to SensorInput
    operator SensorInput&() { return reinterpret_cast<SensorInput&>(*this); }
    operator SensorInput const&() const { return reinterpret_cast<SensorInput const&>(*this); }

    // implicit conversions to GenericVoltageSensorInput
    operator GenericVoltageSensorInput&() { return reinterpret_cast<GenericVoltageSensorInput&>(*this); }
    operator GenericVoltageSensorInput const&() const { return reinterpret_cast<GenericVoltageSensorInput const&>(*this); }
};

using SymVoltageSensorInput = VoltageSensorInput<symmetric_t>;
using AsymVoltageSensorInput = VoltageSensorInput<asymmetric_t>;

struct GenericPowerSensorInput {
    ID id;  // ID of the object
    ID measured_object;  // ID of the measured object
    MeasuredTerminalType measured_terminal_type;  // type of measured terminal
    double power_sigma;  // sigma of error margin of apparent power measurement

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }

    // implicit conversions to SensorInput
    operator SensorInput&() { return reinterpret_cast<SensorInput&>(*this); }
    operator SensorInput const&() const { return reinterpret_cast<SensorInput const&>(*this); }
};

template <symmetry_tag sym_type>
struct PowerSensorInput {
    using sym = sym_type;

    ID id;  // ID of the object
    ID measured_object;  // ID of the measured object
    MeasuredTerminalType measured_terminal_type;  // type of measured terminal
    double power_sigma;  // sigma of error margin of apparent power measurement
    RealValue<sym> p_measured;  // measured active/reactive power
    RealValue<sym> q_measured;  // measured active/reactive power
    RealValue<sym> p_sigma;  // sigma of error margin of active/reactive power measurement
    RealValue<sym> q_sigma;  // sigma of error margin of active/reactive power measurement

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }

    // implicit conversions to SensorInput
    operator SensorInput&() { return reinterpret_cast<SensorInput&>(*this); }
    operator SensorInput const&() const { return reinterpret_cast<SensorInput const&>(*this); }

    // implicit conversions to GenericPowerSensorInput
    operator GenericPowerSensorInput&() { return reinterpret_cast<GenericPowerSensorInput&>(*this); }
    operator GenericPowerSensorInput const&() const { return reinterpret_cast<GenericPowerSensorInput const&>(*this); }
};

using SymPowerSensorInput = PowerSensorInput<symmetric_t>;
using AsymPowerSensorInput = PowerSensorInput<asymmetric_t>;

struct FaultInput {
    ID id;  // ID of the object
    IntS status;  // whether the appliance is connected
    FaultType fault_type;  // type of the fault
    FaultPhase fault_phase;  // phase(s) of the fault
    ID fault_object;  // ID of the faulty object
    double r_f;  // short circuit impedance
    double x_f;  // short circuit impedance

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }
};



} // namespace power_grid_model

// clang-format on