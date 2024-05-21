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
            
            meta_data_gen::get_meta_attribute<BaseUpdate, &BaseUpdate::id, offsetof(BaseUpdate, id), []{ return "id"; }>::value,
    };
};

template<>
struct get_attributes_list<BranchUpdate> {
    static constexpr std::array<MetaAttribute, 3> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<BranchUpdate, &BranchUpdate::id, offsetof(BranchUpdate, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<BranchUpdate, &BranchUpdate::from_status, offsetof(BranchUpdate, from_status), []{ return "from_status"; }>::value,
            meta_data_gen::get_meta_attribute<BranchUpdate, &BranchUpdate::to_status, offsetof(BranchUpdate, to_status), []{ return "to_status"; }>::value,
    };
};

template<>
struct get_attributes_list<Branch3Update> {
    static constexpr std::array<MetaAttribute, 4> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<Branch3Update, &Branch3Update::id, offsetof(Branch3Update, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<Branch3Update, &Branch3Update::status_1, offsetof(Branch3Update, status_1), []{ return "status_1"; }>::value,
            meta_data_gen::get_meta_attribute<Branch3Update, &Branch3Update::status_2, offsetof(Branch3Update, status_2), []{ return "status_2"; }>::value,
            meta_data_gen::get_meta_attribute<Branch3Update, &Branch3Update::status_3, offsetof(Branch3Update, status_3), []{ return "status_3"; }>::value,
    };
};

template<>
struct get_attributes_list<ApplianceUpdate> {
    static constexpr std::array<MetaAttribute, 2> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<ApplianceUpdate, &ApplianceUpdate::id, offsetof(ApplianceUpdate, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<ApplianceUpdate, &ApplianceUpdate::status, offsetof(ApplianceUpdate, status), []{ return "status"; }>::value,
    };
};

template<>
struct get_attributes_list<TransformerUpdate> {
    static constexpr std::array<MetaAttribute, 4> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<TransformerUpdate, &TransformerUpdate::id, offsetof(TransformerUpdate, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerUpdate, &TransformerUpdate::from_status, offsetof(TransformerUpdate, from_status), []{ return "from_status"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerUpdate, &TransformerUpdate::to_status, offsetof(TransformerUpdate, to_status), []{ return "to_status"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerUpdate, &TransformerUpdate::tap_pos, offsetof(TransformerUpdate, tap_pos), []{ return "tap_pos"; }>::value,
    };
};

template<>
struct get_attributes_list<ThreeWindingTransformerUpdate> {
    static constexpr std::array<MetaAttribute, 5> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerUpdate, &ThreeWindingTransformerUpdate::id, offsetof(ThreeWindingTransformerUpdate, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerUpdate, &ThreeWindingTransformerUpdate::status_1, offsetof(ThreeWindingTransformerUpdate, status_1), []{ return "status_1"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerUpdate, &ThreeWindingTransformerUpdate::status_2, offsetof(ThreeWindingTransformerUpdate, status_2), []{ return "status_2"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerUpdate, &ThreeWindingTransformerUpdate::status_3, offsetof(ThreeWindingTransformerUpdate, status_3), []{ return "status_3"; }>::value,
            meta_data_gen::get_meta_attribute<ThreeWindingTransformerUpdate, &ThreeWindingTransformerUpdate::tap_pos, offsetof(ThreeWindingTransformerUpdate, tap_pos), []{ return "tap_pos"; }>::value,
    };
};

template <symmetry_tag sym_type>
struct get_attributes_list<LoadGenUpdate<sym_type>> {
    using sym = sym_type;

    static constexpr std::array<MetaAttribute, 4> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<LoadGenUpdate<sym>, &LoadGenUpdate<sym>::id, offsetof(LoadGenUpdate<sym>, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<LoadGenUpdate<sym>, &LoadGenUpdate<sym>::status, offsetof(LoadGenUpdate<sym>, status), []{ return "status"; }>::value,
            meta_data_gen::get_meta_attribute<LoadGenUpdate<sym>, &LoadGenUpdate<sym>::p_specified, offsetof(LoadGenUpdate<sym>, p_specified), []{ return "p_specified"; }>::value,
            meta_data_gen::get_meta_attribute<LoadGenUpdate<sym>, &LoadGenUpdate<sym>::q_specified, offsetof(LoadGenUpdate<sym>, q_specified), []{ return "q_specified"; }>::value,
    };
};

template<>
struct get_attributes_list<SourceUpdate> {
    static constexpr std::array<MetaAttribute, 4> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<SourceUpdate, &SourceUpdate::id, offsetof(SourceUpdate, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<SourceUpdate, &SourceUpdate::status, offsetof(SourceUpdate, status), []{ return "status"; }>::value,
            meta_data_gen::get_meta_attribute<SourceUpdate, &SourceUpdate::u_ref, offsetof(SourceUpdate, u_ref), []{ return "u_ref"; }>::value,
            meta_data_gen::get_meta_attribute<SourceUpdate, &SourceUpdate::u_ref_angle, offsetof(SourceUpdate, u_ref_angle), []{ return "u_ref_angle"; }>::value,
    };
};

template<>
struct get_attributes_list<ShuntUpdate> {
    static constexpr std::array<MetaAttribute, 6> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<ShuntUpdate, &ShuntUpdate::id, offsetof(ShuntUpdate, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<ShuntUpdate, &ShuntUpdate::status, offsetof(ShuntUpdate, status), []{ return "status"; }>::value,
            meta_data_gen::get_meta_attribute<ShuntUpdate, &ShuntUpdate::g1, offsetof(ShuntUpdate, g1), []{ return "g1"; }>::value,
            meta_data_gen::get_meta_attribute<ShuntUpdate, &ShuntUpdate::b1, offsetof(ShuntUpdate, b1), []{ return "b1"; }>::value,
            meta_data_gen::get_meta_attribute<ShuntUpdate, &ShuntUpdate::g0, offsetof(ShuntUpdate, g0), []{ return "g0"; }>::value,
            meta_data_gen::get_meta_attribute<ShuntUpdate, &ShuntUpdate::b0, offsetof(ShuntUpdate, b0), []{ return "b0"; }>::value,
    };
};

template <symmetry_tag sym_type>
struct get_attributes_list<VoltageSensorUpdate<sym_type>> {
    using sym = sym_type;

    static constexpr std::array<MetaAttribute, 4> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<VoltageSensorUpdate<sym>, &VoltageSensorUpdate<sym>::id, offsetof(VoltageSensorUpdate<sym>, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<VoltageSensorUpdate<sym>, &VoltageSensorUpdate<sym>::u_sigma, offsetof(VoltageSensorUpdate<sym>, u_sigma), []{ return "u_sigma"; }>::value,
            meta_data_gen::get_meta_attribute<VoltageSensorUpdate<sym>, &VoltageSensorUpdate<sym>::u_measured, offsetof(VoltageSensorUpdate<sym>, u_measured), []{ return "u_measured"; }>::value,
            meta_data_gen::get_meta_attribute<VoltageSensorUpdate<sym>, &VoltageSensorUpdate<sym>::u_angle_measured, offsetof(VoltageSensorUpdate<sym>, u_angle_measured), []{ return "u_angle_measured"; }>::value,
    };
};

template <symmetry_tag sym_type>
struct get_attributes_list<PowerSensorUpdate<sym_type>> {
    using sym = sym_type;

    static constexpr std::array<MetaAttribute, 6> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<PowerSensorUpdate<sym>, &PowerSensorUpdate<sym>::id, offsetof(PowerSensorUpdate<sym>, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<PowerSensorUpdate<sym>, &PowerSensorUpdate<sym>::power_sigma, offsetof(PowerSensorUpdate<sym>, power_sigma), []{ return "power_sigma"; }>::value,
            meta_data_gen::get_meta_attribute<PowerSensorUpdate<sym>, &PowerSensorUpdate<sym>::p_measured, offsetof(PowerSensorUpdate<sym>, p_measured), []{ return "p_measured"; }>::value,
            meta_data_gen::get_meta_attribute<PowerSensorUpdate<sym>, &PowerSensorUpdate<sym>::q_measured, offsetof(PowerSensorUpdate<sym>, q_measured), []{ return "q_measured"; }>::value,
            meta_data_gen::get_meta_attribute<PowerSensorUpdate<sym>, &PowerSensorUpdate<sym>::p_sigma, offsetof(PowerSensorUpdate<sym>, p_sigma), []{ return "p_sigma"; }>::value,
            meta_data_gen::get_meta_attribute<PowerSensorUpdate<sym>, &PowerSensorUpdate<sym>::q_sigma, offsetof(PowerSensorUpdate<sym>, q_sigma), []{ return "q_sigma"; }>::value,
    };
};

template<>
struct get_attributes_list<FaultUpdate> {
    static constexpr std::array<MetaAttribute, 7> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<FaultUpdate, &FaultUpdate::id, offsetof(FaultUpdate, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<FaultUpdate, &FaultUpdate::status, offsetof(FaultUpdate, status), []{ return "status"; }>::value,
            meta_data_gen::get_meta_attribute<FaultUpdate, &FaultUpdate::fault_type, offsetof(FaultUpdate, fault_type), []{ return "fault_type"; }>::value,
            meta_data_gen::get_meta_attribute<FaultUpdate, &FaultUpdate::fault_phase, offsetof(FaultUpdate, fault_phase), []{ return "fault_phase"; }>::value,
            meta_data_gen::get_meta_attribute<FaultUpdate, &FaultUpdate::fault_object, offsetof(FaultUpdate, fault_object), []{ return "fault_object"; }>::value,
            meta_data_gen::get_meta_attribute<FaultUpdate, &FaultUpdate::r_f, offsetof(FaultUpdate, r_f), []{ return "r_f"; }>::value,
            meta_data_gen::get_meta_attribute<FaultUpdate, &FaultUpdate::x_f, offsetof(FaultUpdate, x_f), []{ return "x_f"; }>::value,
    };
};

template<>
struct get_attributes_list<RegulatorUpdate> {
    static constexpr std::array<MetaAttribute, 2> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<RegulatorUpdate, &RegulatorUpdate::id, offsetof(RegulatorUpdate, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<RegulatorUpdate, &RegulatorUpdate::status, offsetof(RegulatorUpdate, status), []{ return "status"; }>::value,
    };
};

template<>
struct get_attributes_list<TransformerTapRegulatorUpdate> {
    static constexpr std::array<MetaAttribute, 6> value{
            // all attributes including base class
            
            meta_data_gen::get_meta_attribute<TransformerTapRegulatorUpdate, &TransformerTapRegulatorUpdate::id, offsetof(TransformerTapRegulatorUpdate, id), []{ return "id"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerTapRegulatorUpdate, &TransformerTapRegulatorUpdate::status, offsetof(TransformerTapRegulatorUpdate, status), []{ return "status"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerTapRegulatorUpdate, &TransformerTapRegulatorUpdate::u_set, offsetof(TransformerTapRegulatorUpdate, u_set), []{ return "u_set"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerTapRegulatorUpdate, &TransformerTapRegulatorUpdate::u_band, offsetof(TransformerTapRegulatorUpdate, u_band), []{ return "u_band"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerTapRegulatorUpdate, &TransformerTapRegulatorUpdate::line_drop_compensation_r, offsetof(TransformerTapRegulatorUpdate, line_drop_compensation_r), []{ return "line_drop_compensation_r"; }>::value,
            meta_data_gen::get_meta_attribute<TransformerTapRegulatorUpdate, &TransformerTapRegulatorUpdate::line_drop_compensation_x, offsetof(TransformerTapRegulatorUpdate, line_drop_compensation_x), []{ return "line_drop_compensation_x"; }>::value,
    };
};




} // namespace power_grid_model::meta_data

// clang-format on