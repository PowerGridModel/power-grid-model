// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/auxiliary/serialization/serializer.hpp>

#include <doctest/doctest.h>

namespace power_grid_model::meta_data {

namespace {
constexpr std::string_view single_dataset_dict =
    R"({"attributes":{},"data":{"asym_load":[{"id":5,"p_specified":[10,11,12]},{"id":6,"p_specified":["inf",null,"-inf"]},{"id":7}]},"is_batch":false,"type":"update","version":"1.0"})";
constexpr std::string_view single_dataset_list =
    R"({"attributes":{"asym_load":["id","p_specified"]},"data":{"asym_load":[[5,[10,11,12]],[6,["inf",null,"-inf"]],[7,null]]},"is_batch":false,"type":"update","version":"1.0"})";
constexpr std::string_view batch_dataset_list =
    R"({"attributes":{},"data":[{"asym_load":[{"id":5,"p_specified":[10,11,12]}]},{"asym_gen":[{"id":7}],"asym_load":[{"id":6,"p_specified":["inf",null,"-inf"]}]}],"is_batch":true,"type":"update","version":"1.0"})";
constexpr std::string_view batch_dataset_dict =
    R"({"attributes":{"asym_gen":["id"],"asym_load":["id","p_specified"]},"data":[{"asym_load":[[5,[10,11,12]]]},{"asym_gen":[[7]],"asym_load":[[6,["inf",null,"-inf"]]]}],"is_batch":true,"type":"update","version":"1.0"})";
} // namespace

TEST_CASE("Serializer") {
    std::vector<AsymLoadGenUpdate> asym_load_gen(3);
    meta_data().get_dataset("update").get_component("asym_load").set_nan(asym_load_gen.data(), 0, 3);
    asym_load_gen[0].id = 5;
    asym_load_gen[1].id = 6;
    asym_load_gen[2].id = 7;
    asym_load_gen[0].p_specified = {10.0, 11.0, 12.0};
    asym_load_gen[1].p_specified = {std::numeric_limits<double>::infinity(), nan,
                                    -std::numeric_limits<double>::infinity()};
    // nan for asym_load_gen[2].p_specified

    SUBCASE("Single dataset") {
        ConstDatasetHandler handler{false, 1, "update"};
        handler.add_buffer("asym_load", 3, 3, nullptr, asym_load_gen.data());
        Serializer serializer{handler, SerializationFormat::json};
        CHECK(serializer.get_string(false, -1) == single_dataset_dict);
        CHECK(serializer.get_string(true, -1) == single_dataset_list);
    }

    SUBCASE("Batch dataset") {
        ConstDatasetHandler handler{true, 2, "update"};
        std::array<Idx, 3> const indptr_gen{0, 0, 1};
        handler.add_buffer("asym_load", 1, 2, nullptr, asym_load_gen.data());
        handler.add_buffer("asym_gen", -1, 1, indptr_gen.data(), asym_load_gen.data() + 2);
        Serializer serializer{handler, SerializationFormat::json};
        CHECK(serializer.get_string(false, -1) == batch_dataset_list);
        CHECK(serializer.get_string(true, -1) == batch_dataset_dict);
    }
}

} // namespace power_grid_model::meta_data
