// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "power_grid_model_cpp.hpp"

#include <power_grid_model_c/dataset_definitions.h>

#include <doctest/doctest.h>
#include <nlohmann/json.hpp>

#include <cmath>
#include <limits>
#include <string>
#include <vector>

namespace power_grid_model_cpp {
namespace {
using namespace std::string_literals;

constexpr char const* json_data =
    R"({"version":"1.0","type":"input","is_batch":false,"attributes":{},"data":{"node":[{"id":5}],"source":[{"id":6},{"id":7}]}})";
constexpr char const* complete_json_data =
    R"({"version":"1.0","type":"input","is_batch":false,"attributes":{"node": ["id", "u_rated"]},"data":{"node":[[5, 10500]],"source":[{"id":6, "node": 5, "status": 1, "u_ref": 1.0}]}})";
} // namespace

TEST_CASE("API Serialization and Deserialization") {

    ID node_id = 5;
    Buffer node_buffer{PGM_def_input_node, 1};
    node_buffer.set_nan();
    node_buffer.set_value(PGM_def_input_node_id, &node_id, -1);

    std::vector<ID> source_id = {6, 7};
    std::vector<ID> source_node = {std::numeric_limits<ID>::min(), std::numeric_limits<ID>::min()};
    std::vector<int8_t> source_status = {std::numeric_limits<int8_t>::min(), std::numeric_limits<int8_t>::min()};
    Buffer source_buffer{PGM_def_input_source, 2};
    source_buffer.set_nan();
    source_buffer.set_value(PGM_def_input_source_id, source_id.data(), -1);
    source_buffer.set_value(PGM_def_input_source_node, source_node.data(), -1);
    source_buffer.set_value(PGM_def_input_source_status, source_status.data(), -1);

    // For deserializer
    Buffer node_buffer_2{PGM_def_input_node, 1};
    Buffer source_buffer_2{PGM_def_input_source, 2};

    Idx const n_components = 2;
    Idx const batch_size = 1;
    bool const is_batch = false;
    std::vector<Idx> const elements_per_scenario = {1, 2};
    std::vector<Idx> const elements_per_scenario_complete = {1, 1};
    std::vector<Idx> const total_elements = {1, 2};
    std::vector<Idx> const total_elements_complete = {1, 1};

    SUBCASE("Serializer") {
        DatasetConst dataset{"input", is_batch, batch_size};
        dataset.add_buffer("node", elements_per_scenario[0], total_elements[0], nullptr, node_buffer);
        dataset.add_buffer("source", elements_per_scenario[1], total_elements[1], nullptr, source_buffer);

        SUBCASE("JSON") {
            Serializer json_serializer{dataset, 0};

            SUBCASE("To zero-terminated string") {
                std::string const json_result = json_serializer.get_to_zero_terminated_string(0, -1);
                CHECK(json_result == json_data);
            }

            SUBCASE("To binary buffer") {
                std::vector<std::byte> buffer_data{};
                json_serializer.get_to_binary_buffer(0, buffer_data);
                std::string converted_string(buffer_data.size(), '\0');
                std::ranges::transform(buffer_data, converted_string.begin(),
                                       [](std::byte b) { return static_cast<char>(b); });
                CHECK(converted_string == json_data);
            }
        }

        SUBCASE("MessagePack") {
            Serializer msgpack_serializer{dataset, 1};

            SUBCASE("Round trip") {
                std::vector<std::byte> msgpack_data{};
                msgpack_serializer.get_to_binary_buffer(0, msgpack_data);
                auto const* const char_start = reinterpret_cast<unsigned char const*>(msgpack_data.data());
                auto const* const char_end = char_start + msgpack_data.size();
                auto const json_document = nlohmann::ordered_json::from_msgpack(char_start, char_end);
                auto const json_result = json_document.dump(-1);
                CHECK(json_result == json_data);
            }

            SUBCASE("Cannot serialize msgpack to zero terminated string") {
                CHECK_THROWS_AS(msgpack_serializer.get_to_zero_terminated_string(0, 0), PowerGridSerializationError);
            }
        }

        SUBCASE("Invalid serialization format") {
            try {
                Serializer const unknown_serializer{dataset, -1};
                FAIL("Expected serialization error not thrown.");
            } catch (PowerGridSerializationError const& e) {
                CHECK(e.error_code() == PGM_serialization_error);
            }
        }
    }

    SUBCASE("Deserializer") {
        // msgpack data
        auto const json_document = nlohmann::json::parse(json_data);
        std::vector<char> msgpack_data;

        nlohmann::json::to_msgpack(json_document, msgpack_data);

        // test move-ability
        Deserializer json_deserializer{json_data, 0};
        Deserializer json_dummy{std::move(json_deserializer)};
        json_deserializer = std::move(json_dummy);
        Deserializer msgpack_deserializer{msgpack_data, 1};

        auto check_metadata = [&](DatasetInfo const& info) {
            CHECK(info.name() == "input"s);
            CHECK(info.is_batch() == is_batch);
            CHECK(info.batch_size() == batch_size);
            CHECK(info.n_components() == n_components);
            CHECK(info.component_name(0) == "node"s);
            CHECK(info.component_name(1) == "source"s);
            for (Idx const idx : {0, 1}) {
                CHECK(info.component_elements_per_scenario(idx) == elements_per_scenario[idx]);
                CHECK(info.component_total_elements(idx) == total_elements[idx]);
            }
        };

        auto check_deserializer = [&](Deserializer& deserializer) {
            // get dataset
            auto& dataset = deserializer.get_dataset();
            auto const& info = dataset.get_info();
            // check meta data
            check_metadata(info);
            // set buffer
            dataset.set_buffer("node", nullptr, node_buffer_2);
            dataset.set_buffer("source", nullptr, source_buffer_2);
            // parse
            deserializer.parse_to_buffer();
            // check
            ID node_2_id;
            double node_2_u_rated;
            std::vector<ID> source_2_id(2);
            node_buffer_2.get_value(PGM_def_input_node_id, &node_2_id, -1);
            node_buffer_2.get_value(PGM_def_input_node_u_rated, &node_2_u_rated, -1);
            source_buffer_2.get_value(PGM_def_input_source_id, source_2_id.data(), -1);
            CHECK(node_2_id == 5);
            CHECK(std::isnan(node_2_u_rated));
            CHECK(source_2_id[0] == 6);
            CHECK(source_2_id[1] == 7);
        };

        check_deserializer(json_deserializer);
        check_deserializer(msgpack_deserializer);
    }

    SUBCASE("Deserializer with columnar data") {
        // msgpack data
        auto const json_document = nlohmann::json::parse(complete_json_data);
        std::vector<char> msgpack_data;

        nlohmann::json::to_msgpack(json_document, msgpack_data);

        // test move-ability
        Deserializer json_deserializer{complete_json_data, 0};
        Deserializer json_dummy{std::move(json_deserializer)};
        json_deserializer = std::move(json_dummy);
        Deserializer msgpack_deserializer{msgpack_data, 1};

        auto check_metadata = [&](DatasetInfo const& info) {
            CHECK(info.name() == "input"s);
            CHECK(info.is_batch() == is_batch);
            CHECK(info.batch_size() == batch_size);
            CHECK(info.n_components() == n_components);
            CHECK(info.component_name(0) == "node"s);
            CHECK(info.component_name(1) == "source"s);
            CHECK(info.has_attribute_indications(0));
            CHECK_FALSE(info.has_attribute_indications(1));
            auto const node_attributes = info.attribute_indications(0);
            REQUIRE(node_attributes.size() == 2);
            CHECK(node_attributes[0] == "id"s);
            CHECK(node_attributes[1] == "u_rated"s);
            for (Idx const idx : {0, 1}) {
                CHECK(info.component_elements_per_scenario(idx) == elements_per_scenario_complete[idx]);
                CHECK(info.component_total_elements(idx) == total_elements_complete[idx]);
            }
        };

        auto check_deserializer = [&](Deserializer& deserializer) {
            // get dataset
            auto& dataset = deserializer.get_dataset();
            auto const& info = dataset.get_info();
            // check meta data
            check_metadata(info);
            ID node_id_2{0};
            double node_u_rated_2;
            // set buffer
            Buffer source_buffer_columnar{PGM_def_input_source, 1};
            dataset.set_buffer("node", nullptr, nullptr);
            dataset.set_attribute_buffer("node", "id", &node_id_2);
            dataset.set_attribute_buffer("node", "u_rated", &node_u_rated_2);
            dataset.set_buffer("source", nullptr, source_buffer_columnar);
            // parse
            deserializer.parse_to_buffer();
            // check
            ID source_2_id;
            source_buffer_columnar.get_value(PGM_def_input_source_id, &source_2_id, -1);
            CHECK(node_id_2 == 5);
            CHECK(node_u_rated_2 == doctest::Approx(10.5e3));
            CHECK(source_2_id == 6);
        };

        check_deserializer(json_deserializer);
        check_deserializer(msgpack_deserializer);
    }

    SUBCASE("Use deserialized dataset") {
        Deserializer deserializer_json(complete_json_data, 0);

        // get dataset
        auto& dataset = deserializer_json.get_dataset();
        auto const& info = dataset.get_info();
        // check meta data
        CHECK(info.name() == "input"s);
        CHECK(info.is_batch() == is_batch);
        CHECK(info.batch_size() == batch_size);
        CHECK(info.n_components() == n_components);
        CHECK(info.component_name(0) == "node"s);
        CHECK(info.component_name(1) == "source"s);
        // set buffer
        dataset.set_buffer("node", nullptr, node_buffer_2);
        dataset.set_buffer("source", nullptr, source_buffer_2);
        // parse
        deserializer_json.parse_to_buffer();
        // create model from deserialized dataset
        DatasetConst const input_dataset{dataset};
        Model const model{50.0, input_dataset};
    }
}

TEST_CASE("API Serialization and Deserialization with float precision") {
    // dataset with one double value
    std::vector<double> const u_rated_ref{1.8014398509481982e+16};
    DatasetConst dataset{"input", true, 1};
    dataset.add_buffer("node", 1, 1, nullptr, nullptr);
    dataset.add_attribute_buffer("node", "u_rated", u_rated_ref.data());

    // serialize
    Serializer json_serializer{dataset, PGM_json};
    std::string const json_result = json_serializer.get_to_zero_terminated_string(0, -1);

    // deserialize
    std::vector<double> u_rated(1);
    Deserializer deserializer{json_result, PGM_json};
    auto& deserialized_dataset = deserializer.get_dataset();
    deserialized_dataset.set_buffer("node", nullptr, nullptr);
    deserialized_dataset.set_attribute_buffer("node", "u_rated", u_rated.data());
    deserializer.parse_to_buffer();

    // check
    CHECK(u_rated_ref[0] == u_rated[0]);
}
} // namespace power_grid_model_cpp
