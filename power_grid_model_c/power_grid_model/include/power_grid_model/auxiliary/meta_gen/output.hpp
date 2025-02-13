// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// This header file is automatically generated. DO NOT modify it manually!

// clang-format off
#pragma once

#include "gen_getters.hpp" // NOLINT

#include "../../common/common.hpp"             // NOLINT
#include "../../common/enum.hpp"               // NOLINT
#include "../../common/three_phase_tensor.hpp" // NOLINT
#include "../meta_data.hpp"                    // NOLINT

#include "../output.hpp" // NOLINT


namespace power_grid_model::meta_data {

// template specialization to get list of attributes in the value field

template<>
struct get_attributes_list<BaseOutput> {
    static constexpr std::array<MetaAttribute, 2> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&BaseOutput::id>(offsetof(BaseOutput, id), "id"),
            meta_data_gen::get_meta_attribute<&BaseOutput::energized>(offsetof(BaseOutput, energized), "energized"),
    };
};

template <symmetry_tag sym_type>
struct get_attributes_list<NodeOutput<sym_type>> {
    using sym = sym_type;

    static constexpr std::array<MetaAttribute, 7> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&NodeOutput<sym>::id>(offsetof(NodeOutput<sym>, id), "id"),
            meta_data_gen::get_meta_attribute<&NodeOutput<sym>::energized>(offsetof(NodeOutput<sym>, energized), "energized"),
            meta_data_gen::get_meta_attribute<&NodeOutput<sym>::u_pu>(offsetof(NodeOutput<sym>, u_pu), "u_pu"),
            meta_data_gen::get_meta_attribute<&NodeOutput<sym>::u>(offsetof(NodeOutput<sym>, u), "u"),
            meta_data_gen::get_meta_attribute<&NodeOutput<sym>::u_angle>(offsetof(NodeOutput<sym>, u_angle), "u_angle"),
            meta_data_gen::get_meta_attribute<&NodeOutput<sym>::p>(offsetof(NodeOutput<sym>, p), "p"),
            meta_data_gen::get_meta_attribute<&NodeOutput<sym>::q>(offsetof(NodeOutput<sym>, q), "q"),
    };
};

template <symmetry_tag sym_type>
struct get_attributes_list<BranchOutput<sym_type>> {
    using sym = sym_type;

    static constexpr std::array<MetaAttribute, 11> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&BranchOutput<sym>::id>(offsetof(BranchOutput<sym>, id), "id"),
            meta_data_gen::get_meta_attribute<&BranchOutput<sym>::energized>(offsetof(BranchOutput<sym>, energized), "energized"),
            meta_data_gen::get_meta_attribute<&BranchOutput<sym>::loading>(offsetof(BranchOutput<sym>, loading), "loading"),
            meta_data_gen::get_meta_attribute<&BranchOutput<sym>::p_from>(offsetof(BranchOutput<sym>, p_from), "p_from"),
            meta_data_gen::get_meta_attribute<&BranchOutput<sym>::q_from>(offsetof(BranchOutput<sym>, q_from), "q_from"),
            meta_data_gen::get_meta_attribute<&BranchOutput<sym>::i_from>(offsetof(BranchOutput<sym>, i_from), "i_from"),
            meta_data_gen::get_meta_attribute<&BranchOutput<sym>::s_from>(offsetof(BranchOutput<sym>, s_from), "s_from"),
            meta_data_gen::get_meta_attribute<&BranchOutput<sym>::p_to>(offsetof(BranchOutput<sym>, p_to), "p_to"),
            meta_data_gen::get_meta_attribute<&BranchOutput<sym>::q_to>(offsetof(BranchOutput<sym>, q_to), "q_to"),
            meta_data_gen::get_meta_attribute<&BranchOutput<sym>::i_to>(offsetof(BranchOutput<sym>, i_to), "i_to"),
            meta_data_gen::get_meta_attribute<&BranchOutput<sym>::s_to>(offsetof(BranchOutput<sym>, s_to), "s_to"),
    };
};

template <symmetry_tag sym_type>
struct get_attributes_list<Branch3Output<sym_type>> {
    using sym = sym_type;

    static constexpr std::array<MetaAttribute, 15> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&Branch3Output<sym>::id>(offsetof(Branch3Output<sym>, id), "id"),
            meta_data_gen::get_meta_attribute<&Branch3Output<sym>::energized>(offsetof(Branch3Output<sym>, energized), "energized"),
            meta_data_gen::get_meta_attribute<&Branch3Output<sym>::loading>(offsetof(Branch3Output<sym>, loading), "loading"),
            meta_data_gen::get_meta_attribute<&Branch3Output<sym>::p_1>(offsetof(Branch3Output<sym>, p_1), "p_1"),
            meta_data_gen::get_meta_attribute<&Branch3Output<sym>::q_1>(offsetof(Branch3Output<sym>, q_1), "q_1"),
            meta_data_gen::get_meta_attribute<&Branch3Output<sym>::i_1>(offsetof(Branch3Output<sym>, i_1), "i_1"),
            meta_data_gen::get_meta_attribute<&Branch3Output<sym>::s_1>(offsetof(Branch3Output<sym>, s_1), "s_1"),
            meta_data_gen::get_meta_attribute<&Branch3Output<sym>::p_2>(offsetof(Branch3Output<sym>, p_2), "p_2"),
            meta_data_gen::get_meta_attribute<&Branch3Output<sym>::q_2>(offsetof(Branch3Output<sym>, q_2), "q_2"),
            meta_data_gen::get_meta_attribute<&Branch3Output<sym>::i_2>(offsetof(Branch3Output<sym>, i_2), "i_2"),
            meta_data_gen::get_meta_attribute<&Branch3Output<sym>::s_2>(offsetof(Branch3Output<sym>, s_2), "s_2"),
            meta_data_gen::get_meta_attribute<&Branch3Output<sym>::p_3>(offsetof(Branch3Output<sym>, p_3), "p_3"),
            meta_data_gen::get_meta_attribute<&Branch3Output<sym>::q_3>(offsetof(Branch3Output<sym>, q_3), "q_3"),
            meta_data_gen::get_meta_attribute<&Branch3Output<sym>::i_3>(offsetof(Branch3Output<sym>, i_3), "i_3"),
            meta_data_gen::get_meta_attribute<&Branch3Output<sym>::s_3>(offsetof(Branch3Output<sym>, s_3), "s_3"),
    };
};

template <symmetry_tag sym_type>
struct get_attributes_list<ApplianceOutput<sym_type>> {
    using sym = sym_type;

    static constexpr std::array<MetaAttribute, 7> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&ApplianceOutput<sym>::id>(offsetof(ApplianceOutput<sym>, id), "id"),
            meta_data_gen::get_meta_attribute<&ApplianceOutput<sym>::energized>(offsetof(ApplianceOutput<sym>, energized), "energized"),
            meta_data_gen::get_meta_attribute<&ApplianceOutput<sym>::p>(offsetof(ApplianceOutput<sym>, p), "p"),
            meta_data_gen::get_meta_attribute<&ApplianceOutput<sym>::q>(offsetof(ApplianceOutput<sym>, q), "q"),
            meta_data_gen::get_meta_attribute<&ApplianceOutput<sym>::i>(offsetof(ApplianceOutput<sym>, i), "i"),
            meta_data_gen::get_meta_attribute<&ApplianceOutput<sym>::s>(offsetof(ApplianceOutput<sym>, s), "s"),
            meta_data_gen::get_meta_attribute<&ApplianceOutput<sym>::pf>(offsetof(ApplianceOutput<sym>, pf), "pf"),
    };
};

template <symmetry_tag sym_type>
struct get_attributes_list<VoltageSensorOutput<sym_type>> {
    using sym = sym_type;

    static constexpr std::array<MetaAttribute, 4> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&VoltageSensorOutput<sym>::id>(offsetof(VoltageSensorOutput<sym>, id), "id"),
            meta_data_gen::get_meta_attribute<&VoltageSensorOutput<sym>::energized>(offsetof(VoltageSensorOutput<sym>, energized), "energized"),
            meta_data_gen::get_meta_attribute<&VoltageSensorOutput<sym>::u_residual>(offsetof(VoltageSensorOutput<sym>, u_residual), "u_residual"),
            meta_data_gen::get_meta_attribute<&VoltageSensorOutput<sym>::u_angle_residual>(offsetof(VoltageSensorOutput<sym>, u_angle_residual), "u_angle_residual"),
    };
};

template <symmetry_tag sym_type>
struct get_attributes_list<PowerSensorOutput<sym_type>> {
    using sym = sym_type;

    static constexpr std::array<MetaAttribute, 4> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&PowerSensorOutput<sym>::id>(offsetof(PowerSensorOutput<sym>, id), "id"),
            meta_data_gen::get_meta_attribute<&PowerSensorOutput<sym>::energized>(offsetof(PowerSensorOutput<sym>, energized), "energized"),
            meta_data_gen::get_meta_attribute<&PowerSensorOutput<sym>::p_residual>(offsetof(PowerSensorOutput<sym>, p_residual), "p_residual"),
            meta_data_gen::get_meta_attribute<&PowerSensorOutput<sym>::q_residual>(offsetof(PowerSensorOutput<sym>, q_residual), "q_residual"),
    };
};

template<>
struct get_attributes_list<FaultOutput> {
    static constexpr std::array<MetaAttribute, 2> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&FaultOutput::id>(offsetof(FaultOutput, id), "id"),
            meta_data_gen::get_meta_attribute<&FaultOutput::energized>(offsetof(FaultOutput, energized), "energized"),
    };
};

template<>
struct get_attributes_list<FaultShortCircuitOutput> {
    static constexpr std::array<MetaAttribute, 4> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&FaultShortCircuitOutput::id>(offsetof(FaultShortCircuitOutput, id), "id"),
            meta_data_gen::get_meta_attribute<&FaultShortCircuitOutput::energized>(offsetof(FaultShortCircuitOutput, energized), "energized"),
            meta_data_gen::get_meta_attribute<&FaultShortCircuitOutput::i_f>(offsetof(FaultShortCircuitOutput, i_f), "i_f"),
            meta_data_gen::get_meta_attribute<&FaultShortCircuitOutput::i_f_angle>(offsetof(FaultShortCircuitOutput, i_f_angle), "i_f_angle"),
    };
};

template<>
struct get_attributes_list<NodeShortCircuitOutput> {
    static constexpr std::array<MetaAttribute, 5> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&NodeShortCircuitOutput::id>(offsetof(NodeShortCircuitOutput, id), "id"),
            meta_data_gen::get_meta_attribute<&NodeShortCircuitOutput::energized>(offsetof(NodeShortCircuitOutput, energized), "energized"),
            meta_data_gen::get_meta_attribute<&NodeShortCircuitOutput::u_pu>(offsetof(NodeShortCircuitOutput, u_pu), "u_pu"),
            meta_data_gen::get_meta_attribute<&NodeShortCircuitOutput::u>(offsetof(NodeShortCircuitOutput, u), "u"),
            meta_data_gen::get_meta_attribute<&NodeShortCircuitOutput::u_angle>(offsetof(NodeShortCircuitOutput, u_angle), "u_angle"),
    };
};

template<>
struct get_attributes_list<BranchShortCircuitOutput> {
    static constexpr std::array<MetaAttribute, 6> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&BranchShortCircuitOutput::id>(offsetof(BranchShortCircuitOutput, id), "id"),
            meta_data_gen::get_meta_attribute<&BranchShortCircuitOutput::energized>(offsetof(BranchShortCircuitOutput, energized), "energized"),
            meta_data_gen::get_meta_attribute<&BranchShortCircuitOutput::i_from>(offsetof(BranchShortCircuitOutput, i_from), "i_from"),
            meta_data_gen::get_meta_attribute<&BranchShortCircuitOutput::i_from_angle>(offsetof(BranchShortCircuitOutput, i_from_angle), "i_from_angle"),
            meta_data_gen::get_meta_attribute<&BranchShortCircuitOutput::i_to>(offsetof(BranchShortCircuitOutput, i_to), "i_to"),
            meta_data_gen::get_meta_attribute<&BranchShortCircuitOutput::i_to_angle>(offsetof(BranchShortCircuitOutput, i_to_angle), "i_to_angle"),
    };
};

template<>
struct get_attributes_list<Branch3ShortCircuitOutput> {
    static constexpr std::array<MetaAttribute, 8> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&Branch3ShortCircuitOutput::id>(offsetof(Branch3ShortCircuitOutput, id), "id"),
            meta_data_gen::get_meta_attribute<&Branch3ShortCircuitOutput::energized>(offsetof(Branch3ShortCircuitOutput, energized), "energized"),
            meta_data_gen::get_meta_attribute<&Branch3ShortCircuitOutput::i_1>(offsetof(Branch3ShortCircuitOutput, i_1), "i_1"),
            meta_data_gen::get_meta_attribute<&Branch3ShortCircuitOutput::i_1_angle>(offsetof(Branch3ShortCircuitOutput, i_1_angle), "i_1_angle"),
            meta_data_gen::get_meta_attribute<&Branch3ShortCircuitOutput::i_2>(offsetof(Branch3ShortCircuitOutput, i_2), "i_2"),
            meta_data_gen::get_meta_attribute<&Branch3ShortCircuitOutput::i_2_angle>(offsetof(Branch3ShortCircuitOutput, i_2_angle), "i_2_angle"),
            meta_data_gen::get_meta_attribute<&Branch3ShortCircuitOutput::i_3>(offsetof(Branch3ShortCircuitOutput, i_3), "i_3"),
            meta_data_gen::get_meta_attribute<&Branch3ShortCircuitOutput::i_3_angle>(offsetof(Branch3ShortCircuitOutput, i_3_angle), "i_3_angle"),
    };
};

template<>
struct get_attributes_list<ApplianceShortCircuitOutput> {
    static constexpr std::array<MetaAttribute, 4> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&ApplianceShortCircuitOutput::id>(offsetof(ApplianceShortCircuitOutput, id), "id"),
            meta_data_gen::get_meta_attribute<&ApplianceShortCircuitOutput::energized>(offsetof(ApplianceShortCircuitOutput, energized), "energized"),
            meta_data_gen::get_meta_attribute<&ApplianceShortCircuitOutput::i>(offsetof(ApplianceShortCircuitOutput, i), "i"),
            meta_data_gen::get_meta_attribute<&ApplianceShortCircuitOutput::i_angle>(offsetof(ApplianceShortCircuitOutput, i_angle), "i_angle"),
    };
};

template<>
struct get_attributes_list<SensorShortCircuitOutput> {
    static constexpr std::array<MetaAttribute, 2> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&SensorShortCircuitOutput::id>(offsetof(SensorShortCircuitOutput, id), "id"),
            meta_data_gen::get_meta_attribute<&SensorShortCircuitOutput::energized>(offsetof(SensorShortCircuitOutput, energized), "energized"),
    };
};

template<>
struct get_attributes_list<TransformerTapRegulatorOutput> {
    static constexpr std::array<MetaAttribute, 3> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&TransformerTapRegulatorOutput::id>(offsetof(TransformerTapRegulatorOutput, id), "id"),
            meta_data_gen::get_meta_attribute<&TransformerTapRegulatorOutput::energized>(offsetof(TransformerTapRegulatorOutput, energized), "energized"),
            meta_data_gen::get_meta_attribute<&TransformerTapRegulatorOutput::tap_pos>(offsetof(TransformerTapRegulatorOutput, tap_pos), "tap_pos"),
    };
};

template<>
struct get_attributes_list<RegulatorShortCircuitOutput> {
    static constexpr std::array<MetaAttribute, 2> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&RegulatorShortCircuitOutput::id>(offsetof(RegulatorShortCircuitOutput, id), "id"),
            meta_data_gen::get_meta_attribute<&RegulatorShortCircuitOutput::energized>(offsetof(RegulatorShortCircuitOutput, energized), "energized"),
    };
};

template <symmetry_tag sym_type>
struct get_attributes_list<CurrentSensorOutput<sym_type>> {
    using sym = sym_type;

    static constexpr std::array<MetaAttribute, 4> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&CurrentSensorOutput<sym>::id>(offsetof(CurrentSensorOutput<sym>, id), "id"),
            meta_data_gen::get_meta_attribute<&CurrentSensorOutput<sym>::energized>(offsetof(CurrentSensorOutput<sym>, energized), "energized"),
            meta_data_gen::get_meta_attribute<&CurrentSensorOutput<sym>::i_residual>(offsetof(CurrentSensorOutput<sym>, i_residual), "i_residual"),
            meta_data_gen::get_meta_attribute<&CurrentSensorOutput<sym>::i_angle_residual>(offsetof(CurrentSensorOutput<sym>, i_angle_residual), "i_angle_residual"),
    };
};




} // namespace power_grid_model::meta_data

// clang-format on