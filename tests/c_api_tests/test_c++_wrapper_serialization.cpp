// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

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

TEST_CASE("C API Serialization and Deserialization") {
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
        dataset.add_buffer("node", elements_per_scenario[0], total_elements[0], nullptr,
                                     node.data());
        dataset.add_buffer("source", elements_per_scenario[1], total_elements[1], nullptr,
                                     source.data());

        SUBCASE("JSON") { // currently here.
            SerializerPtr const json_serializer{
                PGM_create_serializer(hl, dataset, static_cast<PGM_Idx>(SerializationFormat::json))};
            auto* const serializer = json_serializer.get();
            CHECK(PGM_error_code(hl) == PGM_no_error);

            SUBCASE("To zero-terminated string") {
                std::string json_result = PGM_serializer_get_to_zero_terminated_string(hl, serializer, 0, -1);
                CHECK(PGM_error_code(hl) == PGM_no_error);
                CHECK(json_result == json_data);
            }

            SUBCASE("To binary buffer") {
                char const* buffer_data{};
                Idx buffer_size{};
                PGM_serializer_get_to_binary_buffer(hl, serializer, 0, &buffer_data, &buffer_size);
                CHECK(PGM_error_code(hl) == PGM_no_error);
                std::string const json_string{buffer_data, static_cast<size_t>(buffer_size)};
                CHECK(json_string == json_data);
            }
        }

        SUBCASE("MessagePack") {
            SerializerPtr const msgpack_serializer{
                PGM_create_serializer(hl, dataset, static_cast<PGM_Idx>(SerializationFormat::msgpack))};
            auto* const serializer = msgpack_serializer.get();

            SUBCASE("Round trip") {
                char const* msgpack_data{};
                Idx msgpack_size{};
                PGM_serializer_get_to_binary_buffer(hl, serializer, 0, &msgpack_data, &msgpack_size);
                CHECK(PGM_error_code(hl) == PGM_no_error);
                auto const json_document =
                    nlohmann::ordered_json::from_msgpack(msgpack_data, msgpack_data + msgpack_size);
                auto const json_result = json_document.dump(-1);
                CHECK(json_result == json_data);
            }

            SUBCASE("Cannot serialize msgpack to zero terminated string") {
                PGM_serializer_get_to_zero_terminated_string(hl, serializer, 0, 0);
                CHECK(PGM_error_code(hl) == PGM_serialization_error);
            }
        }

        SUBCASE("Invalid serialization format") {
            SerializerPtr const unknown_serializer{PGM_create_serializer(hl, dataset, -1)};
            CHECK(PGM_error_code(hl) == PGM_serialization_error);
        }
    }

    SUBCASE("Deserializer") {
        // msgpack data
        auto const json_document = nlohmann::json::parse(json_data);
        std::vector<char> msgpack_data;
        nlohmann::json::to_msgpack(json_document, msgpack_data);

        DeserializerPtr const unique_deserializer_json(PGM_create_deserializer_from_null_terminated_string(
            hl, json_data, static_cast<PGM_Idx>(SerializationFormat::json)));
        CHECK(PGM_error_code(hl) == PGM_no_error);
        DeserializerPtr const unique_deserializer_msgpack(
            PGM_create_deserializer_from_binary_buffer(hl, msgpack_data.data(), static_cast<Idx>(msgpack_data.size()),
                                                       static_cast<PGM_Idx>(SerializationFormat::msgpack)));
        CHECK(PGM_error_code(hl) == PGM_no_error);

        for (PGM_Deserializer* const deserializer :
             {unique_deserializer_json.get(), unique_deserializer_msgpack.get()}) {
            // reset data
            node[0] = {};
            // get dataset
            auto* const dataset = PGM_deserializer_get_dataset(hl, deserializer);
            auto const* const info = PGM_dataset_writable_get_info(hl, dataset);
            // check meta data
            CHECK(PGM_dataset_info_name(hl, info) == "input"s);
            CHECK(PGM_dataset_info_is_batch(hl, info) == is_batch);
            CHECK(PGM_dataset_info_batch_size(hl, info) == batch_size);
            CHECK(PGM_dataset_info_n_components(hl, info) == n_components);
            CHECK(PGM_dataset_info_component_name(hl, info, 0) == "node"s);
            CHECK(PGM_dataset_info_component_name(hl, info, 1) == "source"s);
            for (Idx const idx : {0, 1}) {
                CHECK(PGM_dataset_info_elements_per_scenario(hl, info, idx) == elements_per_scenario[idx]);
                CHECK(PGM_dataset_info_total_elements(hl, info, idx) == total_elements[idx]);
            }
            // set buffer
            PGM_dataset_writable_set_buffer(hl, dataset, "node", nullptr, node.data());
            PGM_dataset_writable_set_buffer(hl, dataset, "source", nullptr, source.data());
            CHECK(PGM_error_code(hl) == PGM_no_error);
            // parse
            PGM_deserializer_parse_to_buffer(hl, deserializer);
            // check
            CHECK(node[0].id == 5);
            CHECK(is_nan(node[0].u_rated));
            CHECK(source[0].id == 6);
            CHECK(source[1].id == 7);
        }
    }

    SUBCASE("Use deserialized dataset") {
        DeserializerPtr const unique_deserializer_json(PGM_create_deserializer_from_null_terminated_string(
            hl, complete_json_data, static_cast<PGM_Idx>(SerializationFormat::json)));
        CHECK(PGM_error_code(hl) == PGM_no_error);
        PGM_Deserializer* const deserializer = unique_deserializer_json.get();
        // get dataset
        auto* const dataset = PGM_deserializer_get_dataset(hl, deserializer);
        auto const* const info = PGM_dataset_writable_get_info(hl, dataset);
        // check meta data
        CHECK(PGM_dataset_info_name(hl, info) == "input"s);
        CHECK(PGM_dataset_info_is_batch(hl, info) == is_batch);
        CHECK(PGM_dataset_info_batch_size(hl, info) == batch_size);
        CHECK(PGM_dataset_info_n_components(hl, info) == n_components);
        CHECK(PGM_dataset_info_component_name(hl, info, 0) == "node"s);
        CHECK(PGM_dataset_info_component_name(hl, info, 1) == "source"s);
        // set buffer
        PGM_dataset_writable_set_buffer(hl, dataset, "node", nullptr, node.data());
        PGM_dataset_writable_set_buffer(hl, dataset, "source", nullptr, source.data());
        CHECK(PGM_error_code(hl) == PGM_no_error);
        // parse
        PGM_deserializer_parse_to_buffer(hl, deserializer);
        // create model from deserialized dataset
        ConstDatasetPtr input_dataset{PGM_create_dataset_const_from_writable(hl, dataset)};
        ModelPtr model{PGM_create_model(hl, 50.0, input_dataset.get())};
        CHECK(PGM_error_code(hl) == PGM_no_error);
    }
}

} // namespace power_grid_model_cpp
