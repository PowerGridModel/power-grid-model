// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../../common/common.hpp"
#include "../../common/exception.hpp"
#include "../dataset.hpp"
#include "../meta_data.hpp"

#include <nlohmann/json.hpp>

#include <msgpack.hpp>

#include <iomanip>
#include <limits>
#include <span>
#include <sstream>
#include <stack>
#include <string_view>

// custom packers
namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    // pack name of component and attribute
    template <class T>
        requires(std::same_as<T, power_grid_model::meta_data::MetaComponent> ||
                 std::same_as<T, power_grid_model::meta_data::MetaAttribute>)
    struct pack<T const*> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& p, T const* const& o) const {
            p.pack(o->name);
            return p;
        }
    };

    // pack double[3]
    template <> struct pack<power_grid_model::RealValue<power_grid_model::asymmetric_t>> {
        template <typename Stream>
        msgpack::packer<Stream>&
        operator()(msgpack::packer<Stream>& p,
                   power_grid_model::RealValue<power_grid_model::asymmetric_t> const& o) const {
            p.pack_array(3);
            for (int8_t i = 0; i != 3; ++i) {
                if (power_grid_model::is_nan(o(i))) {
                    p.pack_nil();
                } else {
                    p.pack(o(i));
                }
            }
            return p;
        }
    };

    } // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack

namespace power_grid_model::meta_data {

namespace json_converter {

struct MapArray {
    MapArray(uint32_t size_input) : size{size_input}, empty{size_input == 0} {}

    uint32_t size;
    bool empty;
    bool begin{true};
};

struct JsonConverter : msgpack::null_visitor {
    static constexpr char sep_char = ' ';

    Idx indent;
    Idx max_indent_level;
    std::stringstream ss{};
    std::stack<MapArray> map_array{};

    void print_indent() {
        if (indent < 0) {
            return;
        }
        Idx const indent_level = static_cast<Idx>(map_array.size());
        if (indent_level > max_indent_level) {
            if (map_array.top().begin) {
                map_array.top().begin = false;
                return;
            }
            ss << sep_char;
            return;
        }
        ss << '\n';
        ss << std::string(indent_level * indent, sep_char);
    }

    void print_key_val_sep() {
        if (indent < 0) {
            return;
        }
        ss << sep_char;
    }

    bool visit_nil() {
        ss << "null";
        return true;
    }
    bool visit_boolean(bool v) {
        if (v) {
            ss << "true";
        } else {
            ss << "false";
        }
        return true;
    }
    bool visit_positive_integer(uint64_t v) {
        ss << v;
        return true;
    }
    bool visit_negative_integer(int64_t v) {
        ss << v;
        return true;
    }
    bool visit_float32(float v) { return visit_float64(v); }
    bool visit_float64(double v) {
        if (std::isinf(v)) {
            using namespace std::string_view_literals;
            ss << '"' << (v > 0.0 ? "inf"sv : "-inf"sv) << '"';
        } else {
            ss << std::setprecision(std::numeric_limits<double>::digits10 + 1) << v;
        }
        return true;
    }
    bool visit_str(const char* v, uint32_t size) {
        ss << '"' << std::string_view{v, size} << '"';
        return true;
    }
    bool start_array(uint32_t num_elements) {
        map_array.emplace(num_elements);
        ss << '[';
        return true;
    }
    bool start_array_item() {
        print_indent();
        return true;
    }
    bool end_array_item() {
        --map_array.top().size;
        if (map_array.top().size > 0) {
            ss << ',';
        }
        return true;
    }
    bool end_array() {
        bool const empty = map_array.top().empty;
        map_array.pop();
        if (static_cast<Idx>(map_array.size()) < max_indent_level && !empty) {
            print_indent();
        }
        ss << ']';
        return true;
    }
    bool start_map(uint32_t num_kv_pairs) {
        map_array.emplace(num_kv_pairs);
        ss << '{';
        return true;
    }
    bool start_map_key() {
        print_indent();
        return true;
    }
    bool end_map_key() {
        ss << ':';
        print_key_val_sep();
        return true;
    }
    bool end_map_value() {
        --map_array.top().size;
        if (map_array.top().size > 0) {
            ss << ',';
        }
        return true;
    }
    bool end_map() {
        bool const empty = map_array.top().empty;
        map_array.pop();
        if (static_cast<Idx>(map_array.size()) < max_indent_level && !empty) {
            print_indent();
        }
        ss << '}';
        return true;
    }
};

} // namespace json_converter

class Serializer {
    using RawElementPtr = void const*;

    struct ComponentBuffer {
        MetaComponent const* component;
        RawElementPtr data;
        Idx size;
    };

    struct ScenarioBuffer {
        std::vector<ComponentBuffer> component_buffers;
    };

  public:
    static constexpr std::string_view version = "1.0";
    // top dict: version, type, is_batch, attributes, data
    static constexpr size_t size_top_dict = 5;

    // not copyable
    Serializer(Serializer const&) = delete;
    Serializer& operator=(Serializer const&) = delete;
    // not movable
    Serializer(Serializer&&) = delete;
    Serializer& operator=(Serializer&&) = delete;
    // destructor
    ~Serializer() = default;

    Serializer(ConstDataset dataset_handler, SerializationFormat serialization_format)
        : serialization_format_{serialization_format},
          dataset_handler_{std::move(dataset_handler)},
          packer_{msgpack_buffer_} {
        switch (serialization_format_) {
        case SerializationFormat::json:
            [[fallthrough]];
        case SerializationFormat::msgpack:
            break;
        default: {
            using namespace std::string_literals;
            throw SerializationError("Unsupported serialization format: "s +
                                     std::to_string(static_cast<IntS>(serialization_format_)));
        }
        }

        store_buffers();
    }

    std::span<char const> get_binary_buffer(bool use_compact_list) {
        switch (serialization_format_) {
        case SerializationFormat::json:
            return get_json(use_compact_list, -1);
        case SerializationFormat::msgpack:
            return get_msgpack(use_compact_list);
        default: {
            using namespace std::string_literals;
            throw SerializationError("Serialization format "s +
                                     std::to_string(static_cast<IntS>(serialization_format_)) +
                                     " does not support binary buffer output"s);
        }
        }
    }

    std::string const& get_string(bool use_compact_list, Idx indent) {
        switch (serialization_format_) {
        case SerializationFormat::json:
            return get_json(use_compact_list, indent);
        case SerializationFormat::msgpack:
            [[fallthrough]];
        default: {
            using namespace std::string_literals;
            throw SerializationError("Serialization format "s +
                                     std::to_string(static_cast<IntS>(serialization_format_)) +
                                     " does not support string output"s);
        }
        }
    }

  private:
    SerializationFormat serialization_format_{};

    ConstDataset dataset_handler_;
    std::vector<ScenarioBuffer> scenario_buffers_;   // list of scenarios, then list of components, omit empty
    std::vector<ComponentBuffer> component_buffers_; // list of components, then all scenario flatten

    // msgpack pakcer
    msgpack::sbuffer msgpack_buffer_{};
    msgpack::packer<msgpack::sbuffer> packer_;
    bool use_compact_list_{};
    std::map<MetaComponent const*, std::vector<MetaAttribute const*>> attributes_;

    // json
    Idx json_indent_{-1};
    std::string json_buffer_;

    void store_buffers() {
        scenario_buffers_.resize(dataset_handler_.batch_size());
        for (Idx scenario = 0; scenario != dataset_handler_.batch_size(); ++scenario) {
            scenario_buffers_[scenario] = create_scenario_buffer_view(scenario);
        }
        component_buffers_ = create_scenario_buffer_view().component_buffers;
    }

    ScenarioBuffer create_scenario_buffer_view(Idx scenario = -1) const {
        ScenarioBuffer scenario_buffer{};
        Idx const begin_scenario = scenario < 0 ? 0 : scenario;
        Idx const end_scenario = scenario < 0 ? dataset_handler_.batch_size() : begin_scenario + 1;
        for (Idx component = 0; component != dataset_handler_.n_components(); ++component) {
            ComponentBuffer component_buffer{};
            ComponentInfo const& info = dataset_handler_.get_component_info(component);
            ConstDataset::Buffer const& buffer = dataset_handler_.get_buffer(component);
            component_buffer.component = info.component;
            if (info.elements_per_scenario < 0) {
                component_buffer.data =
                    component_buffer.component->advance_ptr(buffer.data, buffer.indptr[begin_scenario]);
                component_buffer.size = buffer.indptr[end_scenario] - buffer.indptr[begin_scenario];
            } else {
                component_buffer.data =
                    component_buffer.component->advance_ptr(buffer.data, info.elements_per_scenario * begin_scenario);
                component_buffer.size = info.elements_per_scenario * (end_scenario - begin_scenario);
            }
            // only store the view if it is non-empty
            if (component_buffer.size > 0) {
                scenario_buffer.component_buffers.push_back(component_buffer);
            }
        }
        return scenario_buffer;
    }

    void check_attributes() {
        attributes_ = {};
        for (auto const& buffer : component_buffers_) {
            std::vector<MetaAttribute const*> attributes;
            for (auto const& attribute : buffer.component->attributes) {
                // if not all the values of an attribute are nan
                // add this attribute to the list
                if (!attribute.check_all_nan(buffer.data, buffer.size)) {
                    attributes.push_back(&attribute);
                }
            }
            attributes_[buffer.component] = attributes;
        }
    }

    std::span<char const> get_msgpack(bool use_compact_list) {
        if ((msgpack_buffer_.size() == 0) || (use_compact_list_ != use_compact_list)) {
            serialize(use_compact_list);
        }
        return {msgpack_buffer_.data(), msgpack_buffer_.size()};
    }

    std::string const& get_json(bool use_compact_list, Idx indent) {
        if (json_buffer_.empty() || (use_compact_list_ != use_compact_list) || (json_indent_ != indent)) {
            Idx const max_indent_level = dataset_handler_.is_batch() ? 4 : 3;
            json_converter::JsonConverter visitor{{}, indent, max_indent_level};
            auto const msgpack_data = get_msgpack(use_compact_list);
            msgpack::parse(msgpack_data.data(), msgpack_data.size(), visitor);
            json_buffer_ = visitor.ss.str();
        }
        return json_buffer_;
    }

    static void json_convert_inf(nlohmann::json& json_document) {
        switch (json_document.type()) {
        case nlohmann::json::value_t::object:
            [[fallthrough]];
        case nlohmann::json::value_t::array:
            for (auto& value : json_document) {
                json_convert_inf(value);
            }
            break;
        case nlohmann::json::value_t::number_float:
            json_inf_to_string(json_document);
            break;
        default:
            break;
        }
    }

    static void json_inf_to_string(nlohmann::json& value) {
        double const v = value.get<double>();
        if (std::isinf(v)) {
            value = v > 0.0 ? "inf" : "-inf";
        }
    }

    void serialize(bool use_compact_list) {
        msgpack_buffer_.clear();
        use_compact_list_ = use_compact_list;
        if (use_compact_list_) {
            check_attributes();
        } else {
            attributes_ = {};
        }
        pack_root_dict();
        pack_attributes();
        pack_data();
    }

    void pack_root_dict() {
        pack_map(size_top_dict);

        packer_.pack("version");
        packer_.pack(version);

        packer_.pack("type");
        packer_.pack(dataset_handler_.dataset().name);

        packer_.pack("is_batch");
        packer_.pack(dataset_handler_.is_batch());
    }

    void pack_attributes() {
        packer_.pack("attributes");
        packer_.pack(attributes_);
    }

    void pack_data() {
        packer_.pack("data");
        // as an array for batch
        if (dataset_handler_.is_batch()) {
            pack_array(dataset_handler_.batch_size());
        }
        // pack scenarios
        for (auto const& scenario_buffer : scenario_buffers_) {
            pack_scenario(scenario_buffer);
        }
    }

    void pack_scenario(ScenarioBuffer const& scenario_buffer) {
        pack_map(scenario_buffer.component_buffers.size());
        for (auto const& component_buffer : scenario_buffer.component_buffers) {
            pack_component(component_buffer);
        }
    }

    void pack_component(ComponentBuffer const& component_buffer) {
        packer_.pack(component_buffer.component);
        pack_array(component_buffer.size);
        bool const use_compact_list = use_compact_list_;
        auto const attributes = [&]() -> std::span<MetaAttribute const* const> {
            if (!use_compact_list) {
                return {};
            }
            auto const found = attributes_.find(component_buffer.component);
            assert(found != attributes_.cend());
            return found->second;
        }();
        for (Idx element = 0; element != component_buffer.size; ++element) {
            RawElementPtr element_ptr = component_buffer.component->advance_ptr(component_buffer.data, element);
            if (use_compact_list) {
                pack_element_in_list(element_ptr, attributes);
            } else {
                pack_element_in_dict(element_ptr, component_buffer);
            }
        }
    }

    void pack_element_in_list(RawElementPtr element_ptr, std::span<MetaAttribute const* const> attributes) {
        pack_array(attributes.size());
        for (auto const* const attribute : attributes) {
            if (check_nan(element_ptr, *attribute)) {
                packer_.pack_nil();
            } else {
                pack_attribute(element_ptr, *attribute);
            }
        }
    }

    void pack_element_in_dict(RawElementPtr element_ptr, ComponentBuffer const& component_buffer) {
        uint32_t valid_attributes_count = 0;
        for (auto const& attribute : component_buffer.component->attributes) {
            valid_attributes_count += static_cast<uint32_t>(!check_nan(element_ptr, attribute));
        }
        pack_map(valid_attributes_count);
        for (auto const& attribute : component_buffer.component->attributes) {
            if (!check_nan(element_ptr, attribute)) {
                packer_.pack(attribute.name);
                pack_attribute(element_ptr, attribute);
            }
        }
    }

    void pack_array(std::integral auto count) {
        if (!std::in_range<uint32_t>(count)) {
            using namespace std::string_literals;

            throw SerializationError{"Too many objects to pack in array ("s + std::to_string(count) + ")"s};
        }
        packer_.pack_array(static_cast<uint32_t>(count));
    }

    void pack_map(std::integral auto count) {
        if (!std::in_range<uint32_t>(count)) {
            using namespace std::string_literals;

            throw SerializationError{"Too many objects to pack in map ("s + std::to_string(count) + ")"s};
        }
        packer_.pack_map(static_cast<uint32_t>(count));
    }

    static bool check_nan(RawElementPtr element_ptr, MetaAttribute const& attribute) {
        return ctype_func_selector(attribute.ctype, [element_ptr, &attribute]<class T> {
            return is_nan(attribute.get_attribute<T const>(element_ptr));
        });
    }

    void pack_attribute(RawElementPtr element_ptr, MetaAttribute const& attribute) {
        ctype_func_selector(attribute.ctype, [this, element_ptr, &attribute]<class T> {
            packer_.pack(attribute.get_attribute<T const>(element_ptr));
        });
    }
};

} // namespace power_grid_model::meta_data
