// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/auxiliary/serialization/serializer.hpp>

#include <doctest/doctest.h>

#include <limits>

namespace power_grid_model::meta_data {

namespace {
constexpr std::string_view single_dataset_dict =
    R"({"attributes":{},"data":{"asym_load":[{"id":5,"p_specified":[10,11,12]},{"id":6,"p_specified":[15,null,16]},{"id":7}],"sym_load":[{"id":9,"p_specified":10},{"id":10},{"id":11,"p_specified":"inf"},{"id":12,"p_specified":"-inf"}]},"is_batch":false,"type":"update","version":"1.0"})";
constexpr std::string_view single_dataset_list =
    R"({"attributes":{"asym_load":["id","p_specified"],"sym_load":["id","p_specified"]},"data":{"asym_load":[[5,[10,11,12]],[6,[15,null,16]],[7,null]],"sym_load":[[9,10],[10,null],[11,"inf"],[12,"-inf"]]},"is_batch":false,"type":"update","version":"1.0"})";
constexpr std::string_view batch_dataset_list =
    R"({"attributes":{},"data":[{"asym_load":[{"id":5,"p_specified":[10,11,12]}],"sym_load":[{"id":9,"p_specified":10},{"id":10}]},{"asym_gen":[{"id":7}],"asym_load":[{"id":6,"p_specified":[15,null,16]}],"sym_load":[{"id":11,"p_specified":"inf"},{"id":12,"p_specified":"-inf"}]}],"is_batch":true,"type":"update","version":"1.0"})";
constexpr std::string_view batch_dataset_dict =
    R"({"attributes":{"asym_gen":["id"],"asym_load":["id","p_specified"],"sym_load":["id","p_specified"]},"data":[{"asym_load":[[5,[10,11,12]]],"sym_load":[[9,10],[10,null]]},{"asym_gen":[[7]],"asym_load":[[6,[15,null,16]]],"sym_load":[[11,"inf"],[12,"-inf"]]}],"is_batch":true,"type":"update","version":"1.0"})";
} // namespace

TEST_CASE("Serializer") {

    std::vector<SymLoadGenUpdate> sym_load_gen(4);
    meta_data().get_dataset("update").get_component("sym_load").set_nan(sym_load_gen.data(), 0, 4);
    sym_load_gen[0].id = 9;
    sym_load_gen[1].id = 10;
    sym_load_gen[2].id = 11;
    sym_load_gen[3].id = 12;
    sym_load_gen[0].p_specified = 10.0;
    sym_load_gen[1].p_specified = nan;
    sym_load_gen[2].p_specified = std::numeric_limits<double>::infinity();
    sym_load_gen[3].p_specified = -std::numeric_limits<double>::infinity();

    std::vector<AsymLoadGenUpdate> asym_load_gen(3);
    meta_data().get_dataset("update").get_component("asym_load").set_nan(asym_load_gen.data(), 0, 3);
    asym_load_gen[0].id = 5;
    asym_load_gen[1].id = 6;
    asym_load_gen[2].id = 7;
    asym_load_gen[0].p_specified = {10.0, 11.0, 12.0};
    asym_load_gen[1].p_specified = {15.0, nan, 16.0};
    // nan for asym_load_gen[2].p_specified

    SUBCASE("Single dataset") {
        ConstDatasetHandler handler{false, 1, "update"};
        handler.add_buffer("sym_load", 4, 4, nullptr, sym_load_gen.data());
        handler.add_buffer("asym_load", 3, 3, nullptr, asym_load_gen.data());
        Serializer serializer{handler, SerializationFormat::json};
        CHECK(serializer.get_string(false, -1) == single_dataset_dict);
        CHECK(serializer.get_string(true, -1) == single_dataset_list);
    }

    SUBCASE("Batch dataset") {
        ConstDatasetHandler handler{true, 2, "update"};
        std::array<Idx, 3> const indptr_gen{0, 0, 1};
        handler.add_buffer("sym_load", 2, 4, nullptr, sym_load_gen.data());
        handler.add_buffer("asym_load", 1, 2, nullptr, asym_load_gen.data());
        handler.add_buffer("asym_gen", -1, 1, indptr_gen.data(), asym_load_gen.data() + 2);
        Serializer serializer{handler, SerializationFormat::json};
        CHECK(serializer.get_string(false, -1) == batch_dataset_list);
        CHECK(serializer.get_string(true, -1) == batch_dataset_dict);
    }
}

} // namespace power_grid_model::meta_data
