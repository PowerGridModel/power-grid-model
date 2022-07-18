// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <iostream>

#include "doctest/doctest.h"
#include "power_grid_model/auxiliary/meta_data.hpp"
#include "power_grid_model/auxiliary/meta_data_gen.hpp"
#include "power_grid_model/three_phase_tensor.hpp"

namespace power_grid_model {

TEST_CASE("Test column row conversion") {
    SUBCASE("Test meta input data generation") {
        auto const meta_map = meta_data::meta_data().at("input");
        auto const node = meta_map.at("node");
        auto const node_attr = node.attributes;
        CHECK(node_attr[0].name == "id");
        CHECK(node_attr[0].numpy_type == "i4");
        CHECK(node_attr[1].name == "u_rated");
        CHECK(node_attr[1].numpy_type == "f8");

        auto const sym_voltage_sensor = meta_map.at("sym_voltage_sensor");
        auto const sensor_attr = sym_voltage_sensor.attributes;
        CHECK(sensor_attr[0].name == "id");
        CHECK(sensor_attr[0].numpy_type == "i4");
        CHECK(sensor_attr[1].name == "measured_object");
        CHECK(sensor_attr[1].numpy_type == "i4");
    }

    SUBCASE("Test meta ouput data generation") {
        auto const meta_map = meta_data::meta_data().at("asym_output");
        auto const node = meta_map.at("node");
        auto const node_attr = node.attributes;
        CHECK(node_attr[0].name == "id");
        CHECK(node_attr[0].numpy_type == "i4");
        CHECK(node_attr[2].name == "u_pu");
        CHECK(node_attr[2].numpy_type == "f8");
        CHECK(node_attr[2].dims == std::vector<size_t>{3});

        auto const sym_voltage_sensor = meta_map.at("sym_voltage_sensor");
        auto const sensor_attr = sym_voltage_sensor.attributes;
        CHECK(sensor_attr[0].name == "id");
        CHECK(sensor_attr[0].numpy_type == "i4");
        CHECK(sensor_attr[2].name == "u_residual");
        CHECK(sensor_attr[2].numpy_type == "f8");
    }

    SUBCASE("Test meta update data generation") {
        auto const meta_map = meta_data::meta_data().at("update");
        auto const load = meta_map.at("asym_load");
        auto const load_attr = load.attributes;
        CHECK(load_attr[0].name == "id");
        CHECK(load_attr[0].numpy_type == "i4");
        CHECK(load_attr[2].name == "p_specified");
        CHECK(load_attr[2].numpy_type == "f8");
        CHECK(load_attr[2].dims == std::vector<size_t>{3});

        auto const sym_voltage_sensor = meta_map.at("sym_voltage_sensor");
        auto const sensor_attr = sym_voltage_sensor.attributes;
        CHECK(sensor_attr[0].name == "id");
        CHECK(sensor_attr[0].numpy_type == "i4");
        CHECK(sensor_attr[1].name == "u_sigma");
        CHECK(sensor_attr[1].numpy_type == "f8");
    }
}

}  // namespace power_grid_model
