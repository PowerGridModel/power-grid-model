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

#include <set>
#include <span>
#include <string_view>

namespace power_grid_model::meta_data {

class Deserializer {
   public:
    // struct of buffer data
    struct Buffer {
        MetaComponent const* component;
        bool is_uniform;
        Idx elements_per_scenario;
        Idx total_elements;
        std::vector<std::span<msgpack::object>> msg_data;  // vector of spans of msgpack object of each batch
        void* data;                                        // set by user
        std::span<Idx> indptr;                             // set by user
    };

    // not copyable
    Deserializer(Deserializer const&) = delete;
    Deserializer& operator=(Deserializer const&) = delete;

    void deserialize_from_json(char const* json_string) {
        nlohmann::json const json_document = nlohmann::json::parse(json_string);
        std::vector<char> msgpack_data;
        json_document.to_msgpack(msgpack_data);
        deserialize_from_msgpack(msgpack_data.data(), msgpack_data.size());
    }

    void deserialize_from_msgpack(char const* data, size_t length) {
        handle_ = msgpack::unpack(data, length);
        parse_meta_data();
        count_data();
    }

    std::string const& dataset_name() const {
        return dataset_->name;
    }

    Idx batch_size() const {
        return batch_size_;
    }

    Idx n_components() const {
        return (Idx)buffers_.size();
    }

    Buffer const& get_buffer_info(Idx i) const {
        return buffers_[i];
    }

    void set_buffer(char const** components, void** data, Idx** indptrs) {
        for (Idx i = 0; i != n_components(); ++i) {
            auto const found = std::find_if(buffers_.begin(), buffers_.end(), [components, i](Buffer const& buffer) {
                return buffer.component->name == components[i];
            });
            if (found == buffers_.end()) {
                throw SerializationError{"Unkown component: " + std::string(components[i]) +
                                         "! You need to supply the components which are present.\n"};
            }
            found->data = data[i];
            if (!found->is_uniform) {
                found->indptr = {indptrs[i], (size_t)(batch_size_ + 1)};
            }
        }
    }

    void parse() {
        for (Buffer const& buffer : buffers_) {
            parse_component(buffer);
        }
    }

   private:
    msgpack::object_handle handle_;
    std::string version_;
    bool is_batch_{};
    MetaDataset const* dataset_{};
    std::map<std::string, std::vector<MetaAttribute const*>> attributes_;
    Idx batch_size_;  // for single dataset, the batch size is one
    std::vector<Buffer> buffers_;

    void parse_meta_data() {
        if (handle_.get().type != msgpack::type::MAP) {
            throw SerializationError{"The root level object should be a dictionary!\n"};
        }
        get_value_from_root("version", msgpack::type::STR) >> version_;
        dataset_ = &meta_data().get_dataset(get_value_from_root("type", msgpack::type::STR).as<std::string_view>());
        get_value_from_root("is_batch", msgpack::type::BOOLEAN) >> is_batch_;
        read_predefined_attributes();
    }

    msgpack::object const& get_value_from_root(std::string_view key, msgpack::type::object_type type) {
        msgpack::object const& root = handle_.get();
        return get_value_from_map(root, key, type);
    }

    msgpack::object const& get_value_from_map(msgpack::object const& map, std::string_view key,
                                              msgpack::type::object_type type) {
        Idx const idx = find_key_from_map(map, key);
        if (idx < 0) {
            throw SerializationError{"Cannot find key " + std::string(key) + " in the root level dictionary!\n"};
        }
        msgpack::object const& obj = map.via.map.ptr[idx].val;
        if (obj.type != type) {
            throw SerializationError{"Wrong data type for key " + std::string(key) +
                                     " in the root level dictionary!\n"};
        }
        return obj;
    }

    Idx find_key_from_map(msgpack::object const& map, std::string_view key) {
        std::span const kv_map{map.via.map.ptr, map.via.map.size};
        for (Idx i = 0; i != (Idx)kv_map.size(); ++i) {
            if (key == kv_map[i].key.as<std::string_view>()) {
                return i;
            }
        }
        return -1;
    }

    void read_predefined_attributes() {
        msgpack::object const& attribute_map = get_value_from_root("attributes", msgpack::type::MAP);
        for (auto const& kv : std::span{attribute_map.via.map.ptr, attribute_map.via.map.size}) {
            MetaComponent const& component = dataset_->get_component(kv.key.as<std::string_view>());
            msgpack::object const& attribute_list = kv.val;
            if (attribute_list.type != msgpack::type::ARRAY) {
                throw SerializationError{
                    "Each entry of attribute dictionary should be a list for the corresponding component!\n"};
            }
            for (auto const& attr_obj : std::span{attribute_list.via.array.ptr, attribute_list.via.array.size}) {
                attributes_[component.name].push_back(&component.get_attribute(attr_obj.as<std::string_view>()));
            }
        }
    }

    void count_data() {
        msgpack::object const& obj = get_value_from_root("data", is_batch_ ? msgpack::type::ARRAY : msgpack::type::MAP);
        buffers_ = {};
        // pointer to array (or single value) of msgpack objects to the data
        std::span<msgpack::object const> batch_data;
        if (is_batch_) {
            batch_size_ = (Idx)obj.via.array.size;
            batch_data = {obj.via.array.ptr, obj.via.array.size};
        }
        else {
            batch_size_ = 1;
            batch_data = {&obj, 1};
        }

        // get set of all components
        std::set<std::string> all_components;
        for (msgpack::object const& scenario : batch_data) {
            if (scenario.type != msgpack::type::MAP) {
                throw SerializationError{"The data object of each scenario should be a dictionary!\n"};
            }
            std::span<msgpack::object_kv> const scenario_map{scenario.via.map.ptr, scenario.via.map.size};
            for (msgpack::object_kv const& kv : scenario_map) {
                all_components.insert(kv.val.as<std::string>());
            }
        }

        // create buffer object
        for (std::string const& component : all_components) {
            buffers_.push_back(count_component(batch_data, component));
        }
    }

    Buffer count_component(std::span<msgpack::object const> batch_data, std::string const& component) {
        // count number of element of all scenarios
        IdxVector counter(batch_size_);
        std::vector<std::span<msgpack::object>> msg_data(batch_size_);
        for (Idx scenario_number = 0; scenario_number != batch_size_; ++scenario_number) {
            msgpack::object const& scenario = batch_data[scenario_number];
            Idx const found_component_idx = find_key_from_map(scenario, component);
            if (found_component_idx >= 0) {
                msgpack::object const& component = scenario.via.map.ptr[found_component_idx].val;
                if (component.type != msgpack::type::ARRAY) {
                    throw SerializationError{"Each entry of component per scenario should be a list!\n"};
                }
                counter[scenario_number] = (Idx)component.via.array.size;
                msg_data[scenario_number] = {component.via.array.ptr, component.via.array.size};
            }
        }
        bool const is_uniform = check_uniform(counter);
        Idx const elements_per_scenario = get_elements_per_scenario(counter, is_uniform);
        Idx const total_elements =                              // total element based on is_uniform
            is_uniform ? elements_per_scenario * batch_size_ :  // multiply
                std::reduce(counter.cbegin(), counter.cend());  // aggregation
        return Buffer{.component = &dataset_->get_component(component),
                      .is_uniform = is_uniform,
                      .elements_per_scenario = elements_per_scenario,
                      .total_elements = total_elements,
                      .msg_data = msg_data,
                      .data = nullptr,
                      .indptr = {}};
    }

    bool check_uniform(IdxVector const& counter) {
        if (batch_size_ < 2) {
            return true;
        }
        return std::transform_reduce(counter.cbegin(), counter.cend() - 1, counter.cbegin() + 1, true,
                                     std::logical_and{}, std::equal_to{});
    }

    Idx get_elements_per_scenario(IdxVector const& counter, bool is_uniform) {
        if (!is_uniform) {
            return -1;
        }
        if (batch_size_ == 0) {
            return 0;
        }
        return counter.front();
    }

    void parse_component(Buffer const& buffer) {
        // handle indptr
        if (!buffer.is_uniform) {
            // first always zero
            buffer.indptr.front() = 0;
            // accumulate sum
            std::transform_inclusive_scan(buffer.msg_data.cbegin(), buffer.msg_data.cend(), buffer.indptr.begin() + 1,
                                          std::plus{}, [](auto const& x) {
                                              return (Idx)x.size();
                                          });
        }
        // set nan
        buffer.component->set_nan(buffer.data, 0, buffer.total_elements);
    }
};

}  // namespace power_grid_model::meta_data

#endif