// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

// This header file is automatically generated. DO NOT modify it manually!

// clang-format off
#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_META_GEN_UPDATE_HPP
#define POWER_GRID_MODEL_AUXILIARY_META_GEN_UPDATE_HPP

#include "../../enum.hpp"
#include "../../power_grid_model.hpp"
#include "../../three_phase_tensor.hpp"
#include "../meta_data.hpp"
#include "../update.hpp"


namespace power_grid_model {

// template specialization functors to get meta data
namespace meta_data {

template<>
struct get_meta<BaseUpdate> {
    MetaData operator() () const {
        MetaData meta{};
        meta.name = "BaseUpdate";      
        meta.size = sizeof(BaseUpdate);  
        meta.alignment = alignof(BaseUpdate);

        // all attributes including base class
        
        meta.attributes.push_back(get_data_attribute<BaseUpdate, &BaseUpdate::id>("id"));
        return meta;
    }
};

template<>
struct get_meta<BranchUpdate> {
    MetaData operator() () const {
        MetaData meta{};
        meta.name = "BranchUpdate";      
        meta.size = sizeof(BranchUpdate);  
        meta.alignment = alignof(BranchUpdate);

        // all attributes including base class
        
        meta.attributes.push_back(get_data_attribute<BranchUpdate, &BranchUpdate::id>("id"));
        meta.attributes.push_back(get_data_attribute<BranchUpdate, &BranchUpdate::from_status>("from_status"));
        meta.attributes.push_back(get_data_attribute<BranchUpdate, &BranchUpdate::to_status>("to_status"));
        return meta;
    }
};

template<>
struct get_meta<Branch3Update> {
    MetaData operator() () const {
        MetaData meta{};
        meta.name = "Branch3Update";      
        meta.size = sizeof(Branch3Update);  
        meta.alignment = alignof(Branch3Update);

        // all attributes including base class
        
        meta.attributes.push_back(get_data_attribute<Branch3Update, &Branch3Update::id>("id"));
        meta.attributes.push_back(get_data_attribute<Branch3Update, &Branch3Update::status_1>("status_1"));
        meta.attributes.push_back(get_data_attribute<Branch3Update, &Branch3Update::status_2>("status_2"));
        meta.attributes.push_back(get_data_attribute<Branch3Update, &Branch3Update::status_3>("status_3"));
        return meta;
    }
};

template<>
struct get_meta<ApplianceUpdate> {
    MetaData operator() () const {
        MetaData meta{};
        meta.name = "ApplianceUpdate";      
        meta.size = sizeof(ApplianceUpdate);  
        meta.alignment = alignof(ApplianceUpdate);

        // all attributes including base class
        
        meta.attributes.push_back(get_data_attribute<ApplianceUpdate, &ApplianceUpdate::id>("id"));
        meta.attributes.push_back(get_data_attribute<ApplianceUpdate, &ApplianceUpdate::status>("status"));
        return meta;
    }
};

template<>
struct get_meta<TransformerUpdate> {
    MetaData operator() () const {
        MetaData meta{};
        meta.name = "TransformerUpdate";      
        meta.size = sizeof(TransformerUpdate);  
        meta.alignment = alignof(TransformerUpdate);

        // all attributes including base class
        
        meta.attributes.push_back(get_data_attribute<TransformerUpdate, &TransformerUpdate::id>("id"));
        meta.attributes.push_back(get_data_attribute<TransformerUpdate, &TransformerUpdate::from_status>("from_status"));
        meta.attributes.push_back(get_data_attribute<TransformerUpdate, &TransformerUpdate::to_status>("to_status"));
        meta.attributes.push_back(get_data_attribute<TransformerUpdate, &TransformerUpdate::tap_pos>("tap_pos"));
        return meta;
    }
};

template<>
struct get_meta<ThreeWindingTransformerUpdate> {
    MetaData operator() () const {
        MetaData meta{};
        meta.name = "ThreeWindingTransformerUpdate";      
        meta.size = sizeof(ThreeWindingTransformerUpdate);  
        meta.alignment = alignof(ThreeWindingTransformerUpdate);

        // all attributes including base class
        
        meta.attributes.push_back(get_data_attribute<ThreeWindingTransformerUpdate, &ThreeWindingTransformerUpdate::id>("id"));
        meta.attributes.push_back(get_data_attribute<ThreeWindingTransformerUpdate, &ThreeWindingTransformerUpdate::status_1>("status_1"));
        meta.attributes.push_back(get_data_attribute<ThreeWindingTransformerUpdate, &ThreeWindingTransformerUpdate::status_2>("status_2"));
        meta.attributes.push_back(get_data_attribute<ThreeWindingTransformerUpdate, &ThreeWindingTransformerUpdate::status_3>("status_3"));
        meta.attributes.push_back(get_data_attribute<ThreeWindingTransformerUpdate, &ThreeWindingTransformerUpdate::tap_pos>("tap_pos"));
        return meta;
    }
};

template <bool sym>
struct get_meta<LoadGenUpdate<sym>> {
    MetaData operator() () const {
        MetaData meta{};
        meta.name = "LoadGenUpdate";      
        meta.size = sizeof(LoadGenUpdate<sym>);  
        meta.alignment = alignof(LoadGenUpdate<sym>);

        // all attributes including base class
        
        meta.attributes.push_back(get_data_attribute<LoadGenUpdate<sym>, &LoadGenUpdate<sym>::id>("id"));
        meta.attributes.push_back(get_data_attribute<LoadGenUpdate<sym>, &LoadGenUpdate<sym>::status>("status"));
        meta.attributes.push_back(get_data_attribute<LoadGenUpdate<sym>, &LoadGenUpdate<sym>::p_specified>("p_specified"));
        meta.attributes.push_back(get_data_attribute<LoadGenUpdate<sym>, &LoadGenUpdate<sym>::q_specified>("q_specified"));
        return meta;
    }
};

template<>
struct get_meta<SourceUpdate> {
    MetaData operator() () const {
        MetaData meta{};
        meta.name = "SourceUpdate";      
        meta.size = sizeof(SourceUpdate);  
        meta.alignment = alignof(SourceUpdate);

        // all attributes including base class
        
        meta.attributes.push_back(get_data_attribute<SourceUpdate, &SourceUpdate::id>("id"));
        meta.attributes.push_back(get_data_attribute<SourceUpdate, &SourceUpdate::status>("status"));
        meta.attributes.push_back(get_data_attribute<SourceUpdate, &SourceUpdate::u_ref>("u_ref"));
        meta.attributes.push_back(get_data_attribute<SourceUpdate, &SourceUpdate::u_ref_angle>("u_ref_angle"));
        return meta;
    }
};

template <bool sym>
struct get_meta<VoltageSensorUpdate<sym>> {
    MetaData operator() () const {
        MetaData meta{};
        meta.name = "VoltageSensorUpdate";      
        meta.size = sizeof(VoltageSensorUpdate<sym>);  
        meta.alignment = alignof(VoltageSensorUpdate<sym>);

        // all attributes including base class
        
        meta.attributes.push_back(get_data_attribute<VoltageSensorUpdate<sym>, &VoltageSensorUpdate<sym>::id>("id"));
        meta.attributes.push_back(get_data_attribute<VoltageSensorUpdate<sym>, &VoltageSensorUpdate<sym>::u_sigma>("u_sigma"));
        meta.attributes.push_back(get_data_attribute<VoltageSensorUpdate<sym>, &VoltageSensorUpdate<sym>::u_measured>("u_measured"));
        meta.attributes.push_back(get_data_attribute<VoltageSensorUpdate<sym>, &VoltageSensorUpdate<sym>::u_angle_measured>("u_angle_measured"));
        return meta;
    }
};

template <bool sym>
struct get_meta<PowerSensorUpdate<sym>> {
    MetaData operator() () const {
        MetaData meta{};
        meta.name = "PowerSensorUpdate";      
        meta.size = sizeof(PowerSensorUpdate<sym>);  
        meta.alignment = alignof(PowerSensorUpdate<sym>);

        // all attributes including base class
        
        meta.attributes.push_back(get_data_attribute<PowerSensorUpdate<sym>, &PowerSensorUpdate<sym>::id>("id"));
        meta.attributes.push_back(get_data_attribute<PowerSensorUpdate<sym>, &PowerSensorUpdate<sym>::power_sigma>("power_sigma"));
        meta.attributes.push_back(get_data_attribute<PowerSensorUpdate<sym>, &PowerSensorUpdate<sym>::p_measured>("p_measured"));
        meta.attributes.push_back(get_data_attribute<PowerSensorUpdate<sym>, &PowerSensorUpdate<sym>::q_measured>("q_measured"));
        return meta;
    }
};

template<>
struct get_meta<FaultUpdate> {
    MetaData operator() () const {
        MetaData meta{};
        meta.name = "FaultUpdate";      
        meta.size = sizeof(FaultUpdate);  
        meta.alignment = alignof(FaultUpdate);

        // all attributes including base class
        
        meta.attributes.push_back(get_data_attribute<FaultUpdate, &FaultUpdate::id>("id"));
        meta.attributes.push_back(get_data_attribute<FaultUpdate, &FaultUpdate::status>("status"));
        meta.attributes.push_back(get_data_attribute<FaultUpdate, &FaultUpdate::fault_type>("fault_type"));
        meta.attributes.push_back(get_data_attribute<FaultUpdate, &FaultUpdate::fault_phase>("fault_phase"));
        meta.attributes.push_back(get_data_attribute<FaultUpdate, &FaultUpdate::fault_object>("fault_object"));
        return meta;
    }
};



} // namespace meta_data

} // namespace power_grid_model

#endif
// clang-format on