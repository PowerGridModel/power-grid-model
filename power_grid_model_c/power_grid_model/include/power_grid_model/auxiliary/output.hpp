// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

// This header file is automatically generated. DO NOT modify it manually!

// clang-format off
#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_OUTPUT_HPP
#define POWER_GRID_MODEL_AUXILIARY_OUTPUT_HPP

#include "../enum.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"
#include "meta_data.hpp"

namespace power_grid_model {

struct BaseOutput {
    ID id;  // ID of the object
    IntS energized;  // if the object is energized
};

template <bool sym>
struct NodeOutput : BaseOutput {
    RealValue<sym> u_pu;  // voltage magnitude and angle
    RealValue<sym> u;  // voltage magnitude and angle
    RealValue<sym> u_angle;  // voltage magnitude and angle
    RealValue<sym> p;  // node injection
    RealValue<sym> q;  // node injection
};
using SymNodeOutput = NodeOutput<true>;
using AsymNodeOutput = NodeOutput<false>;

template <bool sym>
struct BranchOutput : BaseOutput {
    double loading;  // loading of the branch
    RealValue<sym> p_from;  // power flow at from-side
    RealValue<sym> q_from;  // power flow at from-side
    RealValue<sym> i_from;  // power flow at from-side
    RealValue<sym> s_from;  // power flow at from-side
    RealValue<sym> p_to;  // power flow at to-side
    RealValue<sym> q_to;  // power flow at to-side
    RealValue<sym> i_to;  // power flow at to-side
    RealValue<sym> s_to;  // power flow at to-side
};
using SymBranchOutput = BranchOutput<true>;
using AsymBranchOutput = BranchOutput<false>;

template <bool sym>
struct Branch3Output : BaseOutput {
    double loading;  // loading of the branch
    RealValue<sym> p_1;  // power flow at side 1
    RealValue<sym> q_1;  // power flow at side 1
    RealValue<sym> i_1;  // power flow at side 1
    RealValue<sym> s_1;  // power flow at side 1
    RealValue<sym> p_2;  // power flow at side 2
    RealValue<sym> q_2;  // power flow at side 2
    RealValue<sym> i_2;  // power flow at side 2
    RealValue<sym> s_2;  // power flow at side 2
    RealValue<sym> p_3;  // power flow at side 3
    RealValue<sym> q_3;  // power flow at side 3
    RealValue<sym> i_3;  // power flow at side 3
    RealValue<sym> s_3;  // power flow at side 3
};
using SymBranch3Output = Branch3Output<true>;
using AsymBranch3Output = Branch3Output<false>;

template <bool sym>
struct ApplianceOutput : BaseOutput {
    RealValue<sym> p;  // power flow of the appliance
    RealValue<sym> q;  // power flow of the appliance
    RealValue<sym> i;  // power flow of the appliance
    RealValue<sym> s;  // power flow of the appliance
    RealValue<sym> pf;  // power flow of the appliance
};
using SymApplianceOutput = ApplianceOutput<true>;
using AsymApplianceOutput = ApplianceOutput<false>;

template <bool sym>
struct VoltageSensorOutput : BaseOutput {
    RealValue<sym> u_residual;  // deviation between the measured value and calculated value
    RealValue<sym> u_angle_residual;  // deviation between the measured value and calculated value
};
using SymVoltageSensorOutput = VoltageSensorOutput<true>;
using AsymVoltageSensorOutput = VoltageSensorOutput<false>;

template <bool sym>
struct PowerSensorOutput : BaseOutput {
    RealValue<sym> p_residual;  // deviation between the measured value and calculated value
    RealValue<sym> q_residual;  // deviation between the measured value and calculated value
};
using SymPowerSensorOutput = PowerSensorOutput<true>;
using AsymPowerSensorOutput = PowerSensorOutput<false>;



// template specialization functors to get meta data
namespace meta_data {

template<>
struct get_meta<BaseOutput> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "BaseOutput";      
        meta.size = sizeof(BaseOutput);  
        meta.alignment = alignof(BaseOutput);
        
        meta.attributes.push_back(get_data_attribute<&BaseOutput::id>("id"));
        meta.attributes.push_back(get_data_attribute<&BaseOutput::energized>("energized"));
        return meta;
    }
};

template <bool sym>
struct get_meta<NodeOutput<sym>> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "NodeOutput";      
        meta.size = sizeof(NodeOutput<sym>);  
        meta.alignment = alignof(NodeOutput<sym>);
        meta.attributes = get_meta<BaseOutput>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&NodeOutput<sym>::u_pu>("u_pu"));
        meta.attributes.push_back(get_data_attribute<&NodeOutput<sym>::u>("u"));
        meta.attributes.push_back(get_data_attribute<&NodeOutput<sym>::u_angle>("u_angle"));
        meta.attributes.push_back(get_data_attribute<&NodeOutput<sym>::p>("p"));
        meta.attributes.push_back(get_data_attribute<&NodeOutput<sym>::q>("q"));
        return meta;
    }
};

template <bool sym>
struct get_meta<BranchOutput<sym>> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "BranchOutput";      
        meta.size = sizeof(BranchOutput<sym>);  
        meta.alignment = alignof(BranchOutput<sym>);
        meta.attributes = get_meta<BaseOutput>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&BranchOutput<sym>::loading>("loading"));
        meta.attributes.push_back(get_data_attribute<&BranchOutput<sym>::p_from>("p_from"));
        meta.attributes.push_back(get_data_attribute<&BranchOutput<sym>::q_from>("q_from"));
        meta.attributes.push_back(get_data_attribute<&BranchOutput<sym>::i_from>("i_from"));
        meta.attributes.push_back(get_data_attribute<&BranchOutput<sym>::s_from>("s_from"));
        meta.attributes.push_back(get_data_attribute<&BranchOutput<sym>::p_to>("p_to"));
        meta.attributes.push_back(get_data_attribute<&BranchOutput<sym>::q_to>("q_to"));
        meta.attributes.push_back(get_data_attribute<&BranchOutput<sym>::i_to>("i_to"));
        meta.attributes.push_back(get_data_attribute<&BranchOutput<sym>::s_to>("s_to"));
        return meta;
    }
};

template <bool sym>
struct get_meta<Branch3Output<sym>> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "Branch3Output";      
        meta.size = sizeof(Branch3Output<sym>);  
        meta.alignment = alignof(Branch3Output<sym>);
        meta.attributes = get_meta<BaseOutput>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&Branch3Output<sym>::loading>("loading"));
        meta.attributes.push_back(get_data_attribute<&Branch3Output<sym>::p_1>("p_1"));
        meta.attributes.push_back(get_data_attribute<&Branch3Output<sym>::q_1>("q_1"));
        meta.attributes.push_back(get_data_attribute<&Branch3Output<sym>::i_1>("i_1"));
        meta.attributes.push_back(get_data_attribute<&Branch3Output<sym>::s_1>("s_1"));
        meta.attributes.push_back(get_data_attribute<&Branch3Output<sym>::p_2>("p_2"));
        meta.attributes.push_back(get_data_attribute<&Branch3Output<sym>::q_2>("q_2"));
        meta.attributes.push_back(get_data_attribute<&Branch3Output<sym>::i_2>("i_2"));
        meta.attributes.push_back(get_data_attribute<&Branch3Output<sym>::s_2>("s_2"));
        meta.attributes.push_back(get_data_attribute<&Branch3Output<sym>::p_3>("p_3"));
        meta.attributes.push_back(get_data_attribute<&Branch3Output<sym>::q_3>("q_3"));
        meta.attributes.push_back(get_data_attribute<&Branch3Output<sym>::i_3>("i_3"));
        meta.attributes.push_back(get_data_attribute<&Branch3Output<sym>::s_3>("s_3"));
        return meta;
    }
};

template <bool sym>
struct get_meta<ApplianceOutput<sym>> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "ApplianceOutput";      
        meta.size = sizeof(ApplianceOutput<sym>);  
        meta.alignment = alignof(ApplianceOutput<sym>);
        meta.attributes = get_meta<BaseOutput>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&ApplianceOutput<sym>::p>("p"));
        meta.attributes.push_back(get_data_attribute<&ApplianceOutput<sym>::q>("q"));
        meta.attributes.push_back(get_data_attribute<&ApplianceOutput<sym>::i>("i"));
        meta.attributes.push_back(get_data_attribute<&ApplianceOutput<sym>::s>("s"));
        meta.attributes.push_back(get_data_attribute<&ApplianceOutput<sym>::pf>("pf"));
        return meta;
    }
};

template <bool sym>
struct get_meta<VoltageSensorOutput<sym>> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "VoltageSensorOutput";      
        meta.size = sizeof(VoltageSensorOutput<sym>);  
        meta.alignment = alignof(VoltageSensorOutput<sym>);
        meta.attributes = get_meta<BaseOutput>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&VoltageSensorOutput<sym>::u_residual>("u_residual"));
        meta.attributes.push_back(get_data_attribute<&VoltageSensorOutput<sym>::u_angle_residual>("u_angle_residual"));
        return meta;
    }
};

template <bool sym>
struct get_meta<PowerSensorOutput<sym>> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "PowerSensorOutput";      
        meta.size = sizeof(PowerSensorOutput<sym>);  
        meta.alignment = alignof(PowerSensorOutput<sym>);
        meta.attributes = get_meta<BaseOutput>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&PowerSensorOutput<sym>::p_residual>("p_residual"));
        meta.attributes.push_back(get_data_attribute<&PowerSensorOutput<sym>::q_residual>("q_residual"));
        return meta;
    }
};



} // namespace meta_data

} // namespace power_grid_model

#endif
// clang-format on