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
    ID id{na_IntID};  // ID of the object
};

struct NodeInput {
    ID id{na_IntID};  // ID of the object
    double u_rated{nan};  // rated line-line voltage

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }
};

struct BranchInput {
    ID id{na_IntID};  // ID of the object
    ID from_node{na_IntID};  // node IDs to which this branch is connected at both sides
    ID to_node{na_IntID};  // node IDs to which this branch is connected at both sides
    IntS from_status{na_IntS};  // whether the branch is connected at each side
    IntS to_status{na_IntS};  // whether the branch is connected at each side

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }
};

struct Branch3Input {
    ID id{na_IntID};  // ID of the object
    ID node_1{na_IntID};  // node IDs to which this branch3 is connected at three sides
    ID node_2{na_IntID};  // node IDs to which this branch3 is connected at three sides
    ID node_3{na_IntID};  // node IDs to which this branch3 is connected at three sides
    IntS status_1{na_IntS};  // whether the branch is connected at each side
    IntS status_2{na_IntS};  // whether the branch is connected at each side
    IntS status_3{na_IntS};  // whether the branch is connected at each side

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }
};

struct SensorInput {
    ID id{na_IntID};  // ID of the object
    ID measured_object{na_IntID};  // ID of the measured object

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }
};

struct ApplianceInput {
    ID id{na_IntID};  // ID of the object
    ID node{na_IntID};  // node ID to which this appliance is connected
    IntS status{na_IntS};  // whether the appliance is connected

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }
};

struct LineInput {
    ID id{na_IntID};  // ID of the object
    ID from_node{na_IntID};  // node IDs to which this branch is connected at both sides
    ID to_node{na_IntID};  // node IDs to which this branch is connected at both sides
    IntS from_status{na_IntS};  // whether the branch is connected at each side
    IntS to_status{na_IntS};  // whether the branch is connected at each side
    double r1{nan};  // positive sequence parameters
    double x1{nan};  // positive sequence parameters
    double c1{nan};  // positive sequence parameters
    double tan1{nan};  // positive sequence parameters
    double r0{nan};  // zero sequence parameters
    double x0{nan};  // zero sequence parameters
    double c0{nan};  // zero sequence parameters
    double tan0{nan};  // zero sequence parameters
    double i_n{nan};  // rated current

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }

    // implicit conversions to BranchInput
    operator BranchInput&() { return reinterpret_cast<BranchInput&>(*this); }
    operator BranchInput const&() const { return reinterpret_cast<BranchInput const&>(*this); }
};

struct AsymLineInput {
    ID id{na_IntID};  // ID of the object
    ID from_node{na_IntID};  // node IDs to which this branch is connected at both sides
    ID to_node{na_IntID};  // node IDs to which this branch is connected at both sides
    IntS from_status{na_IntS};  // whether the branch is connected at each side
    IntS to_status{na_IntS};  // whether the branch is connected at each side
    double r_aa{nan};  // Lower triangle matrix values for R, X and C matrices
    double r_ba{nan};  // Lower triangle matrix values for R, X and C matrices
    double r_bb{nan};  // Lower triangle matrix values for R, X and C matrices
    double r_ca{nan};  // Lower triangle matrix values for R, X and C matrices
    double r_cb{nan};  // Lower triangle matrix values for R, X and C matrices
    double r_cc{nan};  // Lower triangle matrix values for R, X and C matrices
    double r_na{nan};  // Lower triangle matrix values for R, X and C matrices
    double r_nb{nan};  // Lower triangle matrix values for R, X and C matrices
    double r_nc{nan};  // Lower triangle matrix values for R, X and C matrices
    double r_nn{nan};  // Lower triangle matrix values for R, X and C matrices
    double x_aa{nan};  // Lower triangle matrix values for R, X and C matrices
    double x_ba{nan};  // Lower triangle matrix values for R, X and C matrices
    double x_bb{nan};  // Lower triangle matrix values for R, X and C matrices
    double x_ca{nan};  // Lower triangle matrix values for R, X and C matrices
    double x_cb{nan};  // Lower triangle matrix values for R, X and C matrices
    double x_cc{nan};  // Lower triangle matrix values for R, X and C matrices
    double x_na{nan};  // Lower triangle matrix values for R, X and C matrices
    double x_nb{nan};  // Lower triangle matrix values for R, X and C matrices
    double x_nc{nan};  // Lower triangle matrix values for R, X and C matrices
    double x_nn{nan};  // Lower triangle matrix values for R, X and C matrices
    double c_aa{nan};  // Lower triangle matrix values for R, X and C matrices
    double c_ba{nan};  // Lower triangle matrix values for R, X and C matrices
    double c_bb{nan};  // Lower triangle matrix values for R, X and C matrices
    double c_ca{nan};  // Lower triangle matrix values for R, X and C matrices
    double c_cb{nan};  // Lower triangle matrix values for R, X and C matrices
    double c_cc{nan};  // Lower triangle matrix values for R, X and C matrices
    double c0{nan};  // Lower triangle matrix values for R, X and C matrices
    double c1{nan};  // Lower triangle matrix values for R, X and C matrices
    double i_n{nan};  // rated current

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }

    // implicit conversions to BranchInput
    operator BranchInput&() { return reinterpret_cast<BranchInput&>(*this); }
    operator BranchInput const&() const { return reinterpret_cast<BranchInput const&>(*this); }
};

struct GenericBranchInput {
    ID id{na_IntID};  // ID of the object
    ID from_node{na_IntID};  // node IDs to which this branch is connected at both sides
    ID to_node{na_IntID};  // node IDs to which this branch is connected at both sides
    IntS from_status{na_IntS};  // whether the branch is connected at each side
    IntS to_status{na_IntS};  // whether the branch is connected at each side
    double r1{nan};  // positive sequence parameters
    double x1{nan};  // positive sequence parameters
    double g1{nan};  // positive sequence parameters
    double b1{nan};  // positive sequence parameters
    double k{nan};  // off-nominal ratio, default = 1.0
    double theta{nan};  // angle shift in radian
    double sn{nan};  // rated power for calculation of loading (optional)

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }

    // implicit conversions to BranchInput
    operator BranchInput&() { return reinterpret_cast<BranchInput&>(*this); }
    operator BranchInput const&() const { return reinterpret_cast<BranchInput const&>(*this); }
};

struct LinkInput {
    ID id{na_IntID};  // ID of the object
    ID from_node{na_IntID};  // node IDs to which this branch is connected at both sides
    ID to_node{na_IntID};  // node IDs to which this branch is connected at both sides
    IntS from_status{na_IntS};  // whether the branch is connected at each side
    IntS to_status{na_IntS};  // whether the branch is connected at each side

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }

    // implicit conversions to BranchInput
    operator BranchInput&() { return reinterpret_cast<BranchInput&>(*this); }
    operator BranchInput const&() const { return reinterpret_cast<BranchInput const&>(*this); }
};

struct TransformerInput {
    ID id{na_IntID};  // ID of the object
    ID from_node{na_IntID};  // node IDs to which this branch is connected at both sides
    ID to_node{na_IntID};  // node IDs to which this branch is connected at both sides
    IntS from_status{na_IntS};  // whether the branch is connected at each side
    IntS to_status{na_IntS};  // whether the branch is connected at each side
    double u1{nan};  // rated voltage at both side
    double u2{nan};  // rated voltage at both side
    double sn{nan};  // rated power
    double uk{nan};  // short circuit and open testing parameters
    double pk{nan};  // short circuit and open testing parameters
    double i0{nan};  // short circuit and open testing parameters
    double p0{nan};  // short circuit and open testing parameters
    WindingType winding_from{static_cast<WindingType>(na_IntS)};  // winding type at each side
    WindingType winding_to{static_cast<WindingType>(na_IntS)};  // winding type at each side
    IntS clock{na_IntS};  // clock number
    BranchSide tap_side{static_cast<BranchSide>(na_IntS)};  // side of tap changer
    IntS tap_pos{na_IntS};  // tap changer parameters
    IntS tap_min{na_IntS};  // tap changer parameters
    IntS tap_max{na_IntS};  // tap changer parameters
    IntS tap_nom{na_IntS};  // tap changer parameters
    double tap_size{nan};  // size of each tap
    double uk_min{nan};  // tap dependent short circuit parameters
    double uk_max{nan};  // tap dependent short circuit parameters
    double pk_min{nan};  // tap dependent short circuit parameters
    double pk_max{nan};  // tap dependent short circuit parameters
    double r_grounding_from{nan};  // grounding information
    double x_grounding_from{nan};  // grounding information
    double r_grounding_to{nan};  // grounding information
    double x_grounding_to{nan};  // grounding information

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }

    // implicit conversions to BranchInput
    operator BranchInput&() { return reinterpret_cast<BranchInput&>(*this); }
    operator BranchInput const&() const { return reinterpret_cast<BranchInput const&>(*this); }
};

struct ThreeWindingTransformerInput {
    ID id{na_IntID};  // ID of the object
    ID node_1{na_IntID};  // node IDs to which this branch3 is connected at three sides
    ID node_2{na_IntID};  // node IDs to which this branch3 is connected at three sides
    ID node_3{na_IntID};  // node IDs to which this branch3 is connected at three sides
    IntS status_1{na_IntS};  // whether the branch is connected at each side
    IntS status_2{na_IntS};  // whether the branch is connected at each side
    IntS status_3{na_IntS};  // whether the branch is connected at each side
    double u1{nan};  // rated voltage at three sides
    double u2{nan};  // rated voltage at three sides
    double u3{nan};  // rated voltage at three sides
    double sn_1{nan};  // rated power at each side
    double sn_2{nan};  // rated power at each side
    double sn_3{nan};  // rated power at each side
    double uk_12{nan};  // short circuit and open testing parameters
    double uk_13{nan};  // short circuit and open testing parameters
    double uk_23{nan};  // short circuit and open testing parameters
    double pk_12{nan};  // short circuit and open testing parameters
    double pk_13{nan};  // short circuit and open testing parameters
    double pk_23{nan};  // short circuit and open testing parameters
    double i0{nan};  // short circuit and open testing parameters
    double p0{nan};  // short circuit and open testing parameters
    WindingType winding_1{static_cast<WindingType>(na_IntS)};  // winding type at each side
    WindingType winding_2{static_cast<WindingType>(na_IntS)};  // winding type at each side
    WindingType winding_3{static_cast<WindingType>(na_IntS)};  // winding type at each side
    IntS clock_12{na_IntS};  // clock numbers
    IntS clock_13{na_IntS};  // clock numbers
    Branch3Side tap_side{static_cast<Branch3Side>(na_IntS)};  // side of tap changer
    IntS tap_pos{na_IntS};  // tap changer parameters
    IntS tap_min{na_IntS};  // tap changer parameters
    IntS tap_max{na_IntS};  // tap changer parameters
    IntS tap_nom{na_IntS};  // tap changer parameters
    double tap_size{nan};  // size of each tap
    double uk_12_min{nan};  // tap dependent short circuit parameters
    double uk_12_max{nan};  // tap dependent short circuit parameters
    double uk_13_min{nan};  // tap dependent short circuit parameters
    double uk_13_max{nan};  // tap dependent short circuit parameters
    double uk_23_min{nan};  // tap dependent short circuit parameters
    double uk_23_max{nan};  // tap dependent short circuit parameters
    double pk_12_min{nan};  // tap dependent short circuit parameters
    double pk_12_max{nan};  // tap dependent short circuit parameters
    double pk_13_min{nan};  // tap dependent short circuit parameters
    double pk_13_max{nan};  // tap dependent short circuit parameters
    double pk_23_min{nan};  // tap dependent short circuit parameters
    double pk_23_max{nan};  // tap dependent short circuit parameters
    double r_grounding_1{nan};  // grounding information
    double x_grounding_1{nan};  // grounding information
    double r_grounding_2{nan};  // grounding information
    double x_grounding_2{nan};  // grounding information
    double r_grounding_3{nan};  // grounding information
    double x_grounding_3{nan};  // grounding information

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }

    // implicit conversions to Branch3Input
    operator Branch3Input&() { return reinterpret_cast<Branch3Input&>(*this); }
    operator Branch3Input const&() const { return reinterpret_cast<Branch3Input const&>(*this); }
};

struct GenericLoadGenInput {
    ID id{na_IntID};  // ID of the object
    ID node{na_IntID};  // node ID to which this appliance is connected
    IntS status{na_IntS};  // whether the appliance is connected
    LoadGenType type{static_cast<LoadGenType>(na_IntS)};  // type of the load_gen

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

    ID id{na_IntID};  // ID of the object
    ID node{na_IntID};  // node ID to which this appliance is connected
    IntS status{na_IntS};  // whether the appliance is connected
    LoadGenType type{static_cast<LoadGenType>(na_IntS)};  // type of the load_gen
    RealValue<sym> p_specified{nan};  // specified active/reactive power
    RealValue<sym> q_specified{nan};  // specified active/reactive power

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
    ID id{na_IntID};  // ID of the object
    ID node{na_IntID};  // node ID to which this appliance is connected
    IntS status{na_IntS};  // whether the appliance is connected
    double g1{nan};  // positive sequence admittance
    double b1{nan};  // positive sequence admittance
    double g0{nan};  // zero sequence admittance
    double b0{nan};  // zero sequence admittance

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }

    // implicit conversions to ApplianceInput
    operator ApplianceInput&() { return reinterpret_cast<ApplianceInput&>(*this); }
    operator ApplianceInput const&() const { return reinterpret_cast<ApplianceInput const&>(*this); }
};

struct SourceInput {
    ID id{na_IntID};  // ID of the object
    ID node{na_IntID};  // node ID to which this appliance is connected
    IntS status{na_IntS};  // whether the appliance is connected
    double u_ref{nan};  // reference voltage
    double u_ref_angle{nan};  // reference voltage
    double sk{nan};  // short circuit capacity
    double rx_ratio{nan};  // short circuit capacity
    double z01_ratio{nan};  // short circuit capacity

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }

    // implicit conversions to ApplianceInput
    operator ApplianceInput&() { return reinterpret_cast<ApplianceInput&>(*this); }
    operator ApplianceInput const&() const { return reinterpret_cast<ApplianceInput const&>(*this); }
};

struct GenericVoltageSensorInput {
    ID id{na_IntID};  // ID of the object
    ID measured_object{na_IntID};  // ID of the measured object
    double u_sigma{nan};  // sigma of error margin of voltage measurement

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

    ID id{na_IntID};  // ID of the object
    ID measured_object{na_IntID};  // ID of the measured object
    double u_sigma{nan};  // sigma of error margin of voltage measurement
    RealValue<sym> u_measured{nan};  // measured voltage magnitude and angle
    RealValue<sym> u_angle_measured{nan};  // measured voltage magnitude and angle

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
    ID id{na_IntID};  // ID of the object
    ID measured_object{na_IntID};  // ID of the measured object
    MeasuredTerminalType measured_terminal_type{static_cast<MeasuredTerminalType>(na_IntS)};  // type of measured terminal
    double power_sigma{nan};  // sigma of error margin of apparent power measurement

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

    ID id{na_IntID};  // ID of the object
    ID measured_object{na_IntID};  // ID of the measured object
    MeasuredTerminalType measured_terminal_type{static_cast<MeasuredTerminalType>(na_IntS)};  // type of measured terminal
    double power_sigma{nan};  // sigma of error margin of apparent power measurement
    RealValue<sym> p_measured{nan};  // measured active/reactive power
    RealValue<sym> q_measured{nan};  // measured active/reactive power
    RealValue<sym> p_sigma{nan};  // sigma of error margin of active/reactive power measurement
    RealValue<sym> q_sigma{nan};  // sigma of error margin of active/reactive power measurement

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
    ID id{na_IntID};  // ID of the object
    IntS status{na_IntS};  // whether the appliance is connected
    FaultType fault_type{static_cast<FaultType>(na_IntS)};  // type of the fault
    FaultPhase fault_phase{static_cast<FaultPhase>(na_IntS)};  // phase(s) of the fault
    ID fault_object{na_IntID};  // ID of the faulty object
    double r_f{nan};  // short circuit impedance
    double x_f{nan};  // short circuit impedance

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }
};

struct RegulatorInput {
    ID id{na_IntID};  // ID of the object
    ID regulated_object{na_IntID};  // ID of the regulated object
    IntS status{na_IntS};  // regulator enabled

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }
};

struct TransformerTapRegulatorInput {
    ID id{na_IntID};  // ID of the object
    ID regulated_object{na_IntID};  // ID of the regulated object
    IntS status{na_IntS};  // regulator enabled
    ControlSide control_side{static_cast<ControlSide>(na_IntS)};  // control side of the (three winding) transformer
    double u_set{nan};  // voltage setpoint
    double u_band{nan};  // voltage bandwidth
    double line_drop_compensation_r{nan};  // line drop compensation resistance
    double line_drop_compensation_x{nan};  // line drop compensation reactance

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }

    // implicit conversions to RegulatorInput
    operator RegulatorInput&() { return reinterpret_cast<RegulatorInput&>(*this); }
    operator RegulatorInput const&() const { return reinterpret_cast<RegulatorInput const&>(*this); }
};

struct GenericCurrentSensorInput {
    ID id{na_IntID};  // ID of the object
    ID measured_object{na_IntID};  // ID of the measured object
    MeasuredTerminalType measured_terminal_type{static_cast<MeasuredTerminalType>(na_IntS)};  // type of measured terminal
    AngleMeasurementType angle_measurement_type{static_cast<AngleMeasurementType>(na_IntS)};  // type of angle measurement
    double i_sigma{nan};  // sigma of error margin of current (angle) measurement
    double i_angle_sigma{nan};  // sigma of error margin of current (angle) measurement

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }

    // implicit conversions to SensorInput
    operator SensorInput&() { return reinterpret_cast<SensorInput&>(*this); }
    operator SensorInput const&() const { return reinterpret_cast<SensorInput const&>(*this); }
};

template <symmetry_tag sym_type>
struct CurrentSensorInput {
    using sym = sym_type;

    ID id{na_IntID};  // ID of the object
    ID measured_object{na_IntID};  // ID of the measured object
    MeasuredTerminalType measured_terminal_type{static_cast<MeasuredTerminalType>(na_IntS)};  // type of measured terminal
    AngleMeasurementType angle_measurement_type{static_cast<AngleMeasurementType>(na_IntS)};  // type of angle measurement
    double i_sigma{nan};  // sigma of error margin of current (angle) measurement
    double i_angle_sigma{nan};  // sigma of error margin of current (angle) measurement
    RealValue<sym> i_measured{nan};  // measured current and current angle
    RealValue<sym> i_angle_measured{nan};  // measured current and current angle

    // implicit conversions to BaseInput
    operator BaseInput&() { return reinterpret_cast<BaseInput&>(*this); }
    operator BaseInput const&() const { return reinterpret_cast<BaseInput const&>(*this); }

    // implicit conversions to SensorInput
    operator SensorInput&() { return reinterpret_cast<SensorInput&>(*this); }
    operator SensorInput const&() const { return reinterpret_cast<SensorInput const&>(*this); }

    // implicit conversions to GenericCurrentSensorInput
    operator GenericCurrentSensorInput&() { return reinterpret_cast<GenericCurrentSensorInput&>(*this); }
    operator GenericCurrentSensorInput const&() const { return reinterpret_cast<GenericCurrentSensorInput const&>(*this); }
};

using SymCurrentSensorInput = CurrentSensorInput<symmetric_t>;
using AsymCurrentSensorInput = CurrentSensorInput<asymmetric_t>;



} // namespace power_grid_model

// clang-format on