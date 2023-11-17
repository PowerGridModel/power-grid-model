// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

// This header file is automatically generated. DO NOT modify it manually!

// clang-format off
#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_OUTPUT_HPP
#define POWER_GRID_MODEL_AUXILIARY_OUTPUT_HPP

#include "meta_data.hpp"

#include "../enum.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"

namespace power_grid_model {

struct BaseOutput {
    ID id;  // ID of the object
    IntS energized;  // whether the object is energized
};

static_assert(std::is_standard_layout_v<BaseOutput>);

template <bool sym>
struct NodeOutput {
    ID id;  // ID of the object
    IntS energized;  // whether the object is energized
    RealValue<sym> u_pu;  // voltage magnitude and angle
    RealValue<sym> u;  // voltage magnitude and angle
    RealValue<sym> u_angle;  // voltage magnitude and angle
    RealValue<sym> p;  // node injection
    RealValue<sym> q;  // node injection

    // implicit conversions to BaseOutput
    operator BaseOutput&() { return reinterpret_cast<BaseOutput&>(*this); }
    operator BaseOutput const&() const { return reinterpret_cast<BaseOutput const&>(*this); }
};

static_assert(std::is_standard_layout_v<NodeOutput>);

using SymNodeOutput = NodeOutput<true>;
using AsymNodeOutput = NodeOutput<false>;

static_assert(std::is_standard_layout_v<SymNodeOutput>);
static_assert(std::is_standard_layout_v<AsymNodeOutput>);
template <bool sym>
struct BranchOutput {
    ID id;  // ID of the object
    IntS energized;  // whether the object is energized
    double loading;  // loading of the branch
    RealValue<sym> p_from;  // power flow at from-side
    RealValue<sym> q_from;  // power flow at from-side
    RealValue<sym> i_from;  // power flow at from-side
    RealValue<sym> s_from;  // power flow at from-side
    RealValue<sym> p_to;  // power flow at to-side
    RealValue<sym> q_to;  // power flow at to-side
    RealValue<sym> i_to;  // power flow at to-side
    RealValue<sym> s_to;  // power flow at to-side

    // implicit conversions to BaseOutput
    operator BaseOutput&() { return reinterpret_cast<BaseOutput&>(*this); }
    operator BaseOutput const&() const { return reinterpret_cast<BaseOutput const&>(*this); }
};

static_assert(std::is_standard_layout_v<BranchOutput>);

using SymBranchOutput = BranchOutput<true>;
using AsymBranchOutput = BranchOutput<false>;

static_assert(std::is_standard_layout_v<SymBranchOutput>);
static_assert(std::is_standard_layout_v<AsymBranchOutput>);
template <bool sym>
struct Branch3Output {
    ID id;  // ID of the object
    IntS energized;  // whether the object is energized
    double loading;  // loading of the branch
    RealValue<sym> p_1;  // power flow at side 1
    RealValue<sym> q_1;  // power flow at side 1
    RealValue<sym> i_1;  // power flow at side 1
    RealValue<sym> s_1;  // power flow at side 1
    RealValue<sym> p_2;  // power flow at side 2
    RealValue<sym> q_2;  // power flow at side 2
    RealValue<sym> i_2;  // power flow at side 2
    RealValue<sym> s_2;  // power flow at side 2
    RealValue<sym> p_3;  // power flow at side 3
    RealValue<sym> q_3;  // power flow at side 3
    RealValue<sym> i_3;  // power flow at side 3
    RealValue<sym> s_3;  // power flow at side 3

    // implicit conversions to BaseOutput
    operator BaseOutput&() { return reinterpret_cast<BaseOutput&>(*this); }
    operator BaseOutput const&() const { return reinterpret_cast<BaseOutput const&>(*this); }
};

static_assert(std::is_standard_layout_v<Branch3Output>);

using SymBranch3Output = Branch3Output<true>;
using AsymBranch3Output = Branch3Output<false>;

static_assert(std::is_standard_layout_v<SymBranch3Output>);
static_assert(std::is_standard_layout_v<AsymBranch3Output>);
template <bool sym>
struct ApplianceOutput {
    ID id;  // ID of the object
    IntS energized;  // whether the object is energized
    RealValue<sym> p;  // power flow of the appliance
    RealValue<sym> q;  // power flow of the appliance
    RealValue<sym> i;  // power flow of the appliance
    RealValue<sym> s;  // power flow of the appliance
    RealValue<sym> pf;  // power flow of the appliance

    // implicit conversions to BaseOutput
    operator BaseOutput&() { return reinterpret_cast<BaseOutput&>(*this); }
    operator BaseOutput const&() const { return reinterpret_cast<BaseOutput const&>(*this); }
};

static_assert(std::is_standard_layout_v<ApplianceOutput>);

using SymApplianceOutput = ApplianceOutput<true>;
using AsymApplianceOutput = ApplianceOutput<false>;

static_assert(std::is_standard_layout_v<SymApplianceOutput>);
static_assert(std::is_standard_layout_v<AsymApplianceOutput>);
template <bool sym>
struct VoltageSensorOutput {
    ID id;  // ID of the object
    IntS energized;  // whether the object is energized
    RealValue<sym> u_residual;  // deviation between the measured value and calculated value
    RealValue<sym> u_angle_residual;  // deviation between the measured value and calculated value

    // implicit conversions to BaseOutput
    operator BaseOutput&() { return reinterpret_cast<BaseOutput&>(*this); }
    operator BaseOutput const&() const { return reinterpret_cast<BaseOutput const&>(*this); }
};

static_assert(std::is_standard_layout_v<VoltageSensorOutput>);

using SymVoltageSensorOutput = VoltageSensorOutput<true>;
using AsymVoltageSensorOutput = VoltageSensorOutput<false>;

static_assert(std::is_standard_layout_v<SymVoltageSensorOutput>);
static_assert(std::is_standard_layout_v<AsymVoltageSensorOutput>);
template <bool sym>
struct PowerSensorOutput {
    ID id;  // ID of the object
    IntS energized;  // whether the object is energized
    RealValue<sym> p_residual;  // deviation between the measured value and calculated value
    RealValue<sym> q_residual;  // deviation between the measured value and calculated value

    // implicit conversions to BaseOutput
    operator BaseOutput&() { return reinterpret_cast<BaseOutput&>(*this); }
    operator BaseOutput const&() const { return reinterpret_cast<BaseOutput const&>(*this); }
};

static_assert(std::is_standard_layout_v<PowerSensorOutput>);

using SymPowerSensorOutput = PowerSensorOutput<true>;
using AsymPowerSensorOutput = PowerSensorOutput<false>;

static_assert(std::is_standard_layout_v<SymPowerSensorOutput>);
static_assert(std::is_standard_layout_v<AsymPowerSensorOutput>);
struct FaultOutput {
    ID id;  // ID of the object
    IntS energized;  // whether the object is energized

    // implicit conversions to BaseOutput
    operator BaseOutput&() { return reinterpret_cast<BaseOutput&>(*this); }
    operator BaseOutput const&() const { return reinterpret_cast<BaseOutput const&>(*this); }
};

static_assert(std::is_standard_layout_v<FaultOutput>);

struct FaultShortCircuitOutput {
    ID id;  // ID of the object
    IntS energized;  // whether the object is energized
    RealValue<false> i_f;  // three phase short circuit current magnitude
    RealValue<false> i_f_angle;  // three phase short circuit current angle

    // implicit conversions to BaseOutput
    operator BaseOutput&() { return reinterpret_cast<BaseOutput&>(*this); }
    operator BaseOutput const&() const { return reinterpret_cast<BaseOutput const&>(*this); }
};

static_assert(std::is_standard_layout_v<FaultShortCircuitOutput>);

struct NodeShortCircuitOutput {
    ID id;  // ID of the object
    IntS energized;  // whether the object is energized
    RealValue<false> u_pu;  // initial three phase line-to-ground short circuit voltage magnitude and angle
    RealValue<false> u;  // initial three phase line-to-ground short circuit voltage magnitude and angle
    RealValue<false> u_angle;  // initial three phase line-to-ground short circuit voltage magnitude and angle

    // implicit conversions to BaseOutput
    operator BaseOutput&() { return reinterpret_cast<BaseOutput&>(*this); }
    operator BaseOutput const&() const { return reinterpret_cast<BaseOutput const&>(*this); }
};

static_assert(std::is_standard_layout_v<NodeShortCircuitOutput>);

struct BranchShortCircuitOutput {
    ID id;  // ID of the object
    IntS energized;  // whether the object is energized
    RealValue<false> i_from;  // initial three phase short circuit current flow at from-side
    RealValue<false> i_from_angle;  // initial three phase short circuit current flow at from-side
    RealValue<false> i_to;  // initial three phase short circuit current flow at to-side
    RealValue<false> i_to_angle;  // initial three phase short circuit current flow at to-side

    // implicit conversions to BaseOutput
    operator BaseOutput&() { return reinterpret_cast<BaseOutput&>(*this); }
    operator BaseOutput const&() const { return reinterpret_cast<BaseOutput const&>(*this); }
};

static_assert(std::is_standard_layout_v<BranchShortCircuitOutput>);

struct Branch3ShortCircuitOutput {
    ID id;  // ID of the object
    IntS energized;  // whether the object is energized
    RealValue<false> i_1;  // initial three phase short circuit current flow at side 1
    RealValue<false> i_1_angle;  // initial three phase short circuit current flow at side 1
    RealValue<false> i_2;  // initial three phase short circuit current flow at side 2
    RealValue<false> i_2_angle;  // initial three phase short circuit current flow at side 2
    RealValue<false> i_3;  // initial three phase short circuit current flow at side 3
    RealValue<false> i_3_angle;  // initial three phase short circuit current flow at side 3

    // implicit conversions to BaseOutput
    operator BaseOutput&() { return reinterpret_cast<BaseOutput&>(*this); }
    operator BaseOutput const&() const { return reinterpret_cast<BaseOutput const&>(*this); }
};

static_assert(std::is_standard_layout_v<Branch3ShortCircuitOutput>);

struct ApplianceShortCircuitOutput {
    ID id;  // ID of the object
    IntS energized;  // whether the object is energized
    RealValue<false> i;  // initial three phase short circuit current flow of the appliance
    RealValue<false> i_angle;  // initial three phase short circuit current flow of the appliance

    // implicit conversions to BaseOutput
    operator BaseOutput&() { return reinterpret_cast<BaseOutput&>(*this); }
    operator BaseOutput const&() const { return reinterpret_cast<BaseOutput const&>(*this); }
};

static_assert(std::is_standard_layout_v<ApplianceShortCircuitOutput>);

struct SensorShortCircuitOutput {
    ID id;  // ID of the object
    IntS energized;  // whether the object is energized

    // implicit conversions to BaseOutput
    operator BaseOutput&() { return reinterpret_cast<BaseOutput&>(*this); }
    operator BaseOutput const&() const { return reinterpret_cast<BaseOutput const&>(*this); }
};

static_assert(std::is_standard_layout_v<SensorShortCircuitOutput>);



} // namespace power_grid_model

#endif
// clang-format on