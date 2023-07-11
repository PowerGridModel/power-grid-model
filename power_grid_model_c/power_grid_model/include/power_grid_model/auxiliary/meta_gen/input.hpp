// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

// This header file is automatically generated. DO NOT modify it manually!

// clang-format off
#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_META_GEN_INPUT_HPP
#define POWER_GRID_MODEL_AUXILIARY_META_GEN_INPUT_HPP

#include "../../enum.hpp" // NOLINT
#include "../../power_grid_model.hpp" // NOLINT
#include "../../three_phase_tensor.hpp" // NOLINT
#include "../meta_data.hpp" // NOLINT
#include "../input.hpp" // NOLINT


namespace power_grid_model::meta_data {

// template specialization functors to get attributes

template<>
struct get_attributes_list<BaseInput> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<BaseInput, &BaseInput::id>{}, "id"},
        };
    }
};

template<>
struct get_attributes_list<NodeInput> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<NodeInput, &NodeInput::id>{}, "id"},
            {MetaAttributeImpl<NodeInput, &NodeInput::u_rated>{}, "u_rated"},
        };
    }
};

template<>
struct get_attributes_list<BranchInput> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<BranchInput, &BranchInput::id>{}, "id"},
            {MetaAttributeImpl<BranchInput, &BranchInput::from_node>{}, "from_node"},
            {MetaAttributeImpl<BranchInput, &BranchInput::to_node>{}, "to_node"},
            {MetaAttributeImpl<BranchInput, &BranchInput::from_status>{}, "from_status"},
            {MetaAttributeImpl<BranchInput, &BranchInput::to_status>{}, "to_status"},
        };
    }
};

template<>
struct get_attributes_list<Branch3Input> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<Branch3Input, &Branch3Input::id>{}, "id"},
            {MetaAttributeImpl<Branch3Input, &Branch3Input::node_1>{}, "node_1"},
            {MetaAttributeImpl<Branch3Input, &Branch3Input::node_2>{}, "node_2"},
            {MetaAttributeImpl<Branch3Input, &Branch3Input::node_3>{}, "node_3"},
            {MetaAttributeImpl<Branch3Input, &Branch3Input::status_1>{}, "status_1"},
            {MetaAttributeImpl<Branch3Input, &Branch3Input::status_2>{}, "status_2"},
            {MetaAttributeImpl<Branch3Input, &Branch3Input::status_3>{}, "status_3"},
        };
    }
};

template<>
struct get_attributes_list<SensorInput> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<SensorInput, &SensorInput::id>{}, "id"},
            {MetaAttributeImpl<SensorInput, &SensorInput::measured_object>{}, "measured_object"},
        };
    }
};

template<>
struct get_attributes_list<ApplianceInput> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<ApplianceInput, &ApplianceInput::id>{}, "id"},
            {MetaAttributeImpl<ApplianceInput, &ApplianceInput::node>{}, "node"},
            {MetaAttributeImpl<ApplianceInput, &ApplianceInput::status>{}, "status"},
        };
    }
};

template<>
struct get_attributes_list<LineInput> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<LineInput, &LineInput::id>{}, "id"},
            {MetaAttributeImpl<LineInput, &LineInput::from_node>{}, "from_node"},
            {MetaAttributeImpl<LineInput, &LineInput::to_node>{}, "to_node"},
            {MetaAttributeImpl<LineInput, &LineInput::from_status>{}, "from_status"},
            {MetaAttributeImpl<LineInput, &LineInput::to_status>{}, "to_status"},
            {MetaAttributeImpl<LineInput, &LineInput::r1>{}, "r1"},
            {MetaAttributeImpl<LineInput, &LineInput::x1>{}, "x1"},
            {MetaAttributeImpl<LineInput, &LineInput::c1>{}, "c1"},
            {MetaAttributeImpl<LineInput, &LineInput::tan1>{}, "tan1"},
            {MetaAttributeImpl<LineInput, &LineInput::r0>{}, "r0"},
            {MetaAttributeImpl<LineInput, &LineInput::x0>{}, "x0"},
            {MetaAttributeImpl<LineInput, &LineInput::c0>{}, "c0"},
            {MetaAttributeImpl<LineInput, &LineInput::tan0>{}, "tan0"},
            {MetaAttributeImpl<LineInput, &LineInput::i_n>{}, "i_n"},
        };
    }
};

template<>
struct get_attributes_list<LinkInput> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<LinkInput, &LinkInput::id>{}, "id"},
            {MetaAttributeImpl<LinkInput, &LinkInput::from_node>{}, "from_node"},
            {MetaAttributeImpl<LinkInput, &LinkInput::to_node>{}, "to_node"},
            {MetaAttributeImpl<LinkInput, &LinkInput::from_status>{}, "from_status"},
            {MetaAttributeImpl<LinkInput, &LinkInput::to_status>{}, "to_status"},
        };
    }
};

template<>
struct get_attributes_list<TransformerInput> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<TransformerInput, &TransformerInput::id>{}, "id"},
            {MetaAttributeImpl<TransformerInput, &TransformerInput::from_node>{}, "from_node"},
            {MetaAttributeImpl<TransformerInput, &TransformerInput::to_node>{}, "to_node"},
            {MetaAttributeImpl<TransformerInput, &TransformerInput::from_status>{}, "from_status"},
            {MetaAttributeImpl<TransformerInput, &TransformerInput::to_status>{}, "to_status"},
            {MetaAttributeImpl<TransformerInput, &TransformerInput::u1>{}, "u1"},
            {MetaAttributeImpl<TransformerInput, &TransformerInput::u2>{}, "u2"},
            {MetaAttributeImpl<TransformerInput, &TransformerInput::sn>{}, "sn"},
            {MetaAttributeImpl<TransformerInput, &TransformerInput::uk>{}, "uk"},
            {MetaAttributeImpl<TransformerInput, &TransformerInput::pk>{}, "pk"},
            {MetaAttributeImpl<TransformerInput, &TransformerInput::i0>{}, "i0"},
            {MetaAttributeImpl<TransformerInput, &TransformerInput::p0>{}, "p0"},
            {MetaAttributeImpl<TransformerInput, &TransformerInput::winding_from>{}, "winding_from"},
            {MetaAttributeImpl<TransformerInput, &TransformerInput::winding_to>{}, "winding_to"},
            {MetaAttributeImpl<TransformerInput, &TransformerInput::clock>{}, "clock"},
            {MetaAttributeImpl<TransformerInput, &TransformerInput::tap_side>{}, "tap_side"},
            {MetaAttributeImpl<TransformerInput, &TransformerInput::tap_pos>{}, "tap_pos"},
            {MetaAttributeImpl<TransformerInput, &TransformerInput::tap_min>{}, "tap_min"},
            {MetaAttributeImpl<TransformerInput, &TransformerInput::tap_max>{}, "tap_max"},
            {MetaAttributeImpl<TransformerInput, &TransformerInput::tap_nom>{}, "tap_nom"},
            {MetaAttributeImpl<TransformerInput, &TransformerInput::tap_size>{}, "tap_size"},
            {MetaAttributeImpl<TransformerInput, &TransformerInput::uk_min>{}, "uk_min"},
            {MetaAttributeImpl<TransformerInput, &TransformerInput::uk_max>{}, "uk_max"},
            {MetaAttributeImpl<TransformerInput, &TransformerInput::pk_min>{}, "pk_min"},
            {MetaAttributeImpl<TransformerInput, &TransformerInput::pk_max>{}, "pk_max"},
            {MetaAttributeImpl<TransformerInput, &TransformerInput::r_grounding_from>{}, "r_grounding_from"},
            {MetaAttributeImpl<TransformerInput, &TransformerInput::x_grounding_from>{}, "x_grounding_from"},
            {MetaAttributeImpl<TransformerInput, &TransformerInput::r_grounding_to>{}, "r_grounding_to"},
            {MetaAttributeImpl<TransformerInput, &TransformerInput::x_grounding_to>{}, "x_grounding_to"},
        };
    }
};

template<>
struct get_attributes_list<ThreeWindingTransformerInput> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::id>{}, "id"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::node_1>{}, "node_1"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::node_2>{}, "node_2"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::node_3>{}, "node_3"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::status_1>{}, "status_1"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::status_2>{}, "status_2"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::status_3>{}, "status_3"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::u1>{}, "u1"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::u2>{}, "u2"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::u3>{}, "u3"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::sn_1>{}, "sn_1"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::sn_2>{}, "sn_2"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::sn_3>{}, "sn_3"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::uk_12>{}, "uk_12"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::uk_13>{}, "uk_13"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::uk_23>{}, "uk_23"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::pk_12>{}, "pk_12"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::pk_13>{}, "pk_13"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::pk_23>{}, "pk_23"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::i0>{}, "i0"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::p0>{}, "p0"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::winding_1>{}, "winding_1"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::winding_2>{}, "winding_2"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::winding_3>{}, "winding_3"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::clock_12>{}, "clock_12"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::clock_13>{}, "clock_13"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::tap_side>{}, "tap_side"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::tap_pos>{}, "tap_pos"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::tap_min>{}, "tap_min"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::tap_max>{}, "tap_max"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::tap_nom>{}, "tap_nom"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::tap_size>{}, "tap_size"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::uk_12_min>{}, "uk_12_min"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::uk_12_max>{}, "uk_12_max"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::uk_13_min>{}, "uk_13_min"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::uk_13_max>{}, "uk_13_max"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::uk_23_min>{}, "uk_23_min"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::uk_23_max>{}, "uk_23_max"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::pk_12_min>{}, "pk_12_min"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::pk_12_max>{}, "pk_12_max"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::pk_13_min>{}, "pk_13_min"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::pk_13_max>{}, "pk_13_max"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::pk_23_min>{}, "pk_23_min"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::pk_23_max>{}, "pk_23_max"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::r_grounding_1>{}, "r_grounding_1"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::x_grounding_1>{}, "x_grounding_1"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::r_grounding_2>{}, "r_grounding_2"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::x_grounding_2>{}, "x_grounding_2"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::r_grounding_3>{}, "r_grounding_3"},
            {MetaAttributeImpl<ThreeWindingTransformerInput, &ThreeWindingTransformerInput::x_grounding_3>{}, "x_grounding_3"},
        };
    }
};

template<>
struct get_attributes_list<GenericLoadGenInput> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<GenericLoadGenInput, &GenericLoadGenInput::id>{}, "id"},
            {MetaAttributeImpl<GenericLoadGenInput, &GenericLoadGenInput::node>{}, "node"},
            {MetaAttributeImpl<GenericLoadGenInput, &GenericLoadGenInput::status>{}, "status"},
            {MetaAttributeImpl<GenericLoadGenInput, &GenericLoadGenInput::type>{}, "type"},
        };
    }
};

template <bool sym>
struct get_attributes_list<LoadGenInput<sym>> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<LoadGenInput<sym>, &LoadGenInput<sym>::id>{}, "id"},
            {MetaAttributeImpl<LoadGenInput<sym>, &LoadGenInput<sym>::node>{}, "node"},
            {MetaAttributeImpl<LoadGenInput<sym>, &LoadGenInput<sym>::status>{}, "status"},
            {MetaAttributeImpl<LoadGenInput<sym>, &LoadGenInput<sym>::type>{}, "type"},
            {MetaAttributeImpl<LoadGenInput<sym>, &LoadGenInput<sym>::p_specified>{}, "p_specified"},
            {MetaAttributeImpl<LoadGenInput<sym>, &LoadGenInput<sym>::q_specified>{}, "q_specified"},
        };
    }
};

template<>
struct get_attributes_list<ShuntInput> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<ShuntInput, &ShuntInput::id>{}, "id"},
            {MetaAttributeImpl<ShuntInput, &ShuntInput::node>{}, "node"},
            {MetaAttributeImpl<ShuntInput, &ShuntInput::status>{}, "status"},
            {MetaAttributeImpl<ShuntInput, &ShuntInput::g1>{}, "g1"},
            {MetaAttributeImpl<ShuntInput, &ShuntInput::b1>{}, "b1"},
            {MetaAttributeImpl<ShuntInput, &ShuntInput::g0>{}, "g0"},
            {MetaAttributeImpl<ShuntInput, &ShuntInput::b0>{}, "b0"},
        };
    }
};

template<>
struct get_attributes_list<SourceInput> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<SourceInput, &SourceInput::id>{}, "id"},
            {MetaAttributeImpl<SourceInput, &SourceInput::node>{}, "node"},
            {MetaAttributeImpl<SourceInput, &SourceInput::status>{}, "status"},
            {MetaAttributeImpl<SourceInput, &SourceInput::u_ref>{}, "u_ref"},
            {MetaAttributeImpl<SourceInput, &SourceInput::u_ref_angle>{}, "u_ref_angle"},
            {MetaAttributeImpl<SourceInput, &SourceInput::sk>{}, "sk"},
            {MetaAttributeImpl<SourceInput, &SourceInput::rx_ratio>{}, "rx_ratio"},
            {MetaAttributeImpl<SourceInput, &SourceInput::z01_ratio>{}, "z01_ratio"},
        };
    }
};

template<>
struct get_attributes_list<GenericVoltageSensorInput> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<GenericVoltageSensorInput, &GenericVoltageSensorInput::id>{}, "id"},
            {MetaAttributeImpl<GenericVoltageSensorInput, &GenericVoltageSensorInput::measured_object>{}, "measured_object"},
            {MetaAttributeImpl<GenericVoltageSensorInput, &GenericVoltageSensorInput::u_sigma>{}, "u_sigma"},
        };
    }
};

template <bool sym>
struct get_attributes_list<VoltageSensorInput<sym>> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<VoltageSensorInput<sym>, &VoltageSensorInput<sym>::id>{}, "id"},
            {MetaAttributeImpl<VoltageSensorInput<sym>, &VoltageSensorInput<sym>::measured_object>{}, "measured_object"},
            {MetaAttributeImpl<VoltageSensorInput<sym>, &VoltageSensorInput<sym>::u_sigma>{}, "u_sigma"},
            {MetaAttributeImpl<VoltageSensorInput<sym>, &VoltageSensorInput<sym>::u_measured>{}, "u_measured"},
            {MetaAttributeImpl<VoltageSensorInput<sym>, &VoltageSensorInput<sym>::u_angle_measured>{}, "u_angle_measured"},
        };
    }
};

template<>
struct get_attributes_list<GenericPowerSensorInput> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<GenericPowerSensorInput, &GenericPowerSensorInput::id>{}, "id"},
            {MetaAttributeImpl<GenericPowerSensorInput, &GenericPowerSensorInput::measured_object>{}, "measured_object"},
            {MetaAttributeImpl<GenericPowerSensorInput, &GenericPowerSensorInput::measured_terminal_type>{}, "measured_terminal_type"},
            {MetaAttributeImpl<GenericPowerSensorInput, &GenericPowerSensorInput::power_sigma>{}, "power_sigma"},
        };
    }
};

template <bool sym>
struct get_attributes_list<PowerSensorInput<sym>> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<PowerSensorInput<sym>, &PowerSensorInput<sym>::id>{}, "id"},
            {MetaAttributeImpl<PowerSensorInput<sym>, &PowerSensorInput<sym>::measured_object>{}, "measured_object"},
            {MetaAttributeImpl<PowerSensorInput<sym>, &PowerSensorInput<sym>::measured_terminal_type>{}, "measured_terminal_type"},
            {MetaAttributeImpl<PowerSensorInput<sym>, &PowerSensorInput<sym>::power_sigma>{}, "power_sigma"},
            {MetaAttributeImpl<PowerSensorInput<sym>, &PowerSensorInput<sym>::p_measured>{}, "p_measured"},
            {MetaAttributeImpl<PowerSensorInput<sym>, &PowerSensorInput<sym>::q_measured>{}, "q_measured"},
        };
    }
};

template<>
struct get_attributes_list<FaultInput> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<FaultInput, &FaultInput::id>{}, "id"},
            {MetaAttributeImpl<FaultInput, &FaultInput::status>{}, "status"},
            {MetaAttributeImpl<FaultInput, &FaultInput::fault_type>{}, "fault_type"},
            {MetaAttributeImpl<FaultInput, &FaultInput::fault_phase>{}, "fault_phase"},
            {MetaAttributeImpl<FaultInput, &FaultInput::fault_object>{}, "fault_object"},
            {MetaAttributeImpl<FaultInput, &FaultInput::r_f>{}, "r_f"},
            {MetaAttributeImpl<FaultInput, &FaultInput::x_f>{}, "x_f"},
        };
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



} // namespace power_grid_model::meta_data

#endif
// clang-format on