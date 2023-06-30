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
    std::vector<MetaAttribute> operator() () const {
        std::vector<MetaAttribute> attributes{};
        // all attributes including base class
        
        attributes.push_back(MetaAttribute(MetaAttributeImpl<BaseOutput, &BaseOutput::id>{}, "id"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<BaseOutput, &BaseOutput::energized>{}, "energized"));
        return attributes;
    }
};

template <bool sym>
struct get_attributes_list<NodeOutput<sym>> {
    std::vector<MetaAttribute> operator() () const {
        std::vector<MetaAttribute> attributes{};
        // all attributes including base class
        
        attributes.push_back(MetaAttribute(MetaAttributeImpl<NodeOutput<sym>, &NodeOutput<sym>::id>{}, "id"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<NodeOutput<sym>, &NodeOutput<sym>::energized>{}, "energized"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<NodeOutput<sym>, &NodeOutput<sym>::u_pu>{}, "u_pu"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<NodeOutput<sym>, &NodeOutput<sym>::u>{}, "u"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<NodeOutput<sym>, &NodeOutput<sym>::u_angle>{}, "u_angle"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<NodeOutput<sym>, &NodeOutput<sym>::p>{}, "p"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<NodeOutput<sym>, &NodeOutput<sym>::q>{}, "q"));
        return attributes;
    }
};

template <bool sym>
struct get_attributes_list<BranchOutput<sym>> {
    std::vector<MetaAttribute> operator() () const {
        std::vector<MetaAttribute> attributes{};
        // all attributes including base class
        
        attributes.push_back(MetaAttribute(MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::id>{}, "id"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::energized>{}, "energized"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::loading>{}, "loading"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::p_from>{}, "p_from"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::q_from>{}, "q_from"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::i_from>{}, "i_from"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::s_from>{}, "s_from"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::p_to>{}, "p_to"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::q_to>{}, "q_to"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::i_to>{}, "i_to"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::s_to>{}, "s_to"));
        return attributes;
    }
};

template <bool sym>
struct get_attributes_list<Branch3Output<sym>> {
    std::vector<MetaAttribute> operator() () const {
        std::vector<MetaAttribute> attributes{};
        // all attributes including base class
        
        attributes.push_back(MetaAttribute(MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::id>{}, "id"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::energized>{}, "energized"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::loading>{}, "loading"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::p_1>{}, "p_1"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::q_1>{}, "q_1"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::i_1>{}, "i_1"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::s_1>{}, "s_1"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::p_2>{}, "p_2"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::q_2>{}, "q_2"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::i_2>{}, "i_2"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::s_2>{}, "s_2"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::p_3>{}, "p_3"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::q_3>{}, "q_3"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::i_3>{}, "i_3"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::s_3>{}, "s_3"));
        return attributes;
    }
};

template <bool sym>
struct get_attributes_list<ApplianceOutput<sym>> {
    std::vector<MetaAttribute> operator() () const {
        std::vector<MetaAttribute> attributes{};
        // all attributes including base class
        
        attributes.push_back(MetaAttribute(MetaAttributeImpl<ApplianceOutput<sym>, &ApplianceOutput<sym>::id>{}, "id"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<ApplianceOutput<sym>, &ApplianceOutput<sym>::energized>{}, "energized"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<ApplianceOutput<sym>, &ApplianceOutput<sym>::p>{}, "p"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<ApplianceOutput<sym>, &ApplianceOutput<sym>::q>{}, "q"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<ApplianceOutput<sym>, &ApplianceOutput<sym>::i>{}, "i"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<ApplianceOutput<sym>, &ApplianceOutput<sym>::s>{}, "s"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<ApplianceOutput<sym>, &ApplianceOutput<sym>::pf>{}, "pf"));
        return attributes;
    }
};

template <bool sym>
struct get_attributes_list<VoltageSensorOutput<sym>> {
    std::vector<MetaAttribute> operator() () const {
        std::vector<MetaAttribute> attributes{};
        // all attributes including base class
        
        attributes.push_back(MetaAttribute(MetaAttributeImpl<VoltageSensorOutput<sym>, &VoltageSensorOutput<sym>::id>{}, "id"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<VoltageSensorOutput<sym>, &VoltageSensorOutput<sym>::energized>{}, "energized"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<VoltageSensorOutput<sym>, &VoltageSensorOutput<sym>::u_residual>{}, "u_residual"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<VoltageSensorOutput<sym>, &VoltageSensorOutput<sym>::u_angle_residual>{}, "u_angle_residual"));
        return attributes;
    }
};

template <bool sym>
struct get_attributes_list<PowerSensorOutput<sym>> {
    std::vector<MetaAttribute> operator() () const {
        std::vector<MetaAttribute> attributes{};
        // all attributes including base class
        
        attributes.push_back(MetaAttribute(MetaAttributeImpl<PowerSensorOutput<sym>, &PowerSensorOutput<sym>::id>{}, "id"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<PowerSensorOutput<sym>, &PowerSensorOutput<sym>::energized>{}, "energized"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<PowerSensorOutput<sym>, &PowerSensorOutput<sym>::p_residual>{}, "p_residual"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<PowerSensorOutput<sym>, &PowerSensorOutput<sym>::q_residual>{}, "q_residual"));
        return attributes;
    }
};

template<>
struct get_attributes_list<FaultOutput> {
    std::vector<MetaAttribute> operator() () const {
        std::vector<MetaAttribute> attributes{};
        // all attributes including base class
        
        attributes.push_back(MetaAttribute(MetaAttributeImpl<FaultOutput, &FaultOutput::id>{}, "id"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<FaultOutput, &FaultOutput::energized>{}, "energized"));
        return attributes;
    }
};

template<>
struct get_attributes_list<FaultShortCircuitOutput> {
    std::vector<MetaAttribute> operator() () const {
        std::vector<MetaAttribute> attributes{};
        // all attributes including base class
        
        attributes.push_back(MetaAttribute(MetaAttributeImpl<FaultShortCircuitOutput, &FaultShortCircuitOutput::id>{}, "id"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<FaultShortCircuitOutput, &FaultShortCircuitOutput::energized>{}, "energized"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<FaultShortCircuitOutput, &FaultShortCircuitOutput::i_f>{}, "i_f"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<FaultShortCircuitOutput, &FaultShortCircuitOutput::i_f_angle>{}, "i_f_angle"));
        return attributes;
    }
};

template<>
struct get_attributes_list<NodeShortCircuitOutput> {
    std::vector<MetaAttribute> operator() () const {
        std::vector<MetaAttribute> attributes{};
        // all attributes including base class
        
        attributes.push_back(MetaAttribute(MetaAttributeImpl<NodeShortCircuitOutput, &NodeShortCircuitOutput::id>{}, "id"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<NodeShortCircuitOutput, &NodeShortCircuitOutput::energized>{}, "energized"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<NodeShortCircuitOutput, &NodeShortCircuitOutput::u_pu>{}, "u_pu"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<NodeShortCircuitOutput, &NodeShortCircuitOutput::u>{}, "u"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<NodeShortCircuitOutput, &NodeShortCircuitOutput::u_angle>{}, "u_angle"));
        return attributes;
    }
};

template<>
struct get_attributes_list<BranchShortCircuitOutput> {
    std::vector<MetaAttribute> operator() () const {
        std::vector<MetaAttribute> attributes{};
        // all attributes including base class
        
        attributes.push_back(MetaAttribute(MetaAttributeImpl<BranchShortCircuitOutput, &BranchShortCircuitOutput::id>{}, "id"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<BranchShortCircuitOutput, &BranchShortCircuitOutput::energized>{}, "energized"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<BranchShortCircuitOutput, &BranchShortCircuitOutput::i_from>{}, "i_from"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<BranchShortCircuitOutput, &BranchShortCircuitOutput::i_from_angle>{}, "i_from_angle"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<BranchShortCircuitOutput, &BranchShortCircuitOutput::i_to>{}, "i_to"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<BranchShortCircuitOutput, &BranchShortCircuitOutput::i_to_angle>{}, "i_to_angle"));
        return attributes;
    }
};

template<>
struct get_attributes_list<Branch3ShortCircuitOutput> {
    std::vector<MetaAttribute> operator() () const {
        std::vector<MetaAttribute> attributes{};
        // all attributes including base class
        
        attributes.push_back(MetaAttribute(MetaAttributeImpl<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::id>{}, "id"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::energized>{}, "energized"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::i_1>{}, "i_1"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::i_1_angle>{}, "i_1_angle"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::i_2>{}, "i_2"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::i_2_angle>{}, "i_2_angle"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::i_3>{}, "i_3"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::i_3_angle>{}, "i_3_angle"));
        return attributes;
    }
};

template<>
struct get_attributes_list<ApplianceShortCircuitOutput> {
    std::vector<MetaAttribute> operator() () const {
        std::vector<MetaAttribute> attributes{};
        // all attributes including base class
        
        attributes.push_back(MetaAttribute(MetaAttributeImpl<ApplianceShortCircuitOutput, &ApplianceShortCircuitOutput::id>{}, "id"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<ApplianceShortCircuitOutput, &ApplianceShortCircuitOutput::energized>{}, "energized"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<ApplianceShortCircuitOutput, &ApplianceShortCircuitOutput::i>{}, "i"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<ApplianceShortCircuitOutput, &ApplianceShortCircuitOutput::i_angle>{}, "i_angle"));
        return attributes;
    }
};

template<>
struct get_attributes_list<SensorShortCircuitOutput> {
    std::vector<MetaAttribute> operator() () const {
        std::vector<MetaAttribute> attributes{};
        // all attributes including base class
        
        attributes.push_back(MetaAttribute(MetaAttributeImpl<SensorShortCircuitOutput, &SensorShortCircuitOutput::id>{}, "id"));
        attributes.push_back(MetaAttribute(MetaAttributeImpl<SensorShortCircuitOutput, &SensorShortCircuitOutput::energized>{}, "energized"));
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