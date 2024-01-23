// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

// This header file is automatically generated. DO NOT modify it manually!

// clang-format off
#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_META_GEN_OUTPUT_HPP
#define POWER_GRID_MODEL_AUXILIARY_META_GEN_OUTPUT_HPP

#include "gen_getters.hpp" // NOLINT

#include "../../enum.hpp"               // NOLINT
#include "../../power_grid_model.hpp"   // NOLINT
#include "../../three_phase_tensor.hpp" // NOLINT
#include "../meta_data.hpp"             // NOLINT
#include "../output.hpp" // NOLINT


namespace power_grid_model::meta_data {

// template specialization to get list of attributes in the value field

template<>
struct get_attributes_list<BaseOutput> {
    static constexpr std::array<MetaAttribute, 2> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<BaseOutput, &BaseOutput::id, offsetof(BaseOutput, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<BaseOutput, &BaseOutput::energized, offsetof(BaseOutput, energized), []{ return "energized"; }>::value,
    };
};

template <bool sym>
struct get_attributes_list<NodeOutput<sym>> {
    static constexpr std::array<MetaAttribute, 7> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<NodeOutput<sym>, &NodeOutput<sym>::id, offsetof(NodeOutput<sym>, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<NodeOutput<sym>, &NodeOutput<sym>::energized, offsetof(NodeOutput<sym>, energized), []{ return "energized"; }>::value,
            meta_data_gen::get_meta_attribute<NodeOutput<sym>, &NodeOutput<sym>::u_pu, offsetof(NodeOutput<sym>, u_pu), []{ return "u_pu"; }>::value,
            meta_data_gen::get_meta_attribute<NodeOutput<sym>, &NodeOutput<sym>::u, offsetof(NodeOutput<sym>, u), []{ return "u"; }>::value,
            meta_data_gen::get_meta_attribute<NodeOutput<sym>, &NodeOutput<sym>::u_angle, offsetof(NodeOutput<sym>, u_angle), []{ return "u_angle"; }>::value,
            meta_data_gen::get_meta_attribute<NodeOutput<sym>, &NodeOutput<sym>::p, offsetof(NodeOutput<sym>, p), []{ return "p"; }>::value,
            meta_data_gen::get_meta_attribute<NodeOutput<sym>, &NodeOutput<sym>::q, offsetof(NodeOutput<sym>, q), []{ return "q"; }>::value,
    };
};

template <bool sym>
struct get_attributes_list<BranchOutput<sym>> {
    static constexpr std::array<MetaAttribute, 11> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<BranchOutput<sym>, &BranchOutput<sym>::id, offsetof(BranchOutput<sym>, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<BranchOutput<sym>, &BranchOutput<sym>::energized, offsetof(BranchOutput<sym>, energized), []{ return "energized"; }>::value,
            meta_data_gen::get_meta_attribute<BranchOutput<sym>, &BranchOutput<sym>::loading, offsetof(BranchOutput<sym>, loading), []{ return "loading"; }>::value,
            meta_data_gen::get_meta_attribute<BranchOutput<sym>, &BranchOutput<sym>::p_from, offsetof(BranchOutput<sym>, p_from), []{ return "p_from"; }>::value,
            meta_data_gen::get_meta_attribute<BranchOutput<sym>, &BranchOutput<sym>::q_from, offsetof(BranchOutput<sym>, q_from), []{ return "q_from"; }>::value,
            meta_data_gen::get_meta_attribute<BranchOutput<sym>, &BranchOutput<sym>::i_from, offsetof(BranchOutput<sym>, i_from), []{ return "i_from"; }>::value,
            meta_data_gen::get_meta_attribute<BranchOutput<sym>, &BranchOutput<sym>::s_from, offsetof(BranchOutput<sym>, s_from), []{ return "s_from"; }>::value,
            meta_data_gen::get_meta_attribute<BranchOutput<sym>, &BranchOutput<sym>::p_to, offsetof(BranchOutput<sym>, p_to), []{ return "p_to"; }>::value,
            meta_data_gen::get_meta_attribute<BranchOutput<sym>, &BranchOutput<sym>::q_to, offsetof(BranchOutput<sym>, q_to), []{ return "q_to"; }>::value,
            meta_data_gen::get_meta_attribute<BranchOutput<sym>, &BranchOutput<sym>::i_to, offsetof(BranchOutput<sym>, i_to), []{ return "i_to"; }>::value,
            meta_data_gen::get_meta_attribute<BranchOutput<sym>, &BranchOutput<sym>::s_to, offsetof(BranchOutput<sym>, s_to), []{ return "s_to"; }>::value,
    };
};

template <bool sym>
struct get_attributes_list<Branch3Output<sym>> {
    static constexpr std::array<MetaAttribute, 15> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<Branch3Output<sym>, &Branch3Output<sym>::id, offsetof(Branch3Output<sym>, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<Branch3Output<sym>, &Branch3Output<sym>::energized, offsetof(Branch3Output<sym>, energized), []{ return "energized"; }>::value,
            meta_data_gen::get_meta_attribute<Branch3Output<sym>, &Branch3Output<sym>::loading, offsetof(Branch3Output<sym>, loading), []{ return "loading"; }>::value,
            meta_data_gen::get_meta_attribute<Branch3Output<sym>, &Branch3Output<sym>::p_1, offsetof(Branch3Output<sym>, p_1), []{ return "p_1"; }>::value,
            meta_data_gen::get_meta_attribute<Branch3Output<sym>, &Branch3Output<sym>::q_1, offsetof(Branch3Output<sym>, q_1), []{ return "q_1"; }>::value,
            meta_data_gen::get_meta_attribute<Branch3Output<sym>, &Branch3Output<sym>::i_1, offsetof(Branch3Output<sym>, i_1), []{ return "i_1"; }>::value,
            meta_data_gen::get_meta_attribute<Branch3Output<sym>, &Branch3Output<sym>::s_1, offsetof(Branch3Output<sym>, s_1), []{ return "s_1"; }>::value,
            meta_data_gen::get_meta_attribute<Branch3Output<sym>, &Branch3Output<sym>::p_2, offsetof(Branch3Output<sym>, p_2), []{ return "p_2"; }>::value,
            meta_data_gen::get_meta_attribute<Branch3Output<sym>, &Branch3Output<sym>::q_2, offsetof(Branch3Output<sym>, q_2), []{ return "q_2"; }>::value,
            meta_data_gen::get_meta_attribute<Branch3Output<sym>, &Branch3Output<sym>::i_2, offsetof(Branch3Output<sym>, i_2), []{ return "i_2"; }>::value,
            meta_data_gen::get_meta_attribute<Branch3Output<sym>, &Branch3Output<sym>::s_2, offsetof(Branch3Output<sym>, s_2), []{ return "s_2"; }>::value,
            meta_data_gen::get_meta_attribute<Branch3Output<sym>, &Branch3Output<sym>::p_3, offsetof(Branch3Output<sym>, p_3), []{ return "p_3"; }>::value,
            meta_data_gen::get_meta_attribute<Branch3Output<sym>, &Branch3Output<sym>::q_3, offsetof(Branch3Output<sym>, q_3), []{ return "q_3"; }>::value,
            meta_data_gen::get_meta_attribute<Branch3Output<sym>, &Branch3Output<sym>::i_3, offsetof(Branch3Output<sym>, i_3), []{ return "i_3"; }>::value,
            meta_data_gen::get_meta_attribute<Branch3Output<sym>, &Branch3Output<sym>::s_3, offsetof(Branch3Output<sym>, s_3), []{ return "s_3"; }>::value,
    };
};

template <bool sym>
struct get_attributes_list<ApplianceOutput<sym>> {
    static constexpr std::array<MetaAttribute, 7> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<ApplianceOutput<sym>, &ApplianceOutput<sym>::id, offsetof(ApplianceOutput<sym>, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<ApplianceOutput<sym>, &ApplianceOutput<sym>::energized, offsetof(ApplianceOutput<sym>, energized), []{ return "energized"; }>::value,
            meta_data_gen::get_meta_attribute<ApplianceOutput<sym>, &ApplianceOutput<sym>::p, offsetof(ApplianceOutput<sym>, p), []{ return "p"; }>::value,
            meta_data_gen::get_meta_attribute<ApplianceOutput<sym>, &ApplianceOutput<sym>::q, offsetof(ApplianceOutput<sym>, q), []{ return "q"; }>::value,
            meta_data_gen::get_meta_attribute<ApplianceOutput<sym>, &ApplianceOutput<sym>::i, offsetof(ApplianceOutput<sym>, i), []{ return "i"; }>::value,
            meta_data_gen::get_meta_attribute<ApplianceOutput<sym>, &ApplianceOutput<sym>::s, offsetof(ApplianceOutput<sym>, s), []{ return "s"; }>::value,
            meta_data_gen::get_meta_attribute<ApplianceOutput<sym>, &ApplianceOutput<sym>::pf, offsetof(ApplianceOutput<sym>, pf), []{ return "pf"; }>::value,
    };
};

template <bool sym>
struct get_attributes_list<VoltageSensorOutput<sym>> {
    static constexpr std::array<MetaAttribute, 4> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<VoltageSensorOutput<sym>, &VoltageSensorOutput<sym>::id, offsetof(VoltageSensorOutput<sym>, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<VoltageSensorOutput<sym>, &VoltageSensorOutput<sym>::energized, offsetof(VoltageSensorOutput<sym>, energized), []{ return "energized"; }>::value,
            meta_data_gen::get_meta_attribute<VoltageSensorOutput<sym>, &VoltageSensorOutput<sym>::u_residual, offsetof(VoltageSensorOutput<sym>, u_residual), []{ return "u_residual"; }>::value,
            meta_data_gen::get_meta_attribute<VoltageSensorOutput<sym>, &VoltageSensorOutput<sym>::u_angle_residual, offsetof(VoltageSensorOutput<sym>, u_angle_residual), []{ return "u_angle_residual"; }>::value,
    };
};

template <bool sym>
struct get_attributes_list<PowerSensorOutput<sym>> {
    static constexpr std::array<MetaAttribute, 4> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<PowerSensorOutput<sym>, &PowerSensorOutput<sym>::id, offsetof(PowerSensorOutput<sym>, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<PowerSensorOutput<sym>, &PowerSensorOutput<sym>::energized, offsetof(PowerSensorOutput<sym>, energized), []{ return "energized"; }>::value,
            meta_data_gen::get_meta_attribute<PowerSensorOutput<sym>, &PowerSensorOutput<sym>::p_residual, offsetof(PowerSensorOutput<sym>, p_residual), []{ return "p_residual"; }>::value,
            meta_data_gen::get_meta_attribute<PowerSensorOutput<sym>, &PowerSensorOutput<sym>::q_residual, offsetof(PowerSensorOutput<sym>, q_residual), []{ return "q_residual"; }>::value,
    };
};

template<>
struct get_attributes_list<FaultOutput> {
    static constexpr std::array<MetaAttribute, 2> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<FaultOutput, &FaultOutput::id, offsetof(FaultOutput, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<FaultOutput, &FaultOutput::energized, offsetof(FaultOutput, energized), []{ return "energized"; }>::value,
    };
};

template<>
struct get_attributes_list<FaultShortCircuitOutput> {
    static constexpr std::array<MetaAttribute, 4> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<FaultShortCircuitOutput, &FaultShortCircuitOutput::id, offsetof(FaultShortCircuitOutput, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<FaultShortCircuitOutput, &FaultShortCircuitOutput::energized, offsetof(FaultShortCircuitOutput, energized), []{ return "energized"; }>::value,
            meta_data_gen::get_meta_attribute<FaultShortCircuitOutput, &FaultShortCircuitOutput::i_f, offsetof(FaultShortCircuitOutput, i_f), []{ return "i_f"; }>::value,
            meta_data_gen::get_meta_attribute<FaultShortCircuitOutput, &FaultShortCircuitOutput::i_f_angle, offsetof(FaultShortCircuitOutput, i_f_angle), []{ return "i_f_angle"; }>::value,
    };
};

template<>
struct get_attributes_list<NodeShortCircuitOutput> {
    static constexpr std::array<MetaAttribute, 5> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<NodeShortCircuitOutput, &NodeShortCircuitOutput::id, offsetof(NodeShortCircuitOutput, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<NodeShortCircuitOutput, &NodeShortCircuitOutput::energized, offsetof(NodeShortCircuitOutput, energized), []{ return "energized"; }>::value,
            meta_data_gen::get_meta_attribute<NodeShortCircuitOutput, &NodeShortCircuitOutput::u_pu, offsetof(NodeShortCircuitOutput, u_pu), []{ return "u_pu"; }>::value,
            meta_data_gen::get_meta_attribute<NodeShortCircuitOutput, &NodeShortCircuitOutput::u, offsetof(NodeShortCircuitOutput, u), []{ return "u"; }>::value,
            meta_data_gen::get_meta_attribute<NodeShortCircuitOutput, &NodeShortCircuitOutput::u_angle, offsetof(NodeShortCircuitOutput, u_angle), []{ return "u_angle"; }>::value,
    };
};

template<>
struct get_attributes_list<BranchShortCircuitOutput> {
    static constexpr std::array<MetaAttribute, 6> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<BranchShortCircuitOutput, &BranchShortCircuitOutput::id, offsetof(BranchShortCircuitOutput, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<BranchShortCircuitOutput, &BranchShortCircuitOutput::energized, offsetof(BranchShortCircuitOutput, energized), []{ return "energized"; }>::value,
            meta_data_gen::get_meta_attribute<BranchShortCircuitOutput, &BranchShortCircuitOutput::i_from, offsetof(BranchShortCircuitOutput, i_from), []{ return "i_from"; }>::value,
            meta_data_gen::get_meta_attribute<BranchShortCircuitOutput, &BranchShortCircuitOutput::i_from_angle, offsetof(BranchShortCircuitOutput, i_from_angle), []{ return "i_from_angle"; }>::value,
            meta_data_gen::get_meta_attribute<BranchShortCircuitOutput, &BranchShortCircuitOutput::i_to, offsetof(BranchShortCircuitOutput, i_to), []{ return "i_to"; }>::value,
            meta_data_gen::get_meta_attribute<BranchShortCircuitOutput, &BranchShortCircuitOutput::i_to_angle, offsetof(BranchShortCircuitOutput, i_to_angle), []{ return "i_to_angle"; }>::value,
    };
};

template<>
struct get_attributes_list<Branch3ShortCircuitOutput> {
    static constexpr std::array<MetaAttribute, 8> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::id, offsetof(Branch3ShortCircuitOutput, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::energized, offsetof(Branch3ShortCircuitOutput, energized), []{ return "energized"; }>::value,
            meta_data_gen::get_meta_attribute<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::i_1, offsetof(Branch3ShortCircuitOutput, i_1), []{ return "i_1"; }>::value,
            meta_data_gen::get_meta_attribute<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::i_1_angle, offsetof(Branch3ShortCircuitOutput, i_1_angle), []{ return "i_1_angle"; }>::value,
            meta_data_gen::get_meta_attribute<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::i_2, offsetof(Branch3ShortCircuitOutput, i_2), []{ return "i_2"; }>::value,
            meta_data_gen::get_meta_attribute<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::i_2_angle, offsetof(Branch3ShortCircuitOutput, i_2_angle), []{ return "i_2_angle"; }>::value,
            meta_data_gen::get_meta_attribute<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::i_3, offsetof(Branch3ShortCircuitOutput, i_3), []{ return "i_3"; }>::value,
            meta_data_gen::get_meta_attribute<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::i_3_angle, offsetof(Branch3ShortCircuitOutput, i_3_angle), []{ return "i_3_angle"; }>::value,
    };
};

template<>
struct get_attributes_list<ApplianceShortCircuitOutput> {
    static constexpr std::array<MetaAttribute, 4> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<ApplianceShortCircuitOutput, &ApplianceShortCircuitOutput::id, offsetof(ApplianceShortCircuitOutput, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<ApplianceShortCircuitOutput, &ApplianceShortCircuitOutput::energized, offsetof(ApplianceShortCircuitOutput, energized), []{ return "energized"; }>::value,
            meta_data_gen::get_meta_attribute<ApplianceShortCircuitOutput, &ApplianceShortCircuitOutput::i, offsetof(ApplianceShortCircuitOutput, i), []{ return "i"; }>::value,
            meta_data_gen::get_meta_attribute<ApplianceShortCircuitOutput, &ApplianceShortCircuitOutput::i_angle, offsetof(ApplianceShortCircuitOutput, i_angle), []{ return "i_angle"; }>::value,
    };
};

template<>
struct get_attributes_list<SensorShortCircuitOutput> {
    static constexpr std::array<MetaAttribute, 2> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<SensorShortCircuitOutput, &SensorShortCircuitOutput::id, offsetof(SensorShortCircuitOutput, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<SensorShortCircuitOutput, &SensorShortCircuitOutput::energized, offsetof(SensorShortCircuitOutput, energized), []{ return "energized"; }>::value,
    };
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