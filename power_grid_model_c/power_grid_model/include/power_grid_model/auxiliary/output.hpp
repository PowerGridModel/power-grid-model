// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

// This header file is automatically generated. DO NOT modify it manually!

// clang-format off
#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_OUTPUT_HPP
#define POWER_GRID_MODEL_AUXILIARY_OUTPUT_HPP

#include "meta_data.hpp"

#include "../enum.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"

namespace power_grid_model {

struct BaseOutput {
    ID id;  // ID of the object
    IntS energized;  // whether the object is energized
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

struct FaultOutput : BaseOutput {
};

struct FaultShortCircuitOutput : BaseOutput {
    RealValue<false> i_f;  // three phase short circuit current magnitude
    RealValue<false> i_f_angle;  // three phase short circuit current angle
};

struct NodeShortCircuitOutput : BaseOutput {
    RealValue<false> u_pu;  // initial three phase line-to-ground short circuit voltage magnitude and angle
    RealValue<false> u;  // initial three phase line-to-ground short circuit voltage magnitude and angle
    RealValue<false> u_angle;  // initial three phase line-to-ground short circuit voltage magnitude and angle
};

struct BranchShortCircuitOutput : BaseOutput {
    RealValue<false> i_from;  // initial three phase short circuit current flow at from-side
    RealValue<false> i_from_angle;  // initial three phase short circuit current flow at from-side
    RealValue<false> i_to;  // initial three phase short circuit current flow at to-side
    RealValue<false> i_to_angle;  // initial three phase short circuit current flow at to-side
};

struct Branch3ShortCircuitOutput : BaseOutput {
    RealValue<false> i_1;  // initial three phase short circuit current flow at side 1
    RealValue<false> i_1_angle;  // initial three phase short circuit current flow at side 1
    RealValue<false> i_2;  // initial three phase short circuit current flow at side 2
    RealValue<false> i_2_angle;  // initial three phase short circuit current flow at side 2
    RealValue<false> i_3;  // initial three phase short circuit current flow at side 3
    RealValue<false> i_3_angle;  // initial three phase short circuit current flow at side 3
};

struct ApplianceShortCircuitOutput : BaseOutput {
    RealValue<false> i;  // initial three phase short circuit current flow of the appliance
    RealValue<false> i_angle;  // initial three phase short circuit current flow of the appliance
};

struct SensorShortCircuitOutput : BaseOutput {
};



// template specialization functors to get meta data
namespace meta_data {

template<>
struct get_meta<BaseOutput> {
    MetaData operator() () const {
        MetaData meta{};
        meta.name = "BaseOutput";      
        meta.size = sizeof(BaseOutput);  
        meta.alignment = alignof(BaseOutput);
        
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
        meta.attributes = get_meta<BaseOutput>{}().attributes;
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
        meta.attributes = get_meta<BaseOutput>{}().attributes;
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
        meta.attributes = get_meta<BaseOutput>{}().attributes;
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
        meta.attributes = get_meta<BaseOutput>{}().attributes;
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
        meta.attributes = get_meta<BaseOutput>{}().attributes;
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
        meta.attributes = get_meta<BaseOutput>{}().attributes;
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
        meta.attributes = get_meta<BaseOutput>{}().attributes;
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
        meta.attributes = get_meta<BaseOutput>{}().attributes;
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
        meta.attributes = get_meta<BaseOutput>{}().attributes;
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
        meta.attributes = get_meta<BaseOutput>{}().attributes;
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
        meta.attributes = get_meta<BaseOutput>{}().attributes;
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
        meta.attributes = get_meta<BaseOutput>{}().attributes;
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
        meta.attributes = get_meta<BaseOutput>{}().attributes;
        return meta;
    }
};



} // namespace meta_data

} // namespace power_grid_model

#endif
// clang-format on