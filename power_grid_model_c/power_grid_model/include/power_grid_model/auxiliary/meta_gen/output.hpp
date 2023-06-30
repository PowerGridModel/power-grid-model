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

// template specialization functors to get meta data
namespace meta_data {

template<>
struct get_meta<BaseOutput> {
    MetaData operator() () const {
        MetaData meta{};
        meta.name = "BaseOutput";      
        meta.size = sizeof(BaseOutput);  
        meta.alignment = alignof(BaseOutput);

        // all attributes including base class
        
        meta.attributes.push_back(get_data_attribute<BaseOutput, &BaseOutput::id>("id"));
        meta.attributes.push_back(get_data_attribute<BaseOutput, &BaseOutput::energized>("energized"));
        return meta;
    }
};

template <bool sym>
struct get_meta<NodeOutput<sym>> {
    MetaData operator() () const {
        MetaData meta{};
        meta.name = "NodeOutput";      
        meta.size = sizeof(NodeOutput<sym>);  
        meta.alignment = alignof(NodeOutput<sym>);

        // all attributes including base class
        
        meta.attributes.push_back(get_data_attribute<NodeOutput<sym>, &NodeOutput<sym>::id>("id"));
        meta.attributes.push_back(get_data_attribute<NodeOutput<sym>, &NodeOutput<sym>::energized>("energized"));
        meta.attributes.push_back(get_data_attribute<NodeOutput<sym>, &NodeOutput<sym>::u_pu>("u_pu"));
        meta.attributes.push_back(get_data_attribute<NodeOutput<sym>, &NodeOutput<sym>::u>("u"));
        meta.attributes.push_back(get_data_attribute<NodeOutput<sym>, &NodeOutput<sym>::u_angle>("u_angle"));
        meta.attributes.push_back(get_data_attribute<NodeOutput<sym>, &NodeOutput<sym>::p>("p"));
        meta.attributes.push_back(get_data_attribute<NodeOutput<sym>, &NodeOutput<sym>::q>("q"));
        return meta;
    }
};

template <bool sym>
struct get_meta<BranchOutput<sym>> {
    MetaData operator() () const {
        MetaData meta{};
        meta.name = "BranchOutput";      
        meta.size = sizeof(BranchOutput<sym>);  
        meta.alignment = alignof(BranchOutput<sym>);

        // all attributes including base class
        
        meta.attributes.push_back(get_data_attribute<BranchOutput<sym>, &BranchOutput<sym>::id>("id"));
        meta.attributes.push_back(get_data_attribute<BranchOutput<sym>, &BranchOutput<sym>::energized>("energized"));
        meta.attributes.push_back(get_data_attribute<BranchOutput<sym>, &BranchOutput<sym>::loading>("loading"));
        meta.attributes.push_back(get_data_attribute<BranchOutput<sym>, &BranchOutput<sym>::p_from>("p_from"));
        meta.attributes.push_back(get_data_attribute<BranchOutput<sym>, &BranchOutput<sym>::q_from>("q_from"));
        meta.attributes.push_back(get_data_attribute<BranchOutput<sym>, &BranchOutput<sym>::i_from>("i_from"));
        meta.attributes.push_back(get_data_attribute<BranchOutput<sym>, &BranchOutput<sym>::s_from>("s_from"));
        meta.attributes.push_back(get_data_attribute<BranchOutput<sym>, &BranchOutput<sym>::p_to>("p_to"));
        meta.attributes.push_back(get_data_attribute<BranchOutput<sym>, &BranchOutput<sym>::q_to>("q_to"));
        meta.attributes.push_back(get_data_attribute<BranchOutput<sym>, &BranchOutput<sym>::i_to>("i_to"));
        meta.attributes.push_back(get_data_attribute<BranchOutput<sym>, &BranchOutput<sym>::s_to>("s_to"));
        return meta;
    }
};

template <bool sym>
struct get_meta<Branch3Output<sym>> {
    MetaData operator() () const {
        MetaData meta{};
        meta.name = "Branch3Output";      
        meta.size = sizeof(Branch3Output<sym>);  
        meta.alignment = alignof(Branch3Output<sym>);

        // all attributes including base class
        
        meta.attributes.push_back(get_data_attribute<Branch3Output<sym>, &Branch3Output<sym>::id>("id"));
        meta.attributes.push_back(get_data_attribute<Branch3Output<sym>, &Branch3Output<sym>::energized>("energized"));
        meta.attributes.push_back(get_data_attribute<Branch3Output<sym>, &Branch3Output<sym>::loading>("loading"));
        meta.attributes.push_back(get_data_attribute<Branch3Output<sym>, &Branch3Output<sym>::p_1>("p_1"));
        meta.attributes.push_back(get_data_attribute<Branch3Output<sym>, &Branch3Output<sym>::q_1>("q_1"));
        meta.attributes.push_back(get_data_attribute<Branch3Output<sym>, &Branch3Output<sym>::i_1>("i_1"));
        meta.attributes.push_back(get_data_attribute<Branch3Output<sym>, &Branch3Output<sym>::s_1>("s_1"));
        meta.attributes.push_back(get_data_attribute<Branch3Output<sym>, &Branch3Output<sym>::p_2>("p_2"));
        meta.attributes.push_back(get_data_attribute<Branch3Output<sym>, &Branch3Output<sym>::q_2>("q_2"));
        meta.attributes.push_back(get_data_attribute<Branch3Output<sym>, &Branch3Output<sym>::i_2>("i_2"));
        meta.attributes.push_back(get_data_attribute<Branch3Output<sym>, &Branch3Output<sym>::s_2>("s_2"));
        meta.attributes.push_back(get_data_attribute<Branch3Output<sym>, &Branch3Output<sym>::p_3>("p_3"));
        meta.attributes.push_back(get_data_attribute<Branch3Output<sym>, &Branch3Output<sym>::q_3>("q_3"));
        meta.attributes.push_back(get_data_attribute<Branch3Output<sym>, &Branch3Output<sym>::i_3>("i_3"));
        meta.attributes.push_back(get_data_attribute<Branch3Output<sym>, &Branch3Output<sym>::s_3>("s_3"));
        return meta;
    }
};

template <bool sym>
struct get_meta<ApplianceOutput<sym>> {
    MetaData operator() () const {
        MetaData meta{};
        meta.name = "ApplianceOutput";      
        meta.size = sizeof(ApplianceOutput<sym>);  
        meta.alignment = alignof(ApplianceOutput<sym>);

        // all attributes including base class
        
        meta.attributes.push_back(get_data_attribute<ApplianceOutput<sym>, &ApplianceOutput<sym>::id>("id"));
        meta.attributes.push_back(get_data_attribute<ApplianceOutput<sym>, &ApplianceOutput<sym>::energized>("energized"));
        meta.attributes.push_back(get_data_attribute<ApplianceOutput<sym>, &ApplianceOutput<sym>::p>("p"));
        meta.attributes.push_back(get_data_attribute<ApplianceOutput<sym>, &ApplianceOutput<sym>::q>("q"));
        meta.attributes.push_back(get_data_attribute<ApplianceOutput<sym>, &ApplianceOutput<sym>::i>("i"));
        meta.attributes.push_back(get_data_attribute<ApplianceOutput<sym>, &ApplianceOutput<sym>::s>("s"));
        meta.attributes.push_back(get_data_attribute<ApplianceOutput<sym>, &ApplianceOutput<sym>::pf>("pf"));
        return meta;
    }
};

template <bool sym>
struct get_meta<VoltageSensorOutput<sym>> {
    MetaData operator() () const {
        MetaData meta{};
        meta.name = "VoltageSensorOutput";      
        meta.size = sizeof(VoltageSensorOutput<sym>);  
        meta.alignment = alignof(VoltageSensorOutput<sym>);

        // all attributes including base class
        
        meta.attributes.push_back(get_data_attribute<VoltageSensorOutput<sym>, &VoltageSensorOutput<sym>::id>("id"));
        meta.attributes.push_back(get_data_attribute<VoltageSensorOutput<sym>, &VoltageSensorOutput<sym>::energized>("energized"));
        meta.attributes.push_back(get_data_attribute<VoltageSensorOutput<sym>, &VoltageSensorOutput<sym>::u_residual>("u_residual"));
        meta.attributes.push_back(get_data_attribute<VoltageSensorOutput<sym>, &VoltageSensorOutput<sym>::u_angle_residual>("u_angle_residual"));
        return meta;
    }
};

template <bool sym>
struct get_meta<PowerSensorOutput<sym>> {
    MetaData operator() () const {
        MetaData meta{};
        meta.name = "PowerSensorOutput";      
        meta.size = sizeof(PowerSensorOutput<sym>);  
        meta.alignment = alignof(PowerSensorOutput<sym>);

        // all attributes including base class
        
        meta.attributes.push_back(get_data_attribute<PowerSensorOutput<sym>, &PowerSensorOutput<sym>::id>("id"));
        meta.attributes.push_back(get_data_attribute<PowerSensorOutput<sym>, &PowerSensorOutput<sym>::energized>("energized"));
        meta.attributes.push_back(get_data_attribute<PowerSensorOutput<sym>, &PowerSensorOutput<sym>::p_residual>("p_residual"));
        meta.attributes.push_back(get_data_attribute<PowerSensorOutput<sym>, &PowerSensorOutput<sym>::q_residual>("q_residual"));
        return meta;
    }
};

template<>
struct get_meta<FaultOutput> {
    MetaData operator() () const {
        MetaData meta{};
        meta.name = "FaultOutput";      
        meta.size = sizeof(FaultOutput);  
        meta.alignment = alignof(FaultOutput);

        // all attributes including base class
        
        meta.attributes.push_back(get_data_attribute<FaultOutput, &FaultOutput::id>("id"));
        meta.attributes.push_back(get_data_attribute<FaultOutput, &FaultOutput::energized>("energized"));
        return meta;
    }
};

template<>
struct get_meta<FaultShortCircuitOutput> {
    MetaData operator() () const {
        MetaData meta{};
        meta.name = "FaultShortCircuitOutput";      
        meta.size = sizeof(FaultShortCircuitOutput);  
        meta.alignment = alignof(FaultShortCircuitOutput);

        // all attributes including base class
        
        meta.attributes.push_back(get_data_attribute<FaultShortCircuitOutput, &FaultShortCircuitOutput::id>("id"));
        meta.attributes.push_back(get_data_attribute<FaultShortCircuitOutput, &FaultShortCircuitOutput::energized>("energized"));
        meta.attributes.push_back(get_data_attribute<FaultShortCircuitOutput, &FaultShortCircuitOutput::i_f>("i_f"));
        meta.attributes.push_back(get_data_attribute<FaultShortCircuitOutput, &FaultShortCircuitOutput::i_f_angle>("i_f_angle"));
        return meta;
    }
};

template<>
struct get_meta<NodeShortCircuitOutput> {
    MetaData operator() () const {
        MetaData meta{};
        meta.name = "NodeShortCircuitOutput";      
        meta.size = sizeof(NodeShortCircuitOutput);  
        meta.alignment = alignof(NodeShortCircuitOutput);

        // all attributes including base class
        
        meta.attributes.push_back(get_data_attribute<NodeShortCircuitOutput, &NodeShortCircuitOutput::id>("id"));
        meta.attributes.push_back(get_data_attribute<NodeShortCircuitOutput, &NodeShortCircuitOutput::energized>("energized"));
        meta.attributes.push_back(get_data_attribute<NodeShortCircuitOutput, &NodeShortCircuitOutput::u_pu>("u_pu"));
        meta.attributes.push_back(get_data_attribute<NodeShortCircuitOutput, &NodeShortCircuitOutput::u>("u"));
        meta.attributes.push_back(get_data_attribute<NodeShortCircuitOutput, &NodeShortCircuitOutput::u_angle>("u_angle"));
        return meta;
    }
};

template<>
struct get_meta<BranchShortCircuitOutput> {
    MetaData operator() () const {
        MetaData meta{};
        meta.name = "BranchShortCircuitOutput";      
        meta.size = sizeof(BranchShortCircuitOutput);  
        meta.alignment = alignof(BranchShortCircuitOutput);

        // all attributes including base class
        
        meta.attributes.push_back(get_data_attribute<BranchShortCircuitOutput, &BranchShortCircuitOutput::id>("id"));
        meta.attributes.push_back(get_data_attribute<BranchShortCircuitOutput, &BranchShortCircuitOutput::energized>("energized"));
        meta.attributes.push_back(get_data_attribute<BranchShortCircuitOutput, &BranchShortCircuitOutput::i_from>("i_from"));
        meta.attributes.push_back(get_data_attribute<BranchShortCircuitOutput, &BranchShortCircuitOutput::i_from_angle>("i_from_angle"));
        meta.attributes.push_back(get_data_attribute<BranchShortCircuitOutput, &BranchShortCircuitOutput::i_to>("i_to"));
        meta.attributes.push_back(get_data_attribute<BranchShortCircuitOutput, &BranchShortCircuitOutput::i_to_angle>("i_to_angle"));
        return meta;
    }
};

template<>
struct get_meta<Branch3ShortCircuitOutput> {
    MetaData operator() () const {
        MetaData meta{};
        meta.name = "Branch3ShortCircuitOutput";      
        meta.size = sizeof(Branch3ShortCircuitOutput);  
        meta.alignment = alignof(Branch3ShortCircuitOutput);

        // all attributes including base class
        
        meta.attributes.push_back(get_data_attribute<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::id>("id"));
        meta.attributes.push_back(get_data_attribute<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::energized>("energized"));
        meta.attributes.push_back(get_data_attribute<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::i_1>("i_1"));
        meta.attributes.push_back(get_data_attribute<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::i_1_angle>("i_1_angle"));
        meta.attributes.push_back(get_data_attribute<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::i_2>("i_2"));
        meta.attributes.push_back(get_data_attribute<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::i_2_angle>("i_2_angle"));
        meta.attributes.push_back(get_data_attribute<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::i_3>("i_3"));
        meta.attributes.push_back(get_data_attribute<Branch3ShortCircuitOutput, &Branch3ShortCircuitOutput::i_3_angle>("i_3_angle"));
        return meta;
    }
};

template<>
struct get_meta<ApplianceShortCircuitOutput> {
    MetaData operator() () const {
        MetaData meta{};
        meta.name = "ApplianceShortCircuitOutput";      
        meta.size = sizeof(ApplianceShortCircuitOutput);  
        meta.alignment = alignof(ApplianceShortCircuitOutput);

        // all attributes including base class
        
        meta.attributes.push_back(get_data_attribute<ApplianceShortCircuitOutput, &ApplianceShortCircuitOutput::id>("id"));
        meta.attributes.push_back(get_data_attribute<ApplianceShortCircuitOutput, &ApplianceShortCircuitOutput::energized>("energized"));
        meta.attributes.push_back(get_data_attribute<ApplianceShortCircuitOutput, &ApplianceShortCircuitOutput::i>("i"));
        meta.attributes.push_back(get_data_attribute<ApplianceShortCircuitOutput, &ApplianceShortCircuitOutput::i_angle>("i_angle"));
        return meta;
    }
};

template<>
struct get_meta<SensorShortCircuitOutput> {
    MetaData operator() () const {
        MetaData meta{};
        meta.name = "SensorShortCircuitOutput";      
        meta.size = sizeof(SensorShortCircuitOutput);  
        meta.alignment = alignof(SensorShortCircuitOutput);

        // all attributes including base class
        
        meta.attributes.push_back(get_data_attribute<SensorShortCircuitOutput, &SensorShortCircuitOutput::id>("id"));
        meta.attributes.push_back(get_data_attribute<SensorShortCircuitOutput, &SensorShortCircuitOutput::energized>("energized"));
        return meta;
    }
};



} // namespace meta_data

} // namespace power_grid_model

#endif
// clang-format on