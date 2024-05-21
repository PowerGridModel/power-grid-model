// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// This header file is automatically generated. DO NOT modify it manually!

// clang-format off
#pragma once

#include "gen_getters.hpp" // NOLINT

#include "../../common/common.hpp"             // NOLINT
#include "../../common/enum.hpp"               // NOLINT
#include "../../common/three_phase_tensor.hpp" // NOLINT
#include "../meta_data.hpp"                    // NOLINT

#include "../input.hpp" // NOLINT


namespace power_grid_model::meta_data {

// template specialization to get list of attributes in the value field

template<>
struct get_attributes_list<BaseInput> {
    static constexpr std::array<MetaAttribute, 1> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<BaseInput, &BaseInput::id, offsetof(BaseInput, id), []{ return "id"; }>::value,
    };
};

template<>
struct get_attributes_list<NodeInput> {
    static constexpr std::array<MetaAttribute, 2> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<NodeInput, &NodeInput::id, offsetof(NodeInput, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<NodeInput, &NodeInput::u_rated, offsetof(NodeInput, u_rated), []{ return "u_rated"; }>::value,
    };
};

template<>
struct get_attributes_list<BranchInput> {
    static constexpr std::array<MetaAttribute, 5> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<BranchInput, &BranchInput::id, offsetof(BranchInput, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<BranchInput, &BranchInput::from_node, offsetof(BranchInput, from_node), []{ return "from_node"; }>::value,
            meta_data_gen::get_meta_attribute<BranchInput, &BranchInput::to_node, offsetof(BranchInput, to_node), []{ return "to_node"; }>::value,
            meta_data_gen::get_meta_attribute<BranchInput, &BranchInput::from_status, offsetof(BranchInput, from_status), []{ return "from_status"; }>::value,
            meta_data_gen::get_meta_attribute<BranchInput, &BranchInput::to_status, offsetof(BranchInput, to_status), []{ return "to_status"; }>::value,
    };
};

template<>
struct get_attributes_list<Branch3Input> {
    static constexpr std::array<MetaAttribute, 7> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<Branch3Input, &Branch3Input::id, offsetof(Branch3Input, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<Branch3Input, &Branch3Input::node_1, offsetof(Branch3Input, node_1), []{ return "node_1"; }>::value,
            meta_data_gen::get_meta_attribute<Branch3Input, &Branch3Input::node_2, offsetof(Branch3Input, node_2), []{ return "node_2"; }>::value,
            meta_data_gen::get_meta_attribute<Branch3Input, &Branch3Input::node_3, offsetof(Branch3Input, node_3), []{ return "node_3"; }>::value,
            meta_data_gen::get_meta_attribute<Branch3Input, &Branch3Input::status_1, offsetof(Branch3Input, status_1), []{ return "status_1"; }>::value,
            meta_data_gen::get_meta_attribute<Branch3Input, &Branch3Input::status_2, offsetof(Branch3Input, status_2), []{ return "status_2"; }>::value,
            meta_data_gen::get_meta_attribute<Branch3Input, &Branch3Input::status_3, offsetof(Branch3Input, status_3), []{ return "status_3"; }>::value,
    };
};

template<>
struct get_attributes_list<SensorInput> {
    static constexpr std::array<MetaAttribute, 2> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<SensorInput, &SensorInput::id, offsetof(SensorInput, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<SensorInput, &SensorInput::measured_object, offsetof(SensorInput, measured_object), []{ return "measured_object"; }>::value,
    };
};

template<>
struct get_attributes_list<ApplianceInput> {
    static constexpr std::array<MetaAttribute, 3> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<ApplianceInput, &ApplianceInput::id, offsetof(ApplianceInput, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<ApplianceInput, &ApplianceInput::node, offsetof(ApplianceInput, node), []{ return "node"; }>::value,
            meta_data_gen::get_meta_attribute<ApplianceInput, &ApplianceInput::status, offsetof(ApplianceInput, status), []{ return "status"; }>::value,
    };
};

template<>
struct get_attributes_list<LineInput> {
    static constexpr std::array<MetaAttribute, 14> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<LineInput, &LineInput::id, offsetof(LineInput, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<LineInput, &LineInput::from_node, offsetof(LineInput, from_node), []{ return "from_node"; }>::value,
            meta_data_gen::get_meta_attribute<LineInput, &LineInput::to_node, offsetof(LineInput, to_node), []{ return "to_node"; }>::value,
            meta_data_gen::get_meta_attribute<LineInput, &LineInput::from_status, offsetof(LineInput, from_status), []{ return "from_status"; }>::value,
            meta_data_gen::get_meta_attribute<LineInput, &LineInput::to_status, offsetof(LineInput, to_status), []{ return "to_status"; }>::value,
            meta_data_gen::get_meta_attribute<LineInput, &LineInput::r1, offsetof(LineInput, r1), []{ return "r1"; }>::value,
            meta_data_gen::get_meta_attribute<LineInput, &LineInput::x1, offsetof(LineInput, x1), []{ return "x1"; }>::value,
            meta_data_gen::get_meta_attribute<LineInput, &LineInput::c1, offsetof(LineInput, c1), []{ return "c1"; }>::value,
            meta_data_gen::get_meta_attribute<LineInput, &LineInput::tan1, offsetof(LineInput, tan1), []{ return "tan1"; }>::value,
            meta_data_gen::get_meta_attribute<LineInput, &LineInput::r0, offsetof(LineInput, r0), []{ return "r0"; }>::value,
            meta_data_gen::get_meta_attribute<LineInput, &LineInput::x0, offsetof(LineInput, x0), []{ return "x0"; }>::value,
            meta_data_gen::get_meta_attribute<LineInput, &LineInput::c0, offsetof(LineInput, c0), []{ return "c0"; }>::value,
            meta_data_gen::get_meta_attribute<LineInput, &LineInput::tan0, offsetof(LineInput, tan0), []{ return "tan0"; }>::value,
            meta_data_gen::get_meta_attribute<LineInput, &LineInput::i_n, offsetof(LineInput, i_n), []{ return "i_n"; }>::value,
    };
};

template<>
struct get_attributes_list<LinkInput> {
    static constexpr std::array<MetaAttribute, 5> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<LinkInput, &LinkInput::id, offsetof(LinkInput, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<LinkInput, &LinkInput::from_node, offsetof(LinkInput, from_node), []{ return "from_node"; }>::value,
            meta_data_gen::get_meta_attribute<LinkInput, &LinkInput::to_node, offsetof(LinkInput, to_node), []{ return "to_node"; }>::value,
            meta_data_gen::get_meta_attribute<LinkInput, &LinkInput::from_status, offsetof(LinkInput, from_status), []{ return "from_status"; }>::value,
            meta_data_gen::get_meta_attribute<LinkInput, &LinkInput::to_status, offsetof(LinkInput, to_status), []{ return "to_status"; }>::value,
    };
};

template<>
struct get_attributes_list<TransformerInput> {
    static constexpr std::array<MetaAttribute, 29> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<TransformerInput, &TransformerInput::id, offsetof(TransformerInput, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerInput, &TransformerInput::from_node, offsetof(TransformerInput, from_node), []{ return "from_node"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerInput, &TransformerInput::to_node, offsetof(TransformerInput, to_node), []{ return "to_node"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerInput, &TransformerInput::from_status, offsetof(TransformerInput, from_status), []{ return "from_status"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerInput, &TransformerInput::to_status, offsetof(TransformerInput, to_status), []{ return "to_status"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerInput, &TransformerInput::u1, offsetof(TransformerInput, u1), []{ return "u1"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerInput, &TransformerInput::u2, offsetof(TransformerInput, u2), []{ return "u2"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerInput, &TransformerInput::sn, offsetof(TransformerInput, sn), []{ return "sn"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerInput, &TransformerInput::uk, offsetof(TransformerInput, uk), []{ return "uk"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerInput, &TransformerInput::pk, offsetof(TransformerInput, pk), []{ return "pk"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerInput, &TransformerInput::i0, offsetof(TransformerInput, i0), []{ return "i0"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerInput, &TransformerInput::p0, offsetof(TransformerInput, p0), []{ return "p0"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerInput, &TransformerInput::winding_from, offsetof(TransformerInput, winding_from), []{ return "winding_from"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerInput, &TransformerInput::winding_to, offsetof(TransformerInput, winding_to), []{ return "winding_to"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerInput, &TransformerInput::clock, offsetof(TransformerInput, clock), []{ return "clock"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerInput, &TransformerInput::tap_side, offsetof(TransformerInput, tap_side), []{ return "tap_side"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerInput, &TransformerInput::tap_pos, offsetof(TransformerInput, tap_pos), []{ return "tap_pos"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerInput, &TransformerInput::tap_min, offsetof(TransformerInput, tap_min), []{ return "tap_min"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerInput, &TransformerInput::tap_max, offsetof(TransformerInput, tap_max), []{ return "tap_max"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerInput, &TransformerInput::tap_nom, offsetof(TransformerInput, tap_nom), []{ return "tap_nom"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerInput, &TransformerInput::tap_size, offsetof(TransformerInput, tap_size), []{ return "tap_size"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerInput, &TransformerInput::uk_min, offsetof(TransformerInput, uk_min), []{ return "uk_min"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerInput, &TransformerInput::uk_max, offsetof(TransformerInput, uk_max), []{ return "uk_max"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerInput, &TransformerInput::pk_min, offsetof(TransformerInput, pk_min), []{ return "pk_min"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerInput, &TransformerInput::pk_max, offsetof(TransformerInput, pk_max), []{ return "pk_max"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerInput, &TransformerInput::r_grounding_from, offsetof(TransformerInput, r_grounding_from), []{ return "r_grounding_from"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerInput, &TransformerInput::x_grounding_from, offsetof(TransformerInput, x_grounding_from), []{ return "x_grounding_from"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerInput, &TransformerInput::r_grounding_to, offsetof(TransformerInput, r_grounding_to), []{ return "r_grounding_to"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerInput, &TransformerInput::x_grounding_to, offsetof(TransformerInput, x_grounding_to), []{ return "x_grounding_to"; }>::value,
    };
};

template<>
struct get_attributes_list<ThreeWindingTransformerInput> {
    static constexpr std::array<MetaAttribute, 50> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::id, offsetof(ThreeWindingTransformerInput, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::node_1, offsetof(ThreeWindingTransformerInput, node_1), []{ return "node_1"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::node_2, offsetof(ThreeWindingTransformerInput, node_2), []{ return "node_2"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::node_3, offsetof(ThreeWindingTransformerInput, node_3), []{ return "node_3"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::status_1, offsetof(ThreeWindingTransformerInput, status_1), []{ return "status_1"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::status_2, offsetof(ThreeWindingTransformerInput, status_2), []{ return "status_2"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::status_3, offsetof(ThreeWindingTransformerInput, status_3), []{ return "status_3"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::u1, offsetof(ThreeWindingTransformerInput, u1), []{ return "u1"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::u2, offsetof(ThreeWindingTransformerInput, u2), []{ return "u2"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::u3, offsetof(ThreeWindingTransformerInput, u3), []{ return "u3"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::sn_1, offsetof(ThreeWindingTransformerInput, sn_1), []{ return "sn_1"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::sn_2, offsetof(ThreeWindingTransformerInput, sn_2), []{ return "sn_2"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::sn_3, offsetof(ThreeWindingTransformerInput, sn_3), []{ return "sn_3"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::uk_12, offsetof(ThreeWindingTransformerInput, uk_12), []{ return "uk_12"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::uk_13, offsetof(ThreeWindingTransformerInput, uk_13), []{ return "uk_13"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::uk_23, offsetof(ThreeWindingTransformerInput, uk_23), []{ return "uk_23"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::pk_12, offsetof(ThreeWindingTransformerInput, pk_12), []{ return "pk_12"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::pk_13, offsetof(ThreeWindingTransformerInput, pk_13), []{ return "pk_13"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::pk_23, offsetof(ThreeWindingTransformerInput, pk_23), []{ return "pk_23"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::i0, offsetof(ThreeWindingTransformerInput, i0), []{ return "i0"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::p0, offsetof(ThreeWindingTransformerInput, p0), []{ return "p0"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::winding_1, offsetof(ThreeWindingTransformerInput, winding_1), []{ return "winding_1"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::winding_2, offsetof(ThreeWindingTransformerInput, winding_2), []{ return "winding_2"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::winding_3, offsetof(ThreeWindingTransformerInput, winding_3), []{ return "winding_3"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::clock_12, offsetof(ThreeWindingTransformerInput, clock_12), []{ return "clock_12"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::clock_13, offsetof(ThreeWindingTransformerInput, clock_13), []{ return "clock_13"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::tap_side, offsetof(ThreeWindingTransformerInput, tap_side), []{ return "tap_side"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::tap_pos, offsetof(ThreeWindingTransformerInput, tap_pos), []{ return "tap_pos"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::tap_min, offsetof(ThreeWindingTransformerInput, tap_min), []{ return "tap_min"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::tap_max, offsetof(ThreeWindingTransformerInput, tap_max), []{ return "tap_max"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::tap_nom, offsetof(ThreeWindingTransformerInput, tap_nom), []{ return "tap_nom"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::tap_size, offsetof(ThreeWindingTransformerInput, tap_size), []{ return "tap_size"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::uk_12_min, offsetof(ThreeWindingTransformerInput, uk_12_min), []{ return "uk_12_min"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::uk_12_max, offsetof(ThreeWindingTransformerInput, uk_12_max), []{ return "uk_12_max"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::uk_13_min, offsetof(ThreeWindingTransformerInput, uk_13_min), []{ return "uk_13_min"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::uk_13_max, offsetof(ThreeWindingTransformerInput, uk_13_max), []{ return "uk_13_max"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::uk_23_min, offsetof(ThreeWindingTransformerInput, uk_23_min), []{ return "uk_23_min"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::uk_23_max, offsetof(ThreeWindingTransformerInput, uk_23_max), []{ return "uk_23_max"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::pk_12_min, offsetof(ThreeWindingTransformerInput, pk_12_min), []{ return "pk_12_min"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::pk_12_max, offsetof(ThreeWindingTransformerInput, pk_12_max), []{ return "pk_12_max"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::pk_13_min, offsetof(ThreeWindingTransformerInput, pk_13_min), []{ return "pk_13_min"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::pk_13_max, offsetof(ThreeWindingTransformerInput, pk_13_max), []{ return "pk_13_max"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::pk_23_min, offsetof(ThreeWindingTransformerInput, pk_23_min), []{ return "pk_23_min"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::pk_23_max, offsetof(ThreeWindingTransformerInput, pk_23_max), []{ return "pk_23_max"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::r_grounding_1, offsetof(ThreeWindingTransformerInput, r_grounding_1), []{ return "r_grounding_1"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::x_grounding_1, offsetof(ThreeWindingTransformerInput, x_grounding_1), []{ return "x_grounding_1"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::r_grounding_2, offsetof(ThreeWindingTransformerInput, r_grounding_2), []{ return "r_grounding_2"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::x_grounding_2, offsetof(ThreeWindingTransformerInput, x_grounding_2), []{ return "x_grounding_2"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::r_grounding_3, offsetof(ThreeWindingTransformerInput, r_grounding_3), []{ return "r_grounding_3"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::x_grounding_3, offsetof(ThreeWindingTransformerInput, x_grounding_3), []{ return "x_grounding_3"; }>::value,
    };
};

template<>
struct get_attributes_list<GenericLoadGenInput> {
    static constexpr std::array<MetaAttribute, 4> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<GenericLoadGenInput, &GenericLoadGenInput::id, offsetof(GenericLoadGenInput, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<GenericLoadGenInput, &GenericLoadGenInput::node, offsetof(GenericLoadGenInput, node), []{ return "node"; }>::value,
            meta_data_gen::get_meta_attribute<GenericLoadGenInput, &GenericLoadGenInput::status, offsetof(GenericLoadGenInput, status), []{ return "status"; }>::value,
            meta_data_gen::get_meta_attribute<GenericLoadGenInput, &GenericLoadGenInput::type, offsetof(GenericLoadGenInput, type), []{ return "type"; }>::value,
    };
};

template <symmetry_tag sym_type>
struct get_attributes_list<LoadGenInput<sym_type>> {
    using sym = sym_type;

    static constexpr std::array<MetaAttribute, 6> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<LoadGenInput<sym>, &LoadGenInput<sym>::id, offsetof(LoadGenInput<sym>, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<LoadGenInput<sym>, &LoadGenInput<sym>::node, offsetof(LoadGenInput<sym>, node), []{ return "node"; }>::value,
            meta_data_gen::get_meta_attribute<LoadGenInput<sym>, &LoadGenInput<sym>::status, offsetof(LoadGenInput<sym>, status), []{ return "status"; }>::value,
            meta_data_gen::get_meta_attribute<LoadGenInput<sym>, &LoadGenInput<sym>::type, offsetof(LoadGenInput<sym>, type), []{ return "type"; }>::value,
            meta_data_gen::get_meta_attribute<LoadGenInput<sym>, &LoadGenInput<sym>::p_specified, offsetof(LoadGenInput<sym>, p_specified), []{ return "p_specified"; }>::value,
            meta_data_gen::get_meta_attribute<LoadGenInput<sym>, &LoadGenInput<sym>::q_specified, offsetof(LoadGenInput<sym>, q_specified), []{ return "q_specified"; }>::value,
    };
};

template<>
struct get_attributes_list<ShuntInput> {
    static constexpr std::array<MetaAttribute, 7> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<ShuntInput, &ShuntInput::id, offsetof(ShuntInput, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<ShuntInput, &ShuntInput::node, offsetof(ShuntInput, node), []{ return "node"; }>::value,
            meta_data_gen::get_meta_attribute<ShuntInput, &ShuntInput::status, offsetof(ShuntInput, status), []{ return "status"; }>::value,
            meta_data_gen::get_meta_attribute<ShuntInput, &ShuntInput::g1, offsetof(ShuntInput, g1), []{ return "g1"; }>::value,
            meta_data_gen::get_meta_attribute<ShuntInput, &ShuntInput::b1, offsetof(ShuntInput, b1), []{ return "b1"; }>::value,
            meta_data_gen::get_meta_attribute<ShuntInput, &ShuntInput::g0, offsetof(ShuntInput, g0), []{ return "g0"; }>::value,
            meta_data_gen::get_meta_attribute<ShuntInput, &ShuntInput::b0, offsetof(ShuntInput, b0), []{ return "b0"; }>::value,
    };
};

template<>
struct get_attributes_list<SourceInput> {
    static constexpr std::array<MetaAttribute, 8> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<SourceInput, &SourceInput::id, offsetof(SourceInput, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<SourceInput, &SourceInput::node, offsetof(SourceInput, node), []{ return "node"; }>::value,
            meta_data_gen::get_meta_attribute<SourceInput, &SourceInput::status, offsetof(SourceInput, status), []{ return "status"; }>::value,
            meta_data_gen::get_meta_attribute<SourceInput, &SourceInput::u_ref, offsetof(SourceInput, u_ref), []{ return "u_ref"; }>::value,
            meta_data_gen::get_meta_attribute<SourceInput, &SourceInput::u_ref_angle, offsetof(SourceInput, u_ref_angle), []{ return "u_ref_angle"; }>::value,
            meta_data_gen::get_meta_attribute<SourceInput, &SourceInput::sk, offsetof(SourceInput, sk), []{ return "sk"; }>::value,
            meta_data_gen::get_meta_attribute<SourceInput, &SourceInput::rx_ratio, offsetof(SourceInput, rx_ratio), []{ return "rx_ratio"; }>::value,
            meta_data_gen::get_meta_attribute<SourceInput, &SourceInput::z01_ratio, offsetof(SourceInput, z01_ratio), []{ return "z01_ratio"; }>::value,
    };
};

template<>
struct get_attributes_list<GenericVoltageSensorInput> {
    static constexpr std::array<MetaAttribute, 3> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<GenericVoltageSensorInput, &GenericVoltageSensorInput::id, offsetof(GenericVoltageSensorInput, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<GenericVoltageSensorInput, &GenericVoltageSensorInput::measured_object, offsetof(GenericVoltageSensorInput, measured_object), []{ return "measured_object"; }>::value,
            meta_data_gen::get_meta_attribute<GenericVoltageSensorInput, &GenericVoltageSensorInput::u_sigma, offsetof(GenericVoltageSensorInput, u_sigma), []{ return "u_sigma"; }>::value,
    };
};

template <symmetry_tag sym_type>
struct get_attributes_list<VoltageSensorInput<sym_type>> {
    using sym = sym_type;

    static constexpr std::array<MetaAttribute, 5> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<VoltageSensorInput<sym>, &VoltageSensorInput<sym>::id, offsetof(VoltageSensorInput<sym>, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<VoltageSensorInput<sym>, &VoltageSensorInput<sym>::measured_object, offsetof(VoltageSensorInput<sym>, measured_object), []{ return "measured_object"; }>::value,
            meta_data_gen::get_meta_attribute<VoltageSensorInput<sym>, &VoltageSensorInput<sym>::u_sigma, offsetof(VoltageSensorInput<sym>, u_sigma), []{ return "u_sigma"; }>::value,
            meta_data_gen::get_meta_attribute<VoltageSensorInput<sym>, &VoltageSensorInput<sym>::u_measured, offsetof(VoltageSensorInput<sym>, u_measured), []{ return "u_measured"; }>::value,
            meta_data_gen::get_meta_attribute<VoltageSensorInput<sym>, &VoltageSensorInput<sym>::u_angle_measured, offsetof(VoltageSensorInput<sym>, u_angle_measured), []{ return "u_angle_measured"; }>::value,
    };
};

template<>
struct get_attributes_list<GenericPowerSensorInput> {
    static constexpr std::array<MetaAttribute, 4> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<GenericPowerSensorInput, &GenericPowerSensorInput::id, offsetof(GenericPowerSensorInput, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<GenericPowerSensorInput, &GenericPowerSensorInput::measured_object, offsetof(GenericPowerSensorInput, measured_object), []{ return "measured_object"; }>::value,
            meta_data_gen::get_meta_attribute<GenericPowerSensorInput, &GenericPowerSensorInput::measured_terminal_type, offsetof(GenericPowerSensorInput, measured_terminal_type), []{ return "measured_terminal_type"; }>::value,
            meta_data_gen::get_meta_attribute<GenericPowerSensorInput, &GenericPowerSensorInput::power_sigma, offsetof(GenericPowerSensorInput, power_sigma), []{ return "power_sigma"; }>::value,
    };
};

template <symmetry_tag sym_type>
struct get_attributes_list<PowerSensorInput<sym_type>> {
    using sym = sym_type;

    static constexpr std::array<MetaAttribute, 8> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<PowerSensorInput<sym>, &PowerSensorInput<sym>::id, offsetof(PowerSensorInput<sym>, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<PowerSensorInput<sym>, &PowerSensorInput<sym>::measured_object, offsetof(PowerSensorInput<sym>, measured_object), []{ return "measured_object"; }>::value,
            meta_data_gen::get_meta_attribute<PowerSensorInput<sym>, &PowerSensorInput<sym>::measured_terminal_type, offsetof(PowerSensorInput<sym>, measured_terminal_type), []{ return "measured_terminal_type"; }>::value,
            meta_data_gen::get_meta_attribute<PowerSensorInput<sym>, &PowerSensorInput<sym>::power_sigma, offsetof(PowerSensorInput<sym>, power_sigma), []{ return "power_sigma"; }>::value,
            meta_data_gen::get_meta_attribute<PowerSensorInput<sym>, &PowerSensorInput<sym>::p_measured, offsetof(PowerSensorInput<sym>, p_measured), []{ return "p_measured"; }>::value,
            meta_data_gen::get_meta_attribute<PowerSensorInput<sym>, &PowerSensorInput<sym>::q_measured, offsetof(PowerSensorInput<sym>, q_measured), []{ return "q_measured"; }>::value,
            meta_data_gen::get_meta_attribute<PowerSensorInput<sym>, &PowerSensorInput<sym>::p_sigma, offsetof(PowerSensorInput<sym>, p_sigma), []{ return "p_sigma"; }>::value,
            meta_data_gen::get_meta_attribute<PowerSensorInput<sym>, &PowerSensorInput<sym>::q_sigma, offsetof(PowerSensorInput<sym>, q_sigma), []{ return "q_sigma"; }>::value,
    };
};

template<>
struct get_attributes_list<FaultInput> {
    static constexpr std::array<MetaAttribute, 7> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<FaultInput, &FaultInput::id, offsetof(FaultInput, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<FaultInput, &FaultInput::status, offsetof(FaultInput, status), []{ return "status"; }>::value,
            meta_data_gen::get_meta_attribute<FaultInput, &FaultInput::fault_type, offsetof(FaultInput, fault_type), []{ return "fault_type"; }>::value,
            meta_data_gen::get_meta_attribute<FaultInput, &FaultInput::fault_phase, offsetof(FaultInput, fault_phase), []{ return "fault_phase"; }>::value,
            meta_data_gen::get_meta_attribute<FaultInput, &FaultInput::fault_object, offsetof(FaultInput, fault_object), []{ return "fault_object"; }>::value,
            meta_data_gen::get_meta_attribute<FaultInput, &FaultInput::r_f, offsetof(FaultInput, r_f), []{ return "r_f"; }>::value,
            meta_data_gen::get_meta_attribute<FaultInput, &FaultInput::x_f, offsetof(FaultInput, x_f), []{ return "x_f"; }>::value,
    };
};

template<>
struct get_attributes_list<RegulatorInput> {
    static constexpr std::array<MetaAttribute, 3> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<RegulatorInput, &RegulatorInput::id, offsetof(RegulatorInput, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<RegulatorInput, &RegulatorInput::regulated_object, offsetof(RegulatorInput, regulated_object), []{ return "regulated_object"; }>::value,
            meta_data_gen::get_meta_attribute<RegulatorInput, &RegulatorInput::status, offsetof(RegulatorInput, status), []{ return "status"; }>::value,
    };
};

template<>
struct get_attributes_list<TransformerTapRegulatorInput> {
    static constexpr std::array<MetaAttribute, 8> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<TransformerTapRegulatorInput, &TransformerTapRegulatorInput::id, offsetof(TransformerTapRegulatorInput, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerTapRegulatorInput, &TransformerTapRegulatorInput::regulated_object, offsetof(TransformerTapRegulatorInput, regulated_object), []{ return "regulated_object"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerTapRegulatorInput, &TransformerTapRegulatorInput::status, offsetof(TransformerTapRegulatorInput, status), []{ return "status"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerTapRegulatorInput, &TransformerTapRegulatorInput::control_side, offsetof(TransformerTapRegulatorInput, control_side), []{ return "control_side"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerTapRegulatorInput, &TransformerTapRegulatorInput::u_set, offsetof(TransformerTapRegulatorInput, u_set), []{ return "u_set"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerTapRegulatorInput, &TransformerTapRegulatorInput::u_band, offsetof(TransformerTapRegulatorInput, u_band), []{ return "u_band"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerTapRegulatorInput, &TransformerTapRegulatorInput::line_drop_compensation_r, offsetof(TransformerTapRegulatorInput, line_drop_compensation_r), []{ return "line_drop_compensation_r"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerTapRegulatorInput, &TransformerTapRegulatorInput::line_drop_compensation_x, offsetof(TransformerTapRegulatorInput, line_drop_compensation_x), []{ return "line_drop_compensation_x"; }>::value,
    };
};




} // namespace power_grid_model::meta_data

// clang-format on