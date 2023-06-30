// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

// This header file is automatically generated. DO NOT modify it manually!

// clang-format off
#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_META_GEN_INPUT_HPP
#define POWER_GRID_MODEL_AUXILIARY_META_GEN_INPUT_HPP

#include "../../enum.hpp"
#include "../../power_grid_model.hpp"
#include "../../three_phase_tensor.hpp"
#include "../meta_data.hpp"
#include "../input.hpp"


namespace power_grid_model {

// template specialization functors to get meta data
namespace meta_data {

template<>
struct get_attributes_list<BaseInput> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<BaseInput, &BaseInput::id> const>("id"));
        return attributes;
    }
};

template<>
struct get_attributes_list<NodeInput> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<NodeInput, &NodeInput::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<NodeInput, &NodeInput::u_rated> const>("u_rated"));
        return attributes;
    }
};

template<>
struct get_attributes_list<BranchInput> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<BranchInput, &BranchInput::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<BranchInput, &BranchInput::from_node> const>("from_node"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<BranchInput, &BranchInput::to_node> const>("to_node"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<BranchInput, &BranchInput::from_status> const>("from_status"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<BranchInput, &BranchInput::to_status> const>("to_status"));
        return attributes;
    }
};

template<>
struct get_attributes_list<Branch3Input> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3Input, &Branch3Input::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3Input, &Branch3Input::node_1> const>("node_1"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3Input, &Branch3Input::node_2> const>("node_2"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3Input, &Branch3Input::node_3> const>("node_3"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3Input, &Branch3Input::status_1> const>("status_1"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3Input, &Branch3Input::status_2> const>("status_2"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3Input, &Branch3Input::status_3> const>("status_3"));
        return attributes;
    }
};

template<>
struct get_attributes_list<SensorInput> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<SensorInput, &SensorInput::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<SensorInput, &SensorInput::measured_object> const>("measured_object"));
        return attributes;
    }
};

template<>
struct get_attributes_list<ApplianceInput> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<ApplianceInput, &ApplianceInput::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ApplianceInput, &ApplianceInput::node> const>("node"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ApplianceInput, &ApplianceInput::status> const>("status"));
        return attributes;
    }
};

template<>
struct get_attributes_list<LineInput> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<LineInput, &LineInput::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<LineInput, &LineInput::from_node> const>("from_node"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<LineInput, &LineInput::to_node> const>("to_node"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<LineInput, &LineInput::from_status> const>("from_status"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<LineInput, &LineInput::to_status> const>("to_status"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<LineInput, &LineInput::r1> const>("r1"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<LineInput, &LineInput::x1> const>("x1"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<LineInput, &LineInput::c1> const>("c1"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<LineInput, &LineInput::tan1> const>("tan1"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<LineInput, &LineInput::r0> const>("r0"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<LineInput, &LineInput::x0> const>("x0"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<LineInput, &LineInput::c0> const>("c0"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<LineInput, &LineInput::tan0> const>("tan0"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<LineInput, &LineInput::i_n> const>("i_n"));
        return attributes;
    }
};

template<>
struct get_attributes_list<LinkInput> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<LinkInput, &LinkInput::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<LinkInput, &LinkInput::from_node> const>("from_node"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<LinkInput, &LinkInput::to_node> const>("to_node"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<LinkInput, &LinkInput::from_status> const>("from_status"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<LinkInput, &LinkInput::to_status> const>("to_status"));
        return attributes;
    }
};

template<>
struct get_attributes_list<TransformerInput> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerInput, &TransformerInput::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerInput, &TransformerInput::from_node> const>("from_node"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerInput, &TransformerInput::to_node> const>("to_node"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerInput, &TransformerInput::from_status> const>("from_status"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerInput, &TransformerInput::to_status> const>("to_status"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerInput, &TransformerInput::u1> const>("u1"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerInput, &TransformerInput::u2> const>("u2"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerInput, &TransformerInput::sn> const>("sn"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerInput, &TransformerInput::uk> const>("uk"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerInput, &TransformerInput::pk> const>("pk"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerInput, &TransformerInput::i0> const>("i0"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerInput, &TransformerInput::p0> const>("p0"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerInput, &TransformerInput::winding_from> const>("winding_from"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerInput, &TransformerInput::winding_to> const>("winding_to"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerInput, &TransformerInput::clock> const>("clock"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerInput, &TransformerInput::tap_side> const>("tap_side"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerInput, &TransformerInput::tap_pos> const>("tap_pos"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerInput, &TransformerInput::tap_min> const>("tap_min"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerInput, &TransformerInput::tap_max> const>("tap_max"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerInput, &TransformerInput::tap_nom> const>("tap_nom"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerInput, &TransformerInput::tap_size> const>("tap_size"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerInput, &TransformerInput::uk_min> const>("uk_min"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerInput, &TransformerInput::uk_max> const>("uk_max"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerInput, &TransformerInput::pk_min> const>("pk_min"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerInput, &TransformerInput::pk_max> const>("pk_max"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerInput, &TransformerInput::r_grounding_from> const>("r_grounding_from"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerInput, &TransformerInput::x_grounding_from> const>("x_grounding_from"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerInput, &TransformerInput::r_grounding_to> const>("r_grounding_to"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerInput, &TransformerInput::x_grounding_to> const>("x_grounding_to"));
        return attributes;
    }
};

template<>
struct get_attributes_list<ThreeWindingTransformerInput> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::node_1> const>("node_1"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::node_2> const>("node_2"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::node_3> const>("node_3"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::status_1> const>("status_1"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::status_2> const>("status_2"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::status_3> const>("status_3"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::u1> const>("u1"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::u2> const>("u2"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::u3> const>("u3"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::sn_1> const>("sn_1"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::sn_2> const>("sn_2"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::sn_3> const>("sn_3"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::uk_12> const>("uk_12"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::uk_13> const>("uk_13"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::uk_23> const>("uk_23"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::pk_12> const>("pk_12"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::pk_13> const>("pk_13"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::pk_23> const>("pk_23"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::i0> const>("i0"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::p0> const>("p0"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::winding_1> const>("winding_1"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::winding_2> const>("winding_2"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::winding_3> const>("winding_3"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::clock_12> const>("clock_12"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::clock_13> const>("clock_13"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::tap_side> const>("tap_side"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::tap_pos> const>("tap_pos"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::tap_min> const>("tap_min"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::tap_max> const>("tap_max"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::tap_nom> const>("tap_nom"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::tap_size> const>("tap_size"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::uk_12_min> const>("uk_12_min"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::uk_12_max> const>("uk_12_max"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::uk_13_min> const>("uk_13_min"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::uk_13_max> const>("uk_13_max"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::uk_23_min> const>("uk_23_min"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::uk_23_max> const>("uk_23_max"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::pk_12_min> const>("pk_12_min"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::pk_12_max> const>("pk_12_max"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::pk_13_min> const>("pk_13_min"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::pk_13_max> const>("pk_13_max"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::pk_23_min> const>("pk_23_min"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::pk_23_max> const>("pk_23_max"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::r_grounding_1> const>("r_grounding_1"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::x_grounding_1> const>("x_grounding_1"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::r_grounding_2> const>("r_grounding_2"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::x_grounding_2> const>("x_grounding_2"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::r_grounding_3> const>("r_grounding_3"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::x_grounding_3> const>("x_grounding_3"));
        return attributes;
    }
};

template<>
struct get_attributes_list<GenericLoadGenInput> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<GenericLoadGenInput, &GenericLoadGenInput::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<GenericLoadGenInput, &GenericLoadGenInput::node> const>("node"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<GenericLoadGenInput, &GenericLoadGenInput::status> const>("status"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<GenericLoadGenInput, &GenericLoadGenInput::type> const>("type"));
        return attributes;
    }
};

template <bool sym>
struct get_attributes_list<LoadGenInput<sym>> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<LoadGenInput<sym>, &LoadGenInput<sym>::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<LoadGenInput<sym>, &LoadGenInput<sym>::node> const>("node"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<LoadGenInput<sym>, &LoadGenInput<sym>::status> const>("status"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<LoadGenInput<sym>, &LoadGenInput<sym>::type> const>("type"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<LoadGenInput<sym>, &LoadGenInput<sym>::p_specified> const>("p_specified"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<LoadGenInput<sym>, &LoadGenInput<sym>::q_specified> const>("q_specified"));
        return attributes;
    }
};

template<>
struct get_attributes_list<ShuntInput> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<ShuntInput, &ShuntInput::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ShuntInput, &ShuntInput::node> const>("node"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ShuntInput, &ShuntInput::status> const>("status"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ShuntInput, &ShuntInput::g1> const>("g1"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ShuntInput, &ShuntInput::b1> const>("b1"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ShuntInput, &ShuntInput::g0> const>("g0"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ShuntInput, &ShuntInput::b0> const>("b0"));
        return attributes;
    }
};

template<>
struct get_attributes_list<SourceInput> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<SourceInput, &SourceInput::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<SourceInput, &SourceInput::node> const>("node"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<SourceInput, &SourceInput::status> const>("status"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<SourceInput, &SourceInput::u_ref> const>("u_ref"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<SourceInput, &SourceInput::u_ref_angle> const>("u_ref_angle"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<SourceInput, &SourceInput::sk> const>("sk"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<SourceInput, &SourceInput::rx_ratio> const>("rx_ratio"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<SourceInput, &SourceInput::z01_ratio> const>("z01_ratio"));
        return attributes;
    }
};

template<>
struct get_attributes_list<GenericVoltageSensorInput> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<GenericVoltageSensorInput, &GenericVoltageSensorInput::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<GenericVoltageSensorInput, &GenericVoltageSensorInput::measured_object> const>("measured_object"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<GenericVoltageSensorInput, &GenericVoltageSensorInput::u_sigma> const>("u_sigma"));
        return attributes;
    }
};

template <bool sym>
struct get_attributes_list<VoltageSensorInput<sym>> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<VoltageSensorInput<sym>, &VoltageSensorInput<sym>::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<VoltageSensorInput<sym>, &VoltageSensorInput<sym>::measured_object> const>("measured_object"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<VoltageSensorInput<sym>, &VoltageSensorInput<sym>::u_sigma> const>("u_sigma"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<VoltageSensorInput<sym>, &VoltageSensorInput<sym>::u_measured> const>("u_measured"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<VoltageSensorInput<sym>, &VoltageSensorInput<sym>::u_angle_measured> const>("u_angle_measured"));
        return attributes;
    }
};

template<>
struct get_attributes_list<GenericPowerSensorInput> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<GenericPowerSensorInput, &GenericPowerSensorInput::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<GenericPowerSensorInput, &GenericPowerSensorInput::measured_object> const>("measured_object"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<GenericPowerSensorInput, &GenericPowerSensorInput::measured_terminal_type> const>("measured_terminal_type"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<GenericPowerSensorInput, &GenericPowerSensorInput::power_sigma> const>("power_sigma"));
        return attributes;
    }
};

template <bool sym>
struct get_attributes_list<PowerSensorInput<sym>> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<PowerSensorInput<sym>, &PowerSensorInput<sym>::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<PowerSensorInput<sym>, &PowerSensorInput<sym>::measured_object> const>("measured_object"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<PowerSensorInput<sym>, &PowerSensorInput<sym>::measured_terminal_type> const>("measured_terminal_type"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<PowerSensorInput<sym>, &PowerSensorInput<sym>::power_sigma> const>("power_sigma"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<PowerSensorInput<sym>, &PowerSensorInput<sym>::p_measured> const>("p_measured"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<PowerSensorInput<sym>, &PowerSensorInput<sym>::q_measured> const>("q_measured"));
        return attributes;
    }
};

template<>
struct get_attributes_list<FaultInput> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<FaultInput, &FaultInput::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<FaultInput, &FaultInput::status> const>("status"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<FaultInput, &FaultInput::fault_type> const>("fault_type"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<FaultInput, &FaultInput::fault_phase> const>("fault_phase"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<FaultInput, &FaultInput::fault_object> const>("fault_object"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<FaultInput, &FaultInput::r_f> const>("r_f"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<FaultInput, &FaultInput::x_f> const>("x_f"));
        return attributes;
    }
};



} // namespace meta_data

} // namespace power_grid_model

#endif
// clang-format on