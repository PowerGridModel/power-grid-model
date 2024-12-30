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
            
            meta_data_gen::get_meta_attribute<&BaseInput::id>(offsetof(BaseInput, id), "id"),
    };
};

template<>
struct get_attributes_list<NodeInput> {
    static constexpr std::array<MetaAttribute, 2> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&NodeInput::id>(offsetof(NodeInput, id), "id"),
            meta_data_gen::get_meta_attribute<&NodeInput::u_rated>(offsetof(NodeInput, u_rated), "u_rated"),
    };
};

template<>
struct get_attributes_list<BranchInput> {
    static constexpr std::array<MetaAttribute, 5> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&BranchInput::id>(offsetof(BranchInput, id), "id"),
            meta_data_gen::get_meta_attribute<&BranchInput::from_node>(offsetof(BranchInput, from_node), "from_node"),
            meta_data_gen::get_meta_attribute<&BranchInput::to_node>(offsetof(BranchInput, to_node), "to_node"),
            meta_data_gen::get_meta_attribute<&BranchInput::from_status>(offsetof(BranchInput, from_status), "from_status"),
            meta_data_gen::get_meta_attribute<&BranchInput::to_status>(offsetof(BranchInput, to_status), "to_status"),
    };
};

template<>
struct get_attributes_list<Branch3Input> {
    static constexpr std::array<MetaAttribute, 7> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&Branch3Input::id>(offsetof(Branch3Input, id), "id"),
            meta_data_gen::get_meta_attribute<&Branch3Input::node_1>(offsetof(Branch3Input, node_1), "node_1"),
            meta_data_gen::get_meta_attribute<&Branch3Input::node_2>(offsetof(Branch3Input, node_2), "node_2"),
            meta_data_gen::get_meta_attribute<&Branch3Input::node_3>(offsetof(Branch3Input, node_3), "node_3"),
            meta_data_gen::get_meta_attribute<&Branch3Input::status_1>(offsetof(Branch3Input, status_1), "status_1"),
            meta_data_gen::get_meta_attribute<&Branch3Input::status_2>(offsetof(Branch3Input, status_2), "status_2"),
            meta_data_gen::get_meta_attribute<&Branch3Input::status_3>(offsetof(Branch3Input, status_3), "status_3"),
    };
};

template<>
struct get_attributes_list<SensorInput> {
    static constexpr std::array<MetaAttribute, 2> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&SensorInput::id>(offsetof(SensorInput, id), "id"),
            meta_data_gen::get_meta_attribute<&SensorInput::measured_object>(offsetof(SensorInput, measured_object), "measured_object"),
    };
};

template<>
struct get_attributes_list<ApplianceInput> {
    static constexpr std::array<MetaAttribute, 3> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&ApplianceInput::id>(offsetof(ApplianceInput, id), "id"),
            meta_data_gen::get_meta_attribute<&ApplianceInput::node>(offsetof(ApplianceInput, node), "node"),
            meta_data_gen::get_meta_attribute<&ApplianceInput::status>(offsetof(ApplianceInput, status), "status"),
    };
};

template<>
struct get_attributes_list<LineInput> {
    static constexpr std::array<MetaAttribute, 14> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&LineInput::id>(offsetof(LineInput, id), "id"),
            meta_data_gen::get_meta_attribute<&LineInput::from_node>(offsetof(LineInput, from_node), "from_node"),
            meta_data_gen::get_meta_attribute<&LineInput::to_node>(offsetof(LineInput, to_node), "to_node"),
            meta_data_gen::get_meta_attribute<&LineInput::from_status>(offsetof(LineInput, from_status), "from_status"),
            meta_data_gen::get_meta_attribute<&LineInput::to_status>(offsetof(LineInput, to_status), "to_status"),
            meta_data_gen::get_meta_attribute<&LineInput::r1>(offsetof(LineInput, r1), "r1"),
            meta_data_gen::get_meta_attribute<&LineInput::x1>(offsetof(LineInput, x1), "x1"),
            meta_data_gen::get_meta_attribute<&LineInput::c1>(offsetof(LineInput, c1), "c1"),
            meta_data_gen::get_meta_attribute<&LineInput::tan1>(offsetof(LineInput, tan1), "tan1"),
            meta_data_gen::get_meta_attribute<&LineInput::r0>(offsetof(LineInput, r0), "r0"),
            meta_data_gen::get_meta_attribute<&LineInput::x0>(offsetof(LineInput, x0), "x0"),
            meta_data_gen::get_meta_attribute<&LineInput::c0>(offsetof(LineInput, c0), "c0"),
            meta_data_gen::get_meta_attribute<&LineInput::tan0>(offsetof(LineInput, tan0), "tan0"),
            meta_data_gen::get_meta_attribute<&LineInput::i_n>(offsetof(LineInput, i_n), "i_n"),
    };
};

template<>
struct get_attributes_list<GenericBranchInput> {
    static constexpr std::array<MetaAttribute, 12> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&GenericBranchInput::id>(offsetof(GenericBranchInput, id), "id"),
            meta_data_gen::get_meta_attribute<&GenericBranchInput::from_node>(offsetof(GenericBranchInput, from_node), "from_node"),
            meta_data_gen::get_meta_attribute<&GenericBranchInput::to_node>(offsetof(GenericBranchInput, to_node), "to_node"),
            meta_data_gen::get_meta_attribute<&GenericBranchInput::from_status>(offsetof(GenericBranchInput, from_status), "from_status"),
            meta_data_gen::get_meta_attribute<&GenericBranchInput::to_status>(offsetof(GenericBranchInput, to_status), "to_status"),
            meta_data_gen::get_meta_attribute<&GenericBranchInput::r1>(offsetof(GenericBranchInput, r1), "r1"),
            meta_data_gen::get_meta_attribute<&GenericBranchInput::x1>(offsetof(GenericBranchInput, x1), "x1"),
            meta_data_gen::get_meta_attribute<&GenericBranchInput::g1>(offsetof(GenericBranchInput, g1), "g1"),
            meta_data_gen::get_meta_attribute<&GenericBranchInput::b1>(offsetof(GenericBranchInput, b1), "b1"),
            meta_data_gen::get_meta_attribute<&GenericBranchInput::k>(offsetof(GenericBranchInput, k), "k"),
            meta_data_gen::get_meta_attribute<&GenericBranchInput::theta>(offsetof(GenericBranchInput, theta), "theta"),
            meta_data_gen::get_meta_attribute<&GenericBranchInput::sn>(offsetof(GenericBranchInput, sn), "sn"),
    };
};

template<>
struct get_attributes_list<LinkInput> {
    static constexpr std::array<MetaAttribute, 5> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&LinkInput::id>(offsetof(LinkInput, id), "id"),
            meta_data_gen::get_meta_attribute<&LinkInput::from_node>(offsetof(LinkInput, from_node), "from_node"),
            meta_data_gen::get_meta_attribute<&LinkInput::to_node>(offsetof(LinkInput, to_node), "to_node"),
            meta_data_gen::get_meta_attribute<&LinkInput::from_status>(offsetof(LinkInput, from_status), "from_status"),
            meta_data_gen::get_meta_attribute<&LinkInput::to_status>(offsetof(LinkInput, to_status), "to_status"),
    };
};

template<>
struct get_attributes_list<TransformerInput> {
    static constexpr std::array<MetaAttribute, 29> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&TransformerInput::id>(offsetof(TransformerInput, id), "id"),
            meta_data_gen::get_meta_attribute<&TransformerInput::from_node>(offsetof(TransformerInput, from_node), "from_node"),
            meta_data_gen::get_meta_attribute<&TransformerInput::to_node>(offsetof(TransformerInput, to_node), "to_node"),
            meta_data_gen::get_meta_attribute<&TransformerInput::from_status>(offsetof(TransformerInput, from_status), "from_status"),
            meta_data_gen::get_meta_attribute<&TransformerInput::to_status>(offsetof(TransformerInput, to_status), "to_status"),
            meta_data_gen::get_meta_attribute<&TransformerInput::u1>(offsetof(TransformerInput, u1), "u1"),
            meta_data_gen::get_meta_attribute<&TransformerInput::u2>(offsetof(TransformerInput, u2), "u2"),
            meta_data_gen::get_meta_attribute<&TransformerInput::sn>(offsetof(TransformerInput, sn), "sn"),
            meta_data_gen::get_meta_attribute<&TransformerInput::uk>(offsetof(TransformerInput, uk), "uk"),
            meta_data_gen::get_meta_attribute<&TransformerInput::pk>(offsetof(TransformerInput, pk), "pk"),
            meta_data_gen::get_meta_attribute<&TransformerInput::i0>(offsetof(TransformerInput, i0), "i0"),
            meta_data_gen::get_meta_attribute<&TransformerInput::p0>(offsetof(TransformerInput, p0), "p0"),
            meta_data_gen::get_meta_attribute<&TransformerInput::winding_from>(offsetof(TransformerInput, winding_from), "winding_from"),
            meta_data_gen::get_meta_attribute<&TransformerInput::winding_to>(offsetof(TransformerInput, winding_to), "winding_to"),
            meta_data_gen::get_meta_attribute<&TransformerInput::clock>(offsetof(TransformerInput, clock), "clock"),
            meta_data_gen::get_meta_attribute<&TransformerInput::tap_side>(offsetof(TransformerInput, tap_side), "tap_side"),
            meta_data_gen::get_meta_attribute<&TransformerInput::tap_pos>(offsetof(TransformerInput, tap_pos), "tap_pos"),
            meta_data_gen::get_meta_attribute<&TransformerInput::tap_min>(offsetof(TransformerInput, tap_min), "tap_min"),
            meta_data_gen::get_meta_attribute<&TransformerInput::tap_max>(offsetof(TransformerInput, tap_max), "tap_max"),
            meta_data_gen::get_meta_attribute<&TransformerInput::tap_nom>(offsetof(TransformerInput, tap_nom), "tap_nom"),
            meta_data_gen::get_meta_attribute<&TransformerInput::tap_size>(offsetof(TransformerInput, tap_size), "tap_size"),
            meta_data_gen::get_meta_attribute<&TransformerInput::uk_min>(offsetof(TransformerInput, uk_min), "uk_min"),
            meta_data_gen::get_meta_attribute<&TransformerInput::uk_max>(offsetof(TransformerInput, uk_max), "uk_max"),
            meta_data_gen::get_meta_attribute<&TransformerInput::pk_min>(offsetof(TransformerInput, pk_min), "pk_min"),
            meta_data_gen::get_meta_attribute<&TransformerInput::pk_max>(offsetof(TransformerInput, pk_max), "pk_max"),
            meta_data_gen::get_meta_attribute<&TransformerInput::r_grounding_from>(offsetof(TransformerInput, r_grounding_from), "r_grounding_from"),
            meta_data_gen::get_meta_attribute<&TransformerInput::x_grounding_from>(offsetof(TransformerInput, x_grounding_from), "x_grounding_from"),
            meta_data_gen::get_meta_attribute<&TransformerInput::r_grounding_to>(offsetof(TransformerInput, r_grounding_to), "r_grounding_to"),
            meta_data_gen::get_meta_attribute<&TransformerInput::x_grounding_to>(offsetof(TransformerInput, x_grounding_to), "x_grounding_to"),
    };
};

template<>
struct get_attributes_list<ThreeWindingTransformerInput> {
    static constexpr std::array<MetaAttribute, 50> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::id>(offsetof(ThreeWindingTransformerInput, id), "id"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::node_1>(offsetof(ThreeWindingTransformerInput, node_1), "node_1"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::node_2>(offsetof(ThreeWindingTransformerInput, node_2), "node_2"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::node_3>(offsetof(ThreeWindingTransformerInput, node_3), "node_3"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::status_1>(offsetof(ThreeWindingTransformerInput, status_1), "status_1"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::status_2>(offsetof(ThreeWindingTransformerInput, status_2), "status_2"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::status_3>(offsetof(ThreeWindingTransformerInput, status_3), "status_3"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::u1>(offsetof(ThreeWindingTransformerInput, u1), "u1"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::u2>(offsetof(ThreeWindingTransformerInput, u2), "u2"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::u3>(offsetof(ThreeWindingTransformerInput, u3), "u3"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::sn_1>(offsetof(ThreeWindingTransformerInput, sn_1), "sn_1"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::sn_2>(offsetof(ThreeWindingTransformerInput, sn_2), "sn_2"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::sn_3>(offsetof(ThreeWindingTransformerInput, sn_3), "sn_3"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::uk_12>(offsetof(ThreeWindingTransformerInput, uk_12), "uk_12"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::uk_13>(offsetof(ThreeWindingTransformerInput, uk_13), "uk_13"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::uk_23>(offsetof(ThreeWindingTransformerInput, uk_23), "uk_23"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::pk_12>(offsetof(ThreeWindingTransformerInput, pk_12), "pk_12"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::pk_13>(offsetof(ThreeWindingTransformerInput, pk_13), "pk_13"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::pk_23>(offsetof(ThreeWindingTransformerInput, pk_23), "pk_23"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::i0>(offsetof(ThreeWindingTransformerInput, i0), "i0"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::p0>(offsetof(ThreeWindingTransformerInput, p0), "p0"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::winding_1>(offsetof(ThreeWindingTransformerInput, winding_1), "winding_1"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::winding_2>(offsetof(ThreeWindingTransformerInput, winding_2), "winding_2"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::winding_3>(offsetof(ThreeWindingTransformerInput, winding_3), "winding_3"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::clock_12>(offsetof(ThreeWindingTransformerInput, clock_12), "clock_12"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::clock_13>(offsetof(ThreeWindingTransformerInput, clock_13), "clock_13"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::tap_side>(offsetof(ThreeWindingTransformerInput, tap_side), "tap_side"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::tap_pos>(offsetof(ThreeWindingTransformerInput, tap_pos), "tap_pos"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::tap_min>(offsetof(ThreeWindingTransformerInput, tap_min), "tap_min"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::tap_max>(offsetof(ThreeWindingTransformerInput, tap_max), "tap_max"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::tap_nom>(offsetof(ThreeWindingTransformerInput, tap_nom), "tap_nom"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::tap_size>(offsetof(ThreeWindingTransformerInput, tap_size), "tap_size"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::uk_12_min>(offsetof(ThreeWindingTransformerInput, uk_12_min), "uk_12_min"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::uk_12_max>(offsetof(ThreeWindingTransformerInput, uk_12_max), "uk_12_max"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::uk_13_min>(offsetof(ThreeWindingTransformerInput, uk_13_min), "uk_13_min"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::uk_13_max>(offsetof(ThreeWindingTransformerInput, uk_13_max), "uk_13_max"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::uk_23_min>(offsetof(ThreeWindingTransformerInput, uk_23_min), "uk_23_min"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::uk_23_max>(offsetof(ThreeWindingTransformerInput, uk_23_max), "uk_23_max"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::pk_12_min>(offsetof(ThreeWindingTransformerInput, pk_12_min), "pk_12_min"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::pk_12_max>(offsetof(ThreeWindingTransformerInput, pk_12_max), "pk_12_max"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::pk_13_min>(offsetof(ThreeWindingTransformerInput, pk_13_min), "pk_13_min"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::pk_13_max>(offsetof(ThreeWindingTransformerInput, pk_13_max), "pk_13_max"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::pk_23_min>(offsetof(ThreeWindingTransformerInput, pk_23_min), "pk_23_min"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::pk_23_max>(offsetof(ThreeWindingTransformerInput, pk_23_max), "pk_23_max"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::r_grounding_1>(offsetof(ThreeWindingTransformerInput, r_grounding_1), "r_grounding_1"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::x_grounding_1>(offsetof(ThreeWindingTransformerInput, x_grounding_1), "x_grounding_1"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::r_grounding_2>(offsetof(ThreeWindingTransformerInput, r_grounding_2), "r_grounding_2"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::x_grounding_2>(offsetof(ThreeWindingTransformerInput, x_grounding_2), "x_grounding_2"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::r_grounding_3>(offsetof(ThreeWindingTransformerInput, r_grounding_3), "r_grounding_3"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerInput::x_grounding_3>(offsetof(ThreeWindingTransformerInput, x_grounding_3), "x_grounding_3"),
    };
};

template<>
struct get_attributes_list<GenericLoadGenInput> {
    static constexpr std::array<MetaAttribute, 4> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&GenericLoadGenInput::id>(offsetof(GenericLoadGenInput, id), "id"),
            meta_data_gen::get_meta_attribute<&GenericLoadGenInput::node>(offsetof(GenericLoadGenInput, node), "node"),
            meta_data_gen::get_meta_attribute<&GenericLoadGenInput::status>(offsetof(GenericLoadGenInput, status), "status"),
            meta_data_gen::get_meta_attribute<&GenericLoadGenInput::type>(offsetof(GenericLoadGenInput, type), "type"),
    };
};

template <symmetry_tag sym_type>
struct get_attributes_list<LoadGenInput<sym_type>> {
    using sym = sym_type;

    static constexpr std::array<MetaAttribute, 6> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&LoadGenInput<sym>::id>(offsetof(LoadGenInput<sym>, id), "id"),
            meta_data_gen::get_meta_attribute<&LoadGenInput<sym>::node>(offsetof(LoadGenInput<sym>, node), "node"),
            meta_data_gen::get_meta_attribute<&LoadGenInput<sym>::status>(offsetof(LoadGenInput<sym>, status), "status"),
            meta_data_gen::get_meta_attribute<&LoadGenInput<sym>::type>(offsetof(LoadGenInput<sym>, type), "type"),
            meta_data_gen::get_meta_attribute<&LoadGenInput<sym>::p_specified>(offsetof(LoadGenInput<sym>, p_specified), "p_specified"),
            meta_data_gen::get_meta_attribute<&LoadGenInput<sym>::q_specified>(offsetof(LoadGenInput<sym>, q_specified), "q_specified"),
    };
};

template<>
struct get_attributes_list<ShuntInput> {
    static constexpr std::array<MetaAttribute, 7> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&ShuntInput::id>(offsetof(ShuntInput, id), "id"),
            meta_data_gen::get_meta_attribute<&ShuntInput::node>(offsetof(ShuntInput, node), "node"),
            meta_data_gen::get_meta_attribute<&ShuntInput::status>(offsetof(ShuntInput, status), "status"),
            meta_data_gen::get_meta_attribute<&ShuntInput::g1>(offsetof(ShuntInput, g1), "g1"),
            meta_data_gen::get_meta_attribute<&ShuntInput::b1>(offsetof(ShuntInput, b1), "b1"),
            meta_data_gen::get_meta_attribute<&ShuntInput::g0>(offsetof(ShuntInput, g0), "g0"),
            meta_data_gen::get_meta_attribute<&ShuntInput::b0>(offsetof(ShuntInput, b0), "b0"),
    };
};

template<>
struct get_attributes_list<SourceInput> {
    static constexpr std::array<MetaAttribute, 8> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&SourceInput::id>(offsetof(SourceInput, id), "id"),
            meta_data_gen::get_meta_attribute<&SourceInput::node>(offsetof(SourceInput, node), "node"),
            meta_data_gen::get_meta_attribute<&SourceInput::status>(offsetof(SourceInput, status), "status"),
            meta_data_gen::get_meta_attribute<&SourceInput::u_ref>(offsetof(SourceInput, u_ref), "u_ref"),
            meta_data_gen::get_meta_attribute<&SourceInput::u_ref_angle>(offsetof(SourceInput, u_ref_angle), "u_ref_angle"),
            meta_data_gen::get_meta_attribute<&SourceInput::sk>(offsetof(SourceInput, sk), "sk"),
            meta_data_gen::get_meta_attribute<&SourceInput::rx_ratio>(offsetof(SourceInput, rx_ratio), "rx_ratio"),
            meta_data_gen::get_meta_attribute<&SourceInput::z01_ratio>(offsetof(SourceInput, z01_ratio), "z01_ratio"),
    };
};

template<>
struct get_attributes_list<GenericVoltageSensorInput> {
    static constexpr std::array<MetaAttribute, 3> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&GenericVoltageSensorInput::id>(offsetof(GenericVoltageSensorInput, id), "id"),
            meta_data_gen::get_meta_attribute<&GenericVoltageSensorInput::measured_object>(offsetof(GenericVoltageSensorInput, measured_object), "measured_object"),
            meta_data_gen::get_meta_attribute<&GenericVoltageSensorInput::u_sigma>(offsetof(GenericVoltageSensorInput, u_sigma), "u_sigma"),
    };
};

template <symmetry_tag sym_type>
struct get_attributes_list<VoltageSensorInput<sym_type>> {
    using sym = sym_type;

    static constexpr std::array<MetaAttribute, 5> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&VoltageSensorInput<sym>::id>(offsetof(VoltageSensorInput<sym>, id), "id"),
            meta_data_gen::get_meta_attribute<&VoltageSensorInput<sym>::measured_object>(offsetof(VoltageSensorInput<sym>, measured_object), "measured_object"),
            meta_data_gen::get_meta_attribute<&VoltageSensorInput<sym>::u_sigma>(offsetof(VoltageSensorInput<sym>, u_sigma), "u_sigma"),
            meta_data_gen::get_meta_attribute<&VoltageSensorInput<sym>::u_measured>(offsetof(VoltageSensorInput<sym>, u_measured), "u_measured"),
            meta_data_gen::get_meta_attribute<&VoltageSensorInput<sym>::u_angle_measured>(offsetof(VoltageSensorInput<sym>, u_angle_measured), "u_angle_measured"),
    };
};

template<>
struct get_attributes_list<GenericPowerSensorInput> {
    static constexpr std::array<MetaAttribute, 4> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&GenericPowerSensorInput::id>(offsetof(GenericPowerSensorInput, id), "id"),
            meta_data_gen::get_meta_attribute<&GenericPowerSensorInput::measured_object>(offsetof(GenericPowerSensorInput, measured_object), "measured_object"),
            meta_data_gen::get_meta_attribute<&GenericPowerSensorInput::measured_terminal_type>(offsetof(GenericPowerSensorInput, measured_terminal_type), "measured_terminal_type"),
            meta_data_gen::get_meta_attribute<&GenericPowerSensorInput::power_sigma>(offsetof(GenericPowerSensorInput, power_sigma), "power_sigma"),
    };
};

template <symmetry_tag sym_type>
struct get_attributes_list<PowerSensorInput<sym_type>> {
    using sym = sym_type;

    static constexpr std::array<MetaAttribute, 8> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&PowerSensorInput<sym>::id>(offsetof(PowerSensorInput<sym>, id), "id"),
            meta_data_gen::get_meta_attribute<&PowerSensorInput<sym>::measured_object>(offsetof(PowerSensorInput<sym>, measured_object), "measured_object"),
            meta_data_gen::get_meta_attribute<&PowerSensorInput<sym>::measured_terminal_type>(offsetof(PowerSensorInput<sym>, measured_terminal_type), "measured_terminal_type"),
            meta_data_gen::get_meta_attribute<&PowerSensorInput<sym>::power_sigma>(offsetof(PowerSensorInput<sym>, power_sigma), "power_sigma"),
            meta_data_gen::get_meta_attribute<&PowerSensorInput<sym>::p_measured>(offsetof(PowerSensorInput<sym>, p_measured), "p_measured"),
            meta_data_gen::get_meta_attribute<&PowerSensorInput<sym>::q_measured>(offsetof(PowerSensorInput<sym>, q_measured), "q_measured"),
            meta_data_gen::get_meta_attribute<&PowerSensorInput<sym>::p_sigma>(offsetof(PowerSensorInput<sym>, p_sigma), "p_sigma"),
            meta_data_gen::get_meta_attribute<&PowerSensorInput<sym>::q_sigma>(offsetof(PowerSensorInput<sym>, q_sigma), "q_sigma"),
    };
};

template<>
struct get_attributes_list<FaultInput> {
    static constexpr std::array<MetaAttribute, 7> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&FaultInput::id>(offsetof(FaultInput, id), "id"),
            meta_data_gen::get_meta_attribute<&FaultInput::status>(offsetof(FaultInput, status), "status"),
            meta_data_gen::get_meta_attribute<&FaultInput::fault_type>(offsetof(FaultInput, fault_type), "fault_type"),
            meta_data_gen::get_meta_attribute<&FaultInput::fault_phase>(offsetof(FaultInput, fault_phase), "fault_phase"),
            meta_data_gen::get_meta_attribute<&FaultInput::fault_object>(offsetof(FaultInput, fault_object), "fault_object"),
            meta_data_gen::get_meta_attribute<&FaultInput::r_f>(offsetof(FaultInput, r_f), "r_f"),
            meta_data_gen::get_meta_attribute<&FaultInput::x_f>(offsetof(FaultInput, x_f), "x_f"),
    };
};

template<>
struct get_attributes_list<RegulatorInput> {
    static constexpr std::array<MetaAttribute, 3> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&RegulatorInput::id>(offsetof(RegulatorInput, id), "id"),
            meta_data_gen::get_meta_attribute<&RegulatorInput::regulated_object>(offsetof(RegulatorInput, regulated_object), "regulated_object"),
            meta_data_gen::get_meta_attribute<&RegulatorInput::status>(offsetof(RegulatorInput, status), "status"),
    };
};

template<>
struct get_attributes_list<TransformerTapRegulatorInput> {
    static constexpr std::array<MetaAttribute, 8> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&TransformerTapRegulatorInput::id>(offsetof(TransformerTapRegulatorInput, id), "id"),
            meta_data_gen::get_meta_attribute<&TransformerTapRegulatorInput::regulated_object>(offsetof(TransformerTapRegulatorInput, regulated_object), "regulated_object"),
            meta_data_gen::get_meta_attribute<&TransformerTapRegulatorInput::status>(offsetof(TransformerTapRegulatorInput, status), "status"),
            meta_data_gen::get_meta_attribute<&TransformerTapRegulatorInput::control_side>(offsetof(TransformerTapRegulatorInput, control_side), "control_side"),
            meta_data_gen::get_meta_attribute<&TransformerTapRegulatorInput::u_set>(offsetof(TransformerTapRegulatorInput, u_set), "u_set"),
            meta_data_gen::get_meta_attribute<&TransformerTapRegulatorInput::u_band>(offsetof(TransformerTapRegulatorInput, u_band), "u_band"),
            meta_data_gen::get_meta_attribute<&TransformerTapRegulatorInput::line_drop_compensation_r>(offsetof(TransformerTapRegulatorInput, line_drop_compensation_r), "line_drop_compensation_r"),
            meta_data_gen::get_meta_attribute<&TransformerTapRegulatorInput::line_drop_compensation_x>(offsetof(TransformerTapRegulatorInput, line_drop_compensation_x), "line_drop_compensation_x"),
    };
};

template<>
struct get_attributes_list<GenericCurrentSensorInput> {
    static constexpr std::array<MetaAttribute, 6> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&GenericCurrentSensorInput::id>(offsetof(GenericCurrentSensorInput, id), "id"),
            meta_data_gen::get_meta_attribute<&GenericCurrentSensorInput::measured_object>(offsetof(GenericCurrentSensorInput, measured_object), "measured_object"),
            meta_data_gen::get_meta_attribute<&GenericCurrentSensorInput::measured_terminal_type>(offsetof(GenericCurrentSensorInput, measured_terminal_type), "measured_terminal_type"),
            meta_data_gen::get_meta_attribute<&GenericCurrentSensorInput::angle_measurement_type>(offsetof(GenericCurrentSensorInput, angle_measurement_type), "angle_measurement_type"),
            meta_data_gen::get_meta_attribute<&GenericCurrentSensorInput::i_sigma>(offsetof(GenericCurrentSensorInput, i_sigma), "i_sigma"),
            meta_data_gen::get_meta_attribute<&GenericCurrentSensorInput::i_angle_sigma>(offsetof(GenericCurrentSensorInput, i_angle_sigma), "i_angle_sigma"),
    };
};

template <symmetry_tag sym_type>
struct get_attributes_list<CurrentSensorInput<sym_type>> {
    using sym = sym_type;

    static constexpr std::array<MetaAttribute, 8> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&CurrentSensorInput<sym>::id>(offsetof(CurrentSensorInput<sym>, id), "id"),
            meta_data_gen::get_meta_attribute<&CurrentSensorInput<sym>::measured_object>(offsetof(CurrentSensorInput<sym>, measured_object), "measured_object"),
            meta_data_gen::get_meta_attribute<&CurrentSensorInput<sym>::measured_terminal_type>(offsetof(CurrentSensorInput<sym>, measured_terminal_type), "measured_terminal_type"),
            meta_data_gen::get_meta_attribute<&CurrentSensorInput<sym>::angle_measurement_type>(offsetof(CurrentSensorInput<sym>, angle_measurement_type), "angle_measurement_type"),
            meta_data_gen::get_meta_attribute<&CurrentSensorInput<sym>::i_sigma>(offsetof(CurrentSensorInput<sym>, i_sigma), "i_sigma"),
            meta_data_gen::get_meta_attribute<&CurrentSensorInput<sym>::i_angle_sigma>(offsetof(CurrentSensorInput<sym>, i_angle_sigma), "i_angle_sigma"),
            meta_data_gen::get_meta_attribute<&CurrentSensorInput<sym>::i_measured>(offsetof(CurrentSensorInput<sym>, i_measured), "i_measured"),
            meta_data_gen::get_meta_attribute<&CurrentSensorInput<sym>::i_angle_measured>(offsetof(CurrentSensorInput<sym>, i_angle_measured), "i_angle_measured"),
    };
};




} // namespace power_grid_model::meta_data

// clang-format on