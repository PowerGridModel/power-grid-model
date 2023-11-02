// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_SERIALIZTION_DESERIALIZER_HPP
#define POWER_GRID_MODEL_AUXILIARY_SERIALIZTION_DESERIALIZER_HPP

#include "../../exception.hpp"
#include "../../power_grid_model.hpp"
#include "../dataset_handler.hpp"
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
constexpr auto const& as_array(msgpack::object const& obj) { return obj.via.array; }
constexpr auto const& as_map(msgpack::object const& obj) { return obj.via.map; }
// NOLINTEND(cppcoreguidelines-pro-type-union-access)
} // namespace power_grid_model::meta_data

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {
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

struct from_string_t {};
constexpr from_string_t from_string;

struct from_buffer_t {};
constexpr from_buffer_t from_buffer;

struct from_msgpack_t {};
constexpr from_msgpack_t from_msgpack;

struct from_json_t {};
constexpr from_json_t from_json;

class Deserializer {
    static constexpr auto msgpack_string = msgpack::type::STR;
    static constexpr auto msgpack_bool = msgpack::type::BOOLEAN;
    static constexpr auto msgpack_array = msgpack::type::ARRAY;
    static constexpr auto msgpack_map = msgpack::type::MAP;

    // visitors for parsing
    struct DefaultNullVisitor : msgpack::null_visitor {
        static std::string msg_for_parse_error(size_t parsed_offset, size_t error_offset, std::string_view msg) {
            std::stringstream ss;
            ss << msg << ", parsed_offset: " << parsed_offset << ", error_offset: " << error_offset << '.\n';
            return ss.str();
        }

        void parse_error(size_t parsed_offset, size_t error_offset) {
            throw SerializationError{msg_for_parse_error(parsed_offset, error_offset, "Error in parsing")};
        }
        void insufficient_bytes(size_t parsed_offset, size_t error_offset) {
            throw SerializationError{msg_for_parse_error(parsed_offset, error_offset, "Insufficient bytes")};
        }
    };

    template <class T> struct DefaultErrorVisitor : DefaultNullVisitor {
        static constexpr std::string_view static_err_msg = "Unexpected data type!\n";

        bool visit_nil() { return throw_error(); }
        bool visit_boolean(bool /*v*/) { return throw_error(); }
        bool visit_positive_integer(uint64_t /*v*/) { return throw_error(); }
        bool visit_negative_integer(int64_t /*v*/) { return throw_error(); }
        bool visit_float32(float /*v*/) { return throw_error(); }
        bool visit_float64(double /*v*/) { return throw_error(); }
        bool visit_str(const char* /*v*/, uint32_t /*size*/) { return throw_error(); }
        bool visit_bin(const char* /*v*/, uint32_t /*size*/) { return throw_error(); }
        bool visit_ext(const char* /*v*/, uint32_t /*size*/) { return throw_error(); }
        bool start_array(uint32_t /*num_elements*/) { return throw_error(); }
        bool start_array_item() { return throw_error(); }
        bool end_array_item() { return throw_error(); }
        bool end_array() { return throw_error(); }
        bool start_map(uint32_t /*num_kv_pairs*/) { return throw_error(); }
        bool start_map_key() { return throw_error(); }
        bool end_map_key() { return throw_error(); }
        bool start_map_value() { return throw_error(); }
        bool end_map_value() { return throw_error(); }
        bool end_map() { return throw_error(); }

        bool throw_error() { throw SerializationError{(static_cast<T&>(*this)).get_err_msg()}; }

        std::string get_err_msg() { return std::string{T::static_err_msg}; }
    };

    template <bool enable_map, bool enable_array>
        requires(enable_map || enable_array)
    struct MapArrayVisitor : DefaultErrorVisitor<MapArrayVisitor<enable_map, enable_array>> {
        static constexpr std::string_view static_err_msg =
            enable_map ? (enable_array ? "Expect a map or array." : "Expect a map.") : "Expect an array.";

        Idx size{};
        bool is_map{};
        bool start_map(uint32_t num_kv_pairs)
            requires(enable_map)
        {
            size = static_cast<Idx>(num_kv_pairs);
            is_map = true;
            return true;
        }
        bool start_map_key()
            requires(enable_map)
        {
            return false;
        }
        bool end_map()
            requires(enable_map)
        {
            assert(size == 0);
            return true;
        }
        bool start_array(uint32_t num_elements)
            requires(enable_array)
        {
            size = static_cast<Idx>(num_elements);
            is_map = false;
            return true;
        }
        bool start_array_item()
            requires(enable_array)
        {
            return false;
        }
        bool end_array()
            requires(enable_array)
        {
            assert(size == 0);
            return true;
        }
    };

    struct StringVisitor : DefaultErrorVisitor<StringVisitor> {
        static constexpr std::string_view static_err_msg = "Expect a string.";

        std::string_view str{};
        bool visit_str(const char* v, uint32_t size) {
            str = {v, size};
            return true;
        }
    };

    struct BoolVisitor : DefaultErrorVisitor<BoolVisitor> {
        static constexpr std::string_view static_err_msg = "Expect a boolean.";

        bool value{};
        bool visit_boolean(bool v) {
            value = v;
            return true;
        }
    };

  public:
    using ArraySpan = std::span<msgpack::object const>;
    using MapSpan = std::span<msgpack::object_kv const>;

    // not copyable
    Deserializer(Deserializer const&) = delete;
    Deserializer& operator=(Deserializer const&) = delete;
    // movable
    Deserializer(Deserializer&&) = default;
    Deserializer& operator=(Deserializer&&) = default;

    // destructor
    ~Deserializer() = default;

    Deserializer(from_string_t /* tag */, std::string_view data_string, SerializationFormat serialization_format)
        : Deserializer{create_from_format(data_string, serialization_format)} {}

    Deserializer(from_buffer_t /* tag */, std::span<char const> data_buffer, SerializationFormat serialization_format)
        : Deserializer{create_from_format(data_buffer, serialization_format)} {}

    Deserializer(from_json_t /* tag */, std::string_view json_string)
        : buffer_from_json_{json_to_msgpack(json_string)},
          data_{buffer_from_json_.data()},
          size_{buffer_from_json_.size()},
          dataset_handler_{pre_parse()} {}

    Deserializer(from_msgpack_t /* tag */, std::span<char const> msgpack_data)
        : data_{msgpack_data.data()}, size_{msgpack_data.size()}, dataset_handler_{pre_parse()} {}

    WritableDatasetHandler& get_dataset_info() { return dataset_handler_; }

    void parse() {}

  private:
    // data members are order dependent
    // DO NOT modify the order!
    // own buffer if from json
    std::vector<char> buffer_from_json_;
    // pointer to buffers
    char const* data_;
    size_t size_;
    // global offset
    size_t offset_{};
    // attributes to track the movement of the position
    // for error report purpose
    std::string_view root_key_;
    std::string_view component_key_;
    std::string_view attribute_key_;
    Idx scenario_number_{-1};
    Idx element_number_{-1};
    Idx attribute_number_{-1};
    // class members
    std::string version_;
    WritableDatasetHandler dataset_handler_;
    std::map<std::string, std::vector<MetaAttribute const*>, std::less<>> attributes_;
    // vector of components of spans of msgpack object of each batch
    std::vector<std::vector<ArraySpan>> msg_views_;

    static std::vector<char> json_to_msgpack(std::string_view json_string) {
        nlohmann::json json_document = nlohmann::json::parse(json_string);
        json_convert_inf(json_document);
        std::vector<char> msgpack_data;
        nlohmann::json::to_msgpack(json_document, msgpack_data);
        return msgpack_data;
    }

    static void json_convert_inf(nlohmann::json& json_document) {
        switch (json_document.type()) {
        case nlohmann::json::value_t::object:
        case nlohmann::json::value_t::array:
            for (auto& value : json_document) {
                json_convert_inf(value);
            }
            break;
        case nlohmann::json::value_t::string:
            json_string_to_inf(json_document);
            break;
        default:
            break;
        }
    }

    static void json_string_to_inf(nlohmann::json& value) {
        std::string const str = value.get<std::string>();
        if (str == "inf" || str == "+inf") {
            value = std::numeric_limits<double>::infinity();
        }
        if (str == "-inf") {
            value = -std::numeric_limits<double>::infinity();
        }
    }

    static std::string_view key_to_string(msgpack::object_kv const& kv) {
        try {
            return kv.key.as<std::string_view>();
        } catch (msgpack::type_error&) {
            throw SerializationError{"Keys in the dictionary should always be a string!\n"};
        }
    }

    template <bool enable_map, bool enable_array, bool move_forward>
    MapArrayVisitor<enable_map, enable_array> parse_map_array() {
        MapArrayVisitor<enable_map, enable_array> visitor{};
        if constexpr (move_forward) {
            // move offset forward by giving a reference
            msgpack::parse(data_, size_, offset_, visitor);
        } else {
            // parse but without changing offset
            msgpack::parse(data_ + offset_, size_ - offset_, visitor);
        }
        return visitor;
    }

    std::string_view parse_string() {
        StringVisitor visitor{};
        msgpack::parse(data_, size_, offset_, visitor);
        return visitor.str;
    }

    bool parse_bool() {
        BoolVisitor visitor{};
        msgpack::parse(data_, size_, offset_, visitor);
        return visitor.value;
    }

    WritableDatasetHandler pre_parse() {
        try {
            return pre_parse_impl();
        } catch (std::exception& e) {
            handle_error(e);
        }
    }

    WritableDatasetHandler pre_parse_impl() {
        std::string_view dataset;
        bool is_batch{};
        Idx batch_size{};
        Idx global_map_size = parse_map_array<true, false, true>().size;
        bool has_data{}, has_version{}, has_attributes{}, has_is_batch{}, has_type{};

        while (global_map_size-- != 0) {
            std::string_view key = parse_string();
            if (key == "version") {
                root_key_ = "version";
                has_version = true;
                version_ = parse_string();
            } else if (key == "type") {
                root_key_ = "type";
                has_type = true;
                dataset = parse_string();
            } else if (key == "is_batch") {
                root_key_ = "is_batch";
                has_is_batch = true;
                is_batch = parse_bool();
            } else if (key == "attributes") {
                root_key_ = "attributes";
                has_attributes = true;
                // parse attributes
            } else if (key == "data") {
                root_key_ = "data";
                has_data = true;
                // parse data
            }
            root_key_ = {};
        }

        WritableDatasetHandler handler{is_batch, batch_size, dataset};
        return handler;
    }

    void post_serialization() {
        read_predefined_attributes();
        count_data();
        root_key_ = "";
    }

    void read_predefined_attributes() {
        for (auto const& kv : get_value_from_root("attributes", msgpack_map).as<MapSpan>()) {
            component_key_ = key_to_string(kv);
            MetaComponent const& component = dataset_handler_.dataset().get_component(component_key_);
            attributes_[component.name] = read_component_attributes(component, kv.val);
        }
        component_key_ = "";
    }

    std::vector<MetaAttribute const*> read_component_attributes(MetaComponent const& component,
                                                                msgpack::object const& attribute_list) {
        if (attribute_list.type != msgpack_array) {
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
        auto const& obj = get_data_handle(dataset_handler_.is_batch());
        // pointer to array (or single value) of msgpack objects to the data
        auto const batch_data =
            dataset_handler_.is_batch() ? ArraySpan{as_array(obj).ptr, as_array(obj).size} : ArraySpan{&obj, 1};

        // get set of all components
        std::set<MetaComponent const*> all_components;
        for (scenario_number_ = 0; scenario_number_ != static_cast<Idx>(batch_data.size()); ++scenario_number_) {
            msgpack::object const& scenario = batch_data[scenario_number_];
            if (scenario.type != msgpack_map) {
                throw SerializationError{"The data object of each scenario should be a dictionary!\n"};
            }
            for (msgpack::object_kv const& kv : scenario.as<MapSpan>()) {
                component_key_ = key_to_string(kv);
                all_components.insert(&dataset_handler_.dataset().get_component(component_key_));
            }
            component_key_ = "";
        }
        scenario_number_ = -1;

        // create buffer object
        for (MetaComponent const* const component : all_components) {
            count_component(batch_data, *component);
        }
    }

    void count_component(ArraySpan batch_data, MetaComponent const& component) {
        component_key_ = component.name;
        Idx const batch_size = dataset_handler_.batch_size();
        // count number of element of all scenarios
        IdxVector counter(batch_size);
        std::vector<ArraySpan> msg_data(batch_size);
        for (scenario_number_ = 0; scenario_number_ != batch_size; ++scenario_number_) {
            msgpack::object const& scenario = batch_data[scenario_number_];
            Idx const found_component_idx = find_key_from_map(scenario, component.name);
            if (found_component_idx >= 0) {
                msgpack::object const& element_array = as_map(scenario).ptr[found_component_idx].val;
                if (element_array.type != msgpack_array) {
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
                elements_per_scenario * batch_size;                                     // multiply
        dataset_handler_.add_component_info(component_key_, elements_per_scenario, total_elements);
        msg_views_.push_back(msg_data);
        component_key_ = "";
    }

    bool check_uniform(IdxVector const& counter) {
        if (dataset_handler_.batch_size() < 2) {
            return true;
        }
        return std::transform_reduce(counter.cbegin(), counter.cend() - 1, counter.cbegin() + 1, true,
                                     std::logical_and{}, std::equal_to{});
    }

    Idx get_uniform_elements_per_scenario(IdxVector const& counter) {
        if (!check_uniform(counter)) {
            return -1;
        }
        if (dataset_handler_.batch_size() == 0) {
            return 0;
        }
        return counter.front();
    }

    static Deserializer create_from_format(std::string_view data_string, SerializationFormat serialization_format) {
        switch (serialization_format) {
        case SerializationFormat::json:
            return {from_json, data_string};
        case SerializationFormat::msgpack:
            [[fallthrough]];
        default: {
            using namespace std::string_literals;
            throw SerializationError("String data input not supported for serialization format "s +
                                     std::to_string(static_cast<IntS>(serialization_format)));
        }
        }
    }

    static Deserializer create_from_format(std::span<char const> buffer, SerializationFormat serialization_format) {
        switch (serialization_format) {
        case SerializationFormat::json:
            return {from_json, std::string_view{buffer.data(), buffer.size()}};
        case SerializationFormat::msgpack:
            return {from_msgpack, buffer};
        default: {
            using namespace std::string_literals;
            throw SerializationError("Buffer data input not supported for serialization format "s +
                                     std::to_string(static_cast<IntS>(serialization_format)));
        }
        }
    }

    void handle_error(std::exception& e) {
        std::stringstream ss;
        ss << e.what();
        if (!root_key_.empty()) {
            ss << "Position of error: " << root_key_;
            root_key_ = "";
        }
        if (dataset_handler_.is_batch() && scenario_number_ >= 0) {
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
