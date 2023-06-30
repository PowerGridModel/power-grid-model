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

namespace meta_data {

// template specialization functors to get attributes

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



// template specialization functors to get nan

template<>
struct get_component_nan<BaseUpdate> {
    BaseUpdate operator() () const {
        BaseUpdate comp;
        // all attributes including base class
        
        set_nan(comp.id);
        return comp;
    }
};

template<>
struct get_component_nan<BranchUpdate> {
    BranchUpdate operator() () const {
        BranchUpdate comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.from_status);
        set_nan(comp.to_status);
        return comp;
    }
};

template<>
struct get_component_nan<Branch3Update> {
    Branch3Update operator() () const {
        Branch3Update comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.status_1);
        set_nan(comp.status_2);
        set_nan(comp.status_3);
        return comp;
    }
};

template<>
struct get_component_nan<ApplianceUpdate> {
    ApplianceUpdate operator() () const {
        ApplianceUpdate comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.status);
        return comp;
    }
};

template<>
struct get_component_nan<TransformerUpdate> {
    TransformerUpdate operator() () const {
        TransformerUpdate comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.from_status);
        set_nan(comp.to_status);
        set_nan(comp.tap_pos);
        return comp;
    }
};

template<>
struct get_component_nan<ThreeWindingTransformerUpdate> {
    ThreeWindingTransformerUpdate operator() () const {
        ThreeWindingTransformerUpdate comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.status_1);
        set_nan(comp.status_2);
        set_nan(comp.status_3);
        set_nan(comp.tap_pos);
        return comp;
    }
};

template <bool sym>
struct get_component_nan<LoadGenUpdate<sym>> {
    LoadGenUpdate<sym> operator() () const {
        LoadGenUpdate<sym> comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.status);
        set_nan(comp.p_specified);
        set_nan(comp.q_specified);
        return comp;
    }
};

template<>
struct get_component_nan<SourceUpdate> {
    SourceUpdate operator() () const {
        SourceUpdate comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.status);
        set_nan(comp.u_ref);
        set_nan(comp.u_ref_angle);
        return comp;
    }
};

template <bool sym>
struct get_component_nan<VoltageSensorUpdate<sym>> {
    VoltageSensorUpdate<sym> operator() () const {
        VoltageSensorUpdate<sym> comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.u_sigma);
        set_nan(comp.u_measured);
        set_nan(comp.u_angle_measured);
        return comp;
    }
};

template <bool sym>
struct get_component_nan<PowerSensorUpdate<sym>> {
    PowerSensorUpdate<sym> operator() () const {
        PowerSensorUpdate<sym> comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.power_sigma);
        set_nan(comp.p_measured);
        set_nan(comp.q_measured);
        return comp;
    }
};

template<>
struct get_component_nan<FaultUpdate> {
    FaultUpdate operator() () const {
        FaultUpdate comp;
        // all attributes including base class
        
        set_nan(comp.id);
        set_nan(comp.status);
        set_nan(comp.fault_type);
        set_nan(comp.fault_phase);
        set_nan(comp.fault_object);
        return comp;
    }
};



} // namespace meta_data

} // namespace power_grid_model

#endif
// clang-format on