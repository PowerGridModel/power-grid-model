// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_ENABLE_EXPERIMENTAL

#include "power_grid_model_cpp.hpp"

#include <power_grid_model/auxiliary/meta_data_gen.hpp>

#include <doctest/doctest.h>
#include <nlohmann/json.hpp>

#include <string>

namespace power_grid_model_cpp {

namespace {

using namespace std::string_literals;

constexpr char const* json_data =
    R"({"version":"1.0","type":"input","is_batch":false,"attributes":{},"data":{"node":[{"id":5}],"source":[{"id":6},{"id":7}]}})";
constexpr char const* complete_json_data =
    R"({"version":"1.0","type":"input","is_batch":false,"attributes":{},"data":{"node":[{"id":5, "u_rated": 10500}],"source":[{"id":6, "node": 5, "status": 1, "u_ref": 1.0}]}})";
} // namespace

TEST_CASE("C++ API Serialization and Deserialization") {
    std::vector<NodeInput> node{{5, nan}};
    std::vector<SourceInput> source{{6, na_IntID, na_IntS, nan, nan, nan, nan, nan},
                                    {7, na_IntID, na_IntS, nan, nan, nan, nan, nan}};
    Idx const n_components = 2;
    Idx const batch_size = 1;
    Idx const is_batch = 0;
    std::vector<Idx> const elements_per_scenario = {1, 2};
    std::vector<Idx> const total_elements = {1, 2};

    SUBCASE("Serializer") {
        DatasetConst dataset{"input", is_batch, batch_size};
        dataset.add_buffer("node", elements_per_scenario[0], total_elements[0], nullptr, node.data());
        dataset.add_buffer("source", elements_per_scenario[1], total_elements[1], nullptr, source.data());

        SUBCASE("JSON") {
            Serializer json_serializer{dataset, static_cast<Idx>(power_grid_model::SerializationFormat::json)};

            SUBCASE("To zero-terminated string") {
                std::string json_result = json_serializer.get_to_zero_terminated_string(0, -1);
                CHECK(json_result == json_data);
            }

            SUBCASE("To binary buffer") {
                std::vector<std::byte> buffer_data{};
                json_serializer.get_to_binary_buffer(0, buffer_data);
                CHECK(std::string{buffer_data.begin(), buffer_data.end()} == json_data);
            }
        }

        SUBCASE("MessagePack") {
            Serializer msgpack_serializer{dataset, static_cast<Idx>(power_grid_model::SerializationFormat::msgpack)};

            SUBCASE("Round trip") {
                std::vector<std::byte> msgpack_data{};
                msgpack_serializer.get_to_binary_buffer(0, msgpack_data);
                auto const json_document = nlohmann::ordered_json::from_msgpack(msgpack_data);
                auto const json_result = json_document.dump(-1);
                CHECK(json_result == json_data);
            }

            SUBCASE("Cannot serialize msgpack to zero terminated string") {
                CHECK_THROWS_AS(msgpack_serializer.get_to_zero_terminated_string(0, 0), PowerGridSerializationError);
            }
        }

        SUBCASE("Invalid serialization format") {
            CHECK_THROWS_AS(Serializer unknown_serializer{dataset, -1}, PowerGridSerializationError);
        }
    }

    SUBCASE("Deserializer") {
        // msgpack data
        auto const json_document = nlohmann::json::parse(json_data);
        std::vector<std::byte> msgpack_data;
        nlohmann::json::to_msgpack(json_document, msgpack_data);

        Deserializer deserializer_json{json_data, static_cast<Idx>(power_grid_model::SerializationFormat::json)};

        Deserializer deserializer_msgpack{msgpack_data.data(), static_cast<Idx>(msgpack_data.size()),
                                          static_cast<Idx>(power_grid_model::SerializationFormat::msgpack)};

        for (Deserializer deserializer :
             {deserializer_json, deserializer_msgpack}) { // check if needs to be addressed because deserializer might
                                                          // not be copy or move constructable
            // reset data
            node[0] = {};
            // get dataset
            auto dataset = deserializer.get_dataset();
            auto info = dataset.get_info();
            // check meta data
            CHECK(info.name() == "input"s);
            CHECK(info.is_batch() == is_batch);
            CHECK(info.batch_size() == batch_size);
            CHECK(info.n_components() == n_components);
            CHECK(info.component_name(0) == "node"s);
            CHECK(info.component_name(1) == "source"s);
            for (Idx const idx : {0, 1}) {
                CHECK(info.elements_per_scenario(idx) == elements_per_scenario[idx]);
                CHECK(info.total_elements(idx) == total_elements[idx]);
            }
            // set buffer
            dataset.set_buffer("node", nullptr, node.data());
            dataset.set_buffer("source", nullptr, source.data());
            // parse
            deserializer.parse_to_buffer();
            // check
            CHECK(node[0].id == 5);
            CHECK(is_nan(node[0].u_rated));
            CHECK(source[0].id == 6);
            CHECK(source[1].id == 7);
        }
    }

    SUBCASE("Use deserialized dataset") { //////// currently here
        Deserializer deserializer_json(complete_json_data,
                                       static_cast<PGM_Idx>(power_grid_model::SerializationFormat::json));

        // get dataset
        auto dataset = deserializer_json.get_dataset();
        auto info = dataset.get_info();
        // check meta data
        CHECK(info.name() == "input"s);
        CHECK(info.is_batch() == is_batch);
        CHECK(info.batch_size() == batch_size);
        CHECK(info.n_components() == n_components);
        CHECK(info.component_name(0) == "node"s);
        CHECK(info.component_name(1) == "source"s);
        // set buffer
        dataset.set_buffer("node", nullptr, node.data());
        dataset.set_buffer("source", nullptr, source.data());
        // parse
        deserializer_json.parse_to_buffer();
        // create model from deserialized dataset
        DatasetConst input_dataset{dataset};
        Model model{50.0, input_dataset};
    }
}

} // namespace power_grid_model_cpp
