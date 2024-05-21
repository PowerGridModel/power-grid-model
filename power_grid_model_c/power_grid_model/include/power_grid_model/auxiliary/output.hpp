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

struct BaseOutput {
    ID id{na_IntID};  // ID of the object
    IntS energized{na_IntS};  // whether the object is energized
};

template <symmetry_tag sym_type>
struct NodeOutput {
    using sym = sym_type;

    ID id{na_IntID};  // ID of the object
    IntS energized{na_IntS};  // whether the object is energized
    RealValue<sym> u_pu{nan};  // voltage magnitude and angle
    RealValue<sym> u{nan};  // voltage magnitude and angle
    RealValue<sym> u_angle{nan};  // voltage magnitude and angle
    RealValue<sym> p{nan};  // node injection
    RealValue<sym> q{nan};  // node injection

    // implicit conversions to BaseOutput
    operator BaseOutput&() { return reinterpret_cast<BaseOutput&>(*this); }
    operator BaseOutput const&() const { return reinterpret_cast<BaseOutput const&>(*this); }
};

using SymNodeOutput = NodeOutput<symmetric_t>;
using AsymNodeOutput = NodeOutput<asymmetric_t>;

template <symmetry_tag sym_type>
struct BranchOutput {
    using sym = sym_type;

    ID id{na_IntID};  // ID of the object
    IntS energized{na_IntS};  // whether the object is energized
    double loading{nan};  // loading of the branch
    RealValue<sym> p_from{nan};  // power flow at from-side
    RealValue<sym> q_from{nan};  // power flow at from-side
    RealValue<sym> i_from{nan};  // power flow at from-side
    RealValue<sym> s_from{nan};  // power flow at from-side
    RealValue<sym> p_to{nan};  // power flow at to-side
    RealValue<sym> q_to{nan};  // power flow at to-side
    RealValue<sym> i_to{nan};  // power flow at to-side
    RealValue<sym> s_to{nan};  // power flow at to-side

    // implicit conversions to BaseOutput
    operator BaseOutput&() { return reinterpret_cast<BaseOutput&>(*this); }
    operator BaseOutput const&() const { return reinterpret_cast<BaseOutput const&>(*this); }
};

using SymBranchOutput = BranchOutput<symmetric_t>;
using AsymBranchOutput = BranchOutput<asymmetric_t>;

template <symmetry_tag sym_type>
struct Branch3Output {
    using sym = sym_type;

    ID id{na_IntID};  // ID of the object
    IntS energized{na_IntS};  // whether the object is energized
    double loading{nan};  // loading of the branch
    RealValue<sym> p_1{nan};  // power flow at side 1
    RealValue<sym> q_1{nan};  // power flow at side 1
    RealValue<sym> i_1{nan};  // power flow at side 1
    RealValue<sym> s_1{nan};  // power flow at side 1
    RealValue<sym> p_2{nan};  // power flow at side 2
    RealValue<sym> q_2{nan};  // power flow at side 2
    RealValue<sym> i_2{nan};  // power flow at side 2
    RealValue<sym> s_2{nan};  // power flow at side 2
    RealValue<sym> p_3{nan};  // power flow at side 3
    RealValue<sym> q_3{nan};  // power flow at side 3
    RealValue<sym> i_3{nan};  // power flow at side 3
    RealValue<sym> s_3{nan};  // power flow at side 3

    // implicit conversions to BaseOutput
    operator BaseOutput&() { return reinterpret_cast<BaseOutput&>(*this); }
    operator BaseOutput const&() const { return reinterpret_cast<BaseOutput const&>(*this); }
};

using SymBranch3Output = Branch3Output<symmetric_t>;
using AsymBranch3Output = Branch3Output<asymmetric_t>;

template <symmetry_tag sym_type>
struct ApplianceOutput {
    using sym = sym_type;

    ID id{na_IntID};  // ID of the object
    IntS energized{na_IntS};  // whether the object is energized
    RealValue<sym> p{nan};  // power flow of the appliance
    RealValue<sym> q{nan};  // power flow of the appliance
    RealValue<sym> i{nan};  // power flow of the appliance
    RealValue<sym> s{nan};  // power flow of the appliance
    RealValue<sym> pf{nan};  // power flow of the appliance

    // implicit conversions to BaseOutput
    operator BaseOutput&() { return reinterpret_cast<BaseOutput&>(*this); }
    operator BaseOutput const&() const { return reinterpret_cast<BaseOutput const&>(*this); }
};

using SymApplianceOutput = ApplianceOutput<symmetric_t>;
using AsymApplianceOutput = ApplianceOutput<asymmetric_t>;

template <symmetry_tag sym_type>
struct VoltageSensorOutput {
    using sym = sym_type;

    ID id{na_IntID};  // ID of the object
    IntS energized{na_IntS};  // whether the object is energized
    RealValue<sym> u_residual{nan};  // deviation between the measured value and calculated value
    RealValue<sym> u_angle_residual{nan};  // deviation between the measured value and calculated value

    // implicit conversions to BaseOutput
    operator BaseOutput&() { return reinterpret_cast<BaseOutput&>(*this); }
    operator BaseOutput const&() const { return reinterpret_cast<BaseOutput const&>(*this); }
};

using SymVoltageSensorOutput = VoltageSensorOutput<symmetric_t>;
using AsymVoltageSensorOutput = VoltageSensorOutput<asymmetric_t>;

template <symmetry_tag sym_type>
struct PowerSensorOutput {
    using sym = sym_type;

    ID id{na_IntID};  // ID of the object
    IntS energized{na_IntS};  // whether the object is energized
    RealValue<sym> p_residual{nan};  // deviation between the measured value and calculated value
    RealValue<sym> q_residual{nan};  // deviation between the measured value and calculated value

    // implicit conversions to BaseOutput
    operator BaseOutput&() { return reinterpret_cast<BaseOutput&>(*this); }
    operator BaseOutput const&() const { return reinterpret_cast<BaseOutput const&>(*this); }
};

using SymPowerSensorOutput = PowerSensorOutput<symmetric_t>;
using AsymPowerSensorOutput = PowerSensorOutput<asymmetric_t>;

struct FaultOutput {
    ID id{na_IntID};  // ID of the object
    IntS energized{na_IntS};  // whether the object is energized

    // implicit conversions to BaseOutput
    operator BaseOutput&() { return reinterpret_cast<BaseOutput&>(*this); }
    operator BaseOutput const&() const { return reinterpret_cast<BaseOutput const&>(*this); }
};

struct FaultShortCircuitOutput {
    ID id{na_IntID};  // ID of the object
    IntS energized{na_IntS};  // whether the object is energized
    RealValue<asymmetric_t> i_f{nan};  // three phase short circuit current magnitude
    RealValue<asymmetric_t> i_f_angle{nan};  // three phase short circuit current angle

    // implicit conversions to BaseOutput
    operator BaseOutput&() { return reinterpret_cast<BaseOutput&>(*this); }
    operator BaseOutput const&() const { return reinterpret_cast<BaseOutput const&>(*this); }
};

struct NodeShortCircuitOutput {
    ID id{na_IntID};  // ID of the object
    IntS energized{na_IntS};  // whether the object is energized
    RealValue<asymmetric_t> u_pu{nan};  // initial three phase line-to-ground short circuit voltage magnitude and angle
    RealValue<asymmetric_t> u{nan};  // initial three phase line-to-ground short circuit voltage magnitude and angle
    RealValue<asymmetric_t> u_angle{nan};  // initial three phase line-to-ground short circuit voltage magnitude and angle

    // implicit conversions to BaseOutput
    operator BaseOutput&() { return reinterpret_cast<BaseOutput&>(*this); }
    operator BaseOutput const&() const { return reinterpret_cast<BaseOutput const&>(*this); }
};

struct BranchShortCircuitOutput {
    ID id{na_IntID};  // ID of the object
    IntS energized{na_IntS};  // whether the object is energized
    RealValue<asymmetric_t> i_from{nan};  // initial three phase short circuit current flow at from-side
    RealValue<asymmetric_t> i_from_angle{nan};  // initial three phase short circuit current flow at from-side
    RealValue<asymmetric_t> i_to{nan};  // initial three phase short circuit current flow at to-side
    RealValue<asymmetric_t> i_to_angle{nan};  // initial three phase short circuit current flow at to-side

    // implicit conversions to BaseOutput
    operator BaseOutput&() { return reinterpret_cast<BaseOutput&>(*this); }
    operator BaseOutput const&() const { return reinterpret_cast<BaseOutput const&>(*this); }
};

struct Branch3ShortCircuitOutput {
    ID id{na_IntID};  // ID of the object
    IntS energized{na_IntS};  // whether the object is energized
    RealValue<asymmetric_t> i_1{nan};  // initial three phase short circuit current flow at side 1
    RealValue<asymmetric_t> i_1_angle{nan};  // initial three phase short circuit current flow at side 1
    RealValue<asymmetric_t> i_2{nan};  // initial three phase short circuit current flow at side 2
    RealValue<asymmetric_t> i_2_angle{nan};  // initial three phase short circuit current flow at side 2
    RealValue<asymmetric_t> i_3{nan};  // initial three phase short circuit current flow at side 3
    RealValue<asymmetric_t> i_3_angle{nan};  // initial three phase short circuit current flow at side 3

    // implicit conversions to BaseOutput
    operator BaseOutput&() { return reinterpret_cast<BaseOutput&>(*this); }
    operator BaseOutput const&() const { return reinterpret_cast<BaseOutput const&>(*this); }
};

struct ApplianceShortCircuitOutput {
    ID id{na_IntID};  // ID of the object
    IntS energized{na_IntS};  // whether the object is energized
    RealValue<asymmetric_t> i{nan};  // initial three phase short circuit current flow of the appliance
    RealValue<asymmetric_t> i_angle{nan};  // initial three phase short circuit current flow of the appliance

    // implicit conversions to BaseOutput
    operator BaseOutput&() { return reinterpret_cast<BaseOutput&>(*this); }
    operator BaseOutput const&() const { return reinterpret_cast<BaseOutput const&>(*this); }
};

struct SensorShortCircuitOutput {
    ID id{na_IntID};  // ID of the object
    IntS energized{na_IntS};  // whether the object is energized

    // implicit conversions to BaseOutput
    operator BaseOutput&() { return reinterpret_cast<BaseOutput&>(*this); }
    operator BaseOutput const&() const { return reinterpret_cast<BaseOutput const&>(*this); }
};

struct TransformerTapRegulatorOutput {
    ID id{na_IntID};  // ID of the object
    IntS energized{na_IntS};  // whether the object is energized
    IntS tap_pos{na_IntS};  // result of regulated tap position

    // implicit conversions to BaseOutput
    operator BaseOutput&() { return reinterpret_cast<BaseOutput&>(*this); }
    operator BaseOutput const&() const { return reinterpret_cast<BaseOutput const&>(*this); }
};

struct RegulatorShortCircuitOutput {
    ID id{na_IntID};  // ID of the object
    IntS energized{na_IntS};  // whether the object is energized

    // implicit conversions to BaseOutput
    operator BaseOutput&() { return reinterpret_cast<BaseOutput&>(*this); }
    operator BaseOutput const&() const { return reinterpret_cast<BaseOutput const&>(*this); }
};



} // namespace power_grid_model

// clang-format on