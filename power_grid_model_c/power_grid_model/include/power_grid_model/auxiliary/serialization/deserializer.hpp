// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"

#include "../../common/common.hpp"
#include "../../common/exception.hpp"
#include "../../common/typing.hpp"
#include "../dataset.hpp"
#include "../meta_data.hpp"

#include <nlohmann/json.hpp>

#include <msgpack.hpp>

#include <set>
#include <span>
#include <sstream>
#include <stack>
#include <string_view>
#include <utility>

namespace power_grid_model::meta_data {

struct from_string_t {};
constexpr from_string_t from_string;

struct from_buffer_t {};
constexpr from_buffer_t from_buffer;

struct from_msgpack_t {};
constexpr from_msgpack_t from_msgpack;

struct from_json_t {};
constexpr from_json_t from_json;

namespace detail {

using nlohmann::json;

// visitor for json conversion
struct JsonMapArrayData {
    size_t size{};
    msgpack::sbuffer buffer;
};

struct JsonSAXVisitor {
    msgpack::packer<msgpack::sbuffer> top_packer() {
        if (data_buffers.empty()) {
            throw SerializationError{"Json root should be a map!\n"};
        }
        return {data_buffers.top().buffer};
    }

    template <class T> bool pack_data(T const& val) {
        top_packer().pack(val);
        ++data_buffers.top().size;
        return true;
    }

    bool null() {
        top_packer().pack_nil();
        ++data_buffers.top().size;
        return true;
    }
    bool boolean(bool val) { return pack_data(val); }
    bool number_integer(json::number_integer_t val) { return pack_data(val); }
    bool number_unsigned(json::number_unsigned_t val) { return pack_data(val); }
    bool number_float(json::number_float_t val, json::string_t const& /* s */) { return pack_data(val); }
    bool string(json::string_t const& val) {
        if (val == "inf" || val == "+inf") {
            return pack_data(std::numeric_limits<double>::infinity());
        }
        if (val == "-inf") {
            return pack_data(-std::numeric_limits<double>::infinity());
        }
        return pack_data(val);
    }
    bool key(json::string_t const& val) {
        top_packer().pack(val);
        return true;
    }
    static bool binary(json::binary_t const& /* val */) { return true; }

    bool start_object(size_t /* elements */) {
        data_buffers.emplace();
        return true;
    }
    bool end_object() {
        JsonMapArrayData const object_data{std::move(data_buffers.top())};
        data_buffers.pop();
        if (!std::in_range<uint32_t>(object_data.size)) {
            throw SerializationError{"Json map/array size exceeds the msgpack limit (2^32)!\n"};
        }
        if (data_buffers.empty()) {
            msgpack::packer<msgpack::sbuffer> root_packer{root_buffer};
            root_packer.pack_map(static_cast<uint32_t>(object_data.size));
            root_buffer.write(object_data.buffer.data(), object_data.buffer.size());
        } else {
            top_packer().pack_map(static_cast<uint32_t>(object_data.size));
            data_buffers.top().buffer.write(object_data.buffer.data(), object_data.buffer.size());
            ++data_buffers.top().size;
        }
        return true;
    }
    bool start_array(size_t /* elements */) {
        data_buffers.emplace();
        return true;
    }
    bool end_array() {
        JsonMapArrayData const array_data{std::move(data_buffers.top())};
        data_buffers.pop();
        if (!std::in_range<uint32_t>(array_data.size)) {
            throw SerializationError{"Json map/array size exceeds the msgpack limit (2^32)!\n"};
        }
        top_packer().pack_array(static_cast<uint32_t>(array_data.size));
        data_buffers.top().buffer.write(array_data.buffer.data(), array_data.buffer.size());
        ++data_buffers.top().size;
        return true;
    }

    [[noreturn]] static bool parse_error(std::size_t position, std::string const& last_token,
                                         json::exception const& ex) {
        std::stringstream ss;
        ss << "Parse error in JSON. Position: " << position << ", last token: " << last_token
           << ". Exception message: " << ex.what() << '\n';
        throw SerializationError{ss.str()};
    }

    std::stack<JsonMapArrayData> data_buffers;
    msgpack::sbuffer root_buffer;
};

// visitors for parsing
struct DefaultNullVisitor : msgpack::null_visitor {
    static std::string msg_for_parse_error(size_t parsed_offset, size_t error_offset, std::string_view msg) {
        std::stringstream ss;
        ss << msg << ", parsed_offset: " << parsed_offset << ", error_offset: " << error_offset << ".\n";
        return ss.str();
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[noreturn]] void parse_error(size_t parsed_offset, size_t error_offset) {
        throw SerializationError{msg_for_parse_error(parsed_offset, error_offset, "Error in parsing")};
    }
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[noreturn]] void insufficient_bytes(size_t parsed_offset, size_t error_offset) {
        throw SerializationError{msg_for_parse_error(parsed_offset, error_offset, "Insufficient bytes")};
    }
};

struct CheckHasMap : DefaultNullVisitor {
    bool has_map{false};
    bool start_map(uint32_t /*num_kv_pairs*/) {
        has_map = true;
        return true;
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

  private:
    DefaultErrorVisitor() = default;
    friend T; // CRTP compliance
};

struct visit_map_t;
struct visit_array_t;
struct visit_map_array_t;

template <class map_array>
    requires(std::same_as<map_array, visit_map_t> || std::same_as<map_array, visit_array_t> ||
             std::same_as<map_array, visit_map_array_t>)
struct MapArrayVisitor : DefaultErrorVisitor<MapArrayVisitor<map_array>> {
    static constexpr bool enable_map =
        std::same_as<map_array, visit_map_t> || std::same_as<map_array, visit_map_array_t>;
    static constexpr bool enable_array =
        std::same_as<map_array, visit_array_t> || std::same_as<map_array, visit_map_array_t>;
    static constexpr std::string_view static_err_msg = [] {
        if constexpr (enable_map && enable_array) {
            return "Map or an array expected.";
        } else if constexpr (enable_map) {
            return "Map expected.";
        } else {
            return "Array expected.";
        }
    }();

    Idx size{};
    bool is_map{};
    bool start_map(uint32_t num_kv_pairs) {
        if constexpr (!enable_map) {
            this->throw_error();
        }
        size = static_cast<Idx>(num_kv_pairs);
        is_map = true;
        return true;
    }
    bool start_map_key() { return false; }
    bool end_map() {
        assert(size == 0);
        return true;
    }
    bool start_array(uint32_t num_elements) {
        if constexpr (!enable_array) {
            this->throw_error();
        }
        size = static_cast<Idx>(num_elements);
        is_map = false;
        return true;
    }
    bool start_array_item() { return false; }
    bool end_array() {
        assert(size == 0);
        return true;
    }

    MapArrayVisitor() = default;
};

struct StringVisitor : DefaultErrorVisitor<StringVisitor> {
    static constexpr std::string_view static_err_msg = "String expected.";

    std::string_view str;
    bool visit_str(const char* v, uint32_t size) {
        str = {v, size};
        return true;
    }

    StringVisitor() = default;
};

struct BoolVisitor : DefaultErrorVisitor<BoolVisitor> {
    static constexpr std::string_view static_err_msg = "Boolean expected.";

    bool value{};
    bool visit_boolean(bool v) {
        value = v;
        return true;
    }

    BoolVisitor() = default;
};

template <class T> struct ValueVisitor;

template <std::integral T> struct ValueVisitor<T> : DefaultErrorVisitor<ValueVisitor<T>> {
    static constexpr std::string_view static_err_msg = "Integer expected.";

    T& value; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

    bool visit_nil() { return true; }
    bool visit_positive_integer(uint64_t v) {
        if (!std::in_range<T>(v)) {
            throw SerializationError{"Integer value overflows the data type!\n"};
        }
        value = static_cast<T>(v);
        return true;
    }
    bool visit_negative_integer(int64_t v) {
        if (!std::in_range<T>(v)) {
            throw SerializationError{"Integer value overflows the data type!\n"};
        }
        value = static_cast<T>(v);
        return true;
    }

    ValueVisitor(T& v) : DefaultErrorVisitor<ValueVisitor<T>>{}, value{v} {}
};

template <> struct ValueVisitor<double> : DefaultErrorVisitor<ValueVisitor<double>> {
    static constexpr std::string_view static_err_msg = "Number expected.";

    double& value; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)

    bool visit_nil() { return true; } // NOLINT(readability-convert-member-functions-to-static)
    bool visit_positive_integer(uint64_t v) {
        value = static_cast<double>(v);
        return true;
    }
    bool visit_negative_integer(int64_t v) {
        value = static_cast<double>(v);
        return true;
    }
    bool visit_float32(float v) {
        value = v;
        return true;
    }
    bool visit_float64(double v) {
        value = v;
        return true;
    }

    ValueVisitor(double& v) : DefaultErrorVisitor<ValueVisitor<double>>{}, value{v} {}
};

template <> struct ValueVisitor<RealValue<asymmetric_t>> : DefaultErrorVisitor<ValueVisitor<RealValue<asymmetric_t>>> {
    static constexpr std::string_view static_err_msg = "Array of 3 numbers expected.";

    RealValue<asymmetric_t>& value; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
    Idx idx{};
    bool inside_array{};

    bool visit_nil() { return true; } // NOLINT(readability-convert-member-functions-to-static)
    bool start_array(uint32_t num_elements) {
        if (inside_array || num_elements != 3) {
            this->throw_error();
        }
        inside_array = true;
        return true;
    }
    using msgpack::null_visitor::start_array_item;
    bool end_array_item() {
        ++idx;
        return true;
    }
    using msgpack::null_visitor::end_array;
    bool visit_positive_integer(uint64_t v) {
        if (!inside_array) {
            this->throw_error();
        }
        value[idx] = static_cast<double>(v);
        return true;
    }
    bool visit_negative_integer(int64_t v) {
        if (!inside_array) {
            this->throw_error();
        }
        value[idx] = static_cast<double>(v);
        return true;
    }
    bool visit_float32(float v) {
        if (!inside_array) {
            this->throw_error();
        }
        value[idx] = v;
        return true;
    }
    bool visit_float64(double v) {
        if (!inside_array) {
            this->throw_error();
        }
        value[idx] = v;
        return true;
    }

    ValueVisitor(RealValue<asymmetric_t>& v) : DefaultErrorVisitor<ValueVisitor<RealValue<asymmetric_t>>>{}, value{v} {}
};

} // namespace detail

class Deserializer {
    using DefaultNullVisitor = detail::DefaultNullVisitor;
    using CheckHasMap = detail::CheckHasMap;
    template <class map_array> using MapArrayVisitor = detail::MapArrayVisitor<map_array>;
    using StringVisitor = detail::StringVisitor;
    using BoolVisitor = detail::BoolVisitor;
    template <class T> using ValueVisitor = detail::ValueVisitor<T>;
    static constexpr bool move_forward = true;
    static constexpr bool stay_offset = false;
    using visit_map_t = detail::visit_map_t;
    using visit_array_t = detail::visit_array_t;
    using visit_map_array_t = detail::visit_map_array_t;
    using JsonSAXVisitor = detail::JsonSAXVisitor;

    struct ComponentByteMeta {
        std::string_view component;
        Idx size;
        size_t offset;
        bool has_map;
    };
    using DataByteMeta = std::vector<std::vector<ComponentByteMeta>>;
    using AttributeByteMeta = std::vector<std::pair<std::string_view, std::vector<std::string_view>>>;

    using Buffer = WritableDataset::Buffer;
    struct BufferView {
        Buffer const* buffer{nullptr};
        Idx idx{0};
        std::span<AttributeBuffer<void> const> reordered_attribute_buffers;
    };

    using row_based_t = detail::row_based_t;
    using columnar_t = detail::columnar_t;
    static constexpr auto row_based = detail::row_based;
    static constexpr auto columnar = detail::columnar;

  public:
    // not copyable
    Deserializer(Deserializer const&) = delete;
    Deserializer& operator=(Deserializer const&) = delete;
    // movable
    Deserializer(Deserializer&&) = default;
    Deserializer& operator=(Deserializer&&) noexcept = default;

    // destructor
    ~Deserializer() = default;

    Deserializer(from_string_t /* tag */, std::string_view data_string, SerializationFormat serialization_format,
                 MetaData const& meta_data)
        : Deserializer{create_from_format(data_string, serialization_format, meta_data)} {}

    Deserializer(from_buffer_t /* tag */, std::span<char const> data_buffer, SerializationFormat serialization_format,
                 MetaData const& meta_data)
        : Deserializer{create_from_format(data_buffer, serialization_format, meta_data)} {}

    Deserializer(from_json_t /* tag */, std::string_view json_string, MetaData const& meta_data)
        : meta_data_{&meta_data},
          buffer_from_json_{json_to_msgpack(json_string)},
          data_{buffer_from_json_.data()},
          size_{buffer_from_json_.size()},
          dataset_handler_{pre_parse()} {}

    Deserializer(from_msgpack_t /* tag */, std::span<char const> msgpack_data, MetaData const& meta_data)
        : meta_data_{&meta_data},
          data_{msgpack_data.data()},
          size_{msgpack_data.size()},
          dataset_handler_{pre_parse()} {}

    WritableDataset& get_dataset_info() { return dataset_handler_; }

    void parse() {
        root_key_ = "data";
        try {
            for (Idx i = 0; i != dataset_handler_.n_components(); ++i) {
                parse_component(i);
            }
        } catch (std::exception& e) {
            handle_error(e);
        }
        root_key_ = {};
    }

  private:
    // data members are order dependent
    // DO NOT modify the order!
    MetaData const* meta_data_;
    // own buffer if from json
    msgpack::sbuffer buffer_from_json_;
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
    bool is_batch_{};
    std::map<MetaComponent const*, std::vector<MetaAttribute const*>, std::less<>> attributes_;

    // offset of the msgpack bytes, the number of elements,
    //     for the actual data, per component (outer), per batch (inner)
    // if a component has no element for a certain scenario, that offset and size will be zero.
    std::vector<std::vector<ComponentByteMeta>> msg_data_offsets_;
    WritableDataset dataset_handler_;

    static msgpack::sbuffer json_to_msgpack(std::string_view json_string) {
        JsonSAXVisitor visitor{};
        nlohmann::json::sax_parse(json_string, &visitor);
        msgpack::sbuffer msgpack_data{std::move(visitor.root_buffer)};
        return msgpack_data;
    }

    template <class map_array, bool move_forward> MapArrayVisitor<map_array> parse_map_array() {
        MapArrayVisitor<map_array> visitor{};
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

    void parse_skip() {
        DefaultNullVisitor visitor{};
        msgpack::parse(data_, size_, offset_, visitor);
    }

    bool parse_skip_check_map() {
        CheckHasMap visitor{};
        msgpack::parse(data_, size_, offset_, visitor);
        return visitor.has_map;
    }

    WritableDataset pre_parse() {
        try {
            return pre_parse_impl();
        } catch (std::exception& e) {
            handle_error(e);
        }
    }

    WritableDataset pre_parse_impl() {
        std::string_view dataset;
        Idx batch_size{};
        Idx global_map_size = parse_map_array<visit_map_t, move_forward>().size;
        AttributeByteMeta attributes;
        DataByteMeta data_counts{};
        bool has_version{};
        bool has_type{};
        bool has_is_batch{};
        bool has_attributes{};
        bool has_data{};

        while (global_map_size-- != 0) {
            std::string_view const key = parse_string();
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
                bool const is_batch = parse_bool();
                if (has_data && (is_batch_ != is_batch)) {
                    throw SerializationError{"Map/Array type of data does not match is_batch!\n"};
                }
                is_batch_ = is_batch;
                has_is_batch = true;
            } else if (key == "attributes") {
                root_key_ = "attributes";
                has_attributes = true;
                attributes = read_predefined_attributes();
            } else if (key == "data") {
                root_key_ = "data";
                has_data = true;
                data_counts = pre_count_data(has_is_batch);
                batch_size = static_cast<Idx>(data_counts.size());
            } else {
                // null visitor to skip
                parse_skip();
            }
            root_key_ = {};
        }

        if (!has_version) {
            throw SerializationError{"Key version not found!\n"};
        }
        if (!has_type) {
            throw SerializationError{"Key type not found!\n"};
        }
        if (!has_is_batch) {
            throw SerializationError{"Key is_batch not found!\n"};
        }
        if (!has_attributes) {
            throw SerializationError{"Key attributes not found!\n"};
        }
        if (!has_data) {
            throw SerializationError{"Key data not found!\n"};
        }

        WritableDataset handler{is_batch_, batch_size, dataset, *meta_data_};
        count_data(handler, data_counts);
        parse_predefined_attributes(handler, attributes);
        return handler;
    }

    AttributeByteMeta read_predefined_attributes() {
        AttributeByteMeta attributes;
        Idx n_components = parse_map_array<visit_map_t, move_forward>().size;
        while (n_components-- != 0) {
            component_key_ = parse_string();
            attributes.push_back({component_key_, {}});
            auto& attributes_per_component = attributes.back().second;
            Idx const n_attributes_per_component = parse_map_array<visit_array_t, move_forward>().size;
            for (element_number_ = 0; element_number_ != n_attributes_per_component; ++element_number_) {
                attributes_per_component.push_back(parse_string());
            }
            element_number_ = -1;
        }
        component_key_ = {};
        return attributes;
    }

    void parse_predefined_attributes(WritableDataset& handler, AttributeByteMeta const& attributes) {
        root_key_ = "attributes";
        MetaDataset const& dataset = handler.dataset();
        for (auto const& single_component : attributes) {
            component_key_ = single_component.first;
            MetaComponent const* const component = &dataset.get_component(component_key_);
            std::vector<MetaAttribute const*> attributes_per_component;
            for (element_number_ = 0; element_number_ != static_cast<Idx>(single_component.second.size());
                 ++element_number_) {
                attributes_per_component.push_back(&component->get_attribute(single_component.second[element_number_]));
            }
            attributes_[component] = std::move(attributes_per_component);
            // set attribute indication if enabled
            if (handler.get_component_info(component_key_).has_attribute_indications) {
                handler.set_attribute_indications(component_key_, attributes_[component]);
            }
            element_number_ = -1;
        }
        component_key_ = {};
        root_key_ = {};
    }

    DataByteMeta pre_count_data(bool has_is_batch) {
        DataByteMeta data_counts{};
        auto const root_visitor = parse_map_array<visit_map_array_t, stay_offset>();
        Idx batch_size{};
        if (has_is_batch && (is_batch_ == root_visitor.is_map)) {
            throw SerializationError{"Map/Array type of data does not match is_batch!\n"};
        }
        is_batch_ = !root_visitor.is_map;
        if (root_visitor.is_map) {
            batch_size = 1;
        } else {
            batch_size = root_visitor.size;
            parse_map_array<visit_array_t, move_forward>();
        }
        for (scenario_number_ = 0; scenario_number_ != batch_size; ++scenario_number_) {
            data_counts.push_back(pre_count_scenario());
        }
        scenario_number_ = -1;
        return data_counts;
    }

    std::vector<ComponentByteMeta> pre_count_scenario() {
        std::vector<ComponentByteMeta> count_per_scenario;
        Idx n_components = parse_map_array<visit_map_t, move_forward>().size;
        while (n_components-- != 0) {
            component_key_ = parse_string();
            Idx const component_size = parse_map_array<visit_array_t, stay_offset>().size;
            size_t const scenario_offset = offset_;
            // skip all the real content but check if it has map
            bool const has_map = parse_skip_check_map();
            count_per_scenario.push_back(
                {.component = component_key_, .size = component_size, .offset = scenario_offset, .has_map = has_map});
        }
        component_key_ = {};
        return count_per_scenario;
    }

    void count_data(WritableDataset& handler, DataByteMeta const& data_counts) {
        root_key_ = "data";
        // get set of all components
        std::set<MetaComponent const*> all_components;
        for (scenario_number_ = 0; scenario_number_ != static_cast<Idx>(data_counts.size()); ++scenario_number_) {
            for (auto const& component_byte_meta : data_counts[scenario_number_]) {
                component_key_ = component_byte_meta.component;
                all_components.insert(&handler.dataset().get_component(component_key_));
            }
            component_key_ = {};
        }
        scenario_number_ = -1;

        // create buffer object
        for (MetaComponent const* const component : all_components) {
            count_component(handler, data_counts, *component);
        }
        root_key_ = {};
    }

    void count_component(WritableDataset& handler, DataByteMeta const& data_counts, MetaComponent const& component) {
        component_key_ = component.name;
        Idx const batch_size = handler.batch_size();
        // count number of element of all scenarios
        IdxVector counter(batch_size);
        std::vector<ComponentByteMeta> component_byte_meta(batch_size);
        for (scenario_number_ = 0; scenario_number_ != batch_size; ++scenario_number_) {
            auto const& scenario_counts = data_counts[scenario_number_];
            auto const found_component = std::ranges::find_if(
                scenario_counts, [&component](auto const& x) { return x.component == component.name; });
            if (found_component != scenario_counts.cend()) {
                counter[scenario_number_] = found_component->size;
                component_byte_meta[scenario_number_] = *found_component;
            }
        }
        scenario_number_ = -1;

        Idx const elements_per_scenario = get_uniform_elements_per_scenario(counter);
        Idx const total_elements = // total element based on is_uniform
            elements_per_scenario < 0 ? std::reduce(counter.cbegin(), counter.cend()) : // aggregation
                elements_per_scenario * batch_size;                                     // multiply
        handler.add_component_info(component_key_, elements_per_scenario, total_elements);
        // check if all scenarios only contain array data
        bool const only_values_in_data =
            std::ranges::none_of(component_byte_meta, [](auto const& x) { return x.has_map; });
        msg_data_offsets_.push_back(std::move(component_byte_meta));
        // enable attribute indications if possible
        if (only_values_in_data) {
            handler.enable_attribute_indications(component_key_);
        }
        component_key_ = {};
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

    void parse_component(Idx component_idx) {
        if (dataset_handler_.is_row_based(component_idx)) {
            parse_component(row_based, component_idx);
        } else if (dataset_handler_.is_columnar(component_idx, true)) {
            parse_component(columnar, component_idx);
        }
    }

    template <detail::row_based_or_columnar_c row_or_column_t>
    void parse_component(row_or_column_t row_or_column_tag, Idx component_idx) {
        auto const& buffer = dataset_handler_.get_buffer(component_idx);

        assert(dataset_handler_.is_row_based(buffer) == detail::is_row_based_v<row_or_column_t>);
        assert(dataset_handler_.is_columnar(buffer, true) == detail::is_columnar_v<row_or_column_t>);
        assert(is_row_based(buffer) == detail::is_row_based_v<row_or_column_t>);
        assert(is_columnar(buffer) == detail::is_columnar_v<row_or_column_t>);

        auto const& info = dataset_handler_.get_component_info(component_idx);
        auto const& msg_data = msg_data_offsets_[component_idx];

        Idx const batch_size = dataset_handler_.batch_size();
        component_key_ = info.component->name;

        // set nan
        set_nan(row_or_column_tag, buffer, info);

        // handle indptr
        if (info.elements_per_scenario < 0) {
            // first always zero
            buffer.indptr.front() = 0;
            // accumulate sum
            std::transform_inclusive_scan(
                msg_data.cbegin(), msg_data.cend(), buffer.indptr.begin() + 1, std::plus{},
                [](auto const& x) { return x.size; }, Idx{});
        }

        // attributes
        std::span<MetaAttribute const* const> const attributes = [this,
                                                                  &info]() -> std::span<MetaAttribute const* const> {
            if (auto const it = attributes_.find(info.component); it != attributes_.cend()) {
                return it->second;
            }
            return {};
        }();
        auto const reordered_attribute_buffers = detail::is_columnar_v<row_or_column_t>
                                                     ? detail::reordered_attribute_buffers(buffer, attributes)
                                                     : std::vector<AttributeBuffer<void>>{};
        // for columnar buffer
        // if there is no intersection between the pre-defined attributes and the user provided buffer
        // and the whole component does not have map
        // skip the whole component for all scenarios and all elements
        if constexpr (std::same_as<row_or_column_t, columnar_t>) {
            if (info.has_attribute_indications && reordered_attribute_buffers.empty()) {
                component_key_ = "";
                return;
            }
        }

        BufferView const buffer_view{
            .buffer = &buffer, .idx = 0, .reordered_attribute_buffers = reordered_attribute_buffers};

        // all scenarios
        for (scenario_number_ = 0; scenario_number_ != batch_size; ++scenario_number_) {
            Idx const scenario_offset = info.elements_per_scenario < 0 ? buffer_view.buffer->indptr[scenario_number_]
                                                                       : scenario_number_ * info.elements_per_scenario;
#ifndef NDEBUG
            if (info.elements_per_scenario < 0) {
                assert(buffer_view.buffer->indptr[scenario_number_ + 1] -
                           buffer_view.buffer->indptr[scenario_number_] ==
                       msg_data[scenario_number_].size);

            } else {
                assert(info.elements_per_scenario == msg_data[scenario_number_].size);
            }
#endif
            BufferView const scenario = advance(buffer_view, scenario_offset);
            parse_scenario(row_or_column_tag, *info.component, scenario, msg_data[scenario_number_], attributes);
        }
        scenario_number_ = -1;
        component_key_ = "";
    }

    void parse_scenario(detail::row_based_or_columnar_c auto row_or_column_tag, MetaComponent const& component,
                        BufferView const& buffer_view, ComponentByteMeta const& msg_data,
                        std::span<MetaAttribute const* const> attributes) {
        // skip for empty scenario
        if (msg_data.size == 0) {
            return;
        }

        // for columnar buffer
        // if there is no intersection between the pre-defined attributes and the usered provided buffer
        // and this scenario does not have map
        // skip the whole scenario for this compoment for all elements
        if constexpr (std::same_as<decltype(row_or_column_tag), columnar_t>) {
            if (buffer_view.reordered_attribute_buffers.empty() && !msg_data.has_map) {
                return;
            }
        }

        // set offset and skip array header
        offset_ = msg_data.offset;
        parse_map_array<visit_array_t, move_forward>();

        for (element_number_ = 0; element_number_ != msg_data.size; ++element_number_) {
            BufferView const element_buffer = advance(buffer_view, element_number_);
            // check the element is map or array
            parse_element(row_or_column_tag, element_buffer, component, attributes);
        }
        element_number_ = -1;
        offset_ = 0;
    }

    void parse_element(row_based_t tag, BufferView const& buffer_view, MetaComponent const& component,
                       std::span<MetaAttribute const* const> attributes) {
        assert(is_row_based(buffer_view));

        auto const element_visitor = parse_map_array<visit_map_array_t, move_forward>();
        if (element_visitor.is_map) {
            parse_map_element(tag, buffer_view, element_visitor.size, component);
        } else {
            parse_array_element(tag, buffer_view, element_visitor.size, component, attributes);
        }
    }

    void parse_element(columnar_t tag, BufferView const& buffer_view, MetaComponent const& component,
                       std::span<MetaAttribute const* const> attributes) {
        assert(is_columnar(buffer_view));

        auto const element_visitor = parse_map_array<visit_map_array_t, stay_offset>();
        if (element_visitor.is_map) {
            parse_map_array<visit_map_array_t, move_forward>();
            parse_map_element(tag, buffer_view, element_visitor.size, component);
        } else if (!buffer_view.reordered_attribute_buffers.empty()) {
            parse_map_array<visit_map_array_t, move_forward>();
            parse_array_element(tag, buffer_view, element_visitor.size, component, attributes);
        } else {
            parse_skip();
        }
    }

    void parse_map_element(row_based_t tag, BufferView const& buffer_view, Idx map_size,
                           MetaComponent const& component) {
        while (map_size-- != 0) {
            attribute_key_ = parse_string();
            Idx const component_attribute_idx = component.find_attribute(attribute_key_);
            if (component_attribute_idx >= 0) {
                parse_attribute(tag, buffer_view, component, component.attributes[component_attribute_idx]);
            } else {
                attribute_key_ = {}; // allow unknown key for additional user info
                parse_skip();
            }
        }
        attribute_key_ = "";
    }

    void parse_map_element(columnar_t /*tag*/, BufferView const& buffer_view, Idx map_size,
                           MetaComponent const& /*component*/) {
        assert(!buffer_view.buffer->attributes.empty());

        while (map_size-- != 0) {
            attribute_key_ = parse_string();
            if (auto it = std::ranges::find_if(buffer_view.buffer->attributes,
                                               [this](auto const& attribute_buffer) {
                                                   assert(attribute_buffer.meta_attribute != nullptr);
                                                   return attribute_buffer.meta_attribute->name == attribute_key_;
                                               });
                it != buffer_view.buffer->attributes.end()) {
                parse_attribute(*it, buffer_view.idx);
            } else {
                attribute_key_ = {}; // allow unknown key for additional user info
                parse_skip();
            }
        }
        attribute_key_ = "";
    }

    template <detail::row_based_or_columnar_c row_or_column_t>
    void parse_array_element(row_or_column_t row_or_column_tag, BufferView const& buffer_view, Idx array_size,
                             MetaComponent const& component, std::span<MetaAttribute const* const> attributes) {
        if (array_size != static_cast<Idx>(attributes.size())) {
            throw SerializationError{
                "An element of a list should have same length as the list of predefined attributes!\n"};
        }

        for (attribute_number_ = 0; attribute_number_ != array_size; ++attribute_number_) {
            if constexpr (detail::is_row_based_v<row_or_column_t>) {
                parse_attribute(row_or_column_tag, buffer_view, component, *attributes[attribute_number_]);
            } else {
                static_assert(detail::is_columnar_v<row_or_column_t>);
                if (auto const& attribute_buffer = buffer_view.reordered_attribute_buffers[attribute_number_];
                    attribute_buffer.data != nullptr) {
                    parse_attribute(attribute_buffer, buffer_view.idx);
                } else {
                    parse_skip();
                }
            }
        }
        attribute_number_ = -1;
    }

    void parse_attribute(row_based_t /*tag*/, BufferView const& buffer_view, MetaComponent const& component,
                         MetaAttribute const& attribute) { // call relevant parser
        assert(is_row_based(buffer_view));

        ctype_func_selector(attribute.ctype, [&buffer_view, &component, &attribute, this]<class T> {
            ValueVisitor<T> visitor{
                attribute.get_attribute<T>(component.advance_ptr(buffer_view.buffer->data, buffer_view.idx))};
            msgpack::parse(data_, size_, offset_, visitor);
        });
    }

    // parse a single attribute into an attribute buffer (a single column in a columnar buffer)
    void parse_attribute(AttributeBuffer<void> const& buffer, Idx idx) {
        assert(buffer.data != nullptr);
        assert(buffer.meta_attribute != nullptr);

        ctype_func_selector(buffer.meta_attribute->ctype, [&buffer, &idx, this]<class T> {
            ValueVisitor<T> visitor{*(reinterpret_cast<T*>(buffer.data) + idx)};
            msgpack::parse(data_, size_, offset_, visitor);
        });
    }

    static Deserializer create_from_format(std::string_view data_string, SerializationFormat serialization_format,
                                           MetaData const& meta_data) {
        switch (serialization_format) {
        case SerializationFormat::json:
            return {from_json, data_string, meta_data};
        case SerializationFormat::msgpack:
            [[fallthrough]];
        default: {
            using namespace std::string_literals;
            throw SerializationError("String data input not supported for serialization format "s +
                                     std::to_string(static_cast<IntS>(serialization_format)));
        }
        }
    }

    static Deserializer create_from_format(std::span<char const> buffer, SerializationFormat serialization_format,
                                           MetaData const& meta_data) {
        switch (serialization_format) {
        case SerializationFormat::json:
            return {from_json, std::string_view{buffer.data(), buffer.size()}, meta_data};
        case SerializationFormat::msgpack:
            return {from_msgpack, buffer, meta_data};
        default: {
            using namespace std::string_literals;
            throw SerializationError("Buffer data input not supported for serialization format "s +
                                     std::to_string(static_cast<IntS>(serialization_format)));
        }
        }
    }

    static void set_nan(row_based_t /*tag*/, Buffer const& buffer, ComponentInfo const& info) {
        assert(is_row_based(buffer));
        info.component->set_nan(buffer.data, 0, info.total_elements);
    }
    static void set_nan(columnar_t /*tag*/, Buffer const& buffer, ComponentInfo const& info) {
        assert(is_columnar(buffer));
        for (auto const& attribute_buffer : buffer.attributes) {
            if (attribute_buffer.meta_attribute != nullptr) {
                ctype_func_selector(attribute_buffer.meta_attribute->ctype, [&attribute_buffer, &info]<typename T> {
                    std::ranges::fill(std::span{reinterpret_cast<T*>(attribute_buffer.data),
                                                narrow_cast<size_t>(info.total_elements)},
                                      nan_value<T>);
                });
            }
        }
    }

    static constexpr BufferView advance(BufferView buffer_view, Idx offset) {
        buffer_view.idx += offset;
        return buffer_view;
    }
    static constexpr bool is_row_based(BufferView const& buffer_view) {
        assert(buffer_view.buffer != nullptr);
        return is_row_based(*buffer_view.buffer);
    }
    static constexpr bool is_row_based(WritableDataset::Buffer const& buffer) { return buffer.data != nullptr; }
    static constexpr bool is_columnar(BufferView const& buffer_view) {
        assert(buffer_view.buffer != nullptr);
        return is_columnar(*buffer_view.buffer);
    }
    static constexpr bool is_columnar(WritableDataset::Buffer const& buffer) {
        return buffer.data == nullptr && !buffer.attributes.empty();
    }

    [[noreturn]] void handle_error(std::exception const& e) {
        std::stringstream ss;
        ss << e.what();
        if (!root_key_.empty()) {
            ss << " Position of error: " << root_key_;
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
