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

namespace meta_data {

// template specialization functors to get attributes

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



// template specialization functors to get nan

template<>
struct get_component_nan<BaseInput> {
    BaseInput operator() () const {
        BaseInput comp;
        // all attributes including base class
        
        set_nan(comp.id);
        return comp;
    }
};

template<>
struct get_component_nan<NodeInput> {
    NodeInput operator() () const {
        NodeInput comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.u_rated);
        return comp;
    }
};

template<>
struct get_component_nan<BranchInput> {
    BranchInput operator() () const {
        BranchInput comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.from_node);
        set_nan(comp.to_node);
        set_nan(comp.from_status);
        set_nan(comp.to_status);
        return comp;
    }
};

template<>
struct get_component_nan<Branch3Input> {
    Branch3Input operator() () const {
        Branch3Input comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.node_1);
        set_nan(comp.node_2);
        set_nan(comp.node_3);
        set_nan(comp.status_1);
        set_nan(comp.status_2);
        set_nan(comp.status_3);
        return comp;
    }
};

template<>
struct get_component_nan<SensorInput> {
    SensorInput operator() () const {
        SensorInput comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.measured_object);
        return comp;
    }
};

template<>
struct get_component_nan<ApplianceInput> {
    ApplianceInput operator() () const {
        ApplianceInput comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.node);
        set_nan(comp.status);
        return comp;
    }
};

template<>
struct get_component_nan<LineInput> {
    LineInput operator() () const {
        LineInput comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.from_node);
        set_nan(comp.to_node);
        set_nan(comp.from_status);
        set_nan(comp.to_status);
        set_nan(comp.r1);
        set_nan(comp.x1);
        set_nan(comp.c1);
        set_nan(comp.tan1);
        set_nan(comp.r0);
        set_nan(comp.x0);
        set_nan(comp.c0);
        set_nan(comp.tan0);
        set_nan(comp.i_n);
        return comp;
    }
};

template<>
struct get_component_nan<LinkInput> {
    LinkInput operator() () const {
        LinkInput comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.from_node);
        set_nan(comp.to_node);
        set_nan(comp.from_status);
        set_nan(comp.to_status);
        return comp;
    }
};

template<>
struct get_component_nan<TransformerInput> {
    TransformerInput operator() () const {
        TransformerInput comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.from_node);
        set_nan(comp.to_node);
        set_nan(comp.from_status);
        set_nan(comp.to_status);
        set_nan(comp.u1);
        set_nan(comp.u2);
        set_nan(comp.sn);
        set_nan(comp.uk);
        set_nan(comp.pk);
        set_nan(comp.i0);
        set_nan(comp.p0);
        set_nan(comp.winding_from);
        set_nan(comp.winding_to);
        set_nan(comp.clock);
        set_nan(comp.tap_side);
        set_nan(comp.tap_pos);
        set_nan(comp.tap_min);
        set_nan(comp.tap_max);
        set_nan(comp.tap_nom);
        set_nan(comp.tap_size);
        set_nan(comp.uk_min);
        set_nan(comp.uk_max);
        set_nan(comp.pk_min);
        set_nan(comp.pk_max);
        set_nan(comp.r_grounding_from);
        set_nan(comp.x_grounding_from);
        set_nan(comp.r_grounding_to);
        set_nan(comp.x_grounding_to);
        return comp;
    }
};

template<>
struct get_component_nan<ThreeWindingTransformerInput> {
    ThreeWindingTransformerInput operator() () const {
        ThreeWindingTransformerInput comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.node_1);
        set_nan(comp.node_2);
        set_nan(comp.node_3);
        set_nan(comp.status_1);
        set_nan(comp.status_2);
        set_nan(comp.status_3);
        set_nan(comp.u1);
        set_nan(comp.u2);
        set_nan(comp.u3);
        set_nan(comp.sn_1);
        set_nan(comp.sn_2);
        set_nan(comp.sn_3);
        set_nan(comp.uk_12);
        set_nan(comp.uk_13);
        set_nan(comp.uk_23);
        set_nan(comp.pk_12);
        set_nan(comp.pk_13);
        set_nan(comp.pk_23);
        set_nan(comp.i0);
        set_nan(comp.p0);
        set_nan(comp.winding_1);
        set_nan(comp.winding_2);
        set_nan(comp.winding_3);
        set_nan(comp.clock_12);
        set_nan(comp.clock_13);
        set_nan(comp.tap_side);
        set_nan(comp.tap_pos);
        set_nan(comp.tap_min);
        set_nan(comp.tap_max);
        set_nan(comp.tap_nom);
        set_nan(comp.tap_size);
        set_nan(comp.uk_12_min);
        set_nan(comp.uk_12_max);
        set_nan(comp.uk_13_min);
        set_nan(comp.uk_13_max);
        set_nan(comp.uk_23_min);
        set_nan(comp.uk_23_max);
        set_nan(comp.pk_12_min);
        set_nan(comp.pk_12_max);
        set_nan(comp.pk_13_min);
        set_nan(comp.pk_13_max);
        set_nan(comp.pk_23_min);
        set_nan(comp.pk_23_max);
        set_nan(comp.r_grounding_1);
        set_nan(comp.x_grounding_1);
        set_nan(comp.r_grounding_2);
        set_nan(comp.x_grounding_2);
        set_nan(comp.r_grounding_3);
        set_nan(comp.x_grounding_3);
        return comp;
    }
};

template<>
struct get_component_nan<GenericLoadGenInput> {
    GenericLoadGenInput operator() () const {
        GenericLoadGenInput comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.node);
        set_nan(comp.status);
        set_nan(comp.type);
        return comp;
    }
};

template <bool sym>
struct get_component_nan<LoadGenInput<sym>> {
    LoadGenInput<sym> operator() () const {
        LoadGenInput<sym> comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.node);
        set_nan(comp.status);
        set_nan(comp.type);
        set_nan(comp.p_specified);
        set_nan(comp.q_specified);
        return comp;
    }
};

template<>
struct get_component_nan<ShuntInput> {
    ShuntInput operator() () const {
        ShuntInput comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.node);
        set_nan(comp.status);
        set_nan(comp.g1);
        set_nan(comp.b1);
        set_nan(comp.g0);
        set_nan(comp.b0);
        return comp;
    }
};

template<>
struct get_component_nan<SourceInput> {
    SourceInput operator() () const {
        SourceInput comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.node);
        set_nan(comp.status);
        set_nan(comp.u_ref);
        set_nan(comp.u_ref_angle);
        set_nan(comp.sk);
        set_nan(comp.rx_ratio);
        set_nan(comp.z01_ratio);
        return comp;
    }
};

template<>
struct get_component_nan<GenericVoltageSensorInput> {
    GenericVoltageSensorInput operator() () const {
        GenericVoltageSensorInput comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.measured_object);
        set_nan(comp.u_sigma);
        return comp;
    }
};

template <bool sym>
struct get_component_nan<VoltageSensorInput<sym>> {
    VoltageSensorInput<sym> operator() () const {
        VoltageSensorInput<sym> comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.measured_object);
        set_nan(comp.u_sigma);
        set_nan(comp.u_measured);
        set_nan(comp.u_angle_measured);
        return comp;
    }
};

template<>
struct get_component_nan<GenericPowerSensorInput> {
    GenericPowerSensorInput operator() () const {
        GenericPowerSensorInput comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.measured_object);
        set_nan(comp.measured_terminal_type);
        set_nan(comp.power_sigma);
        return comp;
    }
};

template <bool sym>
struct get_component_nan<PowerSensorInput<sym>> {
    PowerSensorInput<sym> operator() () const {
        PowerSensorInput<sym> comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.measured_object);
        set_nan(comp.measured_terminal_type);
        set_nan(comp.power_sigma);
        set_nan(comp.p_measured);
        set_nan(comp.q_measured);
        return comp;
    }
};

template<>
struct get_component_nan<FaultInput> {
    FaultInput operator() () const {
        FaultInput comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.status);
        set_nan(comp.fault_type);
        set_nan(comp.fault_phase);
        set_nan(comp.fault_object);
        set_nan(comp.r_f);
        set_nan(comp.x_f);
        return comp;
    }
};



} // namespace meta_data

} // namespace power_grid_model

#endif
// clang-format on