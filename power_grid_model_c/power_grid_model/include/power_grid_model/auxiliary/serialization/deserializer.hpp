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
    std::map<std::string, msgpack::object> root_map_;
    msgpack::object data_object_;
    std::string version_;
    std::string dataset_type_;
    bool is_batch_{};
    MetaDataset const* dataset_{};
    std::map<std::string, std::vector<MetaAttribute const*>> attributes;

    void parse_meta_data() {
        handle_.get() >> root_map_;
        get_value_from_root("version", msgpack::type::STR) >> version_;
        get_value_from_root("type", msgpack::type::STR) >> dataset_type_;
        get_value_from_root("is_batch", msgpack::type::BOOLEAN) >> is_batch_;
        dataset_ = &meta_data().get_dataset(dataset_type_);
        data_object_ = get_value_from_root("data", is_batch_ ? msgpack::type::ARRAY : msgpack::type::MAP);
    }

    msgpack::object const& get_value_from_root(std::string const& key, msgpack::type::object_type type) {
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
};

}  // namespace power_grid_model::meta_data

#endif