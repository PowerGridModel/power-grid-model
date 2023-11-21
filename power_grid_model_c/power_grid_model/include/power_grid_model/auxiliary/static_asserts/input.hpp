// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

// This header file is automatically generated. DO NOT modify it manually!

// clang-format off
#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_STATIC_ASSERTS_INPUT_HPP
#define POWER_GRID_MODEL_AUXILIARY_STATIC_ASSERTS_INPUT_HPP

#include "../input.hpp" // NOLINT

#include <cstddef>

namespace power_grid_model::test {

// static asserts for BaseInput
static_assert(std::is_standard_layout_v<BaseInput>);

// static asserts for NodeInput
static_assert(std::is_standard_layout_v<NodeInput>);
static_assert(std::alignment_of_v<NodeInput> >= std::alignment_of_v<BaseInput>);
static_assert(std::same_as<decltype(NodeInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(NodeInput, id) == offsetof(BaseInput, id));

// static asserts for BranchInput
static_assert(std::is_standard_layout_v<BranchInput>);
static_assert(std::alignment_of_v<BranchInput> >= std::alignment_of_v<BaseInput>);
static_assert(std::same_as<decltype(BranchInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(BranchInput, id) == offsetof(BaseInput, id));

// static asserts for Branch3Input
static_assert(std::is_standard_layout_v<Branch3Input>);
static_assert(std::alignment_of_v<Branch3Input> >= std::alignment_of_v<BaseInput>);
static_assert(std::same_as<decltype(Branch3Input::id), decltype(BaseInput::id)>);
static_assert(offsetof(Branch3Input, id) == offsetof(BaseInput, id));

// static asserts for SensorInput
static_assert(std::is_standard_layout_v<SensorInput>);
static_assert(std::alignment_of_v<SensorInput> >= std::alignment_of_v<BaseInput>);
static_assert(std::same_as<decltype(SensorInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(SensorInput, id) == offsetof(BaseInput, id));

// static asserts for ApplianceInput
static_assert(std::is_standard_layout_v<ApplianceInput>);
static_assert(std::alignment_of_v<ApplianceInput> >= std::alignment_of_v<BaseInput>);
static_assert(std::same_as<decltype(ApplianceInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(ApplianceInput, id) == offsetof(BaseInput, id));

// static asserts for LineInput
static_assert(std::is_standard_layout_v<LineInput>);
static_assert(std::alignment_of_v<LineInput> >= std::alignment_of_v<BranchInput>);
static_assert(std::same_as<decltype(LineInput::id), decltype(BranchInput::id)>);
static_assert(std::same_as<decltype(LineInput::from_node), decltype(BranchInput::from_node)>);
static_assert(std::same_as<decltype(LineInput::to_node), decltype(BranchInput::to_node)>);
static_assert(std::same_as<decltype(LineInput::from_status), decltype(BranchInput::from_status)>);
static_assert(std::same_as<decltype(LineInput::to_status), decltype(BranchInput::to_status)>);
static_assert(offsetof(LineInput, id) == offsetof(BranchInput, id));
static_assert(offsetof(LineInput, from_node) == offsetof(BranchInput, from_node));
static_assert(offsetof(LineInput, to_node) == offsetof(BranchInput, to_node));
static_assert(offsetof(LineInput, from_status) == offsetof(BranchInput, from_status));
static_assert(offsetof(LineInput, to_status) == offsetof(BranchInput, to_status));

// static asserts for LinkInput
static_assert(std::is_standard_layout_v<LinkInput>);
static_assert(std::alignment_of_v<LinkInput> >= std::alignment_of_v<BranchInput>);
static_assert(std::same_as<decltype(LinkInput::id), decltype(BranchInput::id)>);
static_assert(std::same_as<decltype(LinkInput::from_node), decltype(BranchInput::from_node)>);
static_assert(std::same_as<decltype(LinkInput::to_node), decltype(BranchInput::to_node)>);
static_assert(std::same_as<decltype(LinkInput::from_status), decltype(BranchInput::from_status)>);
static_assert(std::same_as<decltype(LinkInput::to_status), decltype(BranchInput::to_status)>);
static_assert(offsetof(LinkInput, id) == offsetof(BranchInput, id));
static_assert(offsetof(LinkInput, from_node) == offsetof(BranchInput, from_node));
static_assert(offsetof(LinkInput, to_node) == offsetof(BranchInput, to_node));
static_assert(offsetof(LinkInput, from_status) == offsetof(BranchInput, from_status));
static_assert(offsetof(LinkInput, to_status) == offsetof(BranchInput, to_status));

// static asserts for TransformerInput
static_assert(std::is_standard_layout_v<TransformerInput>);
static_assert(std::alignment_of_v<TransformerInput> >= std::alignment_of_v<BranchInput>);
static_assert(std::same_as<decltype(TransformerInput::id), decltype(BranchInput::id)>);
static_assert(std::same_as<decltype(TransformerInput::from_node), decltype(BranchInput::from_node)>);
static_assert(std::same_as<decltype(TransformerInput::to_node), decltype(BranchInput::to_node)>);
static_assert(std::same_as<decltype(TransformerInput::from_status), decltype(BranchInput::from_status)>);
static_assert(std::same_as<decltype(TransformerInput::to_status), decltype(BranchInput::to_status)>);
static_assert(offsetof(TransformerInput, id) == offsetof(BranchInput, id));
static_assert(offsetof(TransformerInput, from_node) == offsetof(BranchInput, from_node));
static_assert(offsetof(TransformerInput, to_node) == offsetof(BranchInput, to_node));
static_assert(offsetof(TransformerInput, from_status) == offsetof(BranchInput, from_status));
static_assert(offsetof(TransformerInput, to_status) == offsetof(BranchInput, to_status));

// static asserts for ThreeWindingTransformerInput
static_assert(std::is_standard_layout_v<ThreeWindingTransformerInput>);
static_assert(std::alignment_of_v<ThreeWindingTransformerInput> >= std::alignment_of_v<Branch3Input>);
static_assert(std::same_as<decltype(ThreeWindingTransformerInput::id), decltype(Branch3Input::id)>);
static_assert(std::same_as<decltype(ThreeWindingTransformerInput::node_1), decltype(Branch3Input::node_1)>);
static_assert(std::same_as<decltype(ThreeWindingTransformerInput::node_2), decltype(Branch3Input::node_2)>);
static_assert(std::same_as<decltype(ThreeWindingTransformerInput::node_3), decltype(Branch3Input::node_3)>);
static_assert(std::same_as<decltype(ThreeWindingTransformerInput::status_1), decltype(Branch3Input::status_1)>);
static_assert(std::same_as<decltype(ThreeWindingTransformerInput::status_2), decltype(Branch3Input::status_2)>);
static_assert(std::same_as<decltype(ThreeWindingTransformerInput::status_3), decltype(Branch3Input::status_3)>);
static_assert(offsetof(ThreeWindingTransformerInput, id) == offsetof(Branch3Input, id));
static_assert(offsetof(ThreeWindingTransformerInput, node_1) == offsetof(Branch3Input, node_1));
static_assert(offsetof(ThreeWindingTransformerInput, node_2) == offsetof(Branch3Input, node_2));
static_assert(offsetof(ThreeWindingTransformerInput, node_3) == offsetof(Branch3Input, node_3));
static_assert(offsetof(ThreeWindingTransformerInput, status_1) == offsetof(Branch3Input, status_1));
static_assert(offsetof(ThreeWindingTransformerInput, status_2) == offsetof(Branch3Input, status_2));
static_assert(offsetof(ThreeWindingTransformerInput, status_3) == offsetof(Branch3Input, status_3));

// static asserts for GenericLoadGenInput
static_assert(std::is_standard_layout_v<GenericLoadGenInput>);
static_assert(std::alignment_of_v<GenericLoadGenInput> >= std::alignment_of_v<ApplianceInput>);
static_assert(std::same_as<decltype(GenericLoadGenInput::id), decltype(ApplianceInput::id)>);
static_assert(std::same_as<decltype(GenericLoadGenInput::node), decltype(ApplianceInput::node)>);
static_assert(std::same_as<decltype(GenericLoadGenInput::status), decltype(ApplianceInput::status)>);
static_assert(offsetof(GenericLoadGenInput, id) == offsetof(ApplianceInput, id));
static_assert(offsetof(GenericLoadGenInput, node) == offsetof(ApplianceInput, node));
static_assert(offsetof(GenericLoadGenInput, status) == offsetof(ApplianceInput, status));

// static asserts for LoadGenInput<true>
static_assert(std::is_standard_layout_v<LoadGenInput<true>>);
static_assert(std::alignment_of_v<LoadGenInput<true>> >= std::alignment_of_v<GenericLoadGenInput>);
static_assert(std::same_as<decltype(LoadGenInput<true>::id), decltype(GenericLoadGenInput::id)>);
static_assert(std::same_as<decltype(LoadGenInput<true>::node), decltype(GenericLoadGenInput::node)>);
static_assert(std::same_as<decltype(LoadGenInput<true>::status), decltype(GenericLoadGenInput::status)>);
static_assert(std::same_as<decltype(LoadGenInput<true>::type), decltype(GenericLoadGenInput::type)>);
static_assert(offsetof(LoadGenInput<true>, id) == offsetof(GenericLoadGenInput, id));
static_assert(offsetof(LoadGenInput<true>, node) == offsetof(GenericLoadGenInput, node));
static_assert(offsetof(LoadGenInput<true>, status) == offsetof(GenericLoadGenInput, status));
static_assert(offsetof(LoadGenInput<true>, type) == offsetof(GenericLoadGenInput, type));

// static asserts for LoadGenInput<false>
static_assert(std::is_standard_layout_v<LoadGenInput<false>>);
static_assert(std::alignment_of_v<LoadGenInput<false>> >= std::alignment_of_v<GenericLoadGenInput>);
static_assert(std::same_as<decltype(LoadGenInput<false>::id), decltype(GenericLoadGenInput::id)>);
static_assert(std::same_as<decltype(LoadGenInput<false>::node), decltype(GenericLoadGenInput::node)>);
static_assert(std::same_as<decltype(LoadGenInput<false>::status), decltype(GenericLoadGenInput::status)>);
static_assert(std::same_as<decltype(LoadGenInput<false>::type), decltype(GenericLoadGenInput::type)>);
static_assert(offsetof(LoadGenInput<false>, id) == offsetof(GenericLoadGenInput, id));
static_assert(offsetof(LoadGenInput<false>, node) == offsetof(GenericLoadGenInput, node));
static_assert(offsetof(LoadGenInput<false>, status) == offsetof(GenericLoadGenInput, status));
static_assert(offsetof(LoadGenInput<false>, type) == offsetof(GenericLoadGenInput, type));

// static asserts for SymLoadGenInput
static_assert(std::is_standard_layout_v<SymLoadGenInput>);
static_assert(std::alignment_of_v<SymLoadGenInput> >= std::alignment_of_v<GenericLoadGenInput>);
static_assert(std::same_as<decltype(SymLoadGenInput::id), decltype(GenericLoadGenInput::id)>);
static_assert(std::same_as<decltype(SymLoadGenInput::node), decltype(GenericLoadGenInput::node)>);
static_assert(std::same_as<decltype(SymLoadGenInput::status), decltype(GenericLoadGenInput::status)>);
static_assert(std::same_as<decltype(SymLoadGenInput::type), decltype(GenericLoadGenInput::type)>);
static_assert(offsetof(SymLoadGenInput, id) == offsetof(GenericLoadGenInput, id));
static_assert(offsetof(SymLoadGenInput, node) == offsetof(GenericLoadGenInput, node));
static_assert(offsetof(SymLoadGenInput, status) == offsetof(GenericLoadGenInput, status));
static_assert(offsetof(SymLoadGenInput, type) == offsetof(GenericLoadGenInput, type));

// static asserts for AsymLoadGenInput
static_assert(std::is_standard_layout_v<AsymLoadGenInput>);
static_assert(std::alignment_of_v<AsymLoadGenInput> >= std::alignment_of_v<GenericLoadGenInput>);
static_assert(std::same_as<decltype(AsymLoadGenInput::id), decltype(GenericLoadGenInput::id)>);
static_assert(std::same_as<decltype(AsymLoadGenInput::node), decltype(GenericLoadGenInput::node)>);
static_assert(std::same_as<decltype(AsymLoadGenInput::status), decltype(GenericLoadGenInput::status)>);
static_assert(std::same_as<decltype(AsymLoadGenInput::type), decltype(GenericLoadGenInput::type)>);
static_assert(offsetof(AsymLoadGenInput, id) == offsetof(GenericLoadGenInput, id));
static_assert(offsetof(AsymLoadGenInput, node) == offsetof(GenericLoadGenInput, node));
static_assert(offsetof(AsymLoadGenInput, status) == offsetof(GenericLoadGenInput, status));
static_assert(offsetof(AsymLoadGenInput, type) == offsetof(GenericLoadGenInput, type));

// static asserts for ShuntInput
static_assert(std::is_standard_layout_v<ShuntInput>);
static_assert(std::alignment_of_v<ShuntInput> >= std::alignment_of_v<ApplianceInput>);
static_assert(std::same_as<decltype(ShuntInput::id), decltype(ApplianceInput::id)>);
static_assert(std::same_as<decltype(ShuntInput::node), decltype(ApplianceInput::node)>);
static_assert(std::same_as<decltype(ShuntInput::status), decltype(ApplianceInput::status)>);
static_assert(offsetof(ShuntInput, id) == offsetof(ApplianceInput, id));
static_assert(offsetof(ShuntInput, node) == offsetof(ApplianceInput, node));
static_assert(offsetof(ShuntInput, status) == offsetof(ApplianceInput, status));

// static asserts for SourceInput
static_assert(std::is_standard_layout_v<SourceInput>);
static_assert(std::alignment_of_v<SourceInput> >= std::alignment_of_v<ApplianceInput>);
static_assert(std::same_as<decltype(SourceInput::id), decltype(ApplianceInput::id)>);
static_assert(std::same_as<decltype(SourceInput::node), decltype(ApplianceInput::node)>);
static_assert(std::same_as<decltype(SourceInput::status), decltype(ApplianceInput::status)>);
static_assert(offsetof(SourceInput, id) == offsetof(ApplianceInput, id));
static_assert(offsetof(SourceInput, node) == offsetof(ApplianceInput, node));
static_assert(offsetof(SourceInput, status) == offsetof(ApplianceInput, status));

// static asserts for GenericVoltageSensorInput
static_assert(std::is_standard_layout_v<GenericVoltageSensorInput>);
static_assert(std::alignment_of_v<GenericVoltageSensorInput> >= std::alignment_of_v<SensorInput>);
static_assert(std::same_as<decltype(GenericVoltageSensorInput::id), decltype(SensorInput::id)>);
static_assert(std::same_as<decltype(GenericVoltageSensorInput::measured_object), decltype(SensorInput::measured_object)>);
static_assert(offsetof(GenericVoltageSensorInput, id) == offsetof(SensorInput, id));
static_assert(offsetof(GenericVoltageSensorInput, measured_object) == offsetof(SensorInput, measured_object));

// static asserts for VoltageSensorInput<true>
static_assert(std::is_standard_layout_v<VoltageSensorInput<true>>);
static_assert(std::alignment_of_v<VoltageSensorInput<true>> >= std::alignment_of_v<GenericVoltageSensorInput>);
static_assert(std::same_as<decltype(VoltageSensorInput<true>::id), decltype(GenericVoltageSensorInput::id)>);
static_assert(std::same_as<decltype(VoltageSensorInput<true>::measured_object), decltype(GenericVoltageSensorInput::measured_object)>);
static_assert(std::same_as<decltype(VoltageSensorInput<true>::u_sigma), decltype(GenericVoltageSensorInput::u_sigma)>);
static_assert(offsetof(VoltageSensorInput<true>, id) == offsetof(GenericVoltageSensorInput, id));
static_assert(offsetof(VoltageSensorInput<true>, measured_object) == offsetof(GenericVoltageSensorInput, measured_object));
static_assert(offsetof(VoltageSensorInput<true>, u_sigma) == offsetof(GenericVoltageSensorInput, u_sigma));

// static asserts for VoltageSensorInput<false>
static_assert(std::is_standard_layout_v<VoltageSensorInput<false>>);
static_assert(std::alignment_of_v<VoltageSensorInput<false>> >= std::alignment_of_v<GenericVoltageSensorInput>);
static_assert(std::same_as<decltype(VoltageSensorInput<false>::id), decltype(GenericVoltageSensorInput::id)>);
static_assert(std::same_as<decltype(VoltageSensorInput<false>::measured_object), decltype(GenericVoltageSensorInput::measured_object)>);
static_assert(std::same_as<decltype(VoltageSensorInput<false>::u_sigma), decltype(GenericVoltageSensorInput::u_sigma)>);
static_assert(offsetof(VoltageSensorInput<false>, id) == offsetof(GenericVoltageSensorInput, id));
static_assert(offsetof(VoltageSensorInput<false>, measured_object) == offsetof(GenericVoltageSensorInput, measured_object));
static_assert(offsetof(VoltageSensorInput<false>, u_sigma) == offsetof(GenericVoltageSensorInput, u_sigma));

// static asserts for SymVoltageSensorInput
static_assert(std::is_standard_layout_v<SymVoltageSensorInput>);
static_assert(std::alignment_of_v<SymVoltageSensorInput> >= std::alignment_of_v<GenericVoltageSensorInput>);
static_assert(std::same_as<decltype(SymVoltageSensorInput::id), decltype(GenericVoltageSensorInput::id)>);
static_assert(std::same_as<decltype(SymVoltageSensorInput::measured_object), decltype(GenericVoltageSensorInput::measured_object)>);
static_assert(std::same_as<decltype(SymVoltageSensorInput::u_sigma), decltype(GenericVoltageSensorInput::u_sigma)>);
static_assert(offsetof(SymVoltageSensorInput, id) == offsetof(GenericVoltageSensorInput, id));
static_assert(offsetof(SymVoltageSensorInput, measured_object) == offsetof(GenericVoltageSensorInput, measured_object));
static_assert(offsetof(SymVoltageSensorInput, u_sigma) == offsetof(GenericVoltageSensorInput, u_sigma));

// static asserts for AsymVoltageSensorInput
static_assert(std::is_standard_layout_v<AsymVoltageSensorInput>);
static_assert(std::alignment_of_v<AsymVoltageSensorInput> >= std::alignment_of_v<GenericVoltageSensorInput>);
static_assert(std::same_as<decltype(AsymVoltageSensorInput::id), decltype(GenericVoltageSensorInput::id)>);
static_assert(std::same_as<decltype(AsymVoltageSensorInput::measured_object), decltype(GenericVoltageSensorInput::measured_object)>);
static_assert(std::same_as<decltype(AsymVoltageSensorInput::u_sigma), decltype(GenericVoltageSensorInput::u_sigma)>);
static_assert(offsetof(AsymVoltageSensorInput, id) == offsetof(GenericVoltageSensorInput, id));
static_assert(offsetof(AsymVoltageSensorInput, measured_object) == offsetof(GenericVoltageSensorInput, measured_object));
static_assert(offsetof(AsymVoltageSensorInput, u_sigma) == offsetof(GenericVoltageSensorInput, u_sigma));

// static asserts for GenericPowerSensorInput
static_assert(std::is_standard_layout_v<GenericPowerSensorInput>);
static_assert(std::alignment_of_v<GenericPowerSensorInput> >= std::alignment_of_v<SensorInput>);
static_assert(std::same_as<decltype(GenericPowerSensorInput::id), decltype(SensorInput::id)>);
static_assert(std::same_as<decltype(GenericPowerSensorInput::measured_object), decltype(SensorInput::measured_object)>);
static_assert(offsetof(GenericPowerSensorInput, id) == offsetof(SensorInput, id));
static_assert(offsetof(GenericPowerSensorInput, measured_object) == offsetof(SensorInput, measured_object));

// static asserts for PowerSensorInput<true>
static_assert(std::is_standard_layout_v<PowerSensorInput<true>>);
static_assert(std::alignment_of_v<PowerSensorInput<true>> >= std::alignment_of_v<GenericPowerSensorInput>);
static_assert(std::same_as<decltype(PowerSensorInput<true>::id), decltype(GenericPowerSensorInput::id)>);
static_assert(std::same_as<decltype(PowerSensorInput<true>::measured_object), decltype(GenericPowerSensorInput::measured_object)>);
static_assert(std::same_as<decltype(PowerSensorInput<true>::measured_terminal_type), decltype(GenericPowerSensorInput::measured_terminal_type)>);
static_assert(std::same_as<decltype(PowerSensorInput<true>::power_sigma), decltype(GenericPowerSensorInput::power_sigma)>);
static_assert(offsetof(PowerSensorInput<true>, id) == offsetof(GenericPowerSensorInput, id));
static_assert(offsetof(PowerSensorInput<true>, measured_object) == offsetof(GenericPowerSensorInput, measured_object));
static_assert(offsetof(PowerSensorInput<true>, measured_terminal_type) == offsetof(GenericPowerSensorInput, measured_terminal_type));
static_assert(offsetof(PowerSensorInput<true>, power_sigma) == offsetof(GenericPowerSensorInput, power_sigma));

// static asserts for PowerSensorInput<false>
static_assert(std::is_standard_layout_v<PowerSensorInput<false>>);
static_assert(std::alignment_of_v<PowerSensorInput<false>> >= std::alignment_of_v<GenericPowerSensorInput>);
static_assert(std::same_as<decltype(PowerSensorInput<false>::id), decltype(GenericPowerSensorInput::id)>);
static_assert(std::same_as<decltype(PowerSensorInput<false>::measured_object), decltype(GenericPowerSensorInput::measured_object)>);
static_assert(std::same_as<decltype(PowerSensorInput<false>::measured_terminal_type), decltype(GenericPowerSensorInput::measured_terminal_type)>);
static_assert(std::same_as<decltype(PowerSensorInput<false>::power_sigma), decltype(GenericPowerSensorInput::power_sigma)>);
static_assert(offsetof(PowerSensorInput<false>, id) == offsetof(GenericPowerSensorInput, id));
static_assert(offsetof(PowerSensorInput<false>, measured_object) == offsetof(GenericPowerSensorInput, measured_object));
static_assert(offsetof(PowerSensorInput<false>, measured_terminal_type) == offsetof(GenericPowerSensorInput, measured_terminal_type));
static_assert(offsetof(PowerSensorInput<false>, power_sigma) == offsetof(GenericPowerSensorInput, power_sigma));

// static asserts for SymPowerSensorInput
static_assert(std::is_standard_layout_v<SymPowerSensorInput>);
static_assert(std::alignment_of_v<SymPowerSensorInput> >= std::alignment_of_v<GenericPowerSensorInput>);
static_assert(std::same_as<decltype(SymPowerSensorInput::id), decltype(GenericPowerSensorInput::id)>);
static_assert(std::same_as<decltype(SymPowerSensorInput::measured_object), decltype(GenericPowerSensorInput::measured_object)>);
static_assert(std::same_as<decltype(SymPowerSensorInput::measured_terminal_type), decltype(GenericPowerSensorInput::measured_terminal_type)>);
static_assert(std::same_as<decltype(SymPowerSensorInput::power_sigma), decltype(GenericPowerSensorInput::power_sigma)>);
static_assert(offsetof(SymPowerSensorInput, id) == offsetof(GenericPowerSensorInput, id));
static_assert(offsetof(SymPowerSensorInput, measured_object) == offsetof(GenericPowerSensorInput, measured_object));
static_assert(offsetof(SymPowerSensorInput, measured_terminal_type) == offsetof(GenericPowerSensorInput, measured_terminal_type));
static_assert(offsetof(SymPowerSensorInput, power_sigma) == offsetof(GenericPowerSensorInput, power_sigma));

// static asserts for AsymPowerSensorInput
static_assert(std::is_standard_layout_v<AsymPowerSensorInput>);
static_assert(std::alignment_of_v<AsymPowerSensorInput> >= std::alignment_of_v<GenericPowerSensorInput>);
static_assert(std::same_as<decltype(AsymPowerSensorInput::id), decltype(GenericPowerSensorInput::id)>);
static_assert(std::same_as<decltype(AsymPowerSensorInput::measured_object), decltype(GenericPowerSensorInput::measured_object)>);
static_assert(std::same_as<decltype(AsymPowerSensorInput::measured_terminal_type), decltype(GenericPowerSensorInput::measured_terminal_type)>);
static_assert(std::same_as<decltype(AsymPowerSensorInput::power_sigma), decltype(GenericPowerSensorInput::power_sigma)>);
static_assert(offsetof(AsymPowerSensorInput, id) == offsetof(GenericPowerSensorInput, id));
static_assert(offsetof(AsymPowerSensorInput, measured_object) == offsetof(GenericPowerSensorInput, measured_object));
static_assert(offsetof(AsymPowerSensorInput, measured_terminal_type) == offsetof(GenericPowerSensorInput, measured_terminal_type));
static_assert(offsetof(AsymPowerSensorInput, power_sigma) == offsetof(GenericPowerSensorInput, power_sigma));

// static asserts for FaultInput
static_assert(std::is_standard_layout_v<FaultInput>);
static_assert(std::alignment_of_v<FaultInput> >= std::alignment_of_v<BaseInput>);
static_assert(std::same_as<decltype(FaultInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(FaultInput, id) == offsetof(BaseInput, id));

} // namespace power_grid_model::test

#endif
// clang-format on