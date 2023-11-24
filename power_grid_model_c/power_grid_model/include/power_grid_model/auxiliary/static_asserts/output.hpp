// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

// This header file is automatically generated. DO NOT modify it manually!

// clang-format off
#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_STATIC_ASSERTS_OUTPUT_HPP
#define POWER_GRID_MODEL_AUXILIARY_STATIC_ASSERTS_OUTPUT_HPP

#include "../output.hpp" // NOLINT

#include <cstddef>

namespace power_grid_model::test {

// static asserts for BaseOutput
static_assert(std::is_standard_layout_v<BaseOutput>);

// static asserts for NodeOutput<true>
static_assert(std::is_standard_layout_v<NodeOutput<true>>);
// static asserts for conversion of NodeOutput<true> to BaseOutput
static_assert(std::alignment_of_v<NodeOutput<true>> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(NodeOutput<true>::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(NodeOutput<true>::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(NodeOutput<true>, id) == offsetof(BaseOutput, id));
static_assert(offsetof(NodeOutput<true>, energized) == offsetof(BaseOutput, energized));
// static asserts for NodeOutput<false>
static_assert(std::is_standard_layout_v<NodeOutput<false>>);
// static asserts for conversion of NodeOutput<false> to BaseOutput
static_assert(std::alignment_of_v<NodeOutput<false>> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(NodeOutput<false>::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(NodeOutput<false>::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(NodeOutput<false>, id) == offsetof(BaseOutput, id));
static_assert(offsetof(NodeOutput<false>, energized) == offsetof(BaseOutput, energized));
// static asserts for SymNodeOutput
static_assert(std::is_standard_layout_v<SymNodeOutput>);
// static asserts for conversion of SymNodeOutput to BaseOutput
static_assert(std::alignment_of_v<SymNodeOutput> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(SymNodeOutput::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(SymNodeOutput::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(SymNodeOutput, id) == offsetof(BaseOutput, id));
static_assert(offsetof(SymNodeOutput, energized) == offsetof(BaseOutput, energized));
// static asserts for AsymNodeOutput
static_assert(std::is_standard_layout_v<AsymNodeOutput>);
// static asserts for conversion of AsymNodeOutput to BaseOutput
static_assert(std::alignment_of_v<AsymNodeOutput> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(AsymNodeOutput::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(AsymNodeOutput::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(AsymNodeOutput, id) == offsetof(BaseOutput, id));
static_assert(offsetof(AsymNodeOutput, energized) == offsetof(BaseOutput, energized));

// static asserts for BranchOutput<true>
static_assert(std::is_standard_layout_v<BranchOutput<true>>);
// static asserts for conversion of BranchOutput<true> to BaseOutput
static_assert(std::alignment_of_v<BranchOutput<true>> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(BranchOutput<true>::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(BranchOutput<true>::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(BranchOutput<true>, id) == offsetof(BaseOutput, id));
static_assert(offsetof(BranchOutput<true>, energized) == offsetof(BaseOutput, energized));
// static asserts for BranchOutput<false>
static_assert(std::is_standard_layout_v<BranchOutput<false>>);
// static asserts for conversion of BranchOutput<false> to BaseOutput
static_assert(std::alignment_of_v<BranchOutput<false>> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(BranchOutput<false>::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(BranchOutput<false>::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(BranchOutput<false>, id) == offsetof(BaseOutput, id));
static_assert(offsetof(BranchOutput<false>, energized) == offsetof(BaseOutput, energized));
// static asserts for SymBranchOutput
static_assert(std::is_standard_layout_v<SymBranchOutput>);
// static asserts for conversion of SymBranchOutput to BaseOutput
static_assert(std::alignment_of_v<SymBranchOutput> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(SymBranchOutput::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(SymBranchOutput::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(SymBranchOutput, id) == offsetof(BaseOutput, id));
static_assert(offsetof(SymBranchOutput, energized) == offsetof(BaseOutput, energized));
// static asserts for AsymBranchOutput
static_assert(std::is_standard_layout_v<AsymBranchOutput>);
// static asserts for conversion of AsymBranchOutput to BaseOutput
static_assert(std::alignment_of_v<AsymBranchOutput> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(AsymBranchOutput::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(AsymBranchOutput::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(AsymBranchOutput, id) == offsetof(BaseOutput, id));
static_assert(offsetof(AsymBranchOutput, energized) == offsetof(BaseOutput, energized));

// static asserts for Branch3Output<true>
static_assert(std::is_standard_layout_v<Branch3Output<true>>);
// static asserts for conversion of Branch3Output<true> to BaseOutput
static_assert(std::alignment_of_v<Branch3Output<true>> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(Branch3Output<true>::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(Branch3Output<true>::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(Branch3Output<true>, id) == offsetof(BaseOutput, id));
static_assert(offsetof(Branch3Output<true>, energized) == offsetof(BaseOutput, energized));
// static asserts for Branch3Output<false>
static_assert(std::is_standard_layout_v<Branch3Output<false>>);
// static asserts for conversion of Branch3Output<false> to BaseOutput
static_assert(std::alignment_of_v<Branch3Output<false>> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(Branch3Output<false>::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(Branch3Output<false>::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(Branch3Output<false>, id) == offsetof(BaseOutput, id));
static_assert(offsetof(Branch3Output<false>, energized) == offsetof(BaseOutput, energized));
// static asserts for SymBranch3Output
static_assert(std::is_standard_layout_v<SymBranch3Output>);
// static asserts for conversion of SymBranch3Output to BaseOutput
static_assert(std::alignment_of_v<SymBranch3Output> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(SymBranch3Output::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(SymBranch3Output::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(SymBranch3Output, id) == offsetof(BaseOutput, id));
static_assert(offsetof(SymBranch3Output, energized) == offsetof(BaseOutput, energized));
// static asserts for AsymBranch3Output
static_assert(std::is_standard_layout_v<AsymBranch3Output>);
// static asserts for conversion of AsymBranch3Output to BaseOutput
static_assert(std::alignment_of_v<AsymBranch3Output> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(AsymBranch3Output::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(AsymBranch3Output::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(AsymBranch3Output, id) == offsetof(BaseOutput, id));
static_assert(offsetof(AsymBranch3Output, energized) == offsetof(BaseOutput, energized));

// static asserts for ApplianceOutput<true>
static_assert(std::is_standard_layout_v<ApplianceOutput<true>>);
// static asserts for conversion of ApplianceOutput<true> to BaseOutput
static_assert(std::alignment_of_v<ApplianceOutput<true>> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(ApplianceOutput<true>::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(ApplianceOutput<true>::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(ApplianceOutput<true>, id) == offsetof(BaseOutput, id));
static_assert(offsetof(ApplianceOutput<true>, energized) == offsetof(BaseOutput, energized));
// static asserts for ApplianceOutput<false>
static_assert(std::is_standard_layout_v<ApplianceOutput<false>>);
// static asserts for conversion of ApplianceOutput<false> to BaseOutput
static_assert(std::alignment_of_v<ApplianceOutput<false>> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(ApplianceOutput<false>::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(ApplianceOutput<false>::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(ApplianceOutput<false>, id) == offsetof(BaseOutput, id));
static_assert(offsetof(ApplianceOutput<false>, energized) == offsetof(BaseOutput, energized));
// static asserts for SymApplianceOutput
static_assert(std::is_standard_layout_v<SymApplianceOutput>);
// static asserts for conversion of SymApplianceOutput to BaseOutput
static_assert(std::alignment_of_v<SymApplianceOutput> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(SymApplianceOutput::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(SymApplianceOutput::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(SymApplianceOutput, id) == offsetof(BaseOutput, id));
static_assert(offsetof(SymApplianceOutput, energized) == offsetof(BaseOutput, energized));
// static asserts for AsymApplianceOutput
static_assert(std::is_standard_layout_v<AsymApplianceOutput>);
// static asserts for conversion of AsymApplianceOutput to BaseOutput
static_assert(std::alignment_of_v<AsymApplianceOutput> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(AsymApplianceOutput::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(AsymApplianceOutput::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(AsymApplianceOutput, id) == offsetof(BaseOutput, id));
static_assert(offsetof(AsymApplianceOutput, energized) == offsetof(BaseOutput, energized));

// static asserts for VoltageSensorOutput<true>
static_assert(std::is_standard_layout_v<VoltageSensorOutput<true>>);
// static asserts for conversion of VoltageSensorOutput<true> to BaseOutput
static_assert(std::alignment_of_v<VoltageSensorOutput<true>> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(VoltageSensorOutput<true>::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(VoltageSensorOutput<true>::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(VoltageSensorOutput<true>, id) == offsetof(BaseOutput, id));
static_assert(offsetof(VoltageSensorOutput<true>, energized) == offsetof(BaseOutput, energized));
// static asserts for VoltageSensorOutput<false>
static_assert(std::is_standard_layout_v<VoltageSensorOutput<false>>);
// static asserts for conversion of VoltageSensorOutput<false> to BaseOutput
static_assert(std::alignment_of_v<VoltageSensorOutput<false>> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(VoltageSensorOutput<false>::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(VoltageSensorOutput<false>::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(VoltageSensorOutput<false>, id) == offsetof(BaseOutput, id));
static_assert(offsetof(VoltageSensorOutput<false>, energized) == offsetof(BaseOutput, energized));
// static asserts for SymVoltageSensorOutput
static_assert(std::is_standard_layout_v<SymVoltageSensorOutput>);
// static asserts for conversion of SymVoltageSensorOutput to BaseOutput
static_assert(std::alignment_of_v<SymVoltageSensorOutput> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(SymVoltageSensorOutput::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(SymVoltageSensorOutput::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(SymVoltageSensorOutput, id) == offsetof(BaseOutput, id));
static_assert(offsetof(SymVoltageSensorOutput, energized) == offsetof(BaseOutput, energized));
// static asserts for AsymVoltageSensorOutput
static_assert(std::is_standard_layout_v<AsymVoltageSensorOutput>);
// static asserts for conversion of AsymVoltageSensorOutput to BaseOutput
static_assert(std::alignment_of_v<AsymVoltageSensorOutput> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(AsymVoltageSensorOutput::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(AsymVoltageSensorOutput::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(AsymVoltageSensorOutput, id) == offsetof(BaseOutput, id));
static_assert(offsetof(AsymVoltageSensorOutput, energized) == offsetof(BaseOutput, energized));

// static asserts for PowerSensorOutput<true>
static_assert(std::is_standard_layout_v<PowerSensorOutput<true>>);
// static asserts for conversion of PowerSensorOutput<true> to BaseOutput
static_assert(std::alignment_of_v<PowerSensorOutput<true>> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(PowerSensorOutput<true>::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(PowerSensorOutput<true>::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(PowerSensorOutput<true>, id) == offsetof(BaseOutput, id));
static_assert(offsetof(PowerSensorOutput<true>, energized) == offsetof(BaseOutput, energized));
// static asserts for PowerSensorOutput<false>
static_assert(std::is_standard_layout_v<PowerSensorOutput<false>>);
// static asserts for conversion of PowerSensorOutput<false> to BaseOutput
static_assert(std::alignment_of_v<PowerSensorOutput<false>> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(PowerSensorOutput<false>::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(PowerSensorOutput<false>::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(PowerSensorOutput<false>, id) == offsetof(BaseOutput, id));
static_assert(offsetof(PowerSensorOutput<false>, energized) == offsetof(BaseOutput, energized));
// static asserts for SymPowerSensorOutput
static_assert(std::is_standard_layout_v<SymPowerSensorOutput>);
// static asserts for conversion of SymPowerSensorOutput to BaseOutput
static_assert(std::alignment_of_v<SymPowerSensorOutput> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(SymPowerSensorOutput::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(SymPowerSensorOutput::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(SymPowerSensorOutput, id) == offsetof(BaseOutput, id));
static_assert(offsetof(SymPowerSensorOutput, energized) == offsetof(BaseOutput, energized));
// static asserts for AsymPowerSensorOutput
static_assert(std::is_standard_layout_v<AsymPowerSensorOutput>);
// static asserts for conversion of AsymPowerSensorOutput to BaseOutput
static_assert(std::alignment_of_v<AsymPowerSensorOutput> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(AsymPowerSensorOutput::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(AsymPowerSensorOutput::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(AsymPowerSensorOutput, id) == offsetof(BaseOutput, id));
static_assert(offsetof(AsymPowerSensorOutput, energized) == offsetof(BaseOutput, energized));

// static asserts for FaultOutput
static_assert(std::is_standard_layout_v<FaultOutput>);
// static asserts for conversion of FaultOutput to BaseOutput
static_assert(std::alignment_of_v<FaultOutput> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(FaultOutput::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(FaultOutput::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(FaultOutput, id) == offsetof(BaseOutput, id));
static_assert(offsetof(FaultOutput, energized) == offsetof(BaseOutput, energized));

// static asserts for FaultShortCircuitOutput
static_assert(std::is_standard_layout_v<FaultShortCircuitOutput>);
// static asserts for conversion of FaultShortCircuitOutput to BaseOutput
static_assert(std::alignment_of_v<FaultShortCircuitOutput> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(FaultShortCircuitOutput::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(FaultShortCircuitOutput::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(FaultShortCircuitOutput, id) == offsetof(BaseOutput, id));
static_assert(offsetof(FaultShortCircuitOutput, energized) == offsetof(BaseOutput, energized));

// static asserts for NodeShortCircuitOutput
static_assert(std::is_standard_layout_v<NodeShortCircuitOutput>);
// static asserts for conversion of NodeShortCircuitOutput to BaseOutput
static_assert(std::alignment_of_v<NodeShortCircuitOutput> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(NodeShortCircuitOutput::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(NodeShortCircuitOutput::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(NodeShortCircuitOutput, id) == offsetof(BaseOutput, id));
static_assert(offsetof(NodeShortCircuitOutput, energized) == offsetof(BaseOutput, energized));

// static asserts for BranchShortCircuitOutput
static_assert(std::is_standard_layout_v<BranchShortCircuitOutput>);
// static asserts for conversion of BranchShortCircuitOutput to BaseOutput
static_assert(std::alignment_of_v<BranchShortCircuitOutput> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(BranchShortCircuitOutput::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(BranchShortCircuitOutput::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(BranchShortCircuitOutput, id) == offsetof(BaseOutput, id));
static_assert(offsetof(BranchShortCircuitOutput, energized) == offsetof(BaseOutput, energized));

// static asserts for Branch3ShortCircuitOutput
static_assert(std::is_standard_layout_v<Branch3ShortCircuitOutput>);
// static asserts for conversion of Branch3ShortCircuitOutput to BaseOutput
static_assert(std::alignment_of_v<Branch3ShortCircuitOutput> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(Branch3ShortCircuitOutput::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(Branch3ShortCircuitOutput::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(Branch3ShortCircuitOutput, id) == offsetof(BaseOutput, id));
static_assert(offsetof(Branch3ShortCircuitOutput, energized) == offsetof(BaseOutput, energized));

// static asserts for ApplianceShortCircuitOutput
static_assert(std::is_standard_layout_v<ApplianceShortCircuitOutput>);
// static asserts for conversion of ApplianceShortCircuitOutput to BaseOutput
static_assert(std::alignment_of_v<ApplianceShortCircuitOutput> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(ApplianceShortCircuitOutput::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(ApplianceShortCircuitOutput::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(ApplianceShortCircuitOutput, id) == offsetof(BaseOutput, id));
static_assert(offsetof(ApplianceShortCircuitOutput, energized) == offsetof(BaseOutput, energized));

// static asserts for SensorShortCircuitOutput
static_assert(std::is_standard_layout_v<SensorShortCircuitOutput>);
// static asserts for conversion of SensorShortCircuitOutput to BaseOutput
static_assert(std::alignment_of_v<SensorShortCircuitOutput> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(SensorShortCircuitOutput::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(SensorShortCircuitOutput::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(SensorShortCircuitOutput, id) == offsetof(BaseOutput, id));
static_assert(offsetof(SensorShortCircuitOutput, energized) == offsetof(BaseOutput, energized));



} // namespace power_grid_model::test

#endif
// clang-format on