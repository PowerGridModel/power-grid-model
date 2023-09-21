// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_SERIALIZTION_SERIALIZER_HPP
#define POWER_GRID_MODEL_AUXILIARY_SERIALIZTION_SERIALIZER_HPP

#include "../../exception.hpp"
#include "../../power_grid_model.hpp"
#include "../dataset_handler.hpp"
#include "../meta_data.hpp"
#include "../meta_data_gen.hpp"

#include <nlohmann/json.hpp>

#include <msgpack.hpp>

#include <span>
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
    template <> struct pack<power_grid_model::RealValue<false>> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& p,
                                            power_grid_model::RealValue<false> const& o) const {
            using namespace std::string_view_literals;
            using namespace power_grid_model::meta_data;
            p.pack_array(3);
            for (int8_t i = 0; i != 3; ++i) {
                if (power_grid_model::is_nan(o(i))) {
                    p.pack_nil();
                } else {
                    pack_inf(p, o(i));
                }
            }
            return p;
        }
    };

    } // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack

namespace power_grid_model::meta_data {

constexpr void pack_inf(auto& packer_inf, auto& attr) {
    using namespace msgpack;
    using namespace std::string_view_literals;
    constexpr auto infinity = "inf"sv;
    constexpr auto neg_infinity = "-inf"sv;

    if (std::isinf(attr)) {
        packer_inf.pack(attr > 0 ? infinity : neg_infinity);
    } else {
        packer_inf.pack(attr);
    }
}

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

    Serializer(ConstDatasetHandler dataset_handler, SerializationFormat serialization_format)
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

    ConstDatasetHandler dataset_handler_;
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
            ConstDatasetHandler::Buffer const& buffer = dataset_handler_.get_buffer(component);
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
            auto const json_document = nlohmann::json::from_msgpack(get_msgpack(use_compact_list));
            json_indent_ = indent;
            json_buffer_ = json_document.dump(static_cast<int>(indent));
        }
        return json_buffer_;
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
        return ctype_func_selector(attribute.ctype, [element_ptr, &attribute]<typename T> {
            return is_nan(attribute.get_attribute<T const>(element_ptr));
        });
    }

    void pack_attribute(RawElementPtr element_ptr, MetaAttribute const& attribute) {
        ctype_func_selector(attribute.ctype, [this, element_ptr, &attribute]<typename T> {
            auto const& attr = attribute.get_attribute<T const>(element_ptr);
            if constexpr (std::floating_point<T>) {
                pack_inf(packer_, attr);
            } else {
                packer_.pack(attr);
            }
        });
    }
};

} // namespace power_grid_model::meta_data

#endif
