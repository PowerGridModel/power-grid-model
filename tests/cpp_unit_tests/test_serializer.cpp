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
constexpr std::string_view single_dataset_list =
    R"({"attributes":{"asym_load":["id","p_specified"]},"data":{"asym_load":[[5,[10,11,12]],[6,[15,null,16]],[7,null]]},"is_batch":false,"type":"update","version":"1.0"})";
constexpr std::string_view batch_dataset_list =
    R"({"attributes":{},"data":[{"asym_load":[{"id":5,"p_specified":[10,11,12]}]},{"asym_gen":[{"id":7}],"asym_load":[{"id":6,"p_specified":[15,null,16]}]}],"is_batch":true,"type":"update","version":"1.0"})";
constexpr std::string_view batch_dataset_dict =
    R"({"attributes":{"asym_gen":["id"],"asym_load":["id","p_specified"]},"data":[{"asym_load":[[5,[10,11,12]]]},{"asym_gen":[[7]],"asym_load":[[6,[15,null,16]]]}],"is_batch":true,"type":"update","version":"1.0"})";
} // namespace

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
        std::vector<Idx> const n_elements{3};
        std::array<void const*, 1> data{asym_load_gen.data()};
        Serializer serializer{"update",          false,   1,          n_components, components.data(),
                              n_elements.data(), nullptr, data.data()};
        CHECK(serializer.get_json(false, -1) == single_dataset_dict);
        CHECK(serializer.get_json(true, -1) == single_dataset_list);
    }

    SUBCASE("Batch dataset") {
        std::array components{"asym_load", "asym_gen"};
        Idx const n_components = 2;
        Idx const batch_size = 2;
        std::array<Idx, 2> const n_elements{1, -1};
        std::array<Idx, 3> const indptr_gen{0, 0, 1};
        std::array<Idx const*, 3> indptrs{nullptr, indptr_gen.data()};
        std::array<void const*, 2> data{asym_load_gen.data(), asym_load_gen.data() + 2};
        Serializer serializer{"update",          true,           batch_size, n_components, components.data(),
                              n_elements.data(), indptrs.data(), data.data()};
        CHECK(serializer.get_json(false, -1) == batch_dataset_list);
        CHECK(serializer.get_json(true, -1) == batch_dataset_dict);
    }
}

} // namespace power_grid_model::meta_data
