// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

// This header file is automatically generated. DO NOT modify it manually!

// clang-format off
#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_INPUT_HPP
#define POWER_GRID_MODEL_AUXILIARY_INPUT_HPP

#include "meta_data.hpp"

#include "../enum.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"

namespace power_grid_model {

struct BaseInput {
    ID id;  // ID of the object
};

struct NodeInput : BaseInput {
    double u_rated;  // rated line-line voltage
};

struct BranchInput : BaseInput {
    ID from_node;  // node IDs to which this branch is connected at both sides
    ID to_node;  // node IDs to which this branch is connected at both sides
    IntS from_status;  // whether the branch is connected at each side
    IntS to_status;  // whether the branch is connected at each side
};

struct Branch3Input : BaseInput {
    ID node_1;  // node IDs to which this branch3 is connected at three sides
    ID node_2;  // node IDs to which this branch3 is connected at three sides
    ID node_3;  // node IDs to which this branch3 is connected at three sides
    IntS status_1;  // whether the branch is connected at each side
    IntS status_2;  // whether the branch is connected at each side
    IntS status_3;  // whether the branch is connected at each side
};

struct SensorInput : BaseInput {
    ID measured_object;  // ID of the measured object
};

struct ApplianceInput : BaseInput {
    ID node;  // node ID to which this appliance is connected
    IntS status;  // whether the appliance is connected
};

struct LineInput : BranchInput {
    double r1;  // positive sequence parameters
    double x1;  // positive sequence parameters
    double c1;  // positive sequence parameters
    double tan1;  // positive sequence parameters
    double r0;  // zero sequence parameters
    double x0;  // zero sequence parameters
    double c0;  // zero sequence parameters
    double tan0;  // zero sequence parameters
    double i_n;  // rated current
};

struct LinkInput : BranchInput {
};

struct TransformerInput : BranchInput {
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
};

struct ThreeWindingTransformerInput : Branch3Input {
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
};

struct GenericLoadGenInput : ApplianceInput {
    LoadGenType type;  // type of the load_gen
};

template <bool sym>
struct LoadGenInput : GenericLoadGenInput {
    RealValue<sym> p_specified;  // specified active/reactive power
    RealValue<sym> q_specified;  // specified active/reactive power
};
using SymLoadGenInput = LoadGenInput<true>;
using AsymLoadGenInput = LoadGenInput<false>;

struct ShuntInput : ApplianceInput {
    double g1;  // positive sequence admittance
    double b1;  // positive sequence admittance
    double g0;  // zero sequence admittance
    double b0;  // zero sequence admittance
};

struct SourceInput : ApplianceInput {
    double u_ref;  // reference voltage
    double u_ref_angle;  // reference voltage
    double sk;  // short circuitl capacity
    double rx_ratio;  // short circuitl capacity
    double z01_ratio;  // short circuitl capacity
};

struct GenericVoltageSensorInput : SensorInput {
    double u_sigma;  // sigma of error margin of voltage measurement
};

template <bool sym>
struct VoltageSensorInput : GenericVoltageSensorInput {
    RealValue<sym> u_measured;  // measured voltage magnitude and angle
    RealValue<sym> u_angle_measured;  // measured voltage magnitude and angle
};
using SymVoltageSensorInput = VoltageSensorInput<true>;
using AsymVoltageSensorInput = VoltageSensorInput<false>;

struct GenericPowerSensorInput : SensorInput {
    MeasuredTerminalType measured_terminal_type;  // type of measured terminal
    double power_sigma;  // sigma of error margin of power measurement
};

template <bool sym>
struct PowerSensorInput : GenericPowerSensorInput {
    RealValue<sym> p_measured;  // measured active/reactive power
    RealValue<sym> q_measured;  // measured active/reactive power
};
using SymPowerSensorInput = PowerSensorInput<true>;
using AsymPowerSensorInput = PowerSensorInput<false>;

struct FaultInput : BaseInput {
    IntS status;  // whether the appliance is connected
    FaultType fault_type;  // type of the fault
    FaultPhase fault_phase;  // phase(s) of the fault
    ID fault_object;  // ID of the faulty object
    double r_f;  // short circuit impedance
    double x_f;  // short circuit impedance
};



} // namespace power_grid_model

#endif
// clang-format on