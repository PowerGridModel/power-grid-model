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
    struct ComponentData {
        std::string name;
        bool is_uniform{false};
        Idx elements_per_scenario{-1};
        Idx total_elements{};
        void* data{};   // set by user
        Idx* indptr{};  // set by user
    };

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
        get_value_from_root("version", msgpack::type::STR) >> version_;
        get_value_from_root("type", msgpack::type::STR) >> dataset_type_;
        get_value_from_root("is_batch", msgpack::type::BOOLEAN) >> is_batch_;
        dataset_ = &meta_data().get_dataset(dataset_type_);
        read_predefined_attributes();
    }

    msgpack::object const& get_value_from_root(std::string const& key, msgpack::type::object_type type) {
        std::vector<std::string> keys;
        msgpack::object const& root = handle_.get();
        msgpack::object const& obj = [&]() -> msgpack::object const& {
            for (Idx i = 0; i != (Idx)root.via.map.size; ++i) {
                if (key == root.via.map.ptr[i].key.as<std::string>()) {
                    return root.via.map.ptr[i].val;
                }
            }
            throw SerializationError{"Cannot find key " + key + " in the root level dictionary!"};
        }();
        if (obj.type != type) {
            throw SerializationError{"Wrong data type for key " + key + " in the root level dictionary!"};
        }
        return obj;
    }

    void read_predefined_attributes() {
        msgpack::object const& attribute_map = get_value_from_root("attributes", msgpack::type::MAP);
        auto const map_ptr = attribute_map.via.map.ptr;
        auto const map_size = attribute_map.via.map.size;
        for (msgpack::object_kv const* kv = map_ptr; kv != map_ptr + map_size; ++kv) {
            MetaComponent const& component = dataset_->get_component(kv->key.as<std::string>());
            msgpack::object const& attribute_list = kv->val;
            if (attribute_list.type != msgpack::type::ARRAY) {
                throw SerializationError{
                    "Each entry of attribute dictionary should be a list for the corresponding component!"};
            }
            auto const list_ptr = attribute_list.via.array.ptr;
            auto const list_size = attribute_list.via.array.size;

            for (msgpack::object const* attr_obj = list_ptr; attr_obj != list_ptr + list_size; ++attr_obj) {
                attributes_[component.name].push_back(&component.get_attribute(attr_obj->as<std::string>()));
            }
        }
    }

    void count_data() {
        msgpack::object const& obj = get_value_from_root("data", is_batch_ ? msgpack::type::ARRAY : msgpack::type::MAP);
        if (is_batch_) {
            batch_size_ = (Idx)obj.via.array.size;
            msgpack_data_ = obj.via.array.ptr;
        }
        else {
            batch_size_ = 1;
            msgpack_data_ = &obj;
        }
    }
};

}  // namespace power_grid_model::meta_data

#endif