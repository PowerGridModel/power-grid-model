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
struct get_attributes_list<BaseUpdate> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<BaseUpdate, &BaseUpdate::id> const>("id"));
        return attributes;
    }
};

template<>
struct get_attributes_list<BranchUpdate> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<BranchUpdate, &BranchUpdate::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<BranchUpdate, &BranchUpdate::from_status> const>("from_status"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<BranchUpdate, &BranchUpdate::to_status> const>("to_status"));
        return attributes;
    }
};

template<>
struct get_attributes_list<Branch3Update> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3Update, &Branch3Update::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3Update, &Branch3Update::status_1> const>("status_1"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3Update, &Branch3Update::status_2> const>("status_2"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<Branch3Update, &Branch3Update::status_3> const>("status_3"));
        return attributes;
    }
};

template<>
struct get_attributes_list<ApplianceUpdate> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<ApplianceUpdate, &ApplianceUpdate::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ApplianceUpdate, &ApplianceUpdate::status> const>("status"));
        return attributes;
    }
};

template<>
struct get_attributes_list<TransformerUpdate> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerUpdate, &TransformerUpdate::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerUpdate, &TransformerUpdate::from_status> const>("from_status"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerUpdate, &TransformerUpdate::to_status> const>("to_status"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<TransformerUpdate, &TransformerUpdate::tap_pos> const>("tap_pos"));
        return attributes;
    }
};

template<>
struct get_attributes_list<ThreeWindingTransformerUpdate> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerUpdate, &ThreeWindingTransformerUpdate::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerUpdate, &ThreeWindingTransformerUpdate::status_1> const>("status_1"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerUpdate, &ThreeWindingTransformerUpdate::status_2> const>("status_2"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerUpdate, &ThreeWindingTransformerUpdate::status_3> const>("status_3"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<ThreeWindingTransformerUpdate, &ThreeWindingTransformerUpdate::tap_pos> const>("tap_pos"));
        return attributes;
    }
};

template <bool sym>
struct get_attributes_list<LoadGenUpdate<sym>> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<LoadGenUpdate<sym>, &LoadGenUpdate<sym>::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<LoadGenUpdate<sym>, &LoadGenUpdate<sym>::status> const>("status"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<LoadGenUpdate<sym>, &LoadGenUpdate<sym>::p_specified> const>("p_specified"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<LoadGenUpdate<sym>, &LoadGenUpdate<sym>::q_specified> const>("q_specified"));
        return attributes;
    }
};

template<>
struct get_attributes_list<SourceUpdate> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<SourceUpdate, &SourceUpdate::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<SourceUpdate, &SourceUpdate::status> const>("status"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<SourceUpdate, &SourceUpdate::u_ref> const>("u_ref"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<SourceUpdate, &SourceUpdate::u_ref_angle> const>("u_ref_angle"));
        return attributes;
    }
};

template <bool sym>
struct get_attributes_list<VoltageSensorUpdate<sym>> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<VoltageSensorUpdate<sym>, &VoltageSensorUpdate<sym>::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<VoltageSensorUpdate<sym>, &VoltageSensorUpdate<sym>::u_sigma> const>("u_sigma"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<VoltageSensorUpdate<sym>, &VoltageSensorUpdate<sym>::u_measured> const>("u_measured"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<VoltageSensorUpdate<sym>, &VoltageSensorUpdate<sym>::u_angle_measured> const>("u_angle_measured"));
        return attributes;
    }
};

template <bool sym>
struct get_attributes_list<PowerSensorUpdate<sym>> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<PowerSensorUpdate<sym>, &PowerSensorUpdate<sym>::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<PowerSensorUpdate<sym>, &PowerSensorUpdate<sym>::power_sigma> const>("power_sigma"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<PowerSensorUpdate<sym>, &PowerSensorUpdate<sym>::p_measured> const>("p_measured"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<PowerSensorUpdate<sym>, &PowerSensorUpdate<sym>::q_measured> const>("q_measured"));
        return attributes;
    }
};

template<>
struct get_attributes_list<FaultUpdate> {
    std::vector<std::unique_ptr<MetaAttribute const>> operator() () const {
        std::vector<std::unique_ptr<MetaAttribute const>> attributes{};
        // all attributes including base class
        
        attributes.push_back(std::make_unique<MetaAttributeImpl<FaultUpdate, &FaultUpdate::id> const>("id"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<FaultUpdate, &FaultUpdate::status> const>("status"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<FaultUpdate, &FaultUpdate::fault_type> const>("fault_type"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<FaultUpdate, &FaultUpdate::fault_phase> const>("fault_phase"));
        attributes.push_back(std::make_unique<MetaAttributeImpl<FaultUpdate, &FaultUpdate::fault_object> const>("fault_object"));
        return attributes;
    }
};



} // namespace meta_data

} // namespace power_grid_model

#endif
// clang-format on