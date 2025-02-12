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

#include "../update.hpp" // NOLINT


namespace power_grid_model::meta_data {

// template specialization to get list of attributes in the value field

template<>
struct get_attributes_list<BaseUpdate> {
    static constexpr std::array<MetaAttribute, 1> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&BaseUpdate::id>(offsetof(BaseUpdate, id), "id"),
    };
};

template<>
struct get_attributes_list<BranchUpdate> {
    static constexpr std::array<MetaAttribute, 3> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&BranchUpdate::id>(offsetof(BranchUpdate, id), "id"),
            meta_data_gen::get_meta_attribute<&BranchUpdate::from_status>(offsetof(BranchUpdate, from_status), "from_status"),
            meta_data_gen::get_meta_attribute<&BranchUpdate::to_status>(offsetof(BranchUpdate, to_status), "to_status"),
    };
};

template<>
struct get_attributes_list<Branch3Update> {
    static constexpr std::array<MetaAttribute, 4> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&Branch3Update::id>(offsetof(Branch3Update, id), "id"),
            meta_data_gen::get_meta_attribute<&Branch3Update::status_1>(offsetof(Branch3Update, status_1), "status_1"),
            meta_data_gen::get_meta_attribute<&Branch3Update::status_2>(offsetof(Branch3Update, status_2), "status_2"),
            meta_data_gen::get_meta_attribute<&Branch3Update::status_3>(offsetof(Branch3Update, status_3), "status_3"),
    };
};

template<>
struct get_attributes_list<ApplianceUpdate> {
    static constexpr std::array<MetaAttribute, 2> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&ApplianceUpdate::id>(offsetof(ApplianceUpdate, id), "id"),
            meta_data_gen::get_meta_attribute<&ApplianceUpdate::status>(offsetof(ApplianceUpdate, status), "status"),
    };
};

template<>
struct get_attributes_list<TransformerUpdate> {
    static constexpr std::array<MetaAttribute, 4> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&TransformerUpdate::id>(offsetof(TransformerUpdate, id), "id"),
            meta_data_gen::get_meta_attribute<&TransformerUpdate::from_status>(offsetof(TransformerUpdate, from_status), "from_status"),
            meta_data_gen::get_meta_attribute<&TransformerUpdate::to_status>(offsetof(TransformerUpdate, to_status), "to_status"),
            meta_data_gen::get_meta_attribute<&TransformerUpdate::tap_pos>(offsetof(TransformerUpdate, tap_pos), "tap_pos"),
    };
};

template<>
struct get_attributes_list<ThreeWindingTransformerUpdate> {
    static constexpr std::array<MetaAttribute, 5> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerUpdate::id>(offsetof(ThreeWindingTransformerUpdate, id), "id"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerUpdate::status_1>(offsetof(ThreeWindingTransformerUpdate, status_1), "status_1"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerUpdate::status_2>(offsetof(ThreeWindingTransformerUpdate, status_2), "status_2"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerUpdate::status_3>(offsetof(ThreeWindingTransformerUpdate, status_3), "status_3"),
            meta_data_gen::get_meta_attribute<&ThreeWindingTransformerUpdate::tap_pos>(offsetof(ThreeWindingTransformerUpdate, tap_pos), "tap_pos"),
    };
};

template <symmetry_tag sym_type>
struct get_attributes_list<LoadGenUpdate<sym_type>> {
    using sym = sym_type;

    static constexpr std::array<MetaAttribute, 4> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&LoadGenUpdate<sym>::id>(offsetof(LoadGenUpdate<sym>, id), "id"),
            meta_data_gen::get_meta_attribute<&LoadGenUpdate<sym>::status>(offsetof(LoadGenUpdate<sym>, status), "status"),
            meta_data_gen::get_meta_attribute<&LoadGenUpdate<sym>::p_specified>(offsetof(LoadGenUpdate<sym>, p_specified), "p_specified"),
            meta_data_gen::get_meta_attribute<&LoadGenUpdate<sym>::q_specified>(offsetof(LoadGenUpdate<sym>, q_specified), "q_specified"),
    };
};

template<>
struct get_attributes_list<SourceUpdate> {
    static constexpr std::array<MetaAttribute, 4> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&SourceUpdate::id>(offsetof(SourceUpdate, id), "id"),
            meta_data_gen::get_meta_attribute<&SourceUpdate::status>(offsetof(SourceUpdate, status), "status"),
            meta_data_gen::get_meta_attribute<&SourceUpdate::u_ref>(offsetof(SourceUpdate, u_ref), "u_ref"),
            meta_data_gen::get_meta_attribute<&SourceUpdate::u_ref_angle>(offsetof(SourceUpdate, u_ref_angle), "u_ref_angle"),
    };
};

template<>
struct get_attributes_list<ShuntUpdate> {
    static constexpr std::array<MetaAttribute, 6> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&ShuntUpdate::id>(offsetof(ShuntUpdate, id), "id"),
            meta_data_gen::get_meta_attribute<&ShuntUpdate::status>(offsetof(ShuntUpdate, status), "status"),
            meta_data_gen::get_meta_attribute<&ShuntUpdate::g1>(offsetof(ShuntUpdate, g1), "g1"),
            meta_data_gen::get_meta_attribute<&ShuntUpdate::b1>(offsetof(ShuntUpdate, b1), "b1"),
            meta_data_gen::get_meta_attribute<&ShuntUpdate::g0>(offsetof(ShuntUpdate, g0), "g0"),
            meta_data_gen::get_meta_attribute<&ShuntUpdate::b0>(offsetof(ShuntUpdate, b0), "b0"),
    };
};

template <symmetry_tag sym_type>
struct get_attributes_list<VoltageSensorUpdate<sym_type>> {
    using sym = sym_type;

    static constexpr std::array<MetaAttribute, 4> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&VoltageSensorUpdate<sym>::id>(offsetof(VoltageSensorUpdate<sym>, id), "id"),
            meta_data_gen::get_meta_attribute<&VoltageSensorUpdate<sym>::u_sigma>(offsetof(VoltageSensorUpdate<sym>, u_sigma), "u_sigma"),
            meta_data_gen::get_meta_attribute<&VoltageSensorUpdate<sym>::u_measured>(offsetof(VoltageSensorUpdate<sym>, u_measured), "u_measured"),
            meta_data_gen::get_meta_attribute<&VoltageSensorUpdate<sym>::u_angle_measured>(offsetof(VoltageSensorUpdate<sym>, u_angle_measured), "u_angle_measured"),
    };
};

template <symmetry_tag sym_type>
struct get_attributes_list<PowerSensorUpdate<sym_type>> {
    using sym = sym_type;

    static constexpr std::array<MetaAttribute, 6> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&PowerSensorUpdate<sym>::id>(offsetof(PowerSensorUpdate<sym>, id), "id"),
            meta_data_gen::get_meta_attribute<&PowerSensorUpdate<sym>::power_sigma>(offsetof(PowerSensorUpdate<sym>, power_sigma), "power_sigma"),
            meta_data_gen::get_meta_attribute<&PowerSensorUpdate<sym>::p_measured>(offsetof(PowerSensorUpdate<sym>, p_measured), "p_measured"),
            meta_data_gen::get_meta_attribute<&PowerSensorUpdate<sym>::q_measured>(offsetof(PowerSensorUpdate<sym>, q_measured), "q_measured"),
            meta_data_gen::get_meta_attribute<&PowerSensorUpdate<sym>::p_sigma>(offsetof(PowerSensorUpdate<sym>, p_sigma), "p_sigma"),
            meta_data_gen::get_meta_attribute<&PowerSensorUpdate<sym>::q_sigma>(offsetof(PowerSensorUpdate<sym>, q_sigma), "q_sigma"),
    };
};

template<>
struct get_attributes_list<FaultUpdate> {
    static constexpr std::array<MetaAttribute, 7> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&FaultUpdate::id>(offsetof(FaultUpdate, id), "id"),
            meta_data_gen::get_meta_attribute<&FaultUpdate::status>(offsetof(FaultUpdate, status), "status"),
            meta_data_gen::get_meta_attribute<&FaultUpdate::fault_type>(offsetof(FaultUpdate, fault_type), "fault_type"),
            meta_data_gen::get_meta_attribute<&FaultUpdate::fault_phase>(offsetof(FaultUpdate, fault_phase), "fault_phase"),
            meta_data_gen::get_meta_attribute<&FaultUpdate::fault_object>(offsetof(FaultUpdate, fault_object), "fault_object"),
            meta_data_gen::get_meta_attribute<&FaultUpdate::r_f>(offsetof(FaultUpdate, r_f), "r_f"),
            meta_data_gen::get_meta_attribute<&FaultUpdate::x_f>(offsetof(FaultUpdate, x_f), "x_f"),
    };
};

template<>
struct get_attributes_list<RegulatorUpdate> {
    static constexpr std::array<MetaAttribute, 2> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&RegulatorUpdate::id>(offsetof(RegulatorUpdate, id), "id"),
            meta_data_gen::get_meta_attribute<&RegulatorUpdate::status>(offsetof(RegulatorUpdate, status), "status"),
    };
};

template<>
struct get_attributes_list<TransformerTapRegulatorUpdate> {
    static constexpr std::array<MetaAttribute, 6> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&TransformerTapRegulatorUpdate::id>(offsetof(TransformerTapRegulatorUpdate, id), "id"),
            meta_data_gen::get_meta_attribute<&TransformerTapRegulatorUpdate::status>(offsetof(TransformerTapRegulatorUpdate, status), "status"),
            meta_data_gen::get_meta_attribute<&TransformerTapRegulatorUpdate::u_set>(offsetof(TransformerTapRegulatorUpdate, u_set), "u_set"),
            meta_data_gen::get_meta_attribute<&TransformerTapRegulatorUpdate::u_band>(offsetof(TransformerTapRegulatorUpdate, u_band), "u_band"),
            meta_data_gen::get_meta_attribute<&TransformerTapRegulatorUpdate::line_drop_compensation_r>(offsetof(TransformerTapRegulatorUpdate, line_drop_compensation_r), "line_drop_compensation_r"),
            meta_data_gen::get_meta_attribute<&TransformerTapRegulatorUpdate::line_drop_compensation_x>(offsetof(TransformerTapRegulatorUpdate, line_drop_compensation_x), "line_drop_compensation_x"),
    };
};

template <symmetry_tag sym_type>
struct get_attributes_list<CurrentSensorUpdate<sym_type>> {
    using sym = sym_type;

    static constexpr std::array<MetaAttribute, 5> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<&CurrentSensorUpdate<sym>::id>(offsetof(CurrentSensorUpdate<sym>, id), "id"),
            meta_data_gen::get_meta_attribute<&CurrentSensorUpdate<sym>::i_sigma>(offsetof(CurrentSensorUpdate<sym>, i_sigma), "i_sigma"),
            meta_data_gen::get_meta_attribute<&CurrentSensorUpdate<sym>::i_angle_sigma>(offsetof(CurrentSensorUpdate<sym>, i_angle_sigma), "i_angle_sigma"),
            meta_data_gen::get_meta_attribute<&CurrentSensorUpdate<sym>::i_measured>(offsetof(CurrentSensorUpdate<sym>, i_measured), "i_measured"),
            meta_data_gen::get_meta_attribute<&CurrentSensorUpdate<sym>::i_angle_measured>(offsetof(CurrentSensorUpdate<sym>, i_angle_measured), "i_angle_measured"),
    };
};




} // namespace power_grid_model::meta_data

// clang-format on