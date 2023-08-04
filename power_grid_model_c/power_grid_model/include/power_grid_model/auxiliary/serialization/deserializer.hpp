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
#include <sstream>
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
                if (o.via.array.ptr[i].is_nil()) {
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
        try {
            post_serialization();
        } catch (std::exception& e) {
            handle_error(e);
        }
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
        root_key_ = "data";
        try {
            for (Buffer const& buffer : buffers_) {
                parse_component(buffer);
            }
        } catch (std::exception& e) {
            handle_error(e);
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
    // attributes to track the movement of the position
    // for error report purpose
    mutable std::string_view root_key_;
    mutable std::string_view component_key_;
    mutable std::string_view attribute_key_;
    mutable Idx scenario_number_{-1};
    mutable Idx element_number_{-1};
    mutable Idx attribute_number_{-1};

    static std::vector<char> json_to_msgpack(char const* json_string) {
        nlohmann::json const json_document = nlohmann::json::parse(json_string);
        std::vector<char> msgpack_data;
        nlohmann::json::to_msgpack(json_document, msgpack_data);
        return msgpack_data;
    }

    static std::string_view key_to_string(msgpack::object_kv const& kv) {
        try {
            return kv.key.as<std::string_view>();
        } catch (msgpack::type_error&) {
            throw SerializationError{"Keys in the dictionary should always be a string!\n"};
        }
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
        root_key_ = "";
    }

    msgpack::object const& get_value_from_root(std::string_view key, msgpack::type::object_type type) {
        msgpack::object const& root = handle_.get();
        root_key_ = key;
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
            if (key == key_to_string(kv_map[i])) {
                return i;
            }
        }
        return -1;
    }

    void read_predefined_attributes() {
        msgpack::object const& attribute_map = get_value_from_root("attributes", msgpack::type::MAP);
        for (auto const& kv : std::span{attribute_map.via.map.ptr, attribute_map.via.map.size}) {
            component_key_ = key_to_string(kv);
            MetaComponent const& component = dataset_->get_component(component_key_);
            msgpack::object const& attribute_list = kv.val;
            if (attribute_list.type != msgpack::type::ARRAY) {
                throw SerializationError{
                    "Each entry of attribute dictionary should be a list for the corresponding component!\n"};
            }
            std::span const list_span{attribute_list.via.array.ptr, attribute_list.via.array.size};
            for (element_number_ = 0; element_number_ != (Idx)list_span.size(); ++element_number_) {
                attributes_[component.name].push_back(
                    &component.get_attribute(list_span[element_number_].as<std::string_view>()));
            }
            element_number_ = -1;
        }
        component_key_ = "";
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
        std::set<MetaComponent const*> all_components;
        for (scenario_number_ = 0; scenario_number_ != (Idx)batch_data.size(); ++scenario_number_) {
            msgpack::object const& scenario = batch_data[scenario_number_];
            if (scenario.type != msgpack::type::MAP) {
                throw SerializationError{"The data object of each scenario should be a dictionary!\n"};
            }
            std::span<msgpack::object_kv> const scenario_map{scenario.via.map.ptr, scenario.via.map.size};
            for (msgpack::object_kv const& kv : scenario_map) {
                component_key_ = key_to_string(kv);
                all_components.insert(&dataset_->get_component(component_key_));
            }
            component_key_ = "";
        }
        scenario_number_ = -1;

        // create buffer object
        for (MetaComponent const* const component : all_components) {
            buffers_.push_back(count_component(batch_data, *component));
        }
    }

    Buffer count_component(std::span<msgpack::object const> batch_data, MetaComponent const& component) {
        component_key_ = component.name;
        // count number of element of all scenarios
        IdxVector counter(batch_size_);
        std::vector<std::span<msgpack::object const>> msg_data(batch_size_);
        for (scenario_number_ = 0; scenario_number_ != batch_size_; ++scenario_number_) {
            msgpack::object const& scenario = batch_data[scenario_number_];
            Idx const found_component_idx = find_key_from_map(scenario, component.name);
            if (found_component_idx >= 0) {
                msgpack::object const& element_array = scenario.via.map.ptr[found_component_idx].val;
                if (element_array.type != msgpack::type::ARRAY) {
                    throw SerializationError{"Each entry of component per scenario should be a list!\n"};
                }
                counter[scenario_number_] = (Idx)element_array.via.array.size;
                msg_data[scenario_number_] = {element_array.via.array.ptr, element_array.via.array.size};
            }
        }
        scenario_number_ = -1;

        Idx const elements_per_scenario = get_elements_per_scenario(counter);
        Idx const total_elements = // total element based on is_uniform
            elements_per_scenario < 0 ? std::reduce(counter.cbegin(), counter.cend()) : // aggregation
                elements_per_scenario * batch_size_;                                    // multiply
        component_key_ = "";
        return Buffer{.component = &component,
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
        component_key_ = buffer.component->name;
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
        for (scenario_number_ = 0; scenario_number_ != batch_size_; ++scenario_number_) {
            Idx const scenario_offset = buffer.elements_per_scenario < 0
                                            ? buffer.indptr[scenario_number_]
                                            : scenario_number_ * buffer.elements_per_scenario;
#ifndef NDEBUG
            if (buffer.elements_per_scenario < 0) {
                assert(buffer.indptr[scenario_number_ + 1] - buffer.indptr[scenario_number_] ==
                       (Idx)buffer.msg_data[scenario_number_].size());

            } else {
                assert(buffer.elements_per_scenario == (Idx)buffer.msg_data[scenario_number_].size());
            }
#endif
            void* scenario_pointer = buffer.component->advance_ptr(buffer.data, scenario_offset);
            parse_scenario(*buffer.component, scenario_pointer, buffer.msg_data[scenario_number_], attributes);
        }
        scenario_number_ = -1;
        component_key_ = "";
    }

    void parse_scenario(MetaComponent const& component, void* scenario_pointer,
                        std::span<msgpack::object const> msg_data,
                        std::span<MetaAttribute const* const> attributes) const {
        for (element_number_ = 0; element_number_ != (Idx)msg_data.size(); ++element_number_) {
            void* element_pointer = component.advance_ptr(scenario_pointer, element_number_);
            msgpack::object const& obj = msg_data[element_number_];
            if (obj.type == msgpack::type::ARRAY) {
                parse_array_element(element_pointer, obj, attributes);
            } else if (obj.type == msgpack::type::MAP) {
                parse_map_element(element_pointer, obj, component);
            } else {
                throw SerializationError{"An element can only be a list or dictionary!\n"};
            }
        }
        element_number_ = -1;
    }

    void parse_array_element(void* element_pointer, msgpack::object const& obj,
                             std::span<MetaAttribute const* const> attributes) const {
        std::span<msgpack::object const> arr{obj.via.array.ptr, obj.via.array.size};
        if (arr.size() != attributes.size()) {
            throw SerializationError{
                "An element of a list should have same length as the list of predefined attributes!\n"};
        }
        for (attribute_number_ = 0; attribute_number_ != (Idx)attributes.size(); ++attribute_number_) {
            parse_attribute(element_pointer, arr[attribute_number_], *attributes[attribute_number_]);
        }
        attribute_number_ = -1;
    }

    void parse_map_element(void* element_pointer, msgpack::object const& obj, MetaComponent const& component) const {
        std::span<msgpack::object_kv const> map{obj.via.map.ptr, obj.via.map.size};
        for (msgpack::object_kv const& kv : map) {
            attribute_key_ = key_to_string(kv);
            Idx const found_idx = component.find_attribute(attribute_key_);
            if (found_idx < 0) {
                attribute_key_ = "";
                continue; // allow unknown key for additional user info
            }
            parse_attribute(element_pointer, kv.val, component.attributes[found_idx]);
        }
        attribute_key_ = "";
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
        obj >> *reinterpret_cast<T*>(reinterpret_cast<char*>(element_pointer) + attribute.offset);
    }

    void handle_error(std::exception& e) const {
        std::stringstream ss;
        ss << e.what();
        if (!root_key_.empty()) {
            ss << "Position of error: " << root_key_;
        }
        if (is_batch_ && scenario_number_ >= 0) {
            ss << "/" << scenario_number_;
        }
        if (!component_key_.empty()) {
            ss << "/" << component_key_;
        }
        if (element_number_ >= 0) {
            ss << "/" << element_number_;
        }
        if (!attribute_key_.empty()) {
            ss << "/" << attribute_key_;
        }
        if (attribute_number_ >= 0) {
            ss << "/" << attribute_number_;
        }
        ss << '\n';
        throw SerializationError{ss.str()};
    }
};

} // namespace power_grid_model::meta_data

#endif