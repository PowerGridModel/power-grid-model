// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

// This header file is automatically generated. DO NOT modify it manually!

// clang-format off
#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_META_GEN_OUTPUT_HPP
#define POWER_GRID_MODEL_AUXILIARY_META_GEN_OUTPUT_HPP

#include "../../enum.hpp" // NOLINT
#include "../../power_grid_model.hpp" // NOLINT
#include "../../three_phase_tensor.hpp" // NOLINT
#include "../meta_data.hpp" // NOLINT
#include "../output.hpp" // NOLINT


namespace power_grid_model::meta_data {

// template specialization functors to get attributes

template<>
struct get_attributes_list<BaseOutput> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<BaseOutput, &BaseOutput::id>{}, "id"},
            {MetaAttributeImpl<BaseOutput, &BaseOutput::energized>{}, "energized"},
        };
    }
};

template <bool sym>
struct get_attributes_list<NodeOutput<sym>> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<NodeOutput<sym>, &NodeOutput<sym>::id>{}, "id"},
            {MetaAttributeImpl<NodeOutput<sym>, &NodeOutput<sym>::energized>{}, "energized"},
            {MetaAttributeImpl<NodeOutput<sym>, &NodeOutput<sym>::u_pu>{}, "u_pu"},
            {MetaAttributeImpl<NodeOutput<sym>, &NodeOutput<sym>::u>{}, "u"},
            {MetaAttributeImpl<NodeOutput<sym>, &NodeOutput<sym>::u_angle>{}, "u_angle"},
            {MetaAttributeImpl<NodeOutput<sym>, &NodeOutput<sym>::p>{}, "p"},
            {MetaAttributeImpl<NodeOutput<sym>, &NodeOutput<sym>::q>{}, "q"},
        };
    }
};

template <bool sym>
struct get_attributes_list<BranchOutput<sym>> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::id>{}, "id"},
            {MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::energized>{}, "energized"},
            {MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::loading>{}, "loading"},
            {MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::p_from>{}, "p_from"},
            {MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::q_from>{}, "q_from"},
            {MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::i_from>{}, "i_from"},
            {MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::s_from>{}, "s_from"},
            {MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::p_to>{}, "p_to"},
            {MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::q_to>{}, "q_to"},
            {MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::i_to>{}, "i_to"},
            {MetaAttributeImpl<BranchOutput<sym>, &BranchOutput<sym>::s_to>{}, "s_to"},
        };
    }
};

template <bool sym>
struct get_attributes_list<Branch3Output<sym>> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::id>{}, "id"},
            {MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::energized>{}, "energized"},
            {MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::loading>{}, "loading"},
            {MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::p_1>{}, "p_1"},
            {MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::q_1>{}, "q_1"},
            {MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::i_1>{}, "i_1"},
            {MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::s_1>{}, "s_1"},
            {MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::p_2>{}, "p_2"},
            {MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::q_2>{}, "q_2"},
            {MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::i_2>{}, "i_2"},
            {MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::s_2>{}, "s_2"},
            {MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::p_3>{}, "p_3"},
            {MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::q_3>{}, "q_3"},
            {MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::i_3>{}, "i_3"},
            {MetaAttributeImpl<Branch3Output<sym>, &Branch3Output<sym>::s_3>{}, "s_3"},
        };
    }
};

template <bool sym>
struct get_attributes_list<ApplianceOutput<sym>> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<ApplianceOutput<sym>, &ApplianceOutput<sym>::id>{}, "id"},
            {MetaAttributeImpl<ApplianceOutput<sym>, &ApplianceOutput<sym>::energized>{}, "energized"},
            {MetaAttributeImpl<ApplianceOutput<sym>, &ApplianceOutput<sym>::p>{}, "p"},
            {MetaAttributeImpl<ApplianceOutput<sym>, &ApplianceOutput<sym>::q>{}, "q"},
            {MetaAttributeImpl<ApplianceOutput<sym>, &ApplianceOutput<sym>::i>{}, "i"},
            {MetaAttributeImpl<ApplianceOutput<sym>, &ApplianceOutput<sym>::s>{}, "s"},
            {MetaAttributeImpl<ApplianceOutput<sym>, &ApplianceOutput<sym>::pf>{}, "pf"},
        };
    }
};

template <bool sym>
struct get_attributes_list<VoltageSensorOutput<sym>> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<VoltageSensorOutput<sym>, &VoltageSensorOutput<sym>::id>{}, "id"},
            {MetaAttributeImpl<VoltageSensorOutput<sym>, &VoltageSensorOutput<sym>::energized>{}, "energized"},
            {MetaAttributeImpl<VoltageSensorOutput<sym>, &VoltageSensorOutput<sym>::u_residual>{}, "u_residual"},
            {MetaAttributeImpl<VoltageSensorOutput<sym>, &VoltageSensorOutput<sym>::u_angle_residual>{}, "u_angle_residual"},
        };
    }
};

template <bool sym>
struct get_attributes_list<PowerSensorOutput<sym>> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<PowerSensorOutput<sym>, &PowerSensorOutput<sym>::id>{}, "id"},
            {MetaAttributeImpl<PowerSensorOutput<sym>, &PowerSensorOutput<sym>::energized>{}, "energized"},
            {MetaAttributeImpl<PowerSensorOutput<sym>, &PowerSensorOutput<sym>::p_residual>{}, "p_residual"},
            {MetaAttributeImpl<PowerSensorOutput<sym>, &PowerSensorOutput<sym>::q_residual>{}, "q_residual"},
        };
    }
};

template<>
struct get_attributes_list<FaultOutput> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<FaultOutput, &FaultOutput::id>{}, "id"},
            {MetaAttributeImpl<FaultOutput, &FaultOutput::energized>{}, "energized"},
        };
    }
};

template<>
struct get_attributes_list<FaultShortCircuitOutput> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<FaultShortCircuitOutput, &FaultShortCircuitOutput::id>{}, "id"},
            {MetaAttributeImpl<FaultShortCircuitOutput, &FaultShortCircuitOutput::energized>{}, "energized"},
            {MetaAttributeImpl<FaultShortCircuitOutput, &FaultShortCircuitOutput::i_f>{}, "i_f"},
            {MetaAttributeImpl<FaultShortCircuitOutput, &FaultShortCircuitOutput::i_f_angle>{}, "i_f_angle"},
        };
    }
};

template<>
struct get_attributes_list<NodeShortCircuitOutput> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<NodeShortCircuitOutput, &NodeShortCircuitOutput::id>{}, "id"},
            {MetaAttributeImpl<NodeShortCircuitOutput, &NodeShortCircuitOutput::energized>{}, "energized"},
            {MetaAttributeImpl<NodeShortCircuitOutput, &NodeShortCircuitOutput::u_pu>{}, "u_pu"},
            {MetaAttributeImpl<NodeShortCircuitOutput, &NodeShortCircuitOutput::u>{}, "u"},
            {MetaAttributeImpl<NodeShortCircuitOutput, &NodeShortCircuitOutput::u_angle>{}, "u_angle"},
        };
    }
};

template<>
struct get_attributes_list<BranchShortCircuitOutput> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<BranchShortCircuitOutput, &BranchShortCircuitOutput::id>{}, "id"},
            {MetaAttributeImpl<BranchShortCircuitOutput, &BranchShortCircuitOutput::energized>{}, "energized"},
            {MetaAttributeImpl<BranchShortCircuitOutput, &BranchShortCircuitOutput::i_from>{}, "i_from"},
            {MetaAttributeImpl<BranchShortCircuitOutput, &BranchShortCircuitOutput::i_from_angle>{}, "i_from_angle"},
            {MetaAttributeImpl<BranchShortCircuitOutput, &BranchShortCircuitOutput::i_to>{}, "i_to"},
            {MetaAttributeImpl<BranchShortCircuitOutput, &BranchShortCircuitOutput::i_to_angle>{}, "i_to_angle"},
        };
    }
};

template<>
struct get_attributes_list<Branch3ShortCircuitOutput> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::id>{}, "id"},
            {MetaAttributeImpl<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::energized>{}, "energized"},
            {MetaAttributeImpl<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::i_1>{}, "i_1"},
            {MetaAttributeImpl<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::i_1_angle>{}, "i_1_angle"},
            {MetaAttributeImpl<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::i_2>{}, "i_2"},
            {MetaAttributeImpl<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::i_2_angle>{}, "i_2_angle"},
            {MetaAttributeImpl<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::i_3>{}, "i_3"},
            {MetaAttributeImpl<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::i_3_angle>{}, "i_3_angle"},
        };
    }
};

template<>
struct get_attributes_list<ApplianceShortCircuitOutput> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<ApplianceShortCircuitOutput, &ApplianceShortCircuitOutput::id>{}, "id"},
            {MetaAttributeImpl<ApplianceShortCircuitOutput, &ApplianceShortCircuitOutput::energized>{}, "energized"},
            {MetaAttributeImpl<ApplianceShortCircuitOutput, &ApplianceShortCircuitOutput::i>{}, "i"},
            {MetaAttributeImpl<ApplianceShortCircuitOutput, &ApplianceShortCircuitOutput::i_angle>{}, "i_angle"},
        };
    }
};

template<>
struct get_attributes_list<SensorShortCircuitOutput> {
    std::vector<MetaAttribute> operator() () const {
        // all attributes including base class
        return {
            
            {MetaAttributeImpl<SensorShortCircuitOutput, &SensorShortCircuitOutput::id>{}, "id"},
            {MetaAttributeImpl<SensorShortCircuitOutput, &SensorShortCircuitOutput::energized>{}, "energized"},
        };
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



} // namespace power_grid_model::meta_data

#endif
// clang-format on