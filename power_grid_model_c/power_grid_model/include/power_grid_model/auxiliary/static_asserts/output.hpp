// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// This header file is automatically generated. DO NOT modify it manually!

// clang-format off
#pragma once

#include "../output.hpp" // NOLINT

#include <cstddef>

namespace power_grid_model::test {

// static asserts for BaseOutput
static_assert(std::is_standard_layout_v<BaseOutput>);

// static asserts for NodeOutput<symmetric_t>
static_assert(std::is_standard_layout_v<NodeOutput<symmetric_t>>);
// static asserts for conversion of NodeOutput<symmetric_t> to BaseOutput
static_assert(std::alignment_of_v<NodeOutput<symmetric_t>> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(NodeOutput<symmetric_t>::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(NodeOutput<symmetric_t>::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(NodeOutput<symmetric_t>, id) == offsetof(BaseOutput, id));
static_assert(offsetof(NodeOutput<symmetric_t>, energized) == offsetof(BaseOutput, energized));
// static asserts for NodeOutput<asymmetric_t>
static_assert(std::is_standard_layout_v<NodeOutput<asymmetric_t>>);
// static asserts for conversion of NodeOutput<asymmetric_t> to BaseOutput
static_assert(std::alignment_of_v<NodeOutput<asymmetric_t>> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(NodeOutput<asymmetric_t>::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(NodeOutput<asymmetric_t>::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(NodeOutput<asymmetric_t>, id) == offsetof(BaseOutput, id));
static_assert(offsetof(NodeOutput<asymmetric_t>, energized) == offsetof(BaseOutput, energized));
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

// static asserts for BranchOutput<symmetric_t>
static_assert(std::is_standard_layout_v<BranchOutput<symmetric_t>>);
// static asserts for conversion of BranchOutput<symmetric_t> to BaseOutput
static_assert(std::alignment_of_v<BranchOutput<symmetric_t>> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(BranchOutput<symmetric_t>::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(BranchOutput<symmetric_t>::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(BranchOutput<symmetric_t>, id) == offsetof(BaseOutput, id));
static_assert(offsetof(BranchOutput<symmetric_t>, energized) == offsetof(BaseOutput, energized));
// static asserts for BranchOutput<asymmetric_t>
static_assert(std::is_standard_layout_v<BranchOutput<asymmetric_t>>);
// static asserts for conversion of BranchOutput<asymmetric_t> to BaseOutput
static_assert(std::alignment_of_v<BranchOutput<asymmetric_t>> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(BranchOutput<asymmetric_t>::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(BranchOutput<asymmetric_t>::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(BranchOutput<asymmetric_t>, id) == offsetof(BaseOutput, id));
static_assert(offsetof(BranchOutput<asymmetric_t>, energized) == offsetof(BaseOutput, energized));
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

// static asserts for Branch3Output<symmetric_t>
static_assert(std::is_standard_layout_v<Branch3Output<symmetric_t>>);
// static asserts for conversion of Branch3Output<symmetric_t> to BaseOutput
static_assert(std::alignment_of_v<Branch3Output<symmetric_t>> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(Branch3Output<symmetric_t>::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(Branch3Output<symmetric_t>::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(Branch3Output<symmetric_t>, id) == offsetof(BaseOutput, id));
static_assert(offsetof(Branch3Output<symmetric_t>, energized) == offsetof(BaseOutput, energized));
// static asserts for Branch3Output<asymmetric_t>
static_assert(std::is_standard_layout_v<Branch3Output<asymmetric_t>>);
// static asserts for conversion of Branch3Output<asymmetric_t> to BaseOutput
static_assert(std::alignment_of_v<Branch3Output<asymmetric_t>> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(Branch3Output<asymmetric_t>::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(Branch3Output<asymmetric_t>::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(Branch3Output<asymmetric_t>, id) == offsetof(BaseOutput, id));
static_assert(offsetof(Branch3Output<asymmetric_t>, energized) == offsetof(BaseOutput, energized));
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

// static asserts for ApplianceOutput<symmetric_t>
static_assert(std::is_standard_layout_v<ApplianceOutput<symmetric_t>>);
// static asserts for conversion of ApplianceOutput<symmetric_t> to BaseOutput
static_assert(std::alignment_of_v<ApplianceOutput<symmetric_t>> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(ApplianceOutput<symmetric_t>::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(ApplianceOutput<symmetric_t>::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(ApplianceOutput<symmetric_t>, id) == offsetof(BaseOutput, id));
static_assert(offsetof(ApplianceOutput<symmetric_t>, energized) == offsetof(BaseOutput, energized));
// static asserts for ApplianceOutput<asymmetric_t>
static_assert(std::is_standard_layout_v<ApplianceOutput<asymmetric_t>>);
// static asserts for conversion of ApplianceOutput<asymmetric_t> to BaseOutput
static_assert(std::alignment_of_v<ApplianceOutput<asymmetric_t>> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(ApplianceOutput<asymmetric_t>::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(ApplianceOutput<asymmetric_t>::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(ApplianceOutput<asymmetric_t>, id) == offsetof(BaseOutput, id));
static_assert(offsetof(ApplianceOutput<asymmetric_t>, energized) == offsetof(BaseOutput, energized));
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

// static asserts for VoltageSensorOutput<symmetric_t>
static_assert(std::is_standard_layout_v<VoltageSensorOutput<symmetric_t>>);
// static asserts for conversion of VoltageSensorOutput<symmetric_t> to BaseOutput
static_assert(std::alignment_of_v<VoltageSensorOutput<symmetric_t>> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(VoltageSensorOutput<symmetric_t>::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(VoltageSensorOutput<symmetric_t>::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(VoltageSensorOutput<symmetric_t>, id) == offsetof(BaseOutput, id));
static_assert(offsetof(VoltageSensorOutput<symmetric_t>, energized) == offsetof(BaseOutput, energized));
// static asserts for VoltageSensorOutput<asymmetric_t>
static_assert(std::is_standard_layout_v<VoltageSensorOutput<asymmetric_t>>);
// static asserts for conversion of VoltageSensorOutput<asymmetric_t> to BaseOutput
static_assert(std::alignment_of_v<VoltageSensorOutput<asymmetric_t>> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(VoltageSensorOutput<asymmetric_t>::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(VoltageSensorOutput<asymmetric_t>::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(VoltageSensorOutput<asymmetric_t>, id) == offsetof(BaseOutput, id));
static_assert(offsetof(VoltageSensorOutput<asymmetric_t>, energized) == offsetof(BaseOutput, energized));
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

// static asserts for PowerSensorOutput<symmetric_t>
static_assert(std::is_standard_layout_v<PowerSensorOutput<symmetric_t>>);
// static asserts for conversion of PowerSensorOutput<symmetric_t> to BaseOutput
static_assert(std::alignment_of_v<PowerSensorOutput<symmetric_t>> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(PowerSensorOutput<symmetric_t>::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(PowerSensorOutput<symmetric_t>::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(PowerSensorOutput<symmetric_t>, id) == offsetof(BaseOutput, id));
static_assert(offsetof(PowerSensorOutput<symmetric_t>, energized) == offsetof(BaseOutput, energized));
// static asserts for PowerSensorOutput<asymmetric_t>
static_assert(std::is_standard_layout_v<PowerSensorOutput<asymmetric_t>>);
// static asserts for conversion of PowerSensorOutput<asymmetric_t> to BaseOutput
static_assert(std::alignment_of_v<PowerSensorOutput<asymmetric_t>> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(PowerSensorOutput<asymmetric_t>::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(PowerSensorOutput<asymmetric_t>::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(PowerSensorOutput<asymmetric_t>, id) == offsetof(BaseOutput, id));
static_assert(offsetof(PowerSensorOutput<asymmetric_t>, energized) == offsetof(BaseOutput, energized));
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

// static asserts for TransformerTapRegulatorOutput
static_assert(std::is_standard_layout_v<TransformerTapRegulatorOutput>);
// static asserts for conversion of TransformerTapRegulatorOutput to BaseOutput
static_assert(std::alignment_of_v<TransformerTapRegulatorOutput> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(TransformerTapRegulatorOutput::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(TransformerTapRegulatorOutput::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(TransformerTapRegulatorOutput, id) == offsetof(BaseOutput, id));
static_assert(offsetof(TransformerTapRegulatorOutput, energized) == offsetof(BaseOutput, energized));

// static asserts for RegulatorShortCircuitOutput
static_assert(std::is_standard_layout_v<RegulatorShortCircuitOutput>);
// static asserts for conversion of RegulatorShortCircuitOutput to BaseOutput
static_assert(std::alignment_of_v<RegulatorShortCircuitOutput> >= std::alignment_of_v<BaseOutput>);
static_assert(std::same_as<decltype(RegulatorShortCircuitOutput::id), decltype(BaseOutput::id)>);
static_assert(std::same_as<decltype(RegulatorShortCircuitOutput::energized), decltype(BaseOutput::energized)>);
static_assert(offsetof(RegulatorShortCircuitOutput, id) == offsetof(BaseOutput, id));
static_assert(offsetof(RegulatorShortCircuitOutput, energized) == offsetof(BaseOutput, energized));



} // namespace power_grid_model::test

// clang-format on