// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

// This header file is automatically generated. DO NOT modify it manually!

// clang-format off
#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_META_GEN_OUTPUT_HPP
#define POWER_GRID_MODEL_AUXILIARY_META_GEN_OUTPUT_HPP

#include "../../enum.hpp"
#include "../../power_grid_model.hpp"
#include "../../three_phase_tensor.hpp"
#include "../meta_data.hpp"
#include "../output.hpp"


namespace power_grid_model {

namespace meta_data {

// template specialization functors to get attributes

template<>
struct get_attributes_list<BaseOutput> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<BaseOutput, &BaseOutput::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<BaseOutput, &BaseOutput::energized> const>("energized"));
        return attributes;
    }
};

template <bool sym>
struct get_attributes_list<NodeOutput<sym>> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<NodeOutput<sym>, &NodeOutput<sym>::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<NodeOutput<sym>, &NodeOutput<sym>::energized> const>("energized"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<NodeOutput<sym>, &NodeOutput<sym>::u_pu> const>("u_pu"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<NodeOutput<sym>, &NodeOutput<sym>::u> const>("u"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<NodeOutput<sym>, &NodeOutput<sym>::u_angle> const>("u_angle"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<NodeOutput<sym>, &NodeOutput<sym>::p> const>("p"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<NodeOutput<sym>, &NodeOutput<sym>::q> const>("q"));
        return attributes;
    }
};

template <bool sym>
struct get_attributes_list<BranchOutput<sym>> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::energized> const>("energized"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::loading> const>("loading"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::p_from> const>("p_from"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::q_from> const>("q_from"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::i_from> const>("i_from"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::s_from> const>("s_from"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::p_to> const>("p_to"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::q_to> const>("q_to"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::i_to> const>("i_to"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::s_to> const>("s_to"));
        return attributes;
    }
};

template <bool sym>
struct get_attributes_list<Branch3Output<sym>> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::energized> const>("energized"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::loading> const>("loading"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::p_1> const>("p_1"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::q_1> const>("q_1"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::i_1> const>("i_1"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::s_1> const>("s_1"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::p_2> const>("p_2"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::q_2> const>("q_2"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::i_2> const>("i_2"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::s_2> const>("s_2"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::p_3> const>("p_3"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::q_3> const>("q_3"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::i_3> const>("i_3"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::s_3> const>("s_3"));
        return attributes;
    }
};

template <bool sym>
struct get_attributes_list<ApplianceOutput<sym>> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<ApplianceOutput<sym>, &ApplianceOutput<sym>::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ApplianceOutput<sym>, &ApplianceOutput<sym>::energized> const>("energized"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ApplianceOutput<sym>, &ApplianceOutput<sym>::p> const>("p"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ApplianceOutput<sym>, &ApplianceOutput<sym>::q> const>("q"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ApplianceOutput<sym>, &ApplianceOutput<sym>::i> const>("i"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ApplianceOutput<sym>, &ApplianceOutput<sym>::s> const>("s"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ApplianceOutput<sym>, &ApplianceOutput<sym>::pf> const>("pf"));
        return attributes;
    }
};

template <bool sym>
struct get_attributes_list<VoltageSensorOutput<sym>> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<VoltageSensorOutput<sym>, &VoltageSensorOutput<sym>::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<VoltageSensorOutput<sym>, &VoltageSensorOutput<sym>::energized> const>("energized"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<VoltageSensorOutput<sym>, &VoltageSensorOutput<sym>::u_residual> const>("u_residual"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<VoltageSensorOutput<sym>, &VoltageSensorOutput<sym>::u_angle_residual> const>("u_angle_residual"));
        return attributes;
    }
};

template <bool sym>
struct get_attributes_list<PowerSensorOutput<sym>> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<PowerSensorOutput<sym>, &PowerSensorOutput<sym>::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<PowerSensorOutput<sym>, &PowerSensorOutput<sym>::energized> const>("energized"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<PowerSensorOutput<sym>, &PowerSensorOutput<sym>::p_residual> const>("p_residual"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<PowerSensorOutput<sym>, &PowerSensorOutput<sym>::q_residual> const>("q_residual"));
        return attributes;
    }
};

template<>
struct get_attributes_list<FaultOutput> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<FaultOutput, &FaultOutput::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<FaultOutput, &FaultOutput::energized> const>("energized"));
        return attributes;
    }
};

template<>
struct get_attributes_list<FaultShortCircuitOutput> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<FaultShortCircuitOutput, &FaultShortCircuitOutput::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<FaultShortCircuitOutput, &FaultShortCircuitOutput::energized> const>("energized"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<FaultShortCircuitOutput, &FaultShortCircuitOutput::i_f> const>("i_f"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<FaultShortCircuitOutput, &FaultShortCircuitOutput::i_f_angle> const>("i_f_angle"));
        return attributes;
    }
};

template<>
struct get_attributes_list<NodeShortCircuitOutput> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<NodeShortCircuitOutput, &NodeShortCircuitOutput::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<NodeShortCircuitOutput, &NodeShortCircuitOutput::energized> const>("energized"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<NodeShortCircuitOutput, &NodeShortCircuitOutput::u_pu> const>("u_pu"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<NodeShortCircuitOutput, &NodeShortCircuitOutput::u> const>("u"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<NodeShortCircuitOutput, &NodeShortCircuitOutput::u_angle> const>("u_angle"));
        return attributes;
    }
};

template<>
struct get_attributes_list<BranchShortCircuitOutput> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<BranchShortCircuitOutput, &BranchShortCircuitOutput::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<BranchShortCircuitOutput, &BranchShortCircuitOutput::energized> const>("energized"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<BranchShortCircuitOutput, &BranchShortCircuitOutput::i_from> const>("i_from"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<BranchShortCircuitOutput, &BranchShortCircuitOutput::i_from_angle> const>("i_from_angle"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<BranchShortCircuitOutput, &BranchShortCircuitOutput::i_to> const>("i_to"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<BranchShortCircuitOutput, &BranchShortCircuitOutput::i_to_angle> const>("i_to_angle"));
        return attributes;
    }
};

template<>
struct get_attributes_list<Branch3ShortCircuitOutput> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::energized> const>("energized"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::i_1> const>("i_1"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::i_1_angle> const>("i_1_angle"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::i_2> const>("i_2"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::i_2_angle> const>("i_2_angle"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::i_3> const>("i_3"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::i_3_angle> const>("i_3_angle"));
        return attributes;
    }
};

template<>
struct get_attributes_list<ApplianceShortCircuitOutput> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<ApplianceShortCircuitOutput, &ApplianceShortCircuitOutput::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ApplianceShortCircuitOutput, &ApplianceShortCircuitOutput::energized> const>("energized"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ApplianceShortCircuitOutput, &ApplianceShortCircuitOutput::i> const>("i"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ApplianceShortCircuitOutput, &ApplianceShortCircuitOutput::i_angle> const>("i_angle"));
        return attributes;
    }
};

template<>
struct get_attributes_list<SensorShortCircuitOutput> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<SensorShortCircuitOutput, &SensorShortCircuitOutput::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<SensorShortCircuitOutput, &SensorShortCircuitOutput::energized> const>("energized"));
        return attributes;
    }
};



// template specialization functors to get nan

template<>
struct get_component_nan<BaseOutput> {
    BaseOutput operator() () const {
        BaseOutput comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.energized);
        return comp;
    }
};

template <bool sym>
struct get_component_nan<NodeOutput<sym>> {
    NodeOutput<sym> operator() () const {
        NodeOutput<sym> comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.energized);
        set_nan(comp.u_pu);
        set_nan(comp.u);
        set_nan(comp.u_angle);
        set_nan(comp.p);
        set_nan(comp.q);
        return comp;
    }
};

template <bool sym>
struct get_component_nan<BranchOutput<sym>> {
    BranchOutput<sym> operator() () const {
        BranchOutput<sym> comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.energized);
        set_nan(comp.loading);
        set_nan(comp.p_from);
        set_nan(comp.q_from);
        set_nan(comp.i_from);
        set_nan(comp.s_from);
        set_nan(comp.p_to);
        set_nan(comp.q_to);
        set_nan(comp.i_to);
        set_nan(comp.s_to);
        return comp;
    }
};

template <bool sym>
struct get_component_nan<Branch3Output<sym>> {
    Branch3Output<sym> operator() () const {
        Branch3Output<sym> comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.energized);
        set_nan(comp.loading);
        set_nan(comp.p_1);
        set_nan(comp.q_1);
        set_nan(comp.i_1);
        set_nan(comp.s_1);
        set_nan(comp.p_2);
        set_nan(comp.q_2);
        set_nan(comp.i_2);
        set_nan(comp.s_2);
        set_nan(comp.p_3);
        set_nan(comp.q_3);
        set_nan(comp.i_3);
        set_nan(comp.s_3);
        return comp;
    }
};

template <bool sym>
struct get_component_nan<ApplianceOutput<sym>> {
    ApplianceOutput<sym> operator() () const {
        ApplianceOutput<sym> comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.energized);
        set_nan(comp.p);
        set_nan(comp.q);
        set_nan(comp.i);
        set_nan(comp.s);
        set_nan(comp.pf);
        return comp;
    }
};

template <bool sym>
struct get_component_nan<VoltageSensorOutput<sym>> {
    VoltageSensorOutput<sym> operator() () const {
        VoltageSensorOutput<sym> comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.energized);
        set_nan(comp.u_residual);
        set_nan(comp.u_angle_residual);
        return comp;
    }
};

template <bool sym>
struct get_component_nan<PowerSensorOutput<sym>> {
    PowerSensorOutput<sym> operator() () const {
        PowerSensorOutput<sym> comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.energized);
        set_nan(comp.p_residual);
        set_nan(comp.q_residual);
        return comp;
    }
};

template<>
struct get_component_nan<FaultOutput> {
    FaultOutput operator() () const {
        FaultOutput comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.energized);
        return comp;
    }
};

template<>
struct get_component_nan<FaultShortCircuitOutput> {
    FaultShortCircuitOutput operator() () const {
        FaultShortCircuitOutput comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.energized);
        set_nan(comp.i_f);
        set_nan(comp.i_f_angle);
        return comp;
    }
};

template<>
struct get_component_nan<NodeShortCircuitOutput> {
    NodeShortCircuitOutput operator() () const {
        NodeShortCircuitOutput comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.energized);
        set_nan(comp.u_pu);
        set_nan(comp.u);
        set_nan(comp.u_angle);
        return comp;
    }
};

template<>
struct get_component_nan<BranchShortCircuitOutput> {
    BranchShortCircuitOutput operator() () const {
        BranchShortCircuitOutput comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.energized);
        set_nan(comp.i_from);
        set_nan(comp.i_from_angle);
        set_nan(comp.i_to);
        set_nan(comp.i_to_angle);
        return comp;
    }
};

template<>
struct get_component_nan<Branch3ShortCircuitOutput> {
    Branch3ShortCircuitOutput operator() () const {
        Branch3ShortCircuitOutput comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.energized);
        set_nan(comp.i_1);
        set_nan(comp.i_1_angle);
        set_nan(comp.i_2);
        set_nan(comp.i_2_angle);
        set_nan(comp.i_3);
        set_nan(comp.i_3_angle);
        return comp;
    }
};

template<>
struct get_component_nan<ApplianceShortCircuitOutput> {
    ApplianceShortCircuitOutput operator() () const {
        ApplianceShortCircuitOutput comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.energized);
        set_nan(comp.i);
        set_nan(comp.i_angle);
        return comp;
    }
};

template<>
struct get_component_nan<SensorShortCircuitOutput> {
    SensorShortCircuitOutput operator() () const {
        SensorShortCircuitOutput comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.energized);
        return comp;
    }
};



} // namespace meta_data

} // namespace power_grid_model

#endif
// clang-format on