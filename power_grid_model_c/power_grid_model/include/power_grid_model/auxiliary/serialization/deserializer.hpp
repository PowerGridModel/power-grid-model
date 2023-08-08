// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_SERIALIZTION_DESERIALIZER_HPP
#define POWER_GRID_MODEL_AUXILIARY_SERIALIZTION_DESERIALIZER_HPP

#include "../../exception.hpp"
#include "../../power_grid_model.hpp"
#include "../meta_data.hpp"
#include "../meta_data_gen.hpp"

#include <nlohmann/json.hpp>

#include <msgpack.hpp>

#include <set>
#include <span>
#include <sstream>
#include <string_view>

// as array and map
namespace power_grid_model::meta_data {
// NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
inline auto const& as_array(msgpack::object const& obj) { return obj.via.array; }
inline auto const& as_map(msgpack::object const& obj) { return obj.via.map; }
// NOLINTEND(cppcoreguidelines-pro-type-union-access)
} // namespace power_grid_model::meta_data

// converter for double[3]
namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <> struct convert<power_grid_model::RealValue<false>> {
        msgpack::object const& operator()(msgpack::object const& o, power_grid_model::RealValue<false>& v) const {
            using power_grid_model::meta_data::as_array;

            if (o.type != msgpack::type::ARRAY) {
                throw msgpack::type_error();
            }
            if (as_array(o).size != 3) {
                throw msgpack::type_error();
            }
            for (int8_t i = 0; i != 3; ++i) {
                if (as_array(o).ptr[i].is_nil()) {
                    continue;
                }
                as_array(o).ptr[i] >> v(i);
            }
            return o;
        }
    };

    template <class T>
        requires(std::same_as<T, msgpack::object> || std::same_as<T, msgpack::object_kv>)
    struct convert<std::span<const T>> {
        msgpack::object const& operator()(msgpack::object const& o, std::span<const T>& span) const {
            using power_grid_model::meta_data::as_array;
            using power_grid_model::meta_data::as_map;

            if constexpr (std::same_as<T, msgpack::object>) {
                span = {as_array(o).ptr, as_array(o).size};
            } else {
                span = {as_map(o).ptr, as_map(o).size};
            }
            return o;
        }
    };

    } // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack

namespace power_grid_model::meta_data {

struct from_msgpack_t {};
constexpr from_msgpack_t from_msgpack;

struct from_json_t {};
constexpr from_json_t from_json;

class Deserializer {
  public:
    using ArraySpan = std::span<msgpack::object const>;
    using MapSpan = std::span<msgpack::object_kv const>;

    // struct of buffer data
    struct Buffer {
        MetaComponent const* component;
        // for non-uniform component, this is -1, we use indptr to describe the elements per scenario
        Idx elements_per_scenario;
        Idx total_elements;
        std::vector<ArraySpan> msg_data; // vector of spans of msgpack object of each batch
        void* data;                      // set by user
        std::span<Idx> indptr;           // set by user
    };

    // not copyable
    Deserializer(Deserializer const&) = delete;
    Deserializer& operator=(Deserializer const&) = delete;
    // movable
    Deserializer(Deserializer&&) = default;
    Deserializer& operator=(Deserializer&&) = default;
    // destructor
    ~Deserializer() = default;

    Deserializer(from_json_t, std::string_view json_string)
        : Deserializer{from_msgpack, json_to_msgpack(json_string)} {}

    Deserializer(from_msgpack_t, std::span<char const> msgpack_data) {
        handle_ = msgpack::unpack(msgpack_data.data(), msgpack_data.size());
        try {
            post_serialization();
        } catch (std::exception& e) {
            handle_error(e);
        }
    }

    std::string const& dataset_name() const { return dataset_->name; }

    bool is_batch() const { return is_batch_; }

    Idx batch_size() const { return batch_size_; }

    Idx n_components() const { return static_cast<Idx>(buffers_.size()); }

    Buffer const& get_buffer_info(Idx i) const { return buffers_[i]; }

    void set_buffer(char const** components, void** data, Idx** indptrs) {
        for (Idx i = 0; i != n_components(); ++i) {
            auto const found =
                std::find_if(buffers_.begin(), buffers_.end(), [component = components[i]](Buffer const& buffer) {
                    return buffer.component->name == component;
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

    void parse() {
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
    std::map<std::string, std::vector<MetaAttribute const*>, std::less<>> attributes_;
    Idx batch_size_{}; // for single dataset, the batch size is one
    std::vector<Buffer> buffers_;
    // attributes to track the movement of the position
    // for error report purpose
    std::string_view root_key_;
    std::string_view component_key_;
    std::string_view attribute_key_;
    Idx scenario_number_{-1};
    Idx element_number_{-1};
    Idx attribute_number_{-1};

    static std::vector<char> json_to_msgpack(std::string_view json_string) {
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

    static msgpack::object const& get_value_from_map(msgpack::object const& map, std::string_view key,
                                                     msgpack::type::object_type type) {
        Idx const idx = find_key_from_map(map, key);
        if (idx < 0) {
            throw SerializationError{"Cannot find key " + std::string(key) + " in the root level dictionary!\n"};
        }
        msgpack::object const& obj = as_map(map).ptr[idx].val;
        if (obj.type != type) {
            throw SerializationError{"Wrong data type for key " + std::string(key) +
                                     " in the root level dictionary!\n"};
        }
        return obj;
    }

    static Idx find_key_from_map(msgpack::object const& map, std::string_view key) {
        auto const kv_map = map.as<MapSpan>();
        auto const found =
            std::find_if(kv_map.begin(), kv_map.end(), [key](auto const& x) { return key_to_string(x) == key; });
        if (found == kv_map.end()) {
            return -1;
        }
        return std::distance(kv_map.begin(), found);
    }

    void read_predefined_attributes() {
        for (auto const& kv : get_value_from_root("attributes", msgpack::type::MAP).as<MapSpan>()) {
            component_key_ = key_to_string(kv);
            MetaComponent const& component = dataset_->get_component(component_key_);
            attributes_[component.name] = read_component_attributes(component, kv.val);
        }
        component_key_ = "";
    }

    std::vector<MetaAttribute const*> read_component_attributes(MetaComponent const& component,
                                                                msgpack::object const& attribute_list) {
        if (attribute_list.type != msgpack::type::ARRAY) {
            throw SerializationError{
                "Each entry of attribute dictionary should be a list for the corresponding component!\n"};
        }
        auto const attributes_span = attribute_list.as<ArraySpan>();
        std::vector<MetaAttribute const*> attributes(attributes_span.size());
        for (element_number_ = 0; element_number_ != static_cast<Idx>(attributes_span.size()); ++element_number_) {
            attributes[element_number_] =
                &component.get_attribute(attributes_span[element_number_].as<std::string_view>());
        }
        element_number_ = -1;
        return attributes;
    }

    void count_data() {
        msgpack::object const& obj = get_value_from_root("data", is_batch_ ? msgpack::type::ARRAY : msgpack::type::MAP);
        buffers_ = {};
        // pointer to array (or single value) of msgpack objects to the data
        ArraySpan batch_data;
        if (is_batch_) {
            batch_size_ = static_cast<Idx>(as_array(obj).size);
            batch_data = {as_array(obj).ptr, as_array(obj).size};
        } else {
            batch_size_ = 1;
            batch_data = {&obj, 1};
        }

        // get set of all components
        std::set<MetaComponent const*> all_components;
        for (scenario_number_ = 0; scenario_number_ != static_cast<Idx>(batch_data.size()); ++scenario_number_) {
            msgpack::object const& scenario = batch_data[scenario_number_];
            if (scenario.type != msgpack::type::MAP) {
                throw SerializationError{"The data object of each scenario should be a dictionary!\n"};
            }
            for (msgpack::object_kv const& kv : scenario.as<MapSpan>()) {
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

    Buffer count_component(ArraySpan batch_data, MetaComponent const& component) {
        component_key_ = component.name;
        // count number of element of all scenarios
        IdxVector counter(batch_size_);
        std::vector<ArraySpan> msg_data(batch_size_);
        for (scenario_number_ = 0; scenario_number_ != batch_size_; ++scenario_number_) {
            msgpack::object const& scenario = batch_data[scenario_number_];
            Idx const found_component_idx = find_key_from_map(scenario, component.name);
            if (found_component_idx >= 0) {
                msgpack::object const& element_array = as_map(scenario).ptr[found_component_idx].val;
                if (element_array.type != msgpack::type::ARRAY) {
                    throw SerializationError{"Each entry of component per scenario should be a list!\n"};
                }
                counter[scenario_number_] = static_cast<Idx>(as_array(element_array).size);
                element_array >> msg_data[scenario_number_];
            }
        }
        scenario_number_ = -1;

        Idx const elements_per_scenario = get_uniform_elements_per_scenario(counter);
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

    Idx get_uniform_elements_per_scenario(IdxVector const& counter) {
        if (!check_uniform(counter)) {
            return -1;
        }
        if (batch_size_ == 0) {
            return 0;
        }
        return counter.front();
    }

    void parse_component(Buffer const& buffer) {
        component_key_ = buffer.component->name;
        // handle indptr
        if (buffer.elements_per_scenario < 0) {
            // first always zero
            buffer.indptr.front() = 0;
            // accumulate sum
            // TODO (TonyXiang8787) Apple Clang cannot compile transform_inclusive_scan correctly
            // So we disable the good code and write the loop manually
            // std::transform_inclusive_scan(buffer.msg_data.cbegin(), buffer.msg_data.cend(),
            //                               buffer.indptr.begin() + 1,
            //                               std::plus{}, [](auto const& x) { return static_cast<Idx>(x.size()); });
            for (Idx i = 0; i != batch_size_; ++i) {
                buffer.indptr[i + 1] = buffer.indptr[i] + static_cast<Idx>(buffer.msg_data[i].size());
            }
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
                       static_cast<Idx>(buffer.msg_data[scenario_number_].size()));

            } else {
                assert(buffer.elements_per_scenario == static_cast<Idx>(buffer.msg_data[scenario_number_].size()));
            }
#endif
            void* scenario_pointer = buffer.component->advance_ptr(buffer.data, scenario_offset);
            parse_scenario(*buffer.component, scenario_pointer, buffer.msg_data[scenario_number_], attributes);
        }
        scenario_number_ = -1;
        component_key_ = "";
    }

    void parse_scenario(MetaComponent const& component, void* scenario_pointer, ArraySpan msg_data,
                        std::span<MetaAttribute const* const> attributes) {
        for (element_number_ = 0; element_number_ != static_cast<Idx>(msg_data.size()); ++element_number_) {
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
                             std::span<MetaAttribute const* const> attributes) {
        auto const arr = obj.as<ArraySpan>();
        if (arr.size() != attributes.size()) {
            throw SerializationError{
                "An element of a list should have same length as the list of predefined attributes!\n"};
        }
        for (attribute_number_ = 0; attribute_number_ != static_cast<Idx>(attributes.size()); ++attribute_number_) {
            parse_attribute(element_pointer, arr[attribute_number_], *attributes[attribute_number_]);
        }
        attribute_number_ = -1;
    }

    void parse_map_element(void* element_pointer, msgpack::object const& obj, MetaComponent const& component) {
        for (msgpack::object_kv const& kv : obj.as<MapSpan>()) {
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
        obj >> attribute.get_attribute<T>(element_pointer);
    }

    void handle_error(std::exception& e) {
        std::stringstream ss;
        ss << e.what();
        if (!root_key_.empty()) {
            ss << "Position of error: " << root_key_;
            root_key_ = "";
        }
        if (is_batch_ && scenario_number_ >= 0) {
            ss << "/" << scenario_number_;
            scenario_number_ = -1;
        }
        if (!component_key_.empty()) {
            ss << "/" << component_key_;
            component_key_ = "";
        }
        if (element_number_ >= 0) {
            ss << "/" << element_number_;
            element_number_ = -1;
        }
        if (!attribute_key_.empty()) {
            ss << "/" << attribute_key_;
            attribute_key_ = "";
        }
        if (attribute_number_ >= 0) {
            ss << "/" << attribute_number_;
            attribute_number_ = -1;
        }
        ss << '\n';
        throw SerializationError{ss.str()};
    }
};

} // namespace power_grid_model::meta_data

#endif
