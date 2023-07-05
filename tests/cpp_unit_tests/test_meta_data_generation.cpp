// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/auxiliary/meta_data.hpp>
#include <power_grid_model/auxiliary/meta_data_gen.hpp>
#include <power_grid_model/three_phase_tensor.hpp>

#include <doctest/doctest.h>

#include <iostream>

namespace power_grid_model {

TEST_CASE("Test column row conversion") {
    SUBCASE("Test meta input data generation") {
        auto const meta_map = meta_data::meta_data().get_dataset("input");
        auto const node = meta_map.get_component("node");
        auto const node_attr = node.attributes;
        CHECK(node_attr[0].name == "id");
        CHECK(node_attr[0].ctype == "int32_t");
        CHECK(node_attr[1].name == "u_rated");
        CHECK(node_attr[1].ctype == "double");

        auto const sym_voltage_sensor = meta_map.get_component("sym_voltage_sensor");
        auto const sensor_attr = sym_voltage_sensor.attributes;
        CHECK(sensor_attr[0].name == "id");
        CHECK(sensor_attr[0].ctype == "int32_t");
        CHECK(sensor_attr[1].name == "measured_object");
        CHECK(sensor_attr[1].ctype == "int32_t");
    }

    SUBCASE("Test meta ouput data generation") {
        auto const meta_map = meta_data::meta_data().get_dataset("asym_output");
        auto const node = meta_map.get_component("node");
        auto const node_attr = node.attributes;
        CHECK(node_attr[0].name == "id");
        CHECK(node_attr[0].ctype == "int32_t");
        CHECK(node_attr[2].name == "u_pu");
        CHECK(node_attr[2].ctype == "double[3]");

        auto const sym_voltage_sensor = meta_map.get_component("sym_voltage_sensor");
        auto const sensor_attr = sym_voltage_sensor.attributes;
        CHECK(sensor_attr[0].name == "id");
        CHECK(sensor_attr[0].ctype == "int32_t");
        CHECK(sensor_attr[2].name == "u_residual");
        CHECK(sensor_attr[2].ctype == "double[3]");
    }

    SUBCASE("Test meta update data generation") {
        auto const meta_map = meta_data::meta_data().get_dataset("update");
        auto const load = meta_map.get_component("asym_load");
        auto const load_attr = load.attributes;
        CHECK(load_attr[0].name == "id");
        CHECK(load_attr[0].ctype == "int32_t");
        CHECK(load_attr[2].name == "p_specified");
        CHECK(load_attr[2].ctype == "double[3]");

        auto const sym_voltage_sensor = meta_map.get_component("sym_voltage_sensor");
        auto const sensor_attr = sym_voltage_sensor.attributes;
        CHECK(sensor_attr[0].name == "id");
        CHECK(sensor_attr[0].ctype == "int32_t");
        CHECK(sensor_attr[1].name == "u_sigma");
        CHECK(sensor_attr[1].ctype == "double");
    }
}

}  // namespace power_grid_model
