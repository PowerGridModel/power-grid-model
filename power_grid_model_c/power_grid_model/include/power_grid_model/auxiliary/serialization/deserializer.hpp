// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_SERIALIZTION_DESERIALIZER_HPP
#define POWER_GRID_MODEL_AUXILIARY_SERIALIZTION_DESERIALIZER_HPP

#include "../../exception.hpp"
#include "../../power_grid_model.hpp"
#include "../meta_data.hpp"

#include <nlohmann/json.hpp>

#include <msgpack.hpp>

namespace power_grid_model::meta_data {

class Deserializer {
   public:
    void deserialize_from_json(char const* json_string) {
        nlohmann::json const json_document = nlohmann::json::parse(json_string);
        std::vector<char> msgpack_data;
        json_document.to_msgpack(msgpack_data);
        deserialize_from_msgpack(msgpack_data.data(), msgpack_data.size());
    }

    void deserialize_from_msgpack(char const* data, size_t length) {
        handle_ = msgpack::unpack(data, length);
        parse_meta_data();
    }

   private:
    msgpack::object_handle handle_;
    std::string version_;
    std::string dataset_type_;
    bool is_batch_{};
    MetaDataset const* dataset_{};
    std::map<std::string, std::vector<MetaAttribute const*>> attributes_;
    Idx batch_size_;                       // for single dataset, the batch size is one
    msgpack::object const* msgpack_data_;  // pointer to array (or single value) of msgpack objects to the data

    void parse_meta_data() {
        if (handle_.get().type != msgpack::type::MAP) {
            throw SerializationError{"The root level object should be a dictionary!"};
        }
        handle_.get() >> root_map_;
        get_value_from_root("version", msgpack::type::STR) >> version_;
        get_value_from_root("type", msgpack::type::STR) >> dataset_type_;
        get_value_from_root("is_batch", msgpack::type::BOOLEAN) >> is_batch_;
        dataset_ = &meta_data().get_dataset(dataset_type_);
        read_predefined_attributes();
    }

    msgpack::object const& get_value_from_root(std::string const& key, msgpack::type::object_type type) {
        std::vector<std::string> keys;
        msgpack::object const& root = handle_.get();
        // for (Idx i = 0; i != root.via.array.)

        auto const found = root_map_.find(key);
        if (found == root_map_.cend()) {
            throw SerializationError{"Cannot find key " + key + " in the root level dictionary!"};
        }
        msgpack::object const& obj = found->second;
        if (obj.type != type) {
            throw SerializationError{"Wrong data type for key " + key + " in the root level dictionary!"};
        }
        return obj;
    }

    void read_predefined_attributes() {
        attributes_ = {};
        if (!root_map_.contains("attributes")) {
            return;
        }
        auto const attribute_map = root_map_.at("attributes").as<std::map<std::string, std::vector<std::string>>>();
        for (auto const& [component_name, attributes] : attribute_map) {
            MetaComponent const& component = dataset_->get_component(component_name);
            for (auto const& attribute : attributes) {
                attributes_[component_name].push_back(&component.get_attribute(attribute));
            }
        }
    }

    void count_data() {
        // msgpack::object const& obj = get_value_from_root("data", is_batch ? msgpack::type::ARRAY :
        // msgpack::type::MAP);
    }
};

}  // namespace power_grid_model::meta_data

#endif