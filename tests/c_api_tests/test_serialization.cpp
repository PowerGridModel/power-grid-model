// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include "c_api_cpp_handle.hpp"
#include "power_grid_model_c.h"

#include <power_grid_model/auxiliary/meta_data_gen.hpp>

#include <doctest/doctest.h>
#include <nlohmann/json.hpp>

#include <string>

namespace power_grid_model::meta_data {

namespace {

using namespace std::string_literals;

constexpr char const* json_data =
    R"({"attributes":{},"data":{"node":[{"id":5}]},"is_batch":false,"type":"input","version":"1.0"})";
} // namespace

TEST_CASE("Serialization") {
    // get handle
    HandlePtr const unique_handle{PGM_create_handle()};
    PGM_Handle* hl = unique_handle.get();
    std::vector<NodeInput> node{{{5}, nan}};
    std::array components{"node"};
    std::array<Idx, 1> elements_per_scenario{1};
    char const* dataset = "input";
    Idx const n_components = 1;
    Idx const batch_size = 1;
    Idx const is_batch = 0;

    SUBCASE("Serializer") {
        Idx const** indptrs = nullptr;
        std::array<void const*, 1> data{node.data()};
        SerializerPtr unique_serializer{PGM_create_serializer(hl, dataset, is_batch, batch_size, n_components,
                                                              components.data(), elements_per_scenario.data(), indptrs,
                                                              data.data())};
        auto const serializer = unique_serializer.get();
        CHECK(PGM_error_code(hl) == PGM_no_error);
        std::string json_result = PGM_get_json(hl, serializer, 0, -1);
        CHECK(PGM_error_code(hl) == PGM_no_error);
        CHECK(json_result == json_data);

        // msgpack round trip
        char const* msgpack_data{};
        Idx msgpack_size{};
        PGM_get_msgpack(hl, serializer, 0, &msgpack_data, &msgpack_size);
        CHECK(PGM_error_code(hl) == PGM_no_error);
        auto const json_document = nlohmann::json::from_msgpack(msgpack_data, msgpack_data + msgpack_size);
        json_result = json_document.dump(-1);
        CHECK(json_result == json_data);
    }

    SUBCASE("Deserializer") {
        std::array<void*, 1> data{node.data()};
        Idx** indptrs = nullptr;

        // msgpack data
        auto const json_document = nlohmann::json::parse(json_data);
        std::vector<char> msgpack_data;
        nlohmann::json::to_msgpack(json_document, msgpack_data);

        DeserializerPtr unique_deserializer_json(PGM_create_deserializer_from_json(hl, json_data));
        CHECK(PGM_error_code(hl) == PGM_no_error);
        DeserializerPtr unique_deserializer_msgpack(
            PGM_create_deserializer_from_msgpack(hl, msgpack_data.data(), static_cast<Idx>(msgpack_data.size())));
        CHECK(PGM_error_code(hl) == PGM_no_error);

        for (PGM_Deserializer* const ptr : {unique_deserializer_json.get(), unique_deserializer_msgpack.get()}) {
            // reset data
            node[0] = {};
            // check meta data
            CHECK(PGM_deserializer_dataset_name(hl, ptr) == "input"s);
            CHECK(PGM_deserializer_is_batch(hl, ptr) == 0);
            CHECK(PGM_deserializer_batch_size(hl, ptr) == 1);
            CHECK(PGM_deserializer_n_components(hl, ptr) == 1);
            CHECK(PGM_deserializer_component_name(hl, ptr, 0) == "node"s);
            CHECK(PGM_deserializer_component_elements_per_scenario(hl, ptr, 0) == elements_per_scenario[0]);
            CHECK(PGM_deserializer_component_total_elements(hl, ptr, 0) == 1);
            // parse
            PGM_deserializer_parse_to_buffer(hl, ptr, components.data(), data.data(), indptrs);
            // check
            CHECK(node[0].id == 5);
            CHECK(is_nan(node[0].u_rated));
        }
    }
}

} // namespace power_grid_model::meta_data
