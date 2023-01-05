// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

// This header file is automatically generated. DO NOT modify it manually!

// clang-format off
#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_UPDATE_HPP
#define POWER_GRID_MODEL_AUXILIARY_UPDATE_HPP

#include "../enum.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"
#include "meta_data.hpp"

namespace power_grid_model {

struct BaseUpdate {
    ID id;  // ID of the object
};

struct BranchUpdate : BaseUpdate {
    IntS from_status;  // If the branch is connected at each side
    IntS to_status;  // If the branch is connected at each side
};

struct Branch3Update : BaseUpdate {
    IntS status_1;  // If the branch is connected at each side
    IntS status_2;  // If the branch is connected at each side
    IntS status_3;  // If the branch is connected at each side
};

struct ApplianceUpdate : BaseUpdate {
    IntS status;  // If the appliance is connected
};

struct TransformerUpdate : BranchUpdate {
    IntS tap_pos;  // tap changer parameters
};

struct ThreeWindingTransformerUpdate : Branch3Update {
    IntS tap_pos;  // tap changer parameters
};

template <bool sym>
struct LoadGenUpdate : ApplianceUpdate {
    RealValue<sym> p_specified;  // Specified active/reactive power
    RealValue<sym> q_specified;  // Specified active/reactive power
};
using SymLoadGenUpdate = LoadGenUpdate<true>;
using AsymLoadGenUpdate = LoadGenUpdate<false>;

struct SourceUpdate : ApplianceUpdate {
    double u_ref;  // reference voltage
    double u_ref_angle;  // reference voltage
};

template <bool sym>
struct VoltageSensorUpdate : BaseUpdate {
    double u_sigma;  // sigma of error margin of voltage measurement
    RealValue<sym> u_measured;  // measured voltage magnitude and angle
    RealValue<sym> u_angle_measured;  // measured voltage magnitude and angle
};
using SymVoltageSensorUpdate = VoltageSensorUpdate<true>;
using AsymVoltageSensorUpdate = VoltageSensorUpdate<false>;

template <bool sym>
struct PowerSensorUpdate : BaseUpdate {
    double power_sigma;  // sigma of error margin of power measurement
    RealValue<sym> p_measured;  // measured active/reactive power
    RealValue<sym> q_measured;  // measured active/reactive power
};
using SymPowerSensorUpdate = PowerSensorUpdate<true>;
using AsymPowerSensorUpdate = PowerSensorUpdate<false>;



// template specialization functors to get meta data
namespace meta_data {

template<>
struct get_meta<BaseUpdate> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "BaseUpdate";      
        meta.size = sizeof(BaseUpdate);  
        meta.alignment = alignof(BaseUpdate);
        
        meta.attributes.push_back(get_data_attribute<&BaseUpdate::id>("id"));
        return meta;
    }
};

template<>
struct get_meta<BranchUpdate> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "BranchUpdate";      
        meta.size = sizeof(BranchUpdate);  
        meta.alignment = alignof(BranchUpdate);
        meta.attributes = get_meta<BaseUpdate>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&BranchUpdate::from_status>("from_status"));
        meta.attributes.push_back(get_data_attribute<&BranchUpdate::to_status>("to_status"));
        return meta;
    }
};

template<>
struct get_meta<Branch3Update> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "Branch3Update";      
        meta.size = sizeof(Branch3Update);  
        meta.alignment = alignof(Branch3Update);
        meta.attributes = get_meta<BaseUpdate>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&Branch3Update::status_1>("status_1"));
        meta.attributes.push_back(get_data_attribute<&Branch3Update::status_2>("status_2"));
        meta.attributes.push_back(get_data_attribute<&Branch3Update::status_3>("status_3"));
        return meta;
    }
};

template<>
struct get_meta<ApplianceUpdate> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "ApplianceUpdate";      
        meta.size = sizeof(ApplianceUpdate);  
        meta.alignment = alignof(ApplianceUpdate);
        meta.attributes = get_meta<BaseUpdate>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&ApplianceUpdate::status>("status"));
        return meta;
    }
};

template<>
struct get_meta<TransformerUpdate> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "TransformerUpdate";      
        meta.size = sizeof(TransformerUpdate);  
        meta.alignment = alignof(TransformerUpdate);
        meta.attributes = get_meta<BranchUpdate>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&TransformerUpdate::tap_pos>("tap_pos"));
        return meta;
    }
};

template<>
struct get_meta<ThreeWindingTransformerUpdate> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "ThreeWindingTransformerUpdate";      
        meta.size = sizeof(ThreeWindingTransformerUpdate);  
        meta.alignment = alignof(ThreeWindingTransformerUpdate);
        meta.attributes = get_meta<Branch3Update>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerUpdate::tap_pos>("tap_pos"));
        return meta;
    }
};

template <bool sym>
struct get_meta<LoadGenUpdate<sym>> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "LoadGenUpdate";      
        meta.size = sizeof(LoadGenUpdate<sym>);  
        meta.alignment = alignof(LoadGenUpdate<sym>);
        meta.attributes = get_meta<ApplianceUpdate>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&LoadGenUpdate<sym>::p_specified>("p_specified"));
        meta.attributes.push_back(get_data_attribute<&LoadGenUpdate<sym>::q_specified>("q_specified"));
        return meta;
    }
};

template<>
struct get_meta<SourceUpdate> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "SourceUpdate";      
        meta.size = sizeof(SourceUpdate);  
        meta.alignment = alignof(SourceUpdate);
        meta.attributes = get_meta<ApplianceUpdate>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&SourceUpdate::u_ref>("u_ref"));
        meta.attributes.push_back(get_data_attribute<&SourceUpdate::u_ref_angle>("u_ref_angle"));
        return meta;
    }
};

template <bool sym>
struct get_meta<VoltageSensorUpdate<sym>> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "VoltageSensorUpdate";      
        meta.size = sizeof(VoltageSensorUpdate<sym>);  
        meta.alignment = alignof(VoltageSensorUpdate<sym>);
        meta.attributes = get_meta<BaseUpdate>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&VoltageSensorUpdate<sym>::u_sigma>("u_sigma"));
        meta.attributes.push_back(get_data_attribute<&VoltageSensorUpdate<sym>::u_measured>("u_measured"));
        meta.attributes.push_back(get_data_attribute<&VoltageSensorUpdate<sym>::u_angle_measured>("u_angle_measured"));
        return meta;
    }
};

template <bool sym>
struct get_meta<PowerSensorUpdate<sym>> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "PowerSensorUpdate";      
        meta.size = sizeof(PowerSensorUpdate<sym>);  
        meta.alignment = alignof(PowerSensorUpdate<sym>);
        meta.attributes = get_meta<BaseUpdate>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&PowerSensorUpdate<sym>::power_sigma>("power_sigma"));
        meta.attributes.push_back(get_data_attribute<&PowerSensorUpdate<sym>::p_measured>("p_measured"));
        meta.attributes.push_back(get_data_attribute<&PowerSensorUpdate<sym>::q_measured>("q_measured"));
        return meta;
    }
};



} // namespace meta_data

} // namespace power_grid_model

#endif
// clang-format on