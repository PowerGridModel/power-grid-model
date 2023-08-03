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

// converter for double[3]
namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {
    template <> struct convert<power_grid_model::RealValue<false>> {
        msgpack::object const& operator()(msgpack::object const& o, power_grid_model::RealValue<false>& v) const {
            if (o.type != msgpack::type::ARRAY)
                throw msgpack::type_error();
            if (o.via.array.size != 3)
                throw msgpack::type_error();
            for (int8_t i = 0; i != 3; ++i) {
                if (o.is_nil()) {
                    continue;
                }
                o.via.array.ptr[i] >> v(i);
            }
            return o;
        }
    };

    } // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack

namespace power_grid_model::meta_data {

class Deserializer {
  public:
    // struct of buffer data
    struct Buffer {
        MetaComponent const* component;
        // for non-uniform component, this is -1, we use indptr to describe the elements per scenario
        Idx elements_per_scenario;
        Idx total_elements;
        std::vector<std::span<msgpack::object const>> msg_data; // vector of spans of msgpack object of each batch
        void* data;                                             // set by user
        std::span<Idx> indptr;                                  // set by user
    };

    Deserializer() = default;
    // not copyable
    Deserializer(Deserializer const&) = delete;
    Deserializer& operator=(Deserializer const&) = delete;

    void deserialize_from_json(char const* json_string) {
        std::vector<char> const msgpack_data = json_to_msgpack(json_string);
        deserialize_from_msgpack(msgpack_data.data(), msgpack_data.size());
    }

    void deserialize_from_msgpack(char const* data, size_t length) {
        handle_ = msgpack::unpack(data, length);
        post_serialization();
    }

    std::string const& dataset_name() const { return dataset_->name; }

    bool is_batch() const { return is_batch_; }

    Idx batch_size() const { return batch_size_; }

    Idx n_components() const { return (Idx)buffers_.size(); }

    Buffer const& get_buffer_info(Idx i) const { return buffers_[i]; }

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
            if (found->elements_per_scenario < 0) {
                found->indptr = {indptrs[i], (size_t)(batch_size_ + 1)};
            }
        }
    }

    void parse() const {
        for (Buffer const& buffer : buffers_) {
            parse_component(buffer);
        }
    }

  private:
    msgpack::object_handle handle_{};
    std::string version_;
    bool is_batch_{};
    MetaDataset const* dataset_{};
    std::map<std::string, std::vector<MetaAttribute const*>> attributes_;
    Idx batch_size_{}; // for single dataset, the batch size is one
    std::vector<Buffer> buffers_;

    static std::vector<char> json_to_msgpack(char const* json_string) {
        nlohmann::json const json_document = nlohmann::json::parse(json_string);
        std::vector<char> msgpack_data;
        nlohmann::json::to_msgpack(json_document, msgpack_data);
        return msgpack_data;
    }

    void post_serialization() {
        if (handle_.get().type != msgpack::type::MAP) {
            throw SerializationError{"The root level object should be a dictionary!\n"};
        }
        get_value_from_root("version", msgpack::type::STR) >> version_;
        dataset_ = &meta_data().get_dataset(get_value_from_root("type", msgpack::type::STR).as<std::string_view>());
        get_value_from_root("is_batch", msgpack::type::BOOLEAN) >> is_batch_;
        read_predefined_attributes();
        count_data();
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
        } else {
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
                all_components.insert(kv.key.as<std::string>());
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
        std::vector<std::span<msgpack::object const>> msg_data(batch_size_);
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

        Idx const elements_per_scenario = get_elements_per_scenario(counter);
        Idx const total_elements = // total element based on is_uniform
            elements_per_scenario < 0 ? std::reduce(counter.cbegin(), counter.cend()) : // aggregation
                elements_per_scenario * batch_size_;                                    // multiply
        return Buffer{.component = &dataset_->get_component(component),
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

    Idx get_elements_per_scenario(IdxVector const& counter) {
        bool const is_uniform = check_uniform(counter);
        if (!is_uniform) {
            return -1;
        }
        if (batch_size_ == 0) {
            return 0;
        }
        return counter.front();
    }

    void parse_component(Buffer const& buffer) const {
        // handle indptr
        if (buffer.elements_per_scenario < 0) {
            // first always zero
            buffer.indptr.front() = 0;
            // accumulate sum
            std::transform_inclusive_scan(buffer.msg_data.cbegin(), buffer.msg_data.cend(), buffer.indptr.begin() + 1,
                                          std::plus{}, [](auto const& x) { return (Idx)x.size(); });
        }
        // set nan
        buffer.component->set_nan(buffer.data, 0, buffer.total_elements);
        // attributes
        auto const attributes = [&]() -> std::span<MetaAttribute const* const> {
            auto const found = attributes_.find(buffer.component->name);
            if (found == attributes_.cend()) {
                return {};
            }
            return found->second;
        }();
        // all scenarios
        for (Idx scenario = 0; scenario != batch_size_; ++scenario) {
            Idx const scenario_offset =
                buffer.elements_per_scenario < 0 ? buffer.indptr[scenario] : scenario * buffer.elements_per_scenario;
#ifndef NDEBUG
            if (buffer.elements_per_scenario < 0) {
                assert(buffer.indptr[scenario + 1] - buffer.indptr[scenario] == (Idx)buffer.msg_data[scenario].size());

            } else {
                assert(buffer.elements_per_scenario == (Idx)buffer.msg_data[scenario].size());
            }
#endif
            void* scenario_pointer = buffer.component->advance_ptr(buffer.data, scenario_offset);
            parse_scenario(*buffer.component, scenario_pointer, buffer.msg_data[scenario], attributes);
        }
    }

    void parse_scenario(MetaComponent const& component, void* scenario_pointer,
                        std::span<msgpack::object const> msg_data,
                        std::span<MetaAttribute const* const> attributes) const {
        for (Idx element = 0; element != (Idx)msg_data.size(); ++element) {
            void* element_pointer = component.advance_ptr(scenario_pointer, element);
            msgpack::object const& obj = msg_data[element];
            if (obj.type == msgpack::type::ARRAY) {
                parse_array_element(element_pointer, obj, attributes);
            } else if (obj.type == msgpack::type::MAP) {
                parse_map_element(element_pointer, obj, component);
            } else {
                throw SerializationError{"An element can only be a list or dictionary!\n"};
            }
        }
    }

    void parse_array_element(void* element_pointer, msgpack::object const& obj,
                             std::span<MetaAttribute const* const> attributes) const {
        std::span<msgpack::object const> arr{obj.via.array.ptr, obj.via.array.size};
        if (arr.size() != attributes.size()) {
            throw SerializationError{
                "An element of a list should have same length as the list of predefined attributes!\n"};
        }
        for (Idx i = 0; i != (Idx)attributes.size(); ++i) {
            parse_attribute(element_pointer, arr[i], *attributes[i]);
        }
    }

    void parse_map_element(void* element_pointer, msgpack::object const& obj, MetaComponent const& component) const {
        std::span<msgpack::object_kv const> map{obj.via.map.ptr, obj.via.map.size};
        for (msgpack::object_kv const& kv : map) {
            Idx const found_idx = component.find_attribute(kv.key.as<std::string_view>());
            if (found_idx < 0) {
                continue; // allow unknown key for additional user info
            }
            parse_attribute(element_pointer, kv.val, component.attributes[found_idx]);
        }
    }

    void parse_attribute(void* element_pointer, msgpack::object const& obj, MetaAttribute const& attribute) const {
        // skip for none
        if (obj.is_nil()) {
            return;
        }
        // call relevant parser
        switch (attribute.ctype) {
        case CType::c_double:
            return parse_attribute_per_type<double>(element_pointer, obj, attribute);
        case CType::c_double3:
            return parse_attribute_per_type<RealValue<false>>(element_pointer, obj, attribute);
        case CType::c_int8:
            return parse_attribute_per_type<int8_t>(element_pointer, obj, attribute);
        case CType::c_int32:
            return parse_attribute_per_type<int32_t>(element_pointer, obj, attribute);
        default:
            throw SerializationError{"Unknown data type for attriute!\n"};
        }
    }

    template <class T>
    void parse_attribute_per_type(void* element_pointer, msgpack::object const& obj,
                                  MetaAttribute const& attribute) const {
        T value;
        obj >> value;
        attribute.set_value(element_pointer, &value, 0);
    }
};

} // namespace power_grid_model::meta_data

#endif