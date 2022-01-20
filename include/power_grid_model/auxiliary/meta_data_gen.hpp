// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_META_DATA_GEN_HPP
#define POWER_GRID_MODEL_META_DATA_GEN_HPP

#include <map>
#include <string>

#include "../power_grid_model.hpp"
#include "input.hpp"
#include "meta_data.hpp"
#include "output.hpp"

// generate of meta data
namespace power_grid_model {

namespace meta_data {

inline PowerGridMetaData get_input_meta_data() {
    PowerGridMetaData meta_map;
    // node
    meta_map["node"] = POWER_GRID_MODEL_META_DATA_TYPE(NodeInput, id, u_rated);
    // branch
    meta_map["line"] = POWER_GRID_MODEL_META_DATA_TYPE(LineInput, id, from_node, to_node, from_status, to_status, r1,
                                                       x1, c1, tan1, r0, x0, c0, tan0, i_n);
    meta_map["link"] = POWER_GRID_MODEL_META_DATA_TYPE(LinkInput, id, from_node, to_node, from_status, to_status);
    meta_map["transformer"] = POWER_GRID_MODEL_META_DATA_TYPE(
        TransformerInput, id, from_node, to_node, from_status, to_status, u1, u2, sn, uk, pk, i0, p0, winding_from,
        winding_to, clock, tap_side, tap_pos, tap_min, tap_max, tap_nom, tap_size, uk_min, uk_max, pk_min, pk_max,
        r_grounding_from, x_grounding_from, r_grounding_to, x_grounding_to);
    // appliance
    meta_map["sym_load"] =
        POWER_GRID_MODEL_META_DATA_TYPE(SymLoadGenInput, id, node, status, type, p_specified, q_specified);
    meta_map["sym_gen"] = meta_map["sym_load"];
    meta_map["asym_load"] =
        POWER_GRID_MODEL_META_DATA_TYPE(AsymLoadGenInput, id, node, status, type, p_specified, q_specified);
    meta_map["asym_gen"] = meta_map["asym_load"];

    meta_map["source"] = POWER_GRID_MODEL_META_DATA_TYPE(SourceInput, id, node, status, u_ref, sk, rx_ratio, z01_ratio);
    meta_map["shunt"] = POWER_GRID_MODEL_META_DATA_TYPE(ShuntInput, id, node, status, g1, b1, g0, b0);
    meta_map["sym_voltage_sensor"] = POWER_GRID_MODEL_META_DATA_TYPE(SymVoltageSensorInput, id, measured_object,
                                                                     u_sigma, u_measured, u_angle_measured);
    meta_map["asym_voltage_sensor"] = POWER_GRID_MODEL_META_DATA_TYPE(AsymVoltageSensorInput, id, measured_object,
                                                                      u_sigma, u_measured, u_angle_measured);
    meta_map["sym_power_sensor"] = POWER_GRID_MODEL_META_DATA_TYPE(
        SymPowerSensorInput, id, measured_object, measured_terminal_type, power_sigma, p_measured, q_measured);
    meta_map["asym_power_sensor"] = POWER_GRID_MODEL_META_DATA_TYPE(
        AsymPowerSensorInput, id, measured_object, measured_terminal_type, power_sigma, p_measured, q_measured);
    return meta_map;
}

template <bool sym>
inline PowerGridMetaData get_output_meta_data() {
    PowerGridMetaData meta_map;
    auto const node = POWER_GRID_MODEL_META_DATA_TYPE(NodeOutput<sym>, id, energized, u_pu, u, u_angle);
    auto const branch = POWER_GRID_MODEL_META_DATA_TYPE(BranchOutput<sym>, id, energized, loading, p_from, q_from,
                                                        i_from, s_from, p_to, q_to, i_to, s_to);
    auto const appliance = POWER_GRID_MODEL_META_DATA_TYPE(ApplianceOutput<sym>, id, energized, p, q, i, s, pf);
    auto const voltage_sensor =
        POWER_GRID_MODEL_META_DATA_TYPE(VoltageSensorOutput<sym>, id, energized, u_residual, u_angle_residual);
    auto const power_sensor =
        POWER_GRID_MODEL_META_DATA_TYPE(PowerSensorOutput<sym>, id, energized, p_residual, q_residual);
    // node
    meta_map["node"] = node;
    // branch
    meta_map["line"] = branch;
    meta_map["link"] = branch;
    meta_map["transformer"] = branch;
    // appliance
    meta_map["sym_load"] = appliance;
    meta_map["sym_gen"] = appliance;
    meta_map["asym_load"] = appliance;
    meta_map["asym_gen"] = appliance;
    meta_map["source"] = appliance;
    meta_map["shunt"] = appliance;
    // sensor
    meta_map["sym_voltage_sensor"] = voltage_sensor;
    meta_map["asym_voltage_sensor"] = voltage_sensor;
    meta_map["sym_power_sensor"] = power_sensor;
    meta_map["asym_power_sensor"] = power_sensor;
    return meta_map;
}

inline PowerGridMetaData get_update_meta_data() {
    PowerGridMetaData meta_map;
    // node: no update
    meta_map["node"] = POWER_GRID_MODEL_META_DATA_TYPE(BaseInput, id);
    // branch
    meta_map["line"] = POWER_GRID_MODEL_META_DATA_TYPE(BranchUpdate, id, from_status, to_status);
    meta_map["link"] = meta_map["line"];
    meta_map["transformer"] = POWER_GRID_MODEL_META_DATA_TYPE(TransformerUpdate, id, from_status, to_status, tap_pos);
    // appliance
    meta_map["sym_load"] = POWER_GRID_MODEL_META_DATA_TYPE(SymLoadGenUpdate, id, status, p_specified, q_specified);
    meta_map["sym_gen"] = meta_map["sym_load"];
    meta_map["asym_load"] = POWER_GRID_MODEL_META_DATA_TYPE(AsymLoadGenUpdate, id, status, p_specified, q_specified);
    meta_map["asym_gen"] = meta_map["asym_load"];

    meta_map["source"] = POWER_GRID_MODEL_META_DATA_TYPE(SourceUpdate, id, status, u_ref);
    meta_map["shunt"] = POWER_GRID_MODEL_META_DATA_TYPE(ApplianceUpdate, id, status);
    // sensor
    meta_map["sym_voltage_sensor"] =
        POWER_GRID_MODEL_META_DATA_TYPE(SymVoltageSensorUpdate, id, u_sigma, u_measured, u_angle_measured);
    meta_map["asym_voltage_sensor"] =
        POWER_GRID_MODEL_META_DATA_TYPE(AsymVoltageSensorUpdate, id, u_sigma, u_measured, u_angle_measured);
    meta_map["sym_power_sensor"] =
        POWER_GRID_MODEL_META_DATA_TYPE(SymPowerSensorUpdate, id, power_sigma, p_measured, q_measured);
    meta_map["asym_power_sensor"] =
        POWER_GRID_MODEL_META_DATA_TYPE(AsymPowerSensorUpdate, id, power_sigma, p_measured, q_measured);
    return meta_map;
}

inline AllPowerGridMetaData get_meta_data() {
    AllPowerGridMetaData meta_data;
    meta_data["input"] = get_input_meta_data();
    meta_data["sym_output"] = get_output_meta_data<true>();
    meta_data["asym_output"] = get_output_meta_data<false>();
    meta_data["update"] = get_update_meta_data();
    return meta_data;
}

inline AllPowerGridMetaData const& meta_data() {
    static AllPowerGridMetaData const meta = get_meta_data();
    return meta;
}

}  // namespace meta_data

}  // namespace power_grid_model

#endif
