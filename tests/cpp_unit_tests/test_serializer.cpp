// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/auxiliary/serialization/serializer.hpp>

#include <doctest/doctest.h>

#include <iostream>

namespace {}

namespace power_grid_model::meta_data {

namespace {
constexpr std::string_view single_dataset_dict =
    R"({"attributes":{},"data":{"asym_load":[{"id":5,"p_specified":[10,11,12]},{"id":6,"p_specified":[15,null,16]},{"id":7}]},"is_batch":false,"type":"update","version":"1.0"})";
}
constexpr std::string_view single_dataset_list =
    R"({"attributes":{"asym_load":["id","p_specified"]},"data":{"asym_load":[[5,[10,11,12]],[6,[15,null,16]],[7,null]]},"is_batch":false,"type":"update","version":"1.0"})";

TEST_CASE("Serializer") {
    std::vector<AsymLoadGenUpdate> asym_load_gen(3);
    meta_data().get_dataset("update").get_component("asym_load").set_nan(asym_load_gen.data(), 0, 3);
    asym_load_gen[0].id = 5;
    asym_load_gen[1].id = 6;
    asym_load_gen[2].id = 7;
    asym_load_gen[0].p_specified = {10.0, 11.0, 12.0};
    asym_load_gen[1].p_specified = {15.0, nan, 16.0};
    // nan for asym_load_gen[2].p_specified

    SUBCASE("Single dataset") {
        std::array components{"asym_load"};
        Idx const n_components = 1;
        Idx const n_elements = 3;
        std::array data{(void const*)asym_load_gen.data()};
        Serializer serializer{"update", false, 1, n_components, components.data(), &n_elements, nullptr, data.data()};
        serializer.serialize(false);
        CHECK(serializer.get_json(-1) == single_dataset_dict);
        serializer.serialize(true);
        CHECK(serializer.get_json(-1) == single_dataset_list);
    }
}

} // namespace power_grid_model::meta_data
