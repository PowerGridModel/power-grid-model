// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/auxiliary/meta_data.hpp>
#include <power_grid_model/auxiliary/meta_data_gen.hpp>
#include <power_grid_model/common/three_phase_tensor.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {

using namespace std::string_literals;

TEST_CASE("Test column row conversion") {
    SUBCASE("Test meta input data generation") {
        auto const& meta_map = meta_data::meta_data_gen::meta_data.get_dataset("input");
        auto const& node = meta_map.get_component("node");
        auto const& node_attr = node.attributes;
        CHECK(node_attr[0].name == "id"s);
        CHECK(node_attr[0].ctype == CType::c_int32);
        CHECK(node_attr[1].name == "u_rated"s);
        CHECK(node_attr[1].ctype == CType::c_double);

        auto const& sym_voltage_sensor = meta_map.get_component("sym_voltage_sensor"s);
        auto const& sensor_attr = sym_voltage_sensor.attributes;
        CHECK(sensor_attr[0].name == "id"s);
        CHECK(sensor_attr[0].ctype == CType::c_int32);
        CHECK(sensor_attr[1].name == "measured_object"s);
        CHECK(sensor_attr[1].ctype == CType::c_int32);
    }

    SUBCASE("Test meta ouput data generation") {
        auto const& meta_map = meta_data::meta_data_gen::meta_data.get_dataset("asym_output");
        auto const& node = meta_map.get_component("node");
        auto const& node_attr = node.attributes;
        CHECK(node_attr[0].name == "id"s);
        CHECK(node_attr[0].ctype == CType::c_int32);
        CHECK(node_attr[2].name == "u_pu"s);
        CHECK(node_attr[2].ctype == CType::c_double3);

        auto const& sym_voltage_sensor = meta_map.get_component("sym_voltage_sensor");
        auto const& sensor_attr = sym_voltage_sensor.attributes;
        CHECK(sensor_attr[0].name == "id"s);
        CHECK(sensor_attr[0].ctype == CType::c_int32);
        CHECK(sensor_attr[2].name == "u_residual"s);
        CHECK(sensor_attr[2].ctype == CType::c_double3);
    }

    SUBCASE("Test meta update data generation") {
        auto const& meta_map = meta_data::meta_data_gen::meta_data.get_dataset("update");
        auto const& load = meta_map.get_component("asym_load");
        auto const& load_attr = load.attributes;
        CHECK(load_attr[0].name == "id"s);
        CHECK(load_attr[0].ctype == CType::c_int32);
        CHECK(load_attr[2].name == "p_specified"s);
        CHECK(load_attr[2].ctype == CType::c_double3);

        auto const& sym_voltage_sensor = meta_map.get_component("sym_voltage_sensor");
        auto const& sensor_attr = sym_voltage_sensor.attributes;
        CHECK(sensor_attr[0].name == "id"s);
        CHECK(sensor_attr[0].ctype == CType::c_int32);
        CHECK(sensor_attr[1].name == "u_sigma"s);
        CHECK(sensor_attr[1].ctype == CType::c_double);
    }
}

} // namespace power_grid_model
