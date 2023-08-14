// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include "c_api_cpp_handle.hpp"
#include "power_grid_model_c.h"

#include <power_grid_model/auxiliary/meta_data_gen.hpp>

#include <doctest/doctest.h>
#include <nlohmann/json.hpp>

namespace power_grid_model::meta_data {

namespace {
constexpr char const* json_data =
    R"({"attributes":{},"data":{"node":[{"id":5}]},"is_batch":false,"type":"input","version":"1.0"})";
} // namespace

TEST_CASE("Serialization") {
    // get handle
    HandlePtr const unique_handle{PGM_create_handle()};
    PGM_Handle* hl = unique_handle.get();
    std::vector<NodeInput> const node{{{5}, nan}};
    std::array components{"node"};
    std::array<Idx, 1> elements_per_scenario{1};
    Idx const** indptrs = nullptr;
    char const* dataset = "input";
    Idx const n_components = 1;
    Idx const batch_size = 1;
    Idx const is_batch = 0;
    std::array<void const*, 1> data{node.data()};

    SUBCASE("Serializer") {
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

    SUBCASE("Deserializer") {}
}

} // namespace power_grid_model::meta_data