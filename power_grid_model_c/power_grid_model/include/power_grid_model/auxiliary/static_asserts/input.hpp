// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// This header file is automatically generated. DO NOT modify it manually!

// clang-format off
#pragma once

#include "../input.hpp" // NOLINT

#include <cstddef>

namespace power_grid_model::test {

// static asserts for BaseInput
static_assert(std::is_standard_layout_v<BaseInput>);

// static asserts for NodeInput
static_assert(std::is_standard_layout_v<NodeInput>);
// static asserts for conversion of NodeInput to BaseInput
static_assert(std::alignment_of_v<NodeInput> >= std::alignment_of_v<BaseInput>);
static_assert(std::same_as<decltype(NodeInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(NodeInput, id) == offsetof(BaseInput, id));

// static asserts for BranchInput
static_assert(std::is_standard_layout_v<BranchInput>);
// static asserts for conversion of BranchInput to BaseInput
static_assert(std::alignment_of_v<BranchInput> >= std::alignment_of_v<BaseInput>);
static_assert(std::same_as<decltype(BranchInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(BranchInput, id) == offsetof(BaseInput, id));

// static asserts for Branch3Input
static_assert(std::is_standard_layout_v<Branch3Input>);
// static asserts for conversion of Branch3Input to BaseInput
static_assert(std::alignment_of_v<Branch3Input> >= std::alignment_of_v<BaseInput>);
static_assert(std::same_as<decltype(Branch3Input::id), decltype(BaseInput::id)>);
static_assert(offsetof(Branch3Input, id) == offsetof(BaseInput, id));

// static asserts for SensorInput
static_assert(std::is_standard_layout_v<SensorInput>);
// static asserts for conversion of SensorInput to BaseInput
static_assert(std::alignment_of_v<SensorInput> >= std::alignment_of_v<BaseInput>);
static_assert(std::same_as<decltype(SensorInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(SensorInput, id) == offsetof(BaseInput, id));

// static asserts for ApplianceInput
static_assert(std::is_standard_layout_v<ApplianceInput>);
// static asserts for conversion of ApplianceInput to BaseInput
static_assert(std::alignment_of_v<ApplianceInput> >= std::alignment_of_v<BaseInput>);
static_assert(std::same_as<decltype(ApplianceInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(ApplianceInput, id) == offsetof(BaseInput, id));

// static asserts for LineInput
static_assert(std::is_standard_layout_v<LineInput>);
// static asserts for conversion of LineInput to BaseInput
static_assert(std::alignment_of_v<LineInput> >= std::alignment_of_v<BranchInput>);
static_assert(std::same_as<decltype(LineInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(LineInput, id) == offsetof(BaseInput, id));
// static asserts for conversion of LineInput to BranchInput
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

// static asserts for AsymLineInput
static_assert(std::is_standard_layout_v<AsymLineInput>);
// static asserts for conversion of AsymLineInput to BaseInput
static_assert(std::alignment_of_v<AsymLineInput> >= std::alignment_of_v<BranchInput>);
static_assert(std::same_as<decltype(AsymLineInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(AsymLineInput, id) == offsetof(BaseInput, id));
// static asserts for conversion of AsymLineInput to BranchInput
static_assert(std::alignment_of_v<AsymLineInput> >= std::alignment_of_v<BranchInput>);
static_assert(std::same_as<decltype(AsymLineInput::id), decltype(BranchInput::id)>);
static_assert(std::same_as<decltype(AsymLineInput::from_node), decltype(BranchInput::from_node)>);
static_assert(std::same_as<decltype(AsymLineInput::to_node), decltype(BranchInput::to_node)>);
static_assert(std::same_as<decltype(AsymLineInput::from_status), decltype(BranchInput::from_status)>);
static_assert(std::same_as<decltype(AsymLineInput::to_status), decltype(BranchInput::to_status)>);
static_assert(offsetof(AsymLineInput, id) == offsetof(BranchInput, id));
static_assert(offsetof(AsymLineInput, from_node) == offsetof(BranchInput, from_node));
static_assert(offsetof(AsymLineInput, to_node) == offsetof(BranchInput, to_node));
static_assert(offsetof(AsymLineInput, from_status) == offsetof(BranchInput, from_status));
static_assert(offsetof(AsymLineInput, to_status) == offsetof(BranchInput, to_status));

// static asserts for GenericBranchInput
static_assert(std::is_standard_layout_v<GenericBranchInput>);
// static asserts for conversion of GenericBranchInput to BaseInput
static_assert(std::alignment_of_v<GenericBranchInput> >= std::alignment_of_v<BranchInput>);
static_assert(std::same_as<decltype(GenericBranchInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(GenericBranchInput, id) == offsetof(BaseInput, id));
// static asserts for conversion of GenericBranchInput to BranchInput
static_assert(std::alignment_of_v<GenericBranchInput> >= std::alignment_of_v<BranchInput>);
static_assert(std::same_as<decltype(GenericBranchInput::id), decltype(BranchInput::id)>);
static_assert(std::same_as<decltype(GenericBranchInput::from_node), decltype(BranchInput::from_node)>);
static_assert(std::same_as<decltype(GenericBranchInput::to_node), decltype(BranchInput::to_node)>);
static_assert(std::same_as<decltype(GenericBranchInput::from_status), decltype(BranchInput::from_status)>);
static_assert(std::same_as<decltype(GenericBranchInput::to_status), decltype(BranchInput::to_status)>);
static_assert(offsetof(GenericBranchInput, id) == offsetof(BranchInput, id));
static_assert(offsetof(GenericBranchInput, from_node) == offsetof(BranchInput, from_node));
static_assert(offsetof(GenericBranchInput, to_node) == offsetof(BranchInput, to_node));
static_assert(offsetof(GenericBranchInput, from_status) == offsetof(BranchInput, from_status));
static_assert(offsetof(GenericBranchInput, to_status) == offsetof(BranchInput, to_status));

// static asserts for LinkInput
static_assert(std::is_standard_layout_v<LinkInput>);
// static asserts for conversion of LinkInput to BaseInput
static_assert(std::alignment_of_v<LinkInput> >= std::alignment_of_v<BranchInput>);
static_assert(std::same_as<decltype(LinkInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(LinkInput, id) == offsetof(BaseInput, id));
// static asserts for conversion of LinkInput to BranchInput
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
// static asserts for conversion of TransformerInput to BaseInput
static_assert(std::alignment_of_v<TransformerInput> >= std::alignment_of_v<BranchInput>);
static_assert(std::same_as<decltype(TransformerInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(TransformerInput, id) == offsetof(BaseInput, id));
// static asserts for conversion of TransformerInput to BranchInput
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
// static asserts for conversion of ThreeWindingTransformerInput to BaseInput
static_assert(std::alignment_of_v<ThreeWindingTransformerInput> >= std::alignment_of_v<Branch3Input>);
static_assert(std::same_as<decltype(ThreeWindingTransformerInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(ThreeWindingTransformerInput, id) == offsetof(BaseInput, id));
// static asserts for conversion of ThreeWindingTransformerInput to Branch3Input
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
// static asserts for conversion of GenericLoadGenInput to BaseInput
static_assert(std::alignment_of_v<GenericLoadGenInput> >= std::alignment_of_v<ApplianceInput>);
static_assert(std::same_as<decltype(GenericLoadGenInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(GenericLoadGenInput, id) == offsetof(BaseInput, id));
// static asserts for conversion of GenericLoadGenInput to ApplianceInput
static_assert(std::alignment_of_v<GenericLoadGenInput> >= std::alignment_of_v<ApplianceInput>);
static_assert(std::same_as<decltype(GenericLoadGenInput::id), decltype(ApplianceInput::id)>);
static_assert(std::same_as<decltype(GenericLoadGenInput::node), decltype(ApplianceInput::node)>);
static_assert(std::same_as<decltype(GenericLoadGenInput::status), decltype(ApplianceInput::status)>);
static_assert(offsetof(GenericLoadGenInput, id) == offsetof(ApplianceInput, id));
static_assert(offsetof(GenericLoadGenInput, node) == offsetof(ApplianceInput, node));
static_assert(offsetof(GenericLoadGenInput, status) == offsetof(ApplianceInput, status));

// static asserts for LoadGenInput<symmetric_t>
static_assert(std::is_standard_layout_v<LoadGenInput<symmetric_t>>);
// static asserts for conversion of LoadGenInput<symmetric_t> to BaseInput
static_assert(std::alignment_of_v<LoadGenInput<symmetric_t>> >= std::alignment_of_v<GenericLoadGenInput>);
static_assert(std::same_as<decltype(LoadGenInput<symmetric_t>::id), decltype(BaseInput::id)>);
static_assert(offsetof(LoadGenInput<symmetric_t>, id) == offsetof(BaseInput, id));
// static asserts for conversion of LoadGenInput<symmetric_t> to ApplianceInput
static_assert(std::alignment_of_v<LoadGenInput<symmetric_t>> >= std::alignment_of_v<GenericLoadGenInput>);
static_assert(std::same_as<decltype(LoadGenInput<symmetric_t>::id), decltype(ApplianceInput::id)>);
static_assert(std::same_as<decltype(LoadGenInput<symmetric_t>::node), decltype(ApplianceInput::node)>);
static_assert(std::same_as<decltype(LoadGenInput<symmetric_t>::status), decltype(ApplianceInput::status)>);
static_assert(offsetof(LoadGenInput<symmetric_t>, id) == offsetof(ApplianceInput, id));
static_assert(offsetof(LoadGenInput<symmetric_t>, node) == offsetof(ApplianceInput, node));
static_assert(offsetof(LoadGenInput<symmetric_t>, status) == offsetof(ApplianceInput, status));
// static asserts for conversion of LoadGenInput<symmetric_t> to GenericLoadGenInput
static_assert(std::alignment_of_v<LoadGenInput<symmetric_t>> >= std::alignment_of_v<GenericLoadGenInput>);
static_assert(std::same_as<decltype(LoadGenInput<symmetric_t>::id), decltype(GenericLoadGenInput::id)>);
static_assert(std::same_as<decltype(LoadGenInput<symmetric_t>::node), decltype(GenericLoadGenInput::node)>);
static_assert(std::same_as<decltype(LoadGenInput<symmetric_t>::status), decltype(GenericLoadGenInput::status)>);
static_assert(std::same_as<decltype(LoadGenInput<symmetric_t>::type), decltype(GenericLoadGenInput::type)>);
static_assert(offsetof(LoadGenInput<symmetric_t>, id) == offsetof(GenericLoadGenInput, id));
static_assert(offsetof(LoadGenInput<symmetric_t>, node) == offsetof(GenericLoadGenInput, node));
static_assert(offsetof(LoadGenInput<symmetric_t>, status) == offsetof(GenericLoadGenInput, status));
static_assert(offsetof(LoadGenInput<symmetric_t>, type) == offsetof(GenericLoadGenInput, type));
// static asserts for LoadGenInput<asymmetric_t>
static_assert(std::is_standard_layout_v<LoadGenInput<asymmetric_t>>);
// static asserts for conversion of LoadGenInput<asymmetric_t> to BaseInput
static_assert(std::alignment_of_v<LoadGenInput<asymmetric_t>> >= std::alignment_of_v<GenericLoadGenInput>);
static_assert(std::same_as<decltype(LoadGenInput<asymmetric_t>::id), decltype(BaseInput::id)>);
static_assert(offsetof(LoadGenInput<asymmetric_t>, id) == offsetof(BaseInput, id));
// static asserts for conversion of LoadGenInput<asymmetric_t> to ApplianceInput
static_assert(std::alignment_of_v<LoadGenInput<asymmetric_t>> >= std::alignment_of_v<GenericLoadGenInput>);
static_assert(std::same_as<decltype(LoadGenInput<asymmetric_t>::id), decltype(ApplianceInput::id)>);
static_assert(std::same_as<decltype(LoadGenInput<asymmetric_t>::node), decltype(ApplianceInput::node)>);
static_assert(std::same_as<decltype(LoadGenInput<asymmetric_t>::status), decltype(ApplianceInput::status)>);
static_assert(offsetof(LoadGenInput<asymmetric_t>, id) == offsetof(ApplianceInput, id));
static_assert(offsetof(LoadGenInput<asymmetric_t>, node) == offsetof(ApplianceInput, node));
static_assert(offsetof(LoadGenInput<asymmetric_t>, status) == offsetof(ApplianceInput, status));
// static asserts for conversion of LoadGenInput<asymmetric_t> to GenericLoadGenInput
static_assert(std::alignment_of_v<LoadGenInput<asymmetric_t>> >= std::alignment_of_v<GenericLoadGenInput>);
static_assert(std::same_as<decltype(LoadGenInput<asymmetric_t>::id), decltype(GenericLoadGenInput::id)>);
static_assert(std::same_as<decltype(LoadGenInput<asymmetric_t>::node), decltype(GenericLoadGenInput::node)>);
static_assert(std::same_as<decltype(LoadGenInput<asymmetric_t>::status), decltype(GenericLoadGenInput::status)>);
static_assert(std::same_as<decltype(LoadGenInput<asymmetric_t>::type), decltype(GenericLoadGenInput::type)>);
static_assert(offsetof(LoadGenInput<asymmetric_t>, id) == offsetof(GenericLoadGenInput, id));
static_assert(offsetof(LoadGenInput<asymmetric_t>, node) == offsetof(GenericLoadGenInput, node));
static_assert(offsetof(LoadGenInput<asymmetric_t>, status) == offsetof(GenericLoadGenInput, status));
static_assert(offsetof(LoadGenInput<asymmetric_t>, type) == offsetof(GenericLoadGenInput, type));
// static asserts for SymLoadGenInput
static_assert(std::is_standard_layout_v<SymLoadGenInput>);
// static asserts for conversion of SymLoadGenInput to BaseInput
static_assert(std::alignment_of_v<SymLoadGenInput> >= std::alignment_of_v<GenericLoadGenInput>);
static_assert(std::same_as<decltype(SymLoadGenInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(SymLoadGenInput, id) == offsetof(BaseInput, id));
// static asserts for conversion of SymLoadGenInput to ApplianceInput
static_assert(std::alignment_of_v<SymLoadGenInput> >= std::alignment_of_v<GenericLoadGenInput>);
static_assert(std::same_as<decltype(SymLoadGenInput::id), decltype(ApplianceInput::id)>);
static_assert(std::same_as<decltype(SymLoadGenInput::node), decltype(ApplianceInput::node)>);
static_assert(std::same_as<decltype(SymLoadGenInput::status), decltype(ApplianceInput::status)>);
static_assert(offsetof(SymLoadGenInput, id) == offsetof(ApplianceInput, id));
static_assert(offsetof(SymLoadGenInput, node) == offsetof(ApplianceInput, node));
static_assert(offsetof(SymLoadGenInput, status) == offsetof(ApplianceInput, status));
// static asserts for conversion of SymLoadGenInput to GenericLoadGenInput
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
// static asserts for conversion of AsymLoadGenInput to BaseInput
static_assert(std::alignment_of_v<AsymLoadGenInput> >= std::alignment_of_v<GenericLoadGenInput>);
static_assert(std::same_as<decltype(AsymLoadGenInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(AsymLoadGenInput, id) == offsetof(BaseInput, id));
// static asserts for conversion of AsymLoadGenInput to ApplianceInput
static_assert(std::alignment_of_v<AsymLoadGenInput> >= std::alignment_of_v<GenericLoadGenInput>);
static_assert(std::same_as<decltype(AsymLoadGenInput::id), decltype(ApplianceInput::id)>);
static_assert(std::same_as<decltype(AsymLoadGenInput::node), decltype(ApplianceInput::node)>);
static_assert(std::same_as<decltype(AsymLoadGenInput::status), decltype(ApplianceInput::status)>);
static_assert(offsetof(AsymLoadGenInput, id) == offsetof(ApplianceInput, id));
static_assert(offsetof(AsymLoadGenInput, node) == offsetof(ApplianceInput, node));
static_assert(offsetof(AsymLoadGenInput, status) == offsetof(ApplianceInput, status));
// static asserts for conversion of AsymLoadGenInput to GenericLoadGenInput
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
// static asserts for conversion of ShuntInput to BaseInput
static_assert(std::alignment_of_v<ShuntInput> >= std::alignment_of_v<ApplianceInput>);
static_assert(std::same_as<decltype(ShuntInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(ShuntInput, id) == offsetof(BaseInput, id));
// static asserts for conversion of ShuntInput to ApplianceInput
static_assert(std::alignment_of_v<ShuntInput> >= std::alignment_of_v<ApplianceInput>);
static_assert(std::same_as<decltype(ShuntInput::id), decltype(ApplianceInput::id)>);
static_assert(std::same_as<decltype(ShuntInput::node), decltype(ApplianceInput::node)>);
static_assert(std::same_as<decltype(ShuntInput::status), decltype(ApplianceInput::status)>);
static_assert(offsetof(ShuntInput, id) == offsetof(ApplianceInput, id));
static_assert(offsetof(ShuntInput, node) == offsetof(ApplianceInput, node));
static_assert(offsetof(ShuntInput, status) == offsetof(ApplianceInput, status));

// static asserts for SourceInput
static_assert(std::is_standard_layout_v<SourceInput>);
// static asserts for conversion of SourceInput to BaseInput
static_assert(std::alignment_of_v<SourceInput> >= std::alignment_of_v<ApplianceInput>);
static_assert(std::same_as<decltype(SourceInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(SourceInput, id) == offsetof(BaseInput, id));
// static asserts for conversion of SourceInput to ApplianceInput
static_assert(std::alignment_of_v<SourceInput> >= std::alignment_of_v<ApplianceInput>);
static_assert(std::same_as<decltype(SourceInput::id), decltype(ApplianceInput::id)>);
static_assert(std::same_as<decltype(SourceInput::node), decltype(ApplianceInput::node)>);
static_assert(std::same_as<decltype(SourceInput::status), decltype(ApplianceInput::status)>);
static_assert(offsetof(SourceInput, id) == offsetof(ApplianceInput, id));
static_assert(offsetof(SourceInput, node) == offsetof(ApplianceInput, node));
static_assert(offsetof(SourceInput, status) == offsetof(ApplianceInput, status));

// static asserts for GenericVoltageSensorInput
static_assert(std::is_standard_layout_v<GenericVoltageSensorInput>);
// static asserts for conversion of GenericVoltageSensorInput to BaseInput
static_assert(std::alignment_of_v<GenericVoltageSensorInput> >= std::alignment_of_v<SensorInput>);
static_assert(std::same_as<decltype(GenericVoltageSensorInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(GenericVoltageSensorInput, id) == offsetof(BaseInput, id));
// static asserts for conversion of GenericVoltageSensorInput to SensorInput
static_assert(std::alignment_of_v<GenericVoltageSensorInput> >= std::alignment_of_v<SensorInput>);
static_assert(std::same_as<decltype(GenericVoltageSensorInput::id), decltype(SensorInput::id)>);
static_assert(std::same_as<decltype(GenericVoltageSensorInput::measured_object), decltype(SensorInput::measured_object)>);
static_assert(offsetof(GenericVoltageSensorInput, id) == offsetof(SensorInput, id));
static_assert(offsetof(GenericVoltageSensorInput, measured_object) == offsetof(SensorInput, measured_object));

// static asserts for VoltageSensorInput<symmetric_t>
static_assert(std::is_standard_layout_v<VoltageSensorInput<symmetric_t>>);
// static asserts for conversion of VoltageSensorInput<symmetric_t> to BaseInput
static_assert(std::alignment_of_v<VoltageSensorInput<symmetric_t>> >= std::alignment_of_v<GenericVoltageSensorInput>);
static_assert(std::same_as<decltype(VoltageSensorInput<symmetric_t>::id), decltype(BaseInput::id)>);
static_assert(offsetof(VoltageSensorInput<symmetric_t>, id) == offsetof(BaseInput, id));
// static asserts for conversion of VoltageSensorInput<symmetric_t> to SensorInput
static_assert(std::alignment_of_v<VoltageSensorInput<symmetric_t>> >= std::alignment_of_v<GenericVoltageSensorInput>);
static_assert(std::same_as<decltype(VoltageSensorInput<symmetric_t>::id), decltype(SensorInput::id)>);
static_assert(std::same_as<decltype(VoltageSensorInput<symmetric_t>::measured_object), decltype(SensorInput::measured_object)>);
static_assert(offsetof(VoltageSensorInput<symmetric_t>, id) == offsetof(SensorInput, id));
static_assert(offsetof(VoltageSensorInput<symmetric_t>, measured_object) == offsetof(SensorInput, measured_object));
// static asserts for conversion of VoltageSensorInput<symmetric_t> to GenericVoltageSensorInput
static_assert(std::alignment_of_v<VoltageSensorInput<symmetric_t>> >= std::alignment_of_v<GenericVoltageSensorInput>);
static_assert(std::same_as<decltype(VoltageSensorInput<symmetric_t>::id), decltype(GenericVoltageSensorInput::id)>);
static_assert(std::same_as<decltype(VoltageSensorInput<symmetric_t>::measured_object), decltype(GenericVoltageSensorInput::measured_object)>);
static_assert(std::same_as<decltype(VoltageSensorInput<symmetric_t>::u_sigma), decltype(GenericVoltageSensorInput::u_sigma)>);
static_assert(offsetof(VoltageSensorInput<symmetric_t>, id) == offsetof(GenericVoltageSensorInput, id));
static_assert(offsetof(VoltageSensorInput<symmetric_t>, measured_object) == offsetof(GenericVoltageSensorInput, measured_object));
static_assert(offsetof(VoltageSensorInput<symmetric_t>, u_sigma) == offsetof(GenericVoltageSensorInput, u_sigma));
// static asserts for VoltageSensorInput<asymmetric_t>
static_assert(std::is_standard_layout_v<VoltageSensorInput<asymmetric_t>>);
// static asserts for conversion of VoltageSensorInput<asymmetric_t> to BaseInput
static_assert(std::alignment_of_v<VoltageSensorInput<asymmetric_t>> >= std::alignment_of_v<GenericVoltageSensorInput>);
static_assert(std::same_as<decltype(VoltageSensorInput<asymmetric_t>::id), decltype(BaseInput::id)>);
static_assert(offsetof(VoltageSensorInput<asymmetric_t>, id) == offsetof(BaseInput, id));
// static asserts for conversion of VoltageSensorInput<asymmetric_t> to SensorInput
static_assert(std::alignment_of_v<VoltageSensorInput<asymmetric_t>> >= std::alignment_of_v<GenericVoltageSensorInput>);
static_assert(std::same_as<decltype(VoltageSensorInput<asymmetric_t>::id), decltype(SensorInput::id)>);
static_assert(std::same_as<decltype(VoltageSensorInput<asymmetric_t>::measured_object), decltype(SensorInput::measured_object)>);
static_assert(offsetof(VoltageSensorInput<asymmetric_t>, id) == offsetof(SensorInput, id));
static_assert(offsetof(VoltageSensorInput<asymmetric_t>, measured_object) == offsetof(SensorInput, measured_object));
// static asserts for conversion of VoltageSensorInput<asymmetric_t> to GenericVoltageSensorInput
static_assert(std::alignment_of_v<VoltageSensorInput<asymmetric_t>> >= std::alignment_of_v<GenericVoltageSensorInput>);
static_assert(std::same_as<decltype(VoltageSensorInput<asymmetric_t>::id), decltype(GenericVoltageSensorInput::id)>);
static_assert(std::same_as<decltype(VoltageSensorInput<asymmetric_t>::measured_object), decltype(GenericVoltageSensorInput::measured_object)>);
static_assert(std::same_as<decltype(VoltageSensorInput<asymmetric_t>::u_sigma), decltype(GenericVoltageSensorInput::u_sigma)>);
static_assert(offsetof(VoltageSensorInput<asymmetric_t>, id) == offsetof(GenericVoltageSensorInput, id));
static_assert(offsetof(VoltageSensorInput<asymmetric_t>, measured_object) == offsetof(GenericVoltageSensorInput, measured_object));
static_assert(offsetof(VoltageSensorInput<asymmetric_t>, u_sigma) == offsetof(GenericVoltageSensorInput, u_sigma));
// static asserts for SymVoltageSensorInput
static_assert(std::is_standard_layout_v<SymVoltageSensorInput>);
// static asserts for conversion of SymVoltageSensorInput to BaseInput
static_assert(std::alignment_of_v<SymVoltageSensorInput> >= std::alignment_of_v<GenericVoltageSensorInput>);
static_assert(std::same_as<decltype(SymVoltageSensorInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(SymVoltageSensorInput, id) == offsetof(BaseInput, id));
// static asserts for conversion of SymVoltageSensorInput to SensorInput
static_assert(std::alignment_of_v<SymVoltageSensorInput> >= std::alignment_of_v<GenericVoltageSensorInput>);
static_assert(std::same_as<decltype(SymVoltageSensorInput::id), decltype(SensorInput::id)>);
static_assert(std::same_as<decltype(SymVoltageSensorInput::measured_object), decltype(SensorInput::measured_object)>);
static_assert(offsetof(SymVoltageSensorInput, id) == offsetof(SensorInput, id));
static_assert(offsetof(SymVoltageSensorInput, measured_object) == offsetof(SensorInput, measured_object));
// static asserts for conversion of SymVoltageSensorInput to GenericVoltageSensorInput
static_assert(std::alignment_of_v<SymVoltageSensorInput> >= std::alignment_of_v<GenericVoltageSensorInput>);
static_assert(std::same_as<decltype(SymVoltageSensorInput::id), decltype(GenericVoltageSensorInput::id)>);
static_assert(std::same_as<decltype(SymVoltageSensorInput::measured_object), decltype(GenericVoltageSensorInput::measured_object)>);
static_assert(std::same_as<decltype(SymVoltageSensorInput::u_sigma), decltype(GenericVoltageSensorInput::u_sigma)>);
static_assert(offsetof(SymVoltageSensorInput, id) == offsetof(GenericVoltageSensorInput, id));
static_assert(offsetof(SymVoltageSensorInput, measured_object) == offsetof(GenericVoltageSensorInput, measured_object));
static_assert(offsetof(SymVoltageSensorInput, u_sigma) == offsetof(GenericVoltageSensorInput, u_sigma));
// static asserts for AsymVoltageSensorInput
static_assert(std::is_standard_layout_v<AsymVoltageSensorInput>);
// static asserts for conversion of AsymVoltageSensorInput to BaseInput
static_assert(std::alignment_of_v<AsymVoltageSensorInput> >= std::alignment_of_v<GenericVoltageSensorInput>);
static_assert(std::same_as<decltype(AsymVoltageSensorInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(AsymVoltageSensorInput, id) == offsetof(BaseInput, id));
// static asserts for conversion of AsymVoltageSensorInput to SensorInput
static_assert(std::alignment_of_v<AsymVoltageSensorInput> >= std::alignment_of_v<GenericVoltageSensorInput>);
static_assert(std::same_as<decltype(AsymVoltageSensorInput::id), decltype(SensorInput::id)>);
static_assert(std::same_as<decltype(AsymVoltageSensorInput::measured_object), decltype(SensorInput::measured_object)>);
static_assert(offsetof(AsymVoltageSensorInput, id) == offsetof(SensorInput, id));
static_assert(offsetof(AsymVoltageSensorInput, measured_object) == offsetof(SensorInput, measured_object));
// static asserts for conversion of AsymVoltageSensorInput to GenericVoltageSensorInput
static_assert(std::alignment_of_v<AsymVoltageSensorInput> >= std::alignment_of_v<GenericVoltageSensorInput>);
static_assert(std::same_as<decltype(AsymVoltageSensorInput::id), decltype(GenericVoltageSensorInput::id)>);
static_assert(std::same_as<decltype(AsymVoltageSensorInput::measured_object), decltype(GenericVoltageSensorInput::measured_object)>);
static_assert(std::same_as<decltype(AsymVoltageSensorInput::u_sigma), decltype(GenericVoltageSensorInput::u_sigma)>);
static_assert(offsetof(AsymVoltageSensorInput, id) == offsetof(GenericVoltageSensorInput, id));
static_assert(offsetof(AsymVoltageSensorInput, measured_object) == offsetof(GenericVoltageSensorInput, measured_object));
static_assert(offsetof(AsymVoltageSensorInput, u_sigma) == offsetof(GenericVoltageSensorInput, u_sigma));

// static asserts for GenericPowerSensorInput
static_assert(std::is_standard_layout_v<GenericPowerSensorInput>);
// static asserts for conversion of GenericPowerSensorInput to BaseInput
static_assert(std::alignment_of_v<GenericPowerSensorInput> >= std::alignment_of_v<SensorInput>);
static_assert(std::same_as<decltype(GenericPowerSensorInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(GenericPowerSensorInput, id) == offsetof(BaseInput, id));
// static asserts for conversion of GenericPowerSensorInput to SensorInput
static_assert(std::alignment_of_v<GenericPowerSensorInput> >= std::alignment_of_v<SensorInput>);
static_assert(std::same_as<decltype(GenericPowerSensorInput::id), decltype(SensorInput::id)>);
static_assert(std::same_as<decltype(GenericPowerSensorInput::measured_object), decltype(SensorInput::measured_object)>);
static_assert(offsetof(GenericPowerSensorInput, id) == offsetof(SensorInput, id));
static_assert(offsetof(GenericPowerSensorInput, measured_object) == offsetof(SensorInput, measured_object));

// static asserts for PowerSensorInput<symmetric_t>
static_assert(std::is_standard_layout_v<PowerSensorInput<symmetric_t>>);
// static asserts for conversion of PowerSensorInput<symmetric_t> to BaseInput
static_assert(std::alignment_of_v<PowerSensorInput<symmetric_t>> >= std::alignment_of_v<GenericPowerSensorInput>);
static_assert(std::same_as<decltype(PowerSensorInput<symmetric_t>::id), decltype(BaseInput::id)>);
static_assert(offsetof(PowerSensorInput<symmetric_t>, id) == offsetof(BaseInput, id));
// static asserts for conversion of PowerSensorInput<symmetric_t> to SensorInput
static_assert(std::alignment_of_v<PowerSensorInput<symmetric_t>> >= std::alignment_of_v<GenericPowerSensorInput>);
static_assert(std::same_as<decltype(PowerSensorInput<symmetric_t>::id), decltype(SensorInput::id)>);
static_assert(std::same_as<decltype(PowerSensorInput<symmetric_t>::measured_object), decltype(SensorInput::measured_object)>);
static_assert(offsetof(PowerSensorInput<symmetric_t>, id) == offsetof(SensorInput, id));
static_assert(offsetof(PowerSensorInput<symmetric_t>, measured_object) == offsetof(SensorInput, measured_object));
// static asserts for conversion of PowerSensorInput<symmetric_t> to GenericPowerSensorInput
static_assert(std::alignment_of_v<PowerSensorInput<symmetric_t>> >= std::alignment_of_v<GenericPowerSensorInput>);
static_assert(std::same_as<decltype(PowerSensorInput<symmetric_t>::id), decltype(GenericPowerSensorInput::id)>);
static_assert(std::same_as<decltype(PowerSensorInput<symmetric_t>::measured_object), decltype(GenericPowerSensorInput::measured_object)>);
static_assert(std::same_as<decltype(PowerSensorInput<symmetric_t>::measured_terminal_type), decltype(GenericPowerSensorInput::measured_terminal_type)>);
static_assert(std::same_as<decltype(PowerSensorInput<symmetric_t>::power_sigma), decltype(GenericPowerSensorInput::power_sigma)>);
static_assert(offsetof(PowerSensorInput<symmetric_t>, id) == offsetof(GenericPowerSensorInput, id));
static_assert(offsetof(PowerSensorInput<symmetric_t>, measured_object) == offsetof(GenericPowerSensorInput, measured_object));
static_assert(offsetof(PowerSensorInput<symmetric_t>, measured_terminal_type) == offsetof(GenericPowerSensorInput, measured_terminal_type));
static_assert(offsetof(PowerSensorInput<symmetric_t>, power_sigma) == offsetof(GenericPowerSensorInput, power_sigma));
// static asserts for PowerSensorInput<asymmetric_t>
static_assert(std::is_standard_layout_v<PowerSensorInput<asymmetric_t>>);
// static asserts for conversion of PowerSensorInput<asymmetric_t> to BaseInput
static_assert(std::alignment_of_v<PowerSensorInput<asymmetric_t>> >= std::alignment_of_v<GenericPowerSensorInput>);
static_assert(std::same_as<decltype(PowerSensorInput<asymmetric_t>::id), decltype(BaseInput::id)>);
static_assert(offsetof(PowerSensorInput<asymmetric_t>, id) == offsetof(BaseInput, id));
// static asserts for conversion of PowerSensorInput<asymmetric_t> to SensorInput
static_assert(std::alignment_of_v<PowerSensorInput<asymmetric_t>> >= std::alignment_of_v<GenericPowerSensorInput>);
static_assert(std::same_as<decltype(PowerSensorInput<asymmetric_t>::id), decltype(SensorInput::id)>);
static_assert(std::same_as<decltype(PowerSensorInput<asymmetric_t>::measured_object), decltype(SensorInput::measured_object)>);
static_assert(offsetof(PowerSensorInput<asymmetric_t>, id) == offsetof(SensorInput, id));
static_assert(offsetof(PowerSensorInput<asymmetric_t>, measured_object) == offsetof(SensorInput, measured_object));
// static asserts for conversion of PowerSensorInput<asymmetric_t> to GenericPowerSensorInput
static_assert(std::alignment_of_v<PowerSensorInput<asymmetric_t>> >= std::alignment_of_v<GenericPowerSensorInput>);
static_assert(std::same_as<decltype(PowerSensorInput<asymmetric_t>::id), decltype(GenericPowerSensorInput::id)>);
static_assert(std::same_as<decltype(PowerSensorInput<asymmetric_t>::measured_object), decltype(GenericPowerSensorInput::measured_object)>);
static_assert(std::same_as<decltype(PowerSensorInput<asymmetric_t>::measured_terminal_type), decltype(GenericPowerSensorInput::measured_terminal_type)>);
static_assert(std::same_as<decltype(PowerSensorInput<asymmetric_t>::power_sigma), decltype(GenericPowerSensorInput::power_sigma)>);
static_assert(offsetof(PowerSensorInput<asymmetric_t>, id) == offsetof(GenericPowerSensorInput, id));
static_assert(offsetof(PowerSensorInput<asymmetric_t>, measured_object) == offsetof(GenericPowerSensorInput, measured_object));
static_assert(offsetof(PowerSensorInput<asymmetric_t>, measured_terminal_type) == offsetof(GenericPowerSensorInput, measured_terminal_type));
static_assert(offsetof(PowerSensorInput<asymmetric_t>, power_sigma) == offsetof(GenericPowerSensorInput, power_sigma));
// static asserts for SymPowerSensorInput
static_assert(std::is_standard_layout_v<SymPowerSensorInput>);
// static asserts for conversion of SymPowerSensorInput to BaseInput
static_assert(std::alignment_of_v<SymPowerSensorInput> >= std::alignment_of_v<GenericPowerSensorInput>);
static_assert(std::same_as<decltype(SymPowerSensorInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(SymPowerSensorInput, id) == offsetof(BaseInput, id));
// static asserts for conversion of SymPowerSensorInput to SensorInput
static_assert(std::alignment_of_v<SymPowerSensorInput> >= std::alignment_of_v<GenericPowerSensorInput>);
static_assert(std::same_as<decltype(SymPowerSensorInput::id), decltype(SensorInput::id)>);
static_assert(std::same_as<decltype(SymPowerSensorInput::measured_object), decltype(SensorInput::measured_object)>);
static_assert(offsetof(SymPowerSensorInput, id) == offsetof(SensorInput, id));
static_assert(offsetof(SymPowerSensorInput, measured_object) == offsetof(SensorInput, measured_object));
// static asserts for conversion of SymPowerSensorInput to GenericPowerSensorInput
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
// static asserts for conversion of AsymPowerSensorInput to BaseInput
static_assert(std::alignment_of_v<AsymPowerSensorInput> >= std::alignment_of_v<GenericPowerSensorInput>);
static_assert(std::same_as<decltype(AsymPowerSensorInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(AsymPowerSensorInput, id) == offsetof(BaseInput, id));
// static asserts for conversion of AsymPowerSensorInput to SensorInput
static_assert(std::alignment_of_v<AsymPowerSensorInput> >= std::alignment_of_v<GenericPowerSensorInput>);
static_assert(std::same_as<decltype(AsymPowerSensorInput::id), decltype(SensorInput::id)>);
static_assert(std::same_as<decltype(AsymPowerSensorInput::measured_object), decltype(SensorInput::measured_object)>);
static_assert(offsetof(AsymPowerSensorInput, id) == offsetof(SensorInput, id));
static_assert(offsetof(AsymPowerSensorInput, measured_object) == offsetof(SensorInput, measured_object));
// static asserts for conversion of AsymPowerSensorInput to GenericPowerSensorInput
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
// static asserts for conversion of FaultInput to BaseInput
static_assert(std::alignment_of_v<FaultInput> >= std::alignment_of_v<BaseInput>);
static_assert(std::same_as<decltype(FaultInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(FaultInput, id) == offsetof(BaseInput, id));

// static asserts for RegulatorInput
static_assert(std::is_standard_layout_v<RegulatorInput>);
// static asserts for conversion of RegulatorInput to BaseInput
static_assert(std::alignment_of_v<RegulatorInput> >= std::alignment_of_v<BaseInput>);
static_assert(std::same_as<decltype(RegulatorInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(RegulatorInput, id) == offsetof(BaseInput, id));

// static asserts for TransformerTapRegulatorInput
static_assert(std::is_standard_layout_v<TransformerTapRegulatorInput>);
// static asserts for conversion of TransformerTapRegulatorInput to BaseInput
static_assert(std::alignment_of_v<TransformerTapRegulatorInput> >= std::alignment_of_v<RegulatorInput>);
static_assert(std::same_as<decltype(TransformerTapRegulatorInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(TransformerTapRegulatorInput, id) == offsetof(BaseInput, id));
// static asserts for conversion of TransformerTapRegulatorInput to RegulatorInput
static_assert(std::alignment_of_v<TransformerTapRegulatorInput> >= std::alignment_of_v<RegulatorInput>);
static_assert(std::same_as<decltype(TransformerTapRegulatorInput::id), decltype(RegulatorInput::id)>);
static_assert(std::same_as<decltype(TransformerTapRegulatorInput::regulated_object), decltype(RegulatorInput::regulated_object)>);
static_assert(std::same_as<decltype(TransformerTapRegulatorInput::status), decltype(RegulatorInput::status)>);
static_assert(offsetof(TransformerTapRegulatorInput, id) == offsetof(RegulatorInput, id));
static_assert(offsetof(TransformerTapRegulatorInput, regulated_object) == offsetof(RegulatorInput, regulated_object));
static_assert(offsetof(TransformerTapRegulatorInput, status) == offsetof(RegulatorInput, status));

// static asserts for GenericCurrentSensorInput
static_assert(std::is_standard_layout_v<GenericCurrentSensorInput>);
// static asserts for conversion of GenericCurrentSensorInput to BaseInput
static_assert(std::alignment_of_v<GenericCurrentSensorInput> >= std::alignment_of_v<SensorInput>);
static_assert(std::same_as<decltype(GenericCurrentSensorInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(GenericCurrentSensorInput, id) == offsetof(BaseInput, id));
// static asserts for conversion of GenericCurrentSensorInput to SensorInput
static_assert(std::alignment_of_v<GenericCurrentSensorInput> >= std::alignment_of_v<SensorInput>);
static_assert(std::same_as<decltype(GenericCurrentSensorInput::id), decltype(SensorInput::id)>);
static_assert(std::same_as<decltype(GenericCurrentSensorInput::measured_object), decltype(SensorInput::measured_object)>);
static_assert(offsetof(GenericCurrentSensorInput, id) == offsetof(SensorInput, id));
static_assert(offsetof(GenericCurrentSensorInput, measured_object) == offsetof(SensorInput, measured_object));

// static asserts for CurrentSensorInput<symmetric_t>
static_assert(std::is_standard_layout_v<CurrentSensorInput<symmetric_t>>);
// static asserts for conversion of CurrentSensorInput<symmetric_t> to BaseInput
static_assert(std::alignment_of_v<CurrentSensorInput<symmetric_t>> >= std::alignment_of_v<GenericCurrentSensorInput>);
static_assert(std::same_as<decltype(CurrentSensorInput<symmetric_t>::id), decltype(BaseInput::id)>);
static_assert(offsetof(CurrentSensorInput<symmetric_t>, id) == offsetof(BaseInput, id));
// static asserts for conversion of CurrentSensorInput<symmetric_t> to SensorInput
static_assert(std::alignment_of_v<CurrentSensorInput<symmetric_t>> >= std::alignment_of_v<GenericCurrentSensorInput>);
static_assert(std::same_as<decltype(CurrentSensorInput<symmetric_t>::id), decltype(SensorInput::id)>);
static_assert(std::same_as<decltype(CurrentSensorInput<symmetric_t>::measured_object), decltype(SensorInput::measured_object)>);
static_assert(offsetof(CurrentSensorInput<symmetric_t>, id) == offsetof(SensorInput, id));
static_assert(offsetof(CurrentSensorInput<symmetric_t>, measured_object) == offsetof(SensorInput, measured_object));
// static asserts for conversion of CurrentSensorInput<symmetric_t> to GenericCurrentSensorInput
static_assert(std::alignment_of_v<CurrentSensorInput<symmetric_t>> >= std::alignment_of_v<GenericCurrentSensorInput>);
static_assert(std::same_as<decltype(CurrentSensorInput<symmetric_t>::id), decltype(GenericCurrentSensorInput::id)>);
static_assert(std::same_as<decltype(CurrentSensorInput<symmetric_t>::measured_object), decltype(GenericCurrentSensorInput::measured_object)>);
static_assert(std::same_as<decltype(CurrentSensorInput<symmetric_t>::measured_terminal_type), decltype(GenericCurrentSensorInput::measured_terminal_type)>);
static_assert(std::same_as<decltype(CurrentSensorInput<symmetric_t>::angle_measurement_type), decltype(GenericCurrentSensorInput::angle_measurement_type)>);
static_assert(std::same_as<decltype(CurrentSensorInput<symmetric_t>::i_sigma), decltype(GenericCurrentSensorInput::i_sigma)>);
static_assert(std::same_as<decltype(CurrentSensorInput<symmetric_t>::i_angle_sigma), decltype(GenericCurrentSensorInput::i_angle_sigma)>);
static_assert(offsetof(CurrentSensorInput<symmetric_t>, id) == offsetof(GenericCurrentSensorInput, id));
static_assert(offsetof(CurrentSensorInput<symmetric_t>, measured_object) == offsetof(GenericCurrentSensorInput, measured_object));
static_assert(offsetof(CurrentSensorInput<symmetric_t>, measured_terminal_type) == offsetof(GenericCurrentSensorInput, measured_terminal_type));
static_assert(offsetof(CurrentSensorInput<symmetric_t>, angle_measurement_type) == offsetof(GenericCurrentSensorInput, angle_measurement_type));
static_assert(offsetof(CurrentSensorInput<symmetric_t>, i_sigma) == offsetof(GenericCurrentSensorInput, i_sigma));
static_assert(offsetof(CurrentSensorInput<symmetric_t>, i_angle_sigma) == offsetof(GenericCurrentSensorInput, i_angle_sigma));
// static asserts for CurrentSensorInput<asymmetric_t>
static_assert(std::is_standard_layout_v<CurrentSensorInput<asymmetric_t>>);
// static asserts for conversion of CurrentSensorInput<asymmetric_t> to BaseInput
static_assert(std::alignment_of_v<CurrentSensorInput<asymmetric_t>> >= std::alignment_of_v<GenericCurrentSensorInput>);
static_assert(std::same_as<decltype(CurrentSensorInput<asymmetric_t>::id), decltype(BaseInput::id)>);
static_assert(offsetof(CurrentSensorInput<asymmetric_t>, id) == offsetof(BaseInput, id));
// static asserts for conversion of CurrentSensorInput<asymmetric_t> to SensorInput
static_assert(std::alignment_of_v<CurrentSensorInput<asymmetric_t>> >= std::alignment_of_v<GenericCurrentSensorInput>);
static_assert(std::same_as<decltype(CurrentSensorInput<asymmetric_t>::id), decltype(SensorInput::id)>);
static_assert(std::same_as<decltype(CurrentSensorInput<asymmetric_t>::measured_object), decltype(SensorInput::measured_object)>);
static_assert(offsetof(CurrentSensorInput<asymmetric_t>, id) == offsetof(SensorInput, id));
static_assert(offsetof(CurrentSensorInput<asymmetric_t>, measured_object) == offsetof(SensorInput, measured_object));
// static asserts for conversion of CurrentSensorInput<asymmetric_t> to GenericCurrentSensorInput
static_assert(std::alignment_of_v<CurrentSensorInput<asymmetric_t>> >= std::alignment_of_v<GenericCurrentSensorInput>);
static_assert(std::same_as<decltype(CurrentSensorInput<asymmetric_t>::id), decltype(GenericCurrentSensorInput::id)>);
static_assert(std::same_as<decltype(CurrentSensorInput<asymmetric_t>::measured_object), decltype(GenericCurrentSensorInput::measured_object)>);
static_assert(std::same_as<decltype(CurrentSensorInput<asymmetric_t>::measured_terminal_type), decltype(GenericCurrentSensorInput::measured_terminal_type)>);
static_assert(std::same_as<decltype(CurrentSensorInput<asymmetric_t>::angle_measurement_type), decltype(GenericCurrentSensorInput::angle_measurement_type)>);
static_assert(std::same_as<decltype(CurrentSensorInput<asymmetric_t>::i_sigma), decltype(GenericCurrentSensorInput::i_sigma)>);
static_assert(std::same_as<decltype(CurrentSensorInput<asymmetric_t>::i_angle_sigma), decltype(GenericCurrentSensorInput::i_angle_sigma)>);
static_assert(offsetof(CurrentSensorInput<asymmetric_t>, id) == offsetof(GenericCurrentSensorInput, id));
static_assert(offsetof(CurrentSensorInput<asymmetric_t>, measured_object) == offsetof(GenericCurrentSensorInput, measured_object));
static_assert(offsetof(CurrentSensorInput<asymmetric_t>, measured_terminal_type) == offsetof(GenericCurrentSensorInput, measured_terminal_type));
static_assert(offsetof(CurrentSensorInput<asymmetric_t>, angle_measurement_type) == offsetof(GenericCurrentSensorInput, angle_measurement_type));
static_assert(offsetof(CurrentSensorInput<asymmetric_t>, i_sigma) == offsetof(GenericCurrentSensorInput, i_sigma));
static_assert(offsetof(CurrentSensorInput<asymmetric_t>, i_angle_sigma) == offsetof(GenericCurrentSensorInput, i_angle_sigma));
// static asserts for SymCurrentSensorInput
static_assert(std::is_standard_layout_v<SymCurrentSensorInput>);
// static asserts for conversion of SymCurrentSensorInput to BaseInput
static_assert(std::alignment_of_v<SymCurrentSensorInput> >= std::alignment_of_v<GenericCurrentSensorInput>);
static_assert(std::same_as<decltype(SymCurrentSensorInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(SymCurrentSensorInput, id) == offsetof(BaseInput, id));
// static asserts for conversion of SymCurrentSensorInput to SensorInput
static_assert(std::alignment_of_v<SymCurrentSensorInput> >= std::alignment_of_v<GenericCurrentSensorInput>);
static_assert(std::same_as<decltype(SymCurrentSensorInput::id), decltype(SensorInput::id)>);
static_assert(std::same_as<decltype(SymCurrentSensorInput::measured_object), decltype(SensorInput::measured_object)>);
static_assert(offsetof(SymCurrentSensorInput, id) == offsetof(SensorInput, id));
static_assert(offsetof(SymCurrentSensorInput, measured_object) == offsetof(SensorInput, measured_object));
// static asserts for conversion of SymCurrentSensorInput to GenericCurrentSensorInput
static_assert(std::alignment_of_v<SymCurrentSensorInput> >= std::alignment_of_v<GenericCurrentSensorInput>);
static_assert(std::same_as<decltype(SymCurrentSensorInput::id), decltype(GenericCurrentSensorInput::id)>);
static_assert(std::same_as<decltype(SymCurrentSensorInput::measured_object), decltype(GenericCurrentSensorInput::measured_object)>);
static_assert(std::same_as<decltype(SymCurrentSensorInput::measured_terminal_type), decltype(GenericCurrentSensorInput::measured_terminal_type)>);
static_assert(std::same_as<decltype(SymCurrentSensorInput::angle_measurement_type), decltype(GenericCurrentSensorInput::angle_measurement_type)>);
static_assert(std::same_as<decltype(SymCurrentSensorInput::i_sigma), decltype(GenericCurrentSensorInput::i_sigma)>);
static_assert(std::same_as<decltype(SymCurrentSensorInput::i_angle_sigma), decltype(GenericCurrentSensorInput::i_angle_sigma)>);
static_assert(offsetof(SymCurrentSensorInput, id) == offsetof(GenericCurrentSensorInput, id));
static_assert(offsetof(SymCurrentSensorInput, measured_object) == offsetof(GenericCurrentSensorInput, measured_object));
static_assert(offsetof(SymCurrentSensorInput, measured_terminal_type) == offsetof(GenericCurrentSensorInput, measured_terminal_type));
static_assert(offsetof(SymCurrentSensorInput, angle_measurement_type) == offsetof(GenericCurrentSensorInput, angle_measurement_type));
static_assert(offsetof(SymCurrentSensorInput, i_sigma) == offsetof(GenericCurrentSensorInput, i_sigma));
static_assert(offsetof(SymCurrentSensorInput, i_angle_sigma) == offsetof(GenericCurrentSensorInput, i_angle_sigma));
// static asserts for AsymCurrentSensorInput
static_assert(std::is_standard_layout_v<AsymCurrentSensorInput>);
// static asserts for conversion of AsymCurrentSensorInput to BaseInput
static_assert(std::alignment_of_v<AsymCurrentSensorInput> >= std::alignment_of_v<GenericCurrentSensorInput>);
static_assert(std::same_as<decltype(AsymCurrentSensorInput::id), decltype(BaseInput::id)>);
static_assert(offsetof(AsymCurrentSensorInput, id) == offsetof(BaseInput, id));
// static asserts for conversion of AsymCurrentSensorInput to SensorInput
static_assert(std::alignment_of_v<AsymCurrentSensorInput> >= std::alignment_of_v<GenericCurrentSensorInput>);
static_assert(std::same_as<decltype(AsymCurrentSensorInput::id), decltype(SensorInput::id)>);
static_assert(std::same_as<decltype(AsymCurrentSensorInput::measured_object), decltype(SensorInput::measured_object)>);
static_assert(offsetof(AsymCurrentSensorInput, id) == offsetof(SensorInput, id));
static_assert(offsetof(AsymCurrentSensorInput, measured_object) == offsetof(SensorInput, measured_object));
// static asserts for conversion of AsymCurrentSensorInput to GenericCurrentSensorInput
static_assert(std::alignment_of_v<AsymCurrentSensorInput> >= std::alignment_of_v<GenericCurrentSensorInput>);
static_assert(std::same_as<decltype(AsymCurrentSensorInput::id), decltype(GenericCurrentSensorInput::id)>);
static_assert(std::same_as<decltype(AsymCurrentSensorInput::measured_object), decltype(GenericCurrentSensorInput::measured_object)>);
static_assert(std::same_as<decltype(AsymCurrentSensorInput::measured_terminal_type), decltype(GenericCurrentSensorInput::measured_terminal_type)>);
static_assert(std::same_as<decltype(AsymCurrentSensorInput::angle_measurement_type), decltype(GenericCurrentSensorInput::angle_measurement_type)>);
static_assert(std::same_as<decltype(AsymCurrentSensorInput::i_sigma), decltype(GenericCurrentSensorInput::i_sigma)>);
static_assert(std::same_as<decltype(AsymCurrentSensorInput::i_angle_sigma), decltype(GenericCurrentSensorInput::i_angle_sigma)>);
static_assert(offsetof(AsymCurrentSensorInput, id) == offsetof(GenericCurrentSensorInput, id));
static_assert(offsetof(AsymCurrentSensorInput, measured_object) == offsetof(GenericCurrentSensorInput, measured_object));
static_assert(offsetof(AsymCurrentSensorInput, measured_terminal_type) == offsetof(GenericCurrentSensorInput, measured_terminal_type));
static_assert(offsetof(AsymCurrentSensorInput, angle_measurement_type) == offsetof(GenericCurrentSensorInput, angle_measurement_type));
static_assert(offsetof(AsymCurrentSensorInput, i_sigma) == offsetof(GenericCurrentSensorInput, i_sigma));
static_assert(offsetof(AsymCurrentSensorInput, i_angle_sigma) == offsetof(GenericCurrentSensorInput, i_angle_sigma));



} // namespace power_grid_model::test

// clang-format on