// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// This header file is automatically generated. DO NOT modify it manually!

// clang-format off

/**
 * @brief header file which includes helper extern global variables of meta data pointers 
 *    to all datasets, compoments, and attributes
 *
 */

#pragma once
#ifndef POWER_GRID_MODEL_C_DATASET_DEFINITIONS_H
#define POWER_GRID_MODEL_C_DATASET_DEFINITIONS_H

#include "basics.h"

#ifdef __cplusplus
extern "C" {
#endif

// dataset input
PGM_API extern PGM_MetaDataset const* const PGM_def_input;
// components of input
// component node
PGM_API extern PGM_MetaComponent const* const PGM_def_input_node;
// attributes of input node
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_node_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_node_u_rated;
// component line
PGM_API extern PGM_MetaComponent const* const PGM_def_input_line;
// attributes of input line
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_line_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_line_from_node;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_line_to_node;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_line_from_status;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_line_to_status;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_line_r1;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_line_x1;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_line_c1;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_line_tan1;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_line_r0;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_line_x0;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_line_c0;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_line_tan0;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_line_i_n;
// component link
PGM_API extern PGM_MetaComponent const* const PGM_def_input_link;
// attributes of input link
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_link_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_link_from_node;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_link_to_node;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_link_from_status;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_link_to_status;
// component transformer
PGM_API extern PGM_MetaComponent const* const PGM_def_input_transformer;
// attributes of input transformer
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_from_node;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_to_node;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_from_status;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_to_status;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_u1;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_u2;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_sn;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_uk;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_pk;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_i0;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_p0;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_winding_from;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_winding_to;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_clock;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_tap_side;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_tap_pos;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_tap_min;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_tap_max;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_tap_nom;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_tap_size;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_uk_min;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_uk_max;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_pk_min;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_pk_max;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_r_grounding_from;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_x_grounding_from;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_r_grounding_to;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_x_grounding_to;
// component transformer_tap_regulator
PGM_API extern PGM_MetaComponent const* const PGM_def_input_transformer_tap_regulator;
// attributes of input transformer_tap_regulator
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_tap_regulator_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_tap_regulator_regulated_object;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_tap_regulator_status;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_tap_regulator_control_side;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_tap_regulator_u_set;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_tap_regulator_u_band;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_tap_regulator_line_drop_compensation_r;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_transformer_tap_regulator_line_drop_compensation_x;
// component three_winding_transformer
PGM_API extern PGM_MetaComponent const* const PGM_def_input_three_winding_transformer;
// attributes of input three_winding_transformer
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_node_1;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_node_2;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_node_3;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_status_1;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_status_2;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_status_3;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_u1;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_u2;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_u3;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_sn_1;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_sn_2;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_sn_3;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_uk_12;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_uk_13;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_uk_23;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_pk_12;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_pk_13;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_pk_23;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_i0;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_p0;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_winding_1;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_winding_2;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_winding_3;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_clock_12;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_clock_13;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_tap_side;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_tap_pos;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_tap_min;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_tap_max;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_tap_nom;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_tap_size;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_uk_12_min;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_uk_12_max;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_uk_13_min;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_uk_13_max;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_uk_23_min;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_uk_23_max;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_pk_12_min;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_pk_12_max;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_pk_13_min;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_pk_13_max;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_pk_23_min;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_pk_23_max;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_r_grounding_1;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_x_grounding_1;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_r_grounding_2;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_x_grounding_2;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_r_grounding_3;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_three_winding_transformer_x_grounding_3;
// component sym_load
PGM_API extern PGM_MetaComponent const* const PGM_def_input_sym_load;
// attributes of input sym_load
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_sym_load_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_sym_load_node;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_sym_load_status;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_sym_load_type;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_sym_load_p_specified;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_sym_load_q_specified;
// component sym_gen
PGM_API extern PGM_MetaComponent const* const PGM_def_input_sym_gen;
// attributes of input sym_gen
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_sym_gen_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_sym_gen_node;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_sym_gen_status;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_sym_gen_type;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_sym_gen_p_specified;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_sym_gen_q_specified;
// component asym_load
PGM_API extern PGM_MetaComponent const* const PGM_def_input_asym_load;
// attributes of input asym_load
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_asym_load_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_asym_load_node;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_asym_load_status;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_asym_load_type;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_asym_load_p_specified;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_asym_load_q_specified;
// component asym_gen
PGM_API extern PGM_MetaComponent const* const PGM_def_input_asym_gen;
// attributes of input asym_gen
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_asym_gen_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_asym_gen_node;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_asym_gen_status;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_asym_gen_type;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_asym_gen_p_specified;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_asym_gen_q_specified;
// component shunt
PGM_API extern PGM_MetaComponent const* const PGM_def_input_shunt;
// attributes of input shunt
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_shunt_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_shunt_node;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_shunt_status;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_shunt_g1;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_shunt_b1;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_shunt_g0;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_shunt_b0;
// component source
PGM_API extern PGM_MetaComponent const* const PGM_def_input_source;
// attributes of input source
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_source_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_source_node;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_source_status;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_source_u_ref;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_source_u_ref_angle;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_source_sk;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_source_rx_ratio;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_source_z01_ratio;
// component sym_voltage_sensor
PGM_API extern PGM_MetaComponent const* const PGM_def_input_sym_voltage_sensor;
// attributes of input sym_voltage_sensor
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_sym_voltage_sensor_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_sym_voltage_sensor_measured_object;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_sym_voltage_sensor_u_sigma;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_sym_voltage_sensor_u_measured;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_sym_voltage_sensor_u_angle_measured;
// component asym_voltage_sensor
PGM_API extern PGM_MetaComponent const* const PGM_def_input_asym_voltage_sensor;
// attributes of input asym_voltage_sensor
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_asym_voltage_sensor_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_asym_voltage_sensor_measured_object;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_asym_voltage_sensor_u_sigma;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_asym_voltage_sensor_u_measured;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_asym_voltage_sensor_u_angle_measured;
// component sym_power_sensor
PGM_API extern PGM_MetaComponent const* const PGM_def_input_sym_power_sensor;
// attributes of input sym_power_sensor
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_sym_power_sensor_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_sym_power_sensor_measured_object;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_sym_power_sensor_measured_terminal_type;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_sym_power_sensor_power_sigma;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_sym_power_sensor_p_measured;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_sym_power_sensor_q_measured;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_sym_power_sensor_p_sigma;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_sym_power_sensor_q_sigma;
// component asym_power_sensor
PGM_API extern PGM_MetaComponent const* const PGM_def_input_asym_power_sensor;
// attributes of input asym_power_sensor
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_asym_power_sensor_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_asym_power_sensor_measured_object;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_asym_power_sensor_measured_terminal_type;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_asym_power_sensor_power_sigma;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_asym_power_sensor_p_measured;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_asym_power_sensor_q_measured;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_asym_power_sensor_p_sigma;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_asym_power_sensor_q_sigma;
// component fault
PGM_API extern PGM_MetaComponent const* const PGM_def_input_fault;
// attributes of input fault
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_fault_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_fault_status;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_fault_fault_type;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_fault_fault_phase;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_fault_fault_object;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_fault_r_f;
PGM_API extern PGM_MetaAttribute const* const PGM_def_input_fault_x_f;
// dataset sym_output
PGM_API extern PGM_MetaDataset const* const PGM_def_sym_output;
// components of sym_output
// component node
PGM_API extern PGM_MetaComponent const* const PGM_def_sym_output_node;
// attributes of sym_output node
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_node_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_node_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_node_u_pu;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_node_u;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_node_u_angle;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_node_p;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_node_q;
// component line
PGM_API extern PGM_MetaComponent const* const PGM_def_sym_output_line;
// attributes of sym_output line
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_line_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_line_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_line_loading;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_line_p_from;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_line_q_from;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_line_i_from;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_line_s_from;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_line_p_to;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_line_q_to;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_line_i_to;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_line_s_to;
// component link
PGM_API extern PGM_MetaComponent const* const PGM_def_sym_output_link;
// attributes of sym_output link
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_link_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_link_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_link_loading;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_link_p_from;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_link_q_from;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_link_i_from;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_link_s_from;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_link_p_to;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_link_q_to;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_link_i_to;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_link_s_to;
// component transformer
PGM_API extern PGM_MetaComponent const* const PGM_def_sym_output_transformer;
// attributes of sym_output transformer
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_transformer_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_transformer_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_transformer_loading;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_transformer_p_from;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_transformer_q_from;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_transformer_i_from;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_transformer_s_from;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_transformer_p_to;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_transformer_q_to;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_transformer_i_to;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_transformer_s_to;
// component transformer_tap_regulator
PGM_API extern PGM_MetaComponent const* const PGM_def_sym_output_transformer_tap_regulator;
// attributes of sym_output transformer_tap_regulator
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_transformer_tap_regulator_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_transformer_tap_regulator_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_transformer_tap_regulator_tap_pos;
// component three_winding_transformer
PGM_API extern PGM_MetaComponent const* const PGM_def_sym_output_three_winding_transformer;
// attributes of sym_output three_winding_transformer
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_three_winding_transformer_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_three_winding_transformer_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_three_winding_transformer_loading;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_three_winding_transformer_p_1;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_three_winding_transformer_q_1;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_three_winding_transformer_i_1;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_three_winding_transformer_s_1;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_three_winding_transformer_p_2;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_three_winding_transformer_q_2;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_three_winding_transformer_i_2;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_three_winding_transformer_s_2;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_three_winding_transformer_p_3;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_three_winding_transformer_q_3;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_three_winding_transformer_i_3;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_three_winding_transformer_s_3;
// component sym_load
PGM_API extern PGM_MetaComponent const* const PGM_def_sym_output_sym_load;
// attributes of sym_output sym_load
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_sym_load_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_sym_load_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_sym_load_p;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_sym_load_q;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_sym_load_i;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_sym_load_s;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_sym_load_pf;
// component sym_gen
PGM_API extern PGM_MetaComponent const* const PGM_def_sym_output_sym_gen;
// attributes of sym_output sym_gen
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_sym_gen_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_sym_gen_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_sym_gen_p;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_sym_gen_q;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_sym_gen_i;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_sym_gen_s;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_sym_gen_pf;
// component asym_load
PGM_API extern PGM_MetaComponent const* const PGM_def_sym_output_asym_load;
// attributes of sym_output asym_load
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_asym_load_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_asym_load_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_asym_load_p;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_asym_load_q;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_asym_load_i;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_asym_load_s;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_asym_load_pf;
// component asym_gen
PGM_API extern PGM_MetaComponent const* const PGM_def_sym_output_asym_gen;
// attributes of sym_output asym_gen
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_asym_gen_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_asym_gen_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_asym_gen_p;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_asym_gen_q;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_asym_gen_i;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_asym_gen_s;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_asym_gen_pf;
// component shunt
PGM_API extern PGM_MetaComponent const* const PGM_def_sym_output_shunt;
// attributes of sym_output shunt
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_shunt_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_shunt_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_shunt_p;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_shunt_q;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_shunt_i;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_shunt_s;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_shunt_pf;
// component source
PGM_API extern PGM_MetaComponent const* const PGM_def_sym_output_source;
// attributes of sym_output source
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_source_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_source_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_source_p;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_source_q;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_source_i;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_source_s;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_source_pf;
// component sym_voltage_sensor
PGM_API extern PGM_MetaComponent const* const PGM_def_sym_output_sym_voltage_sensor;
// attributes of sym_output sym_voltage_sensor
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_sym_voltage_sensor_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_sym_voltage_sensor_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_sym_voltage_sensor_u_residual;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_sym_voltage_sensor_u_angle_residual;
// component asym_voltage_sensor
PGM_API extern PGM_MetaComponent const* const PGM_def_sym_output_asym_voltage_sensor;
// attributes of sym_output asym_voltage_sensor
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_asym_voltage_sensor_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_asym_voltage_sensor_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_asym_voltage_sensor_u_residual;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_asym_voltage_sensor_u_angle_residual;
// component sym_power_sensor
PGM_API extern PGM_MetaComponent const* const PGM_def_sym_output_sym_power_sensor;
// attributes of sym_output sym_power_sensor
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_sym_power_sensor_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_sym_power_sensor_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_sym_power_sensor_p_residual;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_sym_power_sensor_q_residual;
// component asym_power_sensor
PGM_API extern PGM_MetaComponent const* const PGM_def_sym_output_asym_power_sensor;
// attributes of sym_output asym_power_sensor
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_asym_power_sensor_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_asym_power_sensor_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_asym_power_sensor_p_residual;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_asym_power_sensor_q_residual;
// component fault
PGM_API extern PGM_MetaComponent const* const PGM_def_sym_output_fault;
// attributes of sym_output fault
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_fault_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sym_output_fault_energized;
// dataset asym_output
PGM_API extern PGM_MetaDataset const* const PGM_def_asym_output;
// components of asym_output
// component node
PGM_API extern PGM_MetaComponent const* const PGM_def_asym_output_node;
// attributes of asym_output node
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_node_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_node_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_node_u_pu;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_node_u;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_node_u_angle;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_node_p;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_node_q;
// component line
PGM_API extern PGM_MetaComponent const* const PGM_def_asym_output_line;
// attributes of asym_output line
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_line_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_line_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_line_loading;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_line_p_from;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_line_q_from;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_line_i_from;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_line_s_from;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_line_p_to;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_line_q_to;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_line_i_to;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_line_s_to;
// component link
PGM_API extern PGM_MetaComponent const* const PGM_def_asym_output_link;
// attributes of asym_output link
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_link_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_link_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_link_loading;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_link_p_from;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_link_q_from;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_link_i_from;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_link_s_from;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_link_p_to;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_link_q_to;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_link_i_to;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_link_s_to;
// component transformer
PGM_API extern PGM_MetaComponent const* const PGM_def_asym_output_transformer;
// attributes of asym_output transformer
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_transformer_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_transformer_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_transformer_loading;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_transformer_p_from;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_transformer_q_from;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_transformer_i_from;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_transformer_s_from;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_transformer_p_to;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_transformer_q_to;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_transformer_i_to;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_transformer_s_to;
// component transformer_tap_regulator
PGM_API extern PGM_MetaComponent const* const PGM_def_asym_output_transformer_tap_regulator;
// attributes of asym_output transformer_tap_regulator
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_transformer_tap_regulator_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_transformer_tap_regulator_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_transformer_tap_regulator_tap_pos;
// component three_winding_transformer
PGM_API extern PGM_MetaComponent const* const PGM_def_asym_output_three_winding_transformer;
// attributes of asym_output three_winding_transformer
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_three_winding_transformer_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_three_winding_transformer_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_three_winding_transformer_loading;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_three_winding_transformer_p_1;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_three_winding_transformer_q_1;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_three_winding_transformer_i_1;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_three_winding_transformer_s_1;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_three_winding_transformer_p_2;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_three_winding_transformer_q_2;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_three_winding_transformer_i_2;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_three_winding_transformer_s_2;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_three_winding_transformer_p_3;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_three_winding_transformer_q_3;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_three_winding_transformer_i_3;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_three_winding_transformer_s_3;
// component sym_load
PGM_API extern PGM_MetaComponent const* const PGM_def_asym_output_sym_load;
// attributes of asym_output sym_load
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_sym_load_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_sym_load_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_sym_load_p;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_sym_load_q;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_sym_load_i;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_sym_load_s;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_sym_load_pf;
// component sym_gen
PGM_API extern PGM_MetaComponent const* const PGM_def_asym_output_sym_gen;
// attributes of asym_output sym_gen
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_sym_gen_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_sym_gen_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_sym_gen_p;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_sym_gen_q;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_sym_gen_i;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_sym_gen_s;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_sym_gen_pf;
// component asym_load
PGM_API extern PGM_MetaComponent const* const PGM_def_asym_output_asym_load;
// attributes of asym_output asym_load
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_asym_load_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_asym_load_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_asym_load_p;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_asym_load_q;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_asym_load_i;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_asym_load_s;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_asym_load_pf;
// component asym_gen
PGM_API extern PGM_MetaComponent const* const PGM_def_asym_output_asym_gen;
// attributes of asym_output asym_gen
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_asym_gen_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_asym_gen_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_asym_gen_p;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_asym_gen_q;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_asym_gen_i;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_asym_gen_s;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_asym_gen_pf;
// component shunt
PGM_API extern PGM_MetaComponent const* const PGM_def_asym_output_shunt;
// attributes of asym_output shunt
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_shunt_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_shunt_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_shunt_p;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_shunt_q;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_shunt_i;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_shunt_s;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_shunt_pf;
// component source
PGM_API extern PGM_MetaComponent const* const PGM_def_asym_output_source;
// attributes of asym_output source
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_source_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_source_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_source_p;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_source_q;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_source_i;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_source_s;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_source_pf;
// component sym_voltage_sensor
PGM_API extern PGM_MetaComponent const* const PGM_def_asym_output_sym_voltage_sensor;
// attributes of asym_output sym_voltage_sensor
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_sym_voltage_sensor_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_sym_voltage_sensor_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_sym_voltage_sensor_u_residual;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_sym_voltage_sensor_u_angle_residual;
// component asym_voltage_sensor
PGM_API extern PGM_MetaComponent const* const PGM_def_asym_output_asym_voltage_sensor;
// attributes of asym_output asym_voltage_sensor
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_asym_voltage_sensor_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_asym_voltage_sensor_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_asym_voltage_sensor_u_residual;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_asym_voltage_sensor_u_angle_residual;
// component sym_power_sensor
PGM_API extern PGM_MetaComponent const* const PGM_def_asym_output_sym_power_sensor;
// attributes of asym_output sym_power_sensor
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_sym_power_sensor_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_sym_power_sensor_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_sym_power_sensor_p_residual;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_sym_power_sensor_q_residual;
// component asym_power_sensor
PGM_API extern PGM_MetaComponent const* const PGM_def_asym_output_asym_power_sensor;
// attributes of asym_output asym_power_sensor
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_asym_power_sensor_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_asym_power_sensor_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_asym_power_sensor_p_residual;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_asym_power_sensor_q_residual;
// component fault
PGM_API extern PGM_MetaComponent const* const PGM_def_asym_output_fault;
// attributes of asym_output fault
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_fault_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_asym_output_fault_energized;
// dataset update
PGM_API extern PGM_MetaDataset const* const PGM_def_update;
// components of update
// component node
PGM_API extern PGM_MetaComponent const* const PGM_def_update_node;
// attributes of update node
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_node_id;
// component line
PGM_API extern PGM_MetaComponent const* const PGM_def_update_line;
// attributes of update line
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_line_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_line_from_status;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_line_to_status;
// component link
PGM_API extern PGM_MetaComponent const* const PGM_def_update_link;
// attributes of update link
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_link_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_link_from_status;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_link_to_status;
// component transformer
PGM_API extern PGM_MetaComponent const* const PGM_def_update_transformer;
// attributes of update transformer
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_transformer_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_transformer_from_status;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_transformer_to_status;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_transformer_tap_pos;
// component transformer_tap_regulator
PGM_API extern PGM_MetaComponent const* const PGM_def_update_transformer_tap_regulator;
// attributes of update transformer_tap_regulator
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_transformer_tap_regulator_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_transformer_tap_regulator_status;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_transformer_tap_regulator_u_set;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_transformer_tap_regulator_u_band;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_transformer_tap_regulator_line_drop_compensation_r;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_transformer_tap_regulator_line_drop_compensation_x;
// component three_winding_transformer
PGM_API extern PGM_MetaComponent const* const PGM_def_update_three_winding_transformer;
// attributes of update three_winding_transformer
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_three_winding_transformer_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_three_winding_transformer_status_1;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_three_winding_transformer_status_2;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_three_winding_transformer_status_3;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_three_winding_transformer_tap_pos;
// component sym_load
PGM_API extern PGM_MetaComponent const* const PGM_def_update_sym_load;
// attributes of update sym_load
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_sym_load_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_sym_load_status;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_sym_load_p_specified;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_sym_load_q_specified;
// component sym_gen
PGM_API extern PGM_MetaComponent const* const PGM_def_update_sym_gen;
// attributes of update sym_gen
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_sym_gen_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_sym_gen_status;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_sym_gen_p_specified;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_sym_gen_q_specified;
// component asym_load
PGM_API extern PGM_MetaComponent const* const PGM_def_update_asym_load;
// attributes of update asym_load
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_asym_load_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_asym_load_status;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_asym_load_p_specified;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_asym_load_q_specified;
// component asym_gen
PGM_API extern PGM_MetaComponent const* const PGM_def_update_asym_gen;
// attributes of update asym_gen
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_asym_gen_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_asym_gen_status;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_asym_gen_p_specified;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_asym_gen_q_specified;
// component shunt
PGM_API extern PGM_MetaComponent const* const PGM_def_update_shunt;
// attributes of update shunt
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_shunt_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_shunt_status;
// component source
PGM_API extern PGM_MetaComponent const* const PGM_def_update_source;
// attributes of update source
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_source_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_source_status;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_source_u_ref;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_source_u_ref_angle;
// component sym_voltage_sensor
PGM_API extern PGM_MetaComponent const* const PGM_def_update_sym_voltage_sensor;
// attributes of update sym_voltage_sensor
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_sym_voltage_sensor_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_sym_voltage_sensor_u_sigma;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_sym_voltage_sensor_u_measured;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_sym_voltage_sensor_u_angle_measured;
// component asym_voltage_sensor
PGM_API extern PGM_MetaComponent const* const PGM_def_update_asym_voltage_sensor;
// attributes of update asym_voltage_sensor
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_asym_voltage_sensor_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_asym_voltage_sensor_u_sigma;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_asym_voltage_sensor_u_measured;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_asym_voltage_sensor_u_angle_measured;
// component sym_power_sensor
PGM_API extern PGM_MetaComponent const* const PGM_def_update_sym_power_sensor;
// attributes of update sym_power_sensor
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_sym_power_sensor_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_sym_power_sensor_power_sigma;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_sym_power_sensor_p_measured;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_sym_power_sensor_q_measured;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_sym_power_sensor_p_sigma;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_sym_power_sensor_q_sigma;
// component asym_power_sensor
PGM_API extern PGM_MetaComponent const* const PGM_def_update_asym_power_sensor;
// attributes of update asym_power_sensor
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_asym_power_sensor_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_asym_power_sensor_power_sigma;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_asym_power_sensor_p_measured;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_asym_power_sensor_q_measured;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_asym_power_sensor_p_sigma;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_asym_power_sensor_q_sigma;
// component fault
PGM_API extern PGM_MetaComponent const* const PGM_def_update_fault;
// attributes of update fault
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_fault_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_fault_status;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_fault_fault_type;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_fault_fault_phase;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_fault_fault_object;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_fault_r_f;
PGM_API extern PGM_MetaAttribute const* const PGM_def_update_fault_x_f;
// dataset sc_output
PGM_API extern PGM_MetaDataset const* const PGM_def_sc_output;
// components of sc_output
// component node
PGM_API extern PGM_MetaComponent const* const PGM_def_sc_output_node;
// attributes of sc_output node
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_node_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_node_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_node_u_pu;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_node_u;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_node_u_angle;
// component line
PGM_API extern PGM_MetaComponent const* const PGM_def_sc_output_line;
// attributes of sc_output line
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_line_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_line_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_line_i_from;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_line_i_from_angle;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_line_i_to;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_line_i_to_angle;
// component link
PGM_API extern PGM_MetaComponent const* const PGM_def_sc_output_link;
// attributes of sc_output link
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_link_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_link_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_link_i_from;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_link_i_from_angle;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_link_i_to;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_link_i_to_angle;
// component transformer
PGM_API extern PGM_MetaComponent const* const PGM_def_sc_output_transformer;
// attributes of sc_output transformer
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_transformer_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_transformer_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_transformer_i_from;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_transformer_i_from_angle;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_transformer_i_to;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_transformer_i_to_angle;
// component three_winding_transformer
PGM_API extern PGM_MetaComponent const* const PGM_def_sc_output_three_winding_transformer;
// attributes of sc_output three_winding_transformer
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_three_winding_transformer_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_three_winding_transformer_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_three_winding_transformer_i_1;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_three_winding_transformer_i_1_angle;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_three_winding_transformer_i_2;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_three_winding_transformer_i_2_angle;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_three_winding_transformer_i_3;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_three_winding_transformer_i_3_angle;
// component sym_load
PGM_API extern PGM_MetaComponent const* const PGM_def_sc_output_sym_load;
// attributes of sc_output sym_load
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_sym_load_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_sym_load_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_sym_load_i;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_sym_load_i_angle;
// component sym_gen
PGM_API extern PGM_MetaComponent const* const PGM_def_sc_output_sym_gen;
// attributes of sc_output sym_gen
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_sym_gen_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_sym_gen_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_sym_gen_i;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_sym_gen_i_angle;
// component asym_load
PGM_API extern PGM_MetaComponent const* const PGM_def_sc_output_asym_load;
// attributes of sc_output asym_load
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_asym_load_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_asym_load_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_asym_load_i;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_asym_load_i_angle;
// component asym_gen
PGM_API extern PGM_MetaComponent const* const PGM_def_sc_output_asym_gen;
// attributes of sc_output asym_gen
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_asym_gen_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_asym_gen_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_asym_gen_i;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_asym_gen_i_angle;
// component shunt
PGM_API extern PGM_MetaComponent const* const PGM_def_sc_output_shunt;
// attributes of sc_output shunt
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_shunt_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_shunt_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_shunt_i;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_shunt_i_angle;
// component source
PGM_API extern PGM_MetaComponent const* const PGM_def_sc_output_source;
// attributes of sc_output source
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_source_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_source_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_source_i;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_source_i_angle;
// component sym_voltage_sensor
PGM_API extern PGM_MetaComponent const* const PGM_def_sc_output_sym_voltage_sensor;
// attributes of sc_output sym_voltage_sensor
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_sym_voltage_sensor_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_sym_voltage_sensor_energized;
// component asym_voltage_sensor
PGM_API extern PGM_MetaComponent const* const PGM_def_sc_output_asym_voltage_sensor;
// attributes of sc_output asym_voltage_sensor
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_asym_voltage_sensor_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_asym_voltage_sensor_energized;
// component sym_power_sensor
PGM_API extern PGM_MetaComponent const* const PGM_def_sc_output_sym_power_sensor;
// attributes of sc_output sym_power_sensor
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_sym_power_sensor_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_sym_power_sensor_energized;
// component asym_power_sensor
PGM_API extern PGM_MetaComponent const* const PGM_def_sc_output_asym_power_sensor;
// attributes of sc_output asym_power_sensor
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_asym_power_sensor_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_asym_power_sensor_energized;
// component fault
PGM_API extern PGM_MetaComponent const* const PGM_def_sc_output_fault;
// attributes of sc_output fault
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_fault_id;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_fault_energized;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_fault_i_f;
PGM_API extern PGM_MetaAttribute const* const PGM_def_sc_output_fault_i_f_angle;
//

#ifdef __cplusplus
}
#endif

#endif
// clang-format on
