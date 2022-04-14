// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <iostream>

#include "catch2/catch.hpp"
#include "power_grid_model/auxiliary/meta_data.hpp"
#include "power_grid_model/auxiliary/meta_data_gen.hpp"
#include "power_grid_model/three_phase_tensor.hpp"

namespace power_grid_model {

enum class TestEnum : IntS { x, y, z };

struct TestBaseType {
    int32_t x;
};

struct TestType : TestBaseType {
    int32_t y;
    double z;
    TestEnum e;
    RealValue<false> v;
};

TEST_CASE("Test column row conversion") {
    SECTION("Test single attribute") {
        auto const attr = POWER_GRID_MODEL_ONE_DATA_ATTRIBUTE(TestType, z);
        CHECK(attr.name == "z");
        CHECK(attr.numpy_type == "f8");
        CHECK(attr.dims == std::vector<size_t>{});
    }

    SECTION("Test list of attributes") {
        auto const meta_data_type = POWER_GRID_MODEL_META_DATA_TYPE(TestType, x, y, z, e, v);
        CHECK(meta_data_type.name == "TestType");
        auto const& list_attr = meta_data_type.attributes;
        CHECK(list_attr.size() == 5);
        CHECK(list_attr[0].name == "x");
        CHECK(list_attr[0].numpy_type == "i4");
        CHECK(list_attr[0].dims == std::vector<size_t>{});
        CHECK(list_attr[2].name == "z");
        CHECK(list_attr[2].numpy_type == "f8");
        CHECK(list_attr[2].dims == std::vector<size_t>{});
        CHECK(list_attr[3].name == "e");
        CHECK(list_attr[3].numpy_type == "i1");
        CHECK(list_attr[3].dims == std::vector<size_t>{});
        CHECK(list_attr[4].name == "v");
        CHECK(list_attr[4].numpy_type == "f8");
        CHECK(list_attr[4].dims == std::vector<size_t>{3});
    }

    SECTION("Test meta input data generation") {
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

    SECTION("Test meta ouput data generation") {
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

    SECTION("Test meta update data generation") {
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
