// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

// This header file is automatically generated. DO NOT modify it manually!

// clang-format off
#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_INPUT_HPP
#define POWER_GRID_MODEL_AUXILIARY_INPUT_HPP

#include "../enum.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"
#include "meta_data.hpp"

namespace power_grid_model {

struct BaseInput {
    ID id;  // ID of the object
};

struct NodeInput : BaseInput {
    double u_rated;  // Rated line-line voltage
};

struct BranchInput : BaseInput {
    ID from_node;  // Node IDs to which this branch is connected at both sides
    ID to_node;  // Node IDs to which this branch is connected at both sides
    IntS from_status;  // If the branch is connected at each side
    IntS to_status;  // If the branch is connected at each side
};

struct Branch3Input : BaseInput {
    ID node_1;  // Node IDs to which this branch3 is connected at three sides
    ID node_2;  // Node IDs to which this branch3 is connected at three sides
    ID node_3;  // Node IDs to which this branch3 is connected at three sides
    IntS status_1;  // If the branch is connected at each side
    IntS status_2;  // If the branch is connected at each side
    IntS status_3;  // If the branch is connected at each side
};

struct SensorInput : BaseInput {
    ID measured_object;  // ID of the measured object
};

struct ApplianceInput : BaseInput {
    ID node;  // Node ID to which this appliance is connected
    IntS status;  // If the appliance is connected
};

struct LineInput : BranchInput {
    double r1;  // positive sequence parameters
    double x1;  // positive sequence parameters
    double c1;  // positive sequence parameters
    double tan1;  // positive sequence parameters
    double r0;  // zero sequence parameters
    double x0;  // zero sequence parameters
    double c0;  // zero sequence parameters
    double tan0;  // zero sequence parameters
    double i_n;  // rated current
};

struct LinkInput : BranchInput {
};

struct TransformerInput : BranchInput {
    double u1;  // rated voltage at both side
    double u2;  // rated voltage at both side
    double sn;  // rated power
    double uk;  // short circuit and open testing parameters
    double pk;  // short circuit and open testing parameters
    double i0;  // short circuit and open testing parameters
    double p0;  // short circuit and open testing parameters
    WindingType winding_from;  // winding type at each side
    WindingType winding_to;  // winding type at each side
    IntS clock;  // clock number
    BranchSide tap_side;  // side of tap changer
    IntS tap_pos;  // tap changer parameters
    IntS tap_min;  // tap changer parameters
    IntS tap_max;  // tap changer parameters
    IntS tap_nom;  // tap changer parameters
    double tap_size;  // size of each tap
    double uk_min;  // tap dependent short circuit parameters
    double uk_max;  // tap dependent short circuit parameters
    double pk_min;  // tap dependent short circuit parameters
    double pk_max;  // tap dependent short circuit parameters
    double r_grounding_from;  // grounding information
    double x_grounding_from;  // grounding information
    double r_grounding_to;  // grounding information
    double x_grounding_to;  // grounding information
};

struct ThreeWindingTransformerInput : Branch3Input {
    double u1;  // rated voltage at three sides
    double u2;  // rated voltage at three sides
    double u3;  // rated voltage at three sides
    double sn_1;  // rated power at each side
    double sn_2;  // rated power at each side
    double sn_3;  // rated power at each side
    double uk_12;  // short circuit and open testing parameters
    double uk_13;  // short circuit and open testing parameters
    double uk_23;  // short circuit and open testing parameters
    double pk_12;  // short circuit and open testing parameters
    double pk_13;  // short circuit and open testing parameters
    double pk_23;  // short circuit and open testing parameters
    double i0;  // short circuit and open testing parameters
    double p0;  // short circuit and open testing parameters
    WindingType winding_1;  // winding type at each side
    WindingType winding_2;  // winding type at each side
    WindingType winding_3;  // winding type at each side
    IntS clock_12;  // clock numbers
    IntS clock_13;  // clock numbers
    Branch3Side tap_side;  // side of tap changer
    IntS tap_pos;  // tap changer parameters
    IntS tap_min;  // tap changer parameters
    IntS tap_max;  // tap changer parameters
    IntS tap_nom;  // tap changer parameters
    double tap_size;  // size of each tap
    double uk_12_min;  // tap dependent short circuit parameters
    double uk_12_max;  // tap dependent short circuit parameters
    double uk_13_min;  // tap dependent short circuit parameters
    double uk_13_max;  // tap dependent short circuit parameters
    double uk_23_min;  // tap dependent short circuit parameters
    double uk_23_max;  // tap dependent short circuit parameters
    double pk_12_min;  // tap dependent short circuit parameters
    double pk_12_max;  // tap dependent short circuit parameters
    double pk_13_min;  // tap dependent short circuit parameters
    double pk_13_max;  // tap dependent short circuit parameters
    double pk_23_min;  // tap dependent short circuit parameters
    double pk_23_max;  // tap dependent short circuit parameters
    double r_grounding_1;  // grounding information
    double x_grounding_1;  // grounding information
    double r_grounding_2;  // grounding information
    double x_grounding_2;  // grounding information
    double r_grounding_3;  // grounding information
    double x_grounding_3;  // grounding information
};

struct GenericLoadGenInput : ApplianceInput {
    LoadGenType type;  // Type of the load_gen
};

template <bool sym>
struct LoadGenInput : GenericLoadGenInput {
    RealValue<sym> p_specified;  // Specified active/reactive power
    RealValue<sym> q_specified;  // Specified active/reactive power
};
using SymLoadGenInput = LoadGenInput<true>;
using AsymLoadGenInput = LoadGenInput<false>;

struct ShuntInput : ApplianceInput {
    double g1;  // positive sequence admittance
    double b1;  // positive sequence admittance
    double g0;  // zero sequence admittance
    double b0;  // zero sequence admittance
};

struct SourceInput : ApplianceInput {
    double u_ref;  // reference voltage
    double u_ref_angle;  // reference voltage
    double sk;  // short circuitl capacity
    double rx_ratio;  // short circuitl capacity
    double z01_ratio;  // short circuitl capacity
};

struct GenericVoltageSensorInput : SensorInput {
    double u_sigma;  // sigma of error margin of voltage measurement
};

template <bool sym>
struct VoltageSensorInput : GenericVoltageSensorInput {
    RealValue<sym> u_measured;  // measured voltage magnitude and angle
    RealValue<sym> u_angle_measured;  // measured voltage magnitude and angle
};
using SymVoltageSensorInput = VoltageSensorInput<true>;
using AsymVoltageSensorInput = VoltageSensorInput<false>;

struct GenericPowerSensorInput : SensorInput {
    MeasuredTerminalType measured_terminal_type;  // type of measured terminal
    double power_sigma;  // sigma of error margin of power measurement
};

template <bool sym>
struct PowerSensorInput : GenericPowerSensorInput {
    RealValue<sym> p_measured;  // measured active/reactive power
    RealValue<sym> q_measured;  // measured active/reactive power
};
using SymPowerSensorInput = PowerSensorInput<true>;
using AsymPowerSensorInput = PowerSensorInput<false>;



// template specialization functors to get meta data
namespace meta_data {

template<>
struct get_meta<BaseInput> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "BaseInput";      
        meta.size = sizeof(BaseInput);  
        meta.alignment = alignof(BaseInput);
        
        meta.attributes.push_back(get_data_attribute<&BaseInput::id>("id"));
        return meta;
    }
};

template<>
struct get_meta<NodeInput> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "NodeInput";      
        meta.size = sizeof(NodeInput);  
        meta.alignment = alignof(NodeInput);
        meta.attributes = get_meta<BaseInput>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&NodeInput::u_rated>("u_rated"));
        return meta;
    }
};

template<>
struct get_meta<BranchInput> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "BranchInput";      
        meta.size = sizeof(BranchInput);  
        meta.alignment = alignof(BranchInput);
        meta.attributes = get_meta<BaseInput>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&BranchInput::from_node>("from_node"));
        meta.attributes.push_back(get_data_attribute<&BranchInput::to_node>("to_node"));
        meta.attributes.push_back(get_data_attribute<&BranchInput::from_status>("from_status"));
        meta.attributes.push_back(get_data_attribute<&BranchInput::to_status>("to_status"));
        return meta;
    }
};

template<>
struct get_meta<Branch3Input> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "Branch3Input";      
        meta.size = sizeof(Branch3Input);  
        meta.alignment = alignof(Branch3Input);
        meta.attributes = get_meta<BaseInput>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&Branch3Input::node_1>("node_1"));
        meta.attributes.push_back(get_data_attribute<&Branch3Input::node_2>("node_2"));
        meta.attributes.push_back(get_data_attribute<&Branch3Input::node_3>("node_3"));
        meta.attributes.push_back(get_data_attribute<&Branch3Input::status_1>("status_1"));
        meta.attributes.push_back(get_data_attribute<&Branch3Input::status_2>("status_2"));
        meta.attributes.push_back(get_data_attribute<&Branch3Input::status_3>("status_3"));
        return meta;
    }
};

template<>
struct get_meta<SensorInput> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "SensorInput";      
        meta.size = sizeof(SensorInput);  
        meta.alignment = alignof(SensorInput);
        meta.attributes = get_meta<BaseInput>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&SensorInput::measured_object>("measured_object"));
        return meta;
    }
};

template<>
struct get_meta<ApplianceInput> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "ApplianceInput";      
        meta.size = sizeof(ApplianceInput);  
        meta.alignment = alignof(ApplianceInput);
        meta.attributes = get_meta<BaseInput>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&ApplianceInput::node>("node"));
        meta.attributes.push_back(get_data_attribute<&ApplianceInput::status>("status"));
        return meta;
    }
};

template<>
struct get_meta<LineInput> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "LineInput";      
        meta.size = sizeof(LineInput);  
        meta.alignment = alignof(LineInput);
        meta.attributes = get_meta<BranchInput>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&LineInput::r1>("r1"));
        meta.attributes.push_back(get_data_attribute<&LineInput::x1>("x1"));
        meta.attributes.push_back(get_data_attribute<&LineInput::c1>("c1"));
        meta.attributes.push_back(get_data_attribute<&LineInput::tan1>("tan1"));
        meta.attributes.push_back(get_data_attribute<&LineInput::r0>("r0"));
        meta.attributes.push_back(get_data_attribute<&LineInput::x0>("x0"));
        meta.attributes.push_back(get_data_attribute<&LineInput::c0>("c0"));
        meta.attributes.push_back(get_data_attribute<&LineInput::tan0>("tan0"));
        meta.attributes.push_back(get_data_attribute<&LineInput::i_n>("i_n"));
        return meta;
    }
};

template<>
struct get_meta<LinkInput> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "LinkInput";      
        meta.size = sizeof(LinkInput);  
        meta.alignment = alignof(LinkInput);
        meta.attributes = get_meta<BranchInput>{}().attributes;
        return meta;
    }
};

template<>
struct get_meta<TransformerInput> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "TransformerInput";      
        meta.size = sizeof(TransformerInput);  
        meta.alignment = alignof(TransformerInput);
        meta.attributes = get_meta<BranchInput>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&TransformerInput::u1>("u1"));
        meta.attributes.push_back(get_data_attribute<&TransformerInput::u2>("u2"));
        meta.attributes.push_back(get_data_attribute<&TransformerInput::sn>("sn"));
        meta.attributes.push_back(get_data_attribute<&TransformerInput::uk>("uk"));
        meta.attributes.push_back(get_data_attribute<&TransformerInput::pk>("pk"));
        meta.attributes.push_back(get_data_attribute<&TransformerInput::i0>("i0"));
        meta.attributes.push_back(get_data_attribute<&TransformerInput::p0>("p0"));
        meta.attributes.push_back(get_data_attribute<&TransformerInput::winding_from>("winding_from"));
        meta.attributes.push_back(get_data_attribute<&TransformerInput::winding_to>("winding_to"));
        meta.attributes.push_back(get_data_attribute<&TransformerInput::clock>("clock"));
        meta.attributes.push_back(get_data_attribute<&TransformerInput::tap_side>("tap_side"));
        meta.attributes.push_back(get_data_attribute<&TransformerInput::tap_pos>("tap_pos"));
        meta.attributes.push_back(get_data_attribute<&TransformerInput::tap_min>("tap_min"));
        meta.attributes.push_back(get_data_attribute<&TransformerInput::tap_max>("tap_max"));
        meta.attributes.push_back(get_data_attribute<&TransformerInput::tap_nom>("tap_nom"));
        meta.attributes.push_back(get_data_attribute<&TransformerInput::tap_size>("tap_size"));
        meta.attributes.push_back(get_data_attribute<&TransformerInput::uk_min>("uk_min"));
        meta.attributes.push_back(get_data_attribute<&TransformerInput::uk_max>("uk_max"));
        meta.attributes.push_back(get_data_attribute<&TransformerInput::pk_min>("pk_min"));
        meta.attributes.push_back(get_data_attribute<&TransformerInput::pk_max>("pk_max"));
        meta.attributes.push_back(get_data_attribute<&TransformerInput::r_grounding_from>("r_grounding_from"));
        meta.attributes.push_back(get_data_attribute<&TransformerInput::x_grounding_from>("x_grounding_from"));
        meta.attributes.push_back(get_data_attribute<&TransformerInput::r_grounding_to>("r_grounding_to"));
        meta.attributes.push_back(get_data_attribute<&TransformerInput::x_grounding_to>("x_grounding_to"));
        return meta;
    }
};

template<>
struct get_meta<ThreeWindingTransformerInput> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "ThreeWindingTransformerInput";      
        meta.size = sizeof(ThreeWindingTransformerInput);  
        meta.alignment = alignof(ThreeWindingTransformerInput);
        meta.attributes = get_meta<Branch3Input>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::u1>("u1"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::u2>("u2"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::u3>("u3"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::sn_1>("sn_1"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::sn_2>("sn_2"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::sn_3>("sn_3"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::uk_12>("uk_12"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::uk_13>("uk_13"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::uk_23>("uk_23"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::pk_12>("pk_12"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::pk_13>("pk_13"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::pk_23>("pk_23"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::i0>("i0"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::p0>("p0"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::winding_1>("winding_1"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::winding_2>("winding_2"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::winding_3>("winding_3"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::clock_12>("clock_12"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::clock_13>("clock_13"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::tap_side>("tap_side"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::tap_pos>("tap_pos"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::tap_min>("tap_min"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::tap_max>("tap_max"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::tap_nom>("tap_nom"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::tap_size>("tap_size"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::uk_12_min>("uk_12_min"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::uk_12_max>("uk_12_max"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::uk_13_min>("uk_13_min"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::uk_13_max>("uk_13_max"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::uk_23_min>("uk_23_min"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::uk_23_max>("uk_23_max"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::pk_12_min>("pk_12_min"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::pk_12_max>("pk_12_max"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::pk_13_min>("pk_13_min"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::pk_13_max>("pk_13_max"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::pk_23_min>("pk_23_min"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::pk_23_max>("pk_23_max"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::r_grounding_1>("r_grounding_1"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::x_grounding_1>("x_grounding_1"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::r_grounding_2>("r_grounding_2"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::x_grounding_2>("x_grounding_2"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::r_grounding_3>("r_grounding_3"));
        meta.attributes.push_back(get_data_attribute<&ThreeWindingTransformerInput::x_grounding_3>("x_grounding_3"));
        return meta;
    }
};

template<>
struct get_meta<GenericLoadGenInput> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "GenericLoadGenInput";      
        meta.size = sizeof(GenericLoadGenInput);  
        meta.alignment = alignof(GenericLoadGenInput);
        meta.attributes = get_meta<ApplianceInput>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&GenericLoadGenInput::type>("type"));
        return meta;
    }
};

template <bool sym>
struct get_meta<LoadGenInput<sym>> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "LoadGenInput";      
        meta.size = sizeof(LoadGenInput<sym>);  
        meta.alignment = alignof(LoadGenInput<sym>);
        meta.attributes = get_meta<GenericLoadGenInput>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&LoadGenInput<sym>::p_specified>("p_specified"));
        meta.attributes.push_back(get_data_attribute<&LoadGenInput<sym>::q_specified>("q_specified"));
        return meta;
    }
};

template<>
struct get_meta<ShuntInput> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "ShuntInput";      
        meta.size = sizeof(ShuntInput);  
        meta.alignment = alignof(ShuntInput);
        meta.attributes = get_meta<ApplianceInput>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&ShuntInput::g1>("g1"));
        meta.attributes.push_back(get_data_attribute<&ShuntInput::b1>("b1"));
        meta.attributes.push_back(get_data_attribute<&ShuntInput::g0>("g0"));
        meta.attributes.push_back(get_data_attribute<&ShuntInput::b0>("b0"));
        return meta;
    }
};

template<>
struct get_meta<SourceInput> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "SourceInput";      
        meta.size = sizeof(SourceInput);  
        meta.alignment = alignof(SourceInput);
        meta.attributes = get_meta<ApplianceInput>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&SourceInput::u_ref>("u_ref"));
        meta.attributes.push_back(get_data_attribute<&SourceInput::u_ref_angle>("u_ref_angle"));
        meta.attributes.push_back(get_data_attribute<&SourceInput::sk>("sk"));
        meta.attributes.push_back(get_data_attribute<&SourceInput::rx_ratio>("rx_ratio"));
        meta.attributes.push_back(get_data_attribute<&SourceInput::z01_ratio>("z01_ratio"));
        return meta;
    }
};

template<>
struct get_meta<GenericVoltageSensorInput> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "GenericVoltageSensorInput";      
        meta.size = sizeof(GenericVoltageSensorInput);  
        meta.alignment = alignof(GenericVoltageSensorInput);
        meta.attributes = get_meta<SensorInput>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&GenericVoltageSensorInput::u_sigma>("u_sigma"));
        return meta;
    }
};

template <bool sym>
struct get_meta<VoltageSensorInput<sym>> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "VoltageSensorInput";      
        meta.size = sizeof(VoltageSensorInput<sym>);  
        meta.alignment = alignof(VoltageSensorInput<sym>);
        meta.attributes = get_meta<GenericVoltageSensorInput>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&VoltageSensorInput<sym>::u_measured>("u_measured"));
        meta.attributes.push_back(get_data_attribute<&VoltageSensorInput<sym>::u_angle_measured>("u_angle_measured"));
        return meta;
    }
};

template<>
struct get_meta<GenericPowerSensorInput> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "GenericPowerSensorInput";      
        meta.size = sizeof(GenericPowerSensorInput);  
        meta.alignment = alignof(GenericPowerSensorInput);
        meta.attributes = get_meta<SensorInput>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&GenericPowerSensorInput::measured_terminal_type>("measured_terminal_type"));
        meta.attributes.push_back(get_data_attribute<&GenericPowerSensorInput::power_sigma>("power_sigma"));
        return meta;
    }
};

template <bool sym>
struct get_meta<PowerSensorInput<sym>> {
    MetaData operator() () {
        MetaData meta{};
        meta.name = "PowerSensorInput";      
        meta.size = sizeof(PowerSensorInput<sym>);  
        meta.alignment = alignof(PowerSensorInput<sym>);
        meta.attributes = get_meta<GenericPowerSensorInput>{}().attributes;
        meta.attributes.push_back(get_data_attribute<&PowerSensorInput<sym>::p_measured>("p_measured"));
        meta.attributes.push_back(get_data_attribute<&PowerSensorInput<sym>::q_measured>("q_measured"));
        return meta;
    }
};



} // namespace meta_data

} // namespace power_grid_model

#endif
// clang-format on