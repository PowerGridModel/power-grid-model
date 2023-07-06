// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

// This header file is automatically generated. DO NOT modify it manually!

// clang-format off

#define PGM_DLL_EXPORTS
#include "power_grid_model_c/dataset_definitions.h"
#include "power_grid_model_c/meta_data.h"

// dataset input
PGM_MetaDataset const* PGM_zdef_input() {
    static PGM_MetaDataset const* const ptr = PGM_meta_get_dataset_by_name(nullptr, "input");
    return ptr;
}
// components of input
// component node
PGM_MetaComponent const* PGM_zdef_input_node() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "input", "node");
    return ptr;
}
// attributes of input node
PGM_MetaAttribute const* PGM_zdef_input_node_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "node", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_node_u_rated() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "node", "u_rated");
    return ptr;
}
// component line
PGM_MetaComponent const* PGM_zdef_input_line() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "input", "line");
    return ptr;
}
// attributes of input line
PGM_MetaAttribute const* PGM_zdef_input_line_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "line", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_line_from_node() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "line", "from_node");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_line_to_node() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "line", "to_node");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_line_from_status() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "line", "from_status");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_line_to_status() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "line", "to_status");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_line_r1() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "line", "r1");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_line_x1() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "line", "x1");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_line_c1() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "line", "c1");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_line_tan1() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "line", "tan1");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_line_r0() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "line", "r0");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_line_x0() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "line", "x0");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_line_c0() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "line", "c0");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_line_tan0() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "line", "tan0");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_line_i_n() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "line", "i_n");
    return ptr;
}
// component link
PGM_MetaComponent const* PGM_zdef_input_link() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "input", "link");
    return ptr;
}
// attributes of input link
PGM_MetaAttribute const* PGM_zdef_input_link_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "link", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_link_from_node() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "link", "from_node");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_link_to_node() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "link", "to_node");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_link_from_status() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "link", "from_status");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_link_to_status() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "link", "to_status");
    return ptr;
}
// component transformer
PGM_MetaComponent const* PGM_zdef_input_transformer() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "input", "transformer");
    return ptr;
}
// attributes of input transformer
PGM_MetaAttribute const* PGM_zdef_input_transformer_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "transformer", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_transformer_from_node() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "transformer", "from_node");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_transformer_to_node() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "transformer", "to_node");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_transformer_from_status() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "transformer", "from_status");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_transformer_to_status() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "transformer", "to_status");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_transformer_u1() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "transformer", "u1");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_transformer_u2() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "transformer", "u2");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_transformer_sn() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "transformer", "sn");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_transformer_uk() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "transformer", "uk");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_transformer_pk() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "transformer", "pk");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_transformer_i0() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "transformer", "i0");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_transformer_p0() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "transformer", "p0");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_transformer_winding_from() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "transformer", "winding_from");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_transformer_winding_to() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "transformer", "winding_to");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_transformer_clock() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "transformer", "clock");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_transformer_tap_side() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "transformer", "tap_side");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_transformer_tap_pos() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "transformer", "tap_pos");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_transformer_tap_min() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "transformer", "tap_min");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_transformer_tap_max() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "transformer", "tap_max");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_transformer_tap_nom() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "transformer", "tap_nom");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_transformer_tap_size() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "transformer", "tap_size");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_transformer_uk_min() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "transformer", "uk_min");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_transformer_uk_max() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "transformer", "uk_max");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_transformer_pk_min() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "transformer", "pk_min");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_transformer_pk_max() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "transformer", "pk_max");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_transformer_r_grounding_from() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "transformer", "r_grounding_from");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_transformer_x_grounding_from() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "transformer", "x_grounding_from");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_transformer_r_grounding_to() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "transformer", "r_grounding_to");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_transformer_x_grounding_to() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "transformer", "x_grounding_to");
    return ptr;
}
// component three_winding_transformer
PGM_MetaComponent const* PGM_zdef_input_three_winding_transformer() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "input", "three_winding_transformer");
    return ptr;
}
// attributes of input three_winding_transformer
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_node_1() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "node_1");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_node_2() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "node_2");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_node_3() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "node_3");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_status_1() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "status_1");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_status_2() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "status_2");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_status_3() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "status_3");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_u1() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "u1");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_u2() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "u2");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_u3() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "u3");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_sn_1() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "sn_1");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_sn_2() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "sn_2");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_sn_3() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "sn_3");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_uk_12() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "uk_12");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_uk_13() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "uk_13");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_uk_23() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "uk_23");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_pk_12() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "pk_12");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_pk_13() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "pk_13");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_pk_23() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "pk_23");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_i0() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "i0");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_p0() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "p0");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_winding_1() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "winding_1");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_winding_2() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "winding_2");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_winding_3() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "winding_3");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_clock_12() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "clock_12");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_clock_13() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "clock_13");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_tap_side() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "tap_side");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_tap_pos() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "tap_pos");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_tap_min() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "tap_min");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_tap_max() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "tap_max");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_tap_nom() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "tap_nom");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_tap_size() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "tap_size");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_uk_12_min() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "uk_12_min");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_uk_12_max() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "uk_12_max");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_uk_13_min() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "uk_13_min");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_uk_13_max() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "uk_13_max");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_uk_23_min() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "uk_23_min");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_uk_23_max() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "uk_23_max");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_pk_12_min() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "pk_12_min");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_pk_12_max() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "pk_12_max");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_pk_13_min() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "pk_13_min");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_pk_13_max() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "pk_13_max");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_pk_23_min() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "pk_23_min");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_pk_23_max() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "pk_23_max");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_r_grounding_1() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "r_grounding_1");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_x_grounding_1() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "x_grounding_1");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_r_grounding_2() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "r_grounding_2");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_x_grounding_2() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "x_grounding_2");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_r_grounding_3() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "r_grounding_3");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_three_winding_transformer_x_grounding_3() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "three_winding_transformer", "x_grounding_3");
    return ptr;
}
// component sym_load
PGM_MetaComponent const* PGM_zdef_input_sym_load() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "input", "sym_load");
    return ptr;
}
// attributes of input sym_load
PGM_MetaAttribute const* PGM_zdef_input_sym_load_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "sym_load", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_sym_load_node() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "sym_load", "node");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_sym_load_status() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "sym_load", "status");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_sym_load_type() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "sym_load", "type");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_sym_load_p_specified() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "sym_load", "p_specified");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_sym_load_q_specified() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "sym_load", "q_specified");
    return ptr;
}
// component sym_gen
PGM_MetaComponent const* PGM_zdef_input_sym_gen() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "input", "sym_gen");
    return ptr;
}
// attributes of input sym_gen
PGM_MetaAttribute const* PGM_zdef_input_sym_gen_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "sym_gen", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_sym_gen_node() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "sym_gen", "node");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_sym_gen_status() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "sym_gen", "status");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_sym_gen_type() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "sym_gen", "type");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_sym_gen_p_specified() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "sym_gen", "p_specified");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_sym_gen_q_specified() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "sym_gen", "q_specified");
    return ptr;
}
// component asym_load
PGM_MetaComponent const* PGM_zdef_input_asym_load() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "input", "asym_load");
    return ptr;
}
// attributes of input asym_load
PGM_MetaAttribute const* PGM_zdef_input_asym_load_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "asym_load", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_asym_load_node() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "asym_load", "node");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_asym_load_status() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "asym_load", "status");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_asym_load_type() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "asym_load", "type");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_asym_load_p_specified() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "asym_load", "p_specified");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_asym_load_q_specified() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "asym_load", "q_specified");
    return ptr;
}
// component asym_gen
PGM_MetaComponent const* PGM_zdef_input_asym_gen() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "input", "asym_gen");
    return ptr;
}
// attributes of input asym_gen
PGM_MetaAttribute const* PGM_zdef_input_asym_gen_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "asym_gen", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_asym_gen_node() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "asym_gen", "node");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_asym_gen_status() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "asym_gen", "status");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_asym_gen_type() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "asym_gen", "type");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_asym_gen_p_specified() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "asym_gen", "p_specified");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_asym_gen_q_specified() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "asym_gen", "q_specified");
    return ptr;
}
// component shunt
PGM_MetaComponent const* PGM_zdef_input_shunt() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "input", "shunt");
    return ptr;
}
// attributes of input shunt
PGM_MetaAttribute const* PGM_zdef_input_shunt_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "shunt", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_shunt_node() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "shunt", "node");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_shunt_status() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "shunt", "status");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_shunt_g1() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "shunt", "g1");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_shunt_b1() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "shunt", "b1");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_shunt_g0() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "shunt", "g0");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_shunt_b0() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "shunt", "b0");
    return ptr;
}
// component source
PGM_MetaComponent const* PGM_zdef_input_source() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "input", "source");
    return ptr;
}
// attributes of input source
PGM_MetaAttribute const* PGM_zdef_input_source_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "source", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_source_node() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "source", "node");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_source_status() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "source", "status");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_source_u_ref() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "source", "u_ref");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_source_u_ref_angle() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "source", "u_ref_angle");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_source_sk() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "source", "sk");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_source_rx_ratio() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "source", "rx_ratio");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_source_z01_ratio() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "source", "z01_ratio");
    return ptr;
}
// component sym_voltage_sensor
PGM_MetaComponent const* PGM_zdef_input_sym_voltage_sensor() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "input", "sym_voltage_sensor");
    return ptr;
}
// attributes of input sym_voltage_sensor
PGM_MetaAttribute const* PGM_zdef_input_sym_voltage_sensor_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "sym_voltage_sensor", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_sym_voltage_sensor_measured_object() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "sym_voltage_sensor", "measured_object");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_sym_voltage_sensor_u_sigma() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "sym_voltage_sensor", "u_sigma");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_sym_voltage_sensor_u_measured() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "sym_voltage_sensor", "u_measured");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_sym_voltage_sensor_u_angle_measured() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "sym_voltage_sensor", "u_angle_measured");
    return ptr;
}
// component asym_voltage_sensor
PGM_MetaComponent const* PGM_zdef_input_asym_voltage_sensor() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "input", "asym_voltage_sensor");
    return ptr;
}
// attributes of input asym_voltage_sensor
PGM_MetaAttribute const* PGM_zdef_input_asym_voltage_sensor_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "asym_voltage_sensor", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_asym_voltage_sensor_measured_object() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "asym_voltage_sensor", "measured_object");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_asym_voltage_sensor_u_sigma() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "asym_voltage_sensor", "u_sigma");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_asym_voltage_sensor_u_measured() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "asym_voltage_sensor", "u_measured");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_asym_voltage_sensor_u_angle_measured() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "asym_voltage_sensor", "u_angle_measured");
    return ptr;
}
// component sym_power_sensor
PGM_MetaComponent const* PGM_zdef_input_sym_power_sensor() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "input", "sym_power_sensor");
    return ptr;
}
// attributes of input sym_power_sensor
PGM_MetaAttribute const* PGM_zdef_input_sym_power_sensor_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "sym_power_sensor", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_sym_power_sensor_measured_object() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "sym_power_sensor", "measured_object");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_sym_power_sensor_measured_terminal_type() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "sym_power_sensor", "measured_terminal_type");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_sym_power_sensor_power_sigma() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "sym_power_sensor", "power_sigma");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_sym_power_sensor_p_measured() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "sym_power_sensor", "p_measured");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_sym_power_sensor_q_measured() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "sym_power_sensor", "q_measured");
    return ptr;
}
// component asym_power_sensor
PGM_MetaComponent const* PGM_zdef_input_asym_power_sensor() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "input", "asym_power_sensor");
    return ptr;
}
// attributes of input asym_power_sensor
PGM_MetaAttribute const* PGM_zdef_input_asym_power_sensor_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "asym_power_sensor", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_asym_power_sensor_measured_object() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "asym_power_sensor", "measured_object");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_asym_power_sensor_measured_terminal_type() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "asym_power_sensor", "measured_terminal_type");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_asym_power_sensor_power_sigma() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "asym_power_sensor", "power_sigma");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_asym_power_sensor_p_measured() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "asym_power_sensor", "p_measured");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_asym_power_sensor_q_measured() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "asym_power_sensor", "q_measured");
    return ptr;
}
// component fault
PGM_MetaComponent const* PGM_zdef_input_fault() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "input", "fault");
    return ptr;
}
// attributes of input fault
PGM_MetaAttribute const* PGM_zdef_input_fault_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "fault", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_fault_status() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "fault", "status");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_fault_fault_type() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "fault", "fault_type");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_fault_fault_phase() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "fault", "fault_phase");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_fault_fault_object() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "fault", "fault_object");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_fault_r_f() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "fault", "r_f");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_input_fault_x_f() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "input", "fault", "x_f");
    return ptr;
}
// dataset sym_output
PGM_MetaDataset const* PGM_zdef_sym_output() {
    static PGM_MetaDataset const* const ptr = PGM_meta_get_dataset_by_name(nullptr, "sym_output");
    return ptr;
}
// components of sym_output
// component node
PGM_MetaComponent const* PGM_zdef_sym_output_node() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "sym_output", "node");
    return ptr;
}
// attributes of sym_output node
PGM_MetaAttribute const* PGM_zdef_sym_output_node_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "node", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_node_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "node", "energized");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_node_u_pu() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "node", "u_pu");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_node_u() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "node", "u");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_node_u_angle() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "node", "u_angle");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_node_p() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "node", "p");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_node_q() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "node", "q");
    return ptr;
}
// component line
PGM_MetaComponent const* PGM_zdef_sym_output_line() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "sym_output", "line");
    return ptr;
}
// attributes of sym_output line
PGM_MetaAttribute const* PGM_zdef_sym_output_line_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "line", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_line_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "line", "energized");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_line_loading() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "line", "loading");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_line_p_from() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "line", "p_from");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_line_q_from() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "line", "q_from");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_line_i_from() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "line", "i_from");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_line_s_from() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "line", "s_from");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_line_p_to() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "line", "p_to");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_line_q_to() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "line", "q_to");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_line_i_to() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "line", "i_to");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_line_s_to() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "line", "s_to");
    return ptr;
}
// component link
PGM_MetaComponent const* PGM_zdef_sym_output_link() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "sym_output", "link");
    return ptr;
}
// attributes of sym_output link
PGM_MetaAttribute const* PGM_zdef_sym_output_link_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "link", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_link_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "link", "energized");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_link_loading() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "link", "loading");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_link_p_from() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "link", "p_from");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_link_q_from() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "link", "q_from");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_link_i_from() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "link", "i_from");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_link_s_from() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "link", "s_from");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_link_p_to() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "link", "p_to");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_link_q_to() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "link", "q_to");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_link_i_to() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "link", "i_to");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_link_s_to() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "link", "s_to");
    return ptr;
}
// component transformer
PGM_MetaComponent const* PGM_zdef_sym_output_transformer() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "sym_output", "transformer");
    return ptr;
}
// attributes of sym_output transformer
PGM_MetaAttribute const* PGM_zdef_sym_output_transformer_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "transformer", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_transformer_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "transformer", "energized");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_transformer_loading() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "transformer", "loading");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_transformer_p_from() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "transformer", "p_from");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_transformer_q_from() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "transformer", "q_from");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_transformer_i_from() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "transformer", "i_from");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_transformer_s_from() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "transformer", "s_from");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_transformer_p_to() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "transformer", "p_to");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_transformer_q_to() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "transformer", "q_to");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_transformer_i_to() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "transformer", "i_to");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_transformer_s_to() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "transformer", "s_to");
    return ptr;
}
// component three_winding_transformer
PGM_MetaComponent const* PGM_zdef_sym_output_three_winding_transformer() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "sym_output", "three_winding_transformer");
    return ptr;
}
// attributes of sym_output three_winding_transformer
PGM_MetaAttribute const* PGM_zdef_sym_output_three_winding_transformer_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "three_winding_transformer", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_three_winding_transformer_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "three_winding_transformer", "energized");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_three_winding_transformer_loading() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "three_winding_transformer", "loading");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_three_winding_transformer_p_1() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "three_winding_transformer", "p_1");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_three_winding_transformer_q_1() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "three_winding_transformer", "q_1");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_three_winding_transformer_i_1() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "three_winding_transformer", "i_1");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_three_winding_transformer_s_1() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "three_winding_transformer", "s_1");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_three_winding_transformer_p_2() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "three_winding_transformer", "p_2");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_three_winding_transformer_q_2() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "three_winding_transformer", "q_2");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_three_winding_transformer_i_2() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "three_winding_transformer", "i_2");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_three_winding_transformer_s_2() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "three_winding_transformer", "s_2");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_three_winding_transformer_p_3() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "three_winding_transformer", "p_3");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_three_winding_transformer_q_3() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "three_winding_transformer", "q_3");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_three_winding_transformer_i_3() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "three_winding_transformer", "i_3");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_three_winding_transformer_s_3() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "three_winding_transformer", "s_3");
    return ptr;
}
// component sym_load
PGM_MetaComponent const* PGM_zdef_sym_output_sym_load() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "sym_output", "sym_load");
    return ptr;
}
// attributes of sym_output sym_load
PGM_MetaAttribute const* PGM_zdef_sym_output_sym_load_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "sym_load", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_sym_load_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "sym_load", "energized");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_sym_load_p() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "sym_load", "p");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_sym_load_q() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "sym_load", "q");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_sym_load_i() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "sym_load", "i");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_sym_load_s() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "sym_load", "s");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_sym_load_pf() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "sym_load", "pf");
    return ptr;
}
// component sym_gen
PGM_MetaComponent const* PGM_zdef_sym_output_sym_gen() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "sym_output", "sym_gen");
    return ptr;
}
// attributes of sym_output sym_gen
PGM_MetaAttribute const* PGM_zdef_sym_output_sym_gen_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "sym_gen", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_sym_gen_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "sym_gen", "energized");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_sym_gen_p() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "sym_gen", "p");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_sym_gen_q() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "sym_gen", "q");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_sym_gen_i() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "sym_gen", "i");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_sym_gen_s() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "sym_gen", "s");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_sym_gen_pf() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "sym_gen", "pf");
    return ptr;
}
// component asym_load
PGM_MetaComponent const* PGM_zdef_sym_output_asym_load() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "sym_output", "asym_load");
    return ptr;
}
// attributes of sym_output asym_load
PGM_MetaAttribute const* PGM_zdef_sym_output_asym_load_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "asym_load", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_asym_load_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "asym_load", "energized");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_asym_load_p() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "asym_load", "p");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_asym_load_q() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "asym_load", "q");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_asym_load_i() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "asym_load", "i");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_asym_load_s() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "asym_load", "s");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_asym_load_pf() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "asym_load", "pf");
    return ptr;
}
// component asym_gen
PGM_MetaComponent const* PGM_zdef_sym_output_asym_gen() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "sym_output", "asym_gen");
    return ptr;
}
// attributes of sym_output asym_gen
PGM_MetaAttribute const* PGM_zdef_sym_output_asym_gen_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "asym_gen", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_asym_gen_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "asym_gen", "energized");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_asym_gen_p() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "asym_gen", "p");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_asym_gen_q() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "asym_gen", "q");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_asym_gen_i() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "asym_gen", "i");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_asym_gen_s() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "asym_gen", "s");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_asym_gen_pf() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "asym_gen", "pf");
    return ptr;
}
// component shunt
PGM_MetaComponent const* PGM_zdef_sym_output_shunt() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "sym_output", "shunt");
    return ptr;
}
// attributes of sym_output shunt
PGM_MetaAttribute const* PGM_zdef_sym_output_shunt_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "shunt", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_shunt_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "shunt", "energized");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_shunt_p() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "shunt", "p");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_shunt_q() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "shunt", "q");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_shunt_i() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "shunt", "i");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_shunt_s() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "shunt", "s");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_shunt_pf() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "shunt", "pf");
    return ptr;
}
// component source
PGM_MetaComponent const* PGM_zdef_sym_output_source() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "sym_output", "source");
    return ptr;
}
// attributes of sym_output source
PGM_MetaAttribute const* PGM_zdef_sym_output_source_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "source", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_source_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "source", "energized");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_source_p() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "source", "p");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_source_q() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "source", "q");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_source_i() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "source", "i");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_source_s() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "source", "s");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_source_pf() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "source", "pf");
    return ptr;
}
// component sym_voltage_sensor
PGM_MetaComponent const* PGM_zdef_sym_output_sym_voltage_sensor() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "sym_output", "sym_voltage_sensor");
    return ptr;
}
// attributes of sym_output sym_voltage_sensor
PGM_MetaAttribute const* PGM_zdef_sym_output_sym_voltage_sensor_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "sym_voltage_sensor", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_sym_voltage_sensor_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "sym_voltage_sensor", "energized");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_sym_voltage_sensor_u_residual() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "sym_voltage_sensor", "u_residual");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_sym_voltage_sensor_u_angle_residual() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "sym_voltage_sensor", "u_angle_residual");
    return ptr;
}
// component asym_voltage_sensor
PGM_MetaComponent const* PGM_zdef_sym_output_asym_voltage_sensor() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "sym_output", "asym_voltage_sensor");
    return ptr;
}
// attributes of sym_output asym_voltage_sensor
PGM_MetaAttribute const* PGM_zdef_sym_output_asym_voltage_sensor_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "asym_voltage_sensor", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_asym_voltage_sensor_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "asym_voltage_sensor", "energized");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_asym_voltage_sensor_u_residual() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "asym_voltage_sensor", "u_residual");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_asym_voltage_sensor_u_angle_residual() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "asym_voltage_sensor", "u_angle_residual");
    return ptr;
}
// component sym_power_sensor
PGM_MetaComponent const* PGM_zdef_sym_output_sym_power_sensor() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "sym_output", "sym_power_sensor");
    return ptr;
}
// attributes of sym_output sym_power_sensor
PGM_MetaAttribute const* PGM_zdef_sym_output_sym_power_sensor_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "sym_power_sensor", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_sym_power_sensor_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "sym_power_sensor", "energized");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_sym_power_sensor_p_residual() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "sym_power_sensor", "p_residual");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_sym_power_sensor_q_residual() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "sym_power_sensor", "q_residual");
    return ptr;
}
// component asym_power_sensor
PGM_MetaComponent const* PGM_zdef_sym_output_asym_power_sensor() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "sym_output", "asym_power_sensor");
    return ptr;
}
// attributes of sym_output asym_power_sensor
PGM_MetaAttribute const* PGM_zdef_sym_output_asym_power_sensor_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "asym_power_sensor", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_asym_power_sensor_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "asym_power_sensor", "energized");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_asym_power_sensor_p_residual() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "asym_power_sensor", "p_residual");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_asym_power_sensor_q_residual() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "asym_power_sensor", "q_residual");
    return ptr;
}
// component fault
PGM_MetaComponent const* PGM_zdef_sym_output_fault() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "sym_output", "fault");
    return ptr;
}
// attributes of sym_output fault
PGM_MetaAttribute const* PGM_zdef_sym_output_fault_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "fault", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_sym_output_fault_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "sym_output", "fault", "energized");
    return ptr;
}
// dataset asym_output
PGM_MetaDataset const* PGM_zdef_asym_output() {
    static PGM_MetaDataset const* const ptr = PGM_meta_get_dataset_by_name(nullptr, "asym_output");
    return ptr;
}
// components of asym_output
// component node
PGM_MetaComponent const* PGM_zdef_asym_output_node() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "asym_output", "node");
    return ptr;
}
// attributes of asym_output node
PGM_MetaAttribute const* PGM_zdef_asym_output_node_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "node", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_node_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "node", "energized");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_node_u_pu() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "node", "u_pu");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_node_u() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "node", "u");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_node_u_angle() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "node", "u_angle");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_node_p() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "node", "p");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_node_q() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "node", "q");
    return ptr;
}
// component line
PGM_MetaComponent const* PGM_zdef_asym_output_line() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "asym_output", "line");
    return ptr;
}
// attributes of asym_output line
PGM_MetaAttribute const* PGM_zdef_asym_output_line_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "line", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_line_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "line", "energized");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_line_loading() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "line", "loading");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_line_p_from() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "line", "p_from");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_line_q_from() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "line", "q_from");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_line_i_from() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "line", "i_from");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_line_s_from() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "line", "s_from");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_line_p_to() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "line", "p_to");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_line_q_to() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "line", "q_to");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_line_i_to() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "line", "i_to");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_line_s_to() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "line", "s_to");
    return ptr;
}
// component link
PGM_MetaComponent const* PGM_zdef_asym_output_link() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "asym_output", "link");
    return ptr;
}
// attributes of asym_output link
PGM_MetaAttribute const* PGM_zdef_asym_output_link_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "link", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_link_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "link", "energized");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_link_loading() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "link", "loading");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_link_p_from() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "link", "p_from");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_link_q_from() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "link", "q_from");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_link_i_from() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "link", "i_from");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_link_s_from() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "link", "s_from");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_link_p_to() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "link", "p_to");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_link_q_to() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "link", "q_to");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_link_i_to() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "link", "i_to");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_link_s_to() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "link", "s_to");
    return ptr;
}
// component transformer
PGM_MetaComponent const* PGM_zdef_asym_output_transformer() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "asym_output", "transformer");
    return ptr;
}
// attributes of asym_output transformer
PGM_MetaAttribute const* PGM_zdef_asym_output_transformer_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "transformer", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_transformer_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "transformer", "energized");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_transformer_loading() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "transformer", "loading");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_transformer_p_from() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "transformer", "p_from");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_transformer_q_from() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "transformer", "q_from");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_transformer_i_from() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "transformer", "i_from");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_transformer_s_from() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "transformer", "s_from");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_transformer_p_to() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "transformer", "p_to");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_transformer_q_to() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "transformer", "q_to");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_transformer_i_to() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "transformer", "i_to");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_transformer_s_to() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "transformer", "s_to");
    return ptr;
}
// component three_winding_transformer
PGM_MetaComponent const* PGM_zdef_asym_output_three_winding_transformer() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "asym_output", "three_winding_transformer");
    return ptr;
}
// attributes of asym_output three_winding_transformer
PGM_MetaAttribute const* PGM_zdef_asym_output_three_winding_transformer_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "three_winding_transformer", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_three_winding_transformer_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "three_winding_transformer", "energized");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_three_winding_transformer_loading() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "three_winding_transformer", "loading");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_three_winding_transformer_p_1() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "three_winding_transformer", "p_1");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_three_winding_transformer_q_1() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "three_winding_transformer", "q_1");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_three_winding_transformer_i_1() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "three_winding_transformer", "i_1");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_three_winding_transformer_s_1() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "three_winding_transformer", "s_1");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_three_winding_transformer_p_2() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "three_winding_transformer", "p_2");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_three_winding_transformer_q_2() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "three_winding_transformer", "q_2");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_three_winding_transformer_i_2() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "three_winding_transformer", "i_2");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_three_winding_transformer_s_2() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "three_winding_transformer", "s_2");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_three_winding_transformer_p_3() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "three_winding_transformer", "p_3");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_three_winding_transformer_q_3() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "three_winding_transformer", "q_3");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_three_winding_transformer_i_3() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "three_winding_transformer", "i_3");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_three_winding_transformer_s_3() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "three_winding_transformer", "s_3");
    return ptr;
}
// component sym_load
PGM_MetaComponent const* PGM_zdef_asym_output_sym_load() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "asym_output", "sym_load");
    return ptr;
}
// attributes of asym_output sym_load
PGM_MetaAttribute const* PGM_zdef_asym_output_sym_load_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "sym_load", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_sym_load_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "sym_load", "energized");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_sym_load_p() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "sym_load", "p");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_sym_load_q() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "sym_load", "q");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_sym_load_i() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "sym_load", "i");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_sym_load_s() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "sym_load", "s");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_sym_load_pf() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "sym_load", "pf");
    return ptr;
}
// component sym_gen
PGM_MetaComponent const* PGM_zdef_asym_output_sym_gen() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "asym_output", "sym_gen");
    return ptr;
}
// attributes of asym_output sym_gen
PGM_MetaAttribute const* PGM_zdef_asym_output_sym_gen_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "sym_gen", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_sym_gen_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "sym_gen", "energized");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_sym_gen_p() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "sym_gen", "p");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_sym_gen_q() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "sym_gen", "q");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_sym_gen_i() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "sym_gen", "i");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_sym_gen_s() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "sym_gen", "s");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_sym_gen_pf() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "sym_gen", "pf");
    return ptr;
}
// component asym_load
PGM_MetaComponent const* PGM_zdef_asym_output_asym_load() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "asym_output", "asym_load");
    return ptr;
}
// attributes of asym_output asym_load
PGM_MetaAttribute const* PGM_zdef_asym_output_asym_load_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "asym_load", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_asym_load_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "asym_load", "energized");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_asym_load_p() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "asym_load", "p");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_asym_load_q() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "asym_load", "q");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_asym_load_i() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "asym_load", "i");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_asym_load_s() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "asym_load", "s");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_asym_load_pf() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "asym_load", "pf");
    return ptr;
}
// component asym_gen
PGM_MetaComponent const* PGM_zdef_asym_output_asym_gen() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "asym_output", "asym_gen");
    return ptr;
}
// attributes of asym_output asym_gen
PGM_MetaAttribute const* PGM_zdef_asym_output_asym_gen_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "asym_gen", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_asym_gen_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "asym_gen", "energized");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_asym_gen_p() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "asym_gen", "p");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_asym_gen_q() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "asym_gen", "q");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_asym_gen_i() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "asym_gen", "i");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_asym_gen_s() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "asym_gen", "s");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_asym_gen_pf() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "asym_gen", "pf");
    return ptr;
}
// component shunt
PGM_MetaComponent const* PGM_zdef_asym_output_shunt() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "asym_output", "shunt");
    return ptr;
}
// attributes of asym_output shunt
PGM_MetaAttribute const* PGM_zdef_asym_output_shunt_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "shunt", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_shunt_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "shunt", "energized");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_shunt_p() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "shunt", "p");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_shunt_q() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "shunt", "q");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_shunt_i() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "shunt", "i");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_shunt_s() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "shunt", "s");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_shunt_pf() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "shunt", "pf");
    return ptr;
}
// component source
PGM_MetaComponent const* PGM_zdef_asym_output_source() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "asym_output", "source");
    return ptr;
}
// attributes of asym_output source
PGM_MetaAttribute const* PGM_zdef_asym_output_source_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "source", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_source_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "source", "energized");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_source_p() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "source", "p");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_source_q() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "source", "q");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_source_i() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "source", "i");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_source_s() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "source", "s");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_source_pf() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "source", "pf");
    return ptr;
}
// component sym_voltage_sensor
PGM_MetaComponent const* PGM_zdef_asym_output_sym_voltage_sensor() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "asym_output", "sym_voltage_sensor");
    return ptr;
}
// attributes of asym_output sym_voltage_sensor
PGM_MetaAttribute const* PGM_zdef_asym_output_sym_voltage_sensor_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "sym_voltage_sensor", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_sym_voltage_sensor_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "sym_voltage_sensor", "energized");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_sym_voltage_sensor_u_residual() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "sym_voltage_sensor", "u_residual");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_sym_voltage_sensor_u_angle_residual() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "sym_voltage_sensor", "u_angle_residual");
    return ptr;
}
// component asym_voltage_sensor
PGM_MetaComponent const* PGM_zdef_asym_output_asym_voltage_sensor() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "asym_output", "asym_voltage_sensor");
    return ptr;
}
// attributes of asym_output asym_voltage_sensor
PGM_MetaAttribute const* PGM_zdef_asym_output_asym_voltage_sensor_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "asym_voltage_sensor", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_asym_voltage_sensor_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "asym_voltage_sensor", "energized");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_asym_voltage_sensor_u_residual() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "asym_voltage_sensor", "u_residual");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_asym_voltage_sensor_u_angle_residual() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "asym_voltage_sensor", "u_angle_residual");
    return ptr;
}
// component sym_power_sensor
PGM_MetaComponent const* PGM_zdef_asym_output_sym_power_sensor() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "asym_output", "sym_power_sensor");
    return ptr;
}
// attributes of asym_output sym_power_sensor
PGM_MetaAttribute const* PGM_zdef_asym_output_sym_power_sensor_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "sym_power_sensor", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_sym_power_sensor_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "sym_power_sensor", "energized");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_sym_power_sensor_p_residual() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "sym_power_sensor", "p_residual");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_sym_power_sensor_q_residual() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "sym_power_sensor", "q_residual");
    return ptr;
}
// component asym_power_sensor
PGM_MetaComponent const* PGM_zdef_asym_output_asym_power_sensor() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "asym_output", "asym_power_sensor");
    return ptr;
}
// attributes of asym_output asym_power_sensor
PGM_MetaAttribute const* PGM_zdef_asym_output_asym_power_sensor_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "asym_power_sensor", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_asym_power_sensor_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "asym_power_sensor", "energized");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_asym_power_sensor_p_residual() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "asym_power_sensor", "p_residual");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_asym_power_sensor_q_residual() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "asym_power_sensor", "q_residual");
    return ptr;
}
// component fault
PGM_MetaComponent const* PGM_zdef_asym_output_fault() {
    static PGM_MetaComponent const* const ptr = PGM_meta_get_component_by_name(nullptr, "asym_output", "fault");
    return ptr;
}
// attributes of asym_output fault
PGM_MetaAttribute const* PGM_zdef_asym_output_fault_id() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "fault", "id");
    return ptr;
}
PGM_MetaAttribute const* PGM_zdef_asym_output_fault_energized() {
    static PGM_MetaAttribute const* const ptr = PGM_meta_get_attribute_by_name(nullptr, "asym_output", "fault", "energized");
    return ptr;
}
// dataset update
PGM_MetaDataset const* PGM_zdef_update() {
    static PGM_MetaDataset const* const ptr = PGM_meta_get_dataset_by_name(nullptr, "update");
    return ptr;
}
// components of update
// dataset sc_output
PGM_MetaDataset const* PGM_zdef_sc_output() {
    static PGM_MetaDataset const* const ptr = PGM_meta_get_dataset_by_name(nullptr, "sc_output");
    return ptr;
}
// components of sc_output
//

// clang-format on