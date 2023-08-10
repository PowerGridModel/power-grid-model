// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_SERIALIZTION_SERIALIZER_HPP
#define POWER_GRID_MODEL_AUXILIARY_SERIALIZTION_SERIALIZER_HPP

#include "../../exception.hpp"
#include "../../power_grid_model.hpp"
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
        requires(std::same_as<T, PGM_MetaComponent> || std::same_as<T, PGM_MetaAttribute>)
    struct pack<T const*> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& p, T const* const& o) const {
            p.pack(o->name);
            return p;
        }
    };

    } // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack

namespace power_grid_model::meta_data {

class Serializer {

    struct ComponentBuffer {
        MetaComponent const* component;
        void const* data;
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
    // movable
    Serializer(Serializer&&) = default;
    Serializer& operator=(Serializer&&) = default;
    // destructor
    ~Serializer() = default;

    Serializer(std::string_view dataset, bool is_batch, Idx batch_size, Idx n_components, char const** components,
               Idx const* elements_per_scenario, Idx const** indptrs, void const** data)
        : dataset_{&meta_data().get_dataset(dataset)},
          is_batch_{is_batch},
          batch_size_{batch_size},
          n_components_{n_components},
          packer_{msgpack_buffer_} {
        if (!is_batch_ && (batch_size_ != 1)) {
            throw SerializationError{"For non-batch dataset, batch size should be one!\n"};
        }
        store_buffers(components, elements_per_scenario, indptrs, data);
    }

    void serialize(bool use_compact_list) {
        msgpack_buffer_.clear();
        use_compact_list_ = use_compact_list;
        if (use_compact_list_) {
            check_attributes();
        }
        pack_root_dict();
        pack_attributes();
        pack_data();
    }

  private:
    MetaDataset const* dataset_;
    bool is_batch_;
    Idx batch_size_;
    Idx n_components_;
    std::vector<ScenarioBuffer> scenario_buffers_;   // list of scenarios, then list of components, omit empty
    std::vector<ComponentBuffer> component_buffers_; // list of components, then all scenario flatten
    // msgpack pakcer
    msgpack::sbuffer msgpack_buffer_{};
    msgpack::packer<msgpack::sbuffer> packer_;
    bool use_compact_list_{};
    std::map<MetaComponent const*, std::vector<MetaAttribute const*>> attributes_;

    void store_buffers(char const** components, Idx const* elements_per_scenario, Idx const** indptrs,
                       void const** data) {
        scenario_buffers_.resize(batch_size_);
        for (Idx scenario = 0; scenario != batch_size_; ++scenario) {
            scenario_buffers_[scenario] =
                create_scenario_buffer_view(components, elements_per_scenario, indptrs, data, scenario);
        }
        component_buffers_ =
            create_scenario_buffer_view(components, elements_per_scenario, indptrs, data).component_buffers;
    }

    ScenarioBuffer create_scenario_buffer_view(char const** components, Idx const* elements_per_scenario,
                                               Idx const** indptrs, void const** data, Idx scenario = -1) {
        ScenarioBuffer scenario_buffer{};
        Idx const begin_scenario = scenario < 0 ? 0 : scenario;
        Idx const end_scenario = scenario < 0 ? batch_size_ : begin_scenario + 1;
        for (Idx component = 0; component != n_components_; ++component) {
            ComponentBuffer component_buffer{};
            component_buffer.component = &dataset_->get_component(components[component]);
            if (elements_per_scenario[component] < 0) {
                component_buffer.data =
                    component_buffer.component->advance_ptr(data[component], indptrs[component][begin_scenario]);
                component_buffer.size = indptrs[component][end_scenario] - indptrs[component][begin_scenario];
            } else {
                component_buffer.data = component_buffer.component->advance_ptr(
                    data[component], elements_per_scenario[component] * begin_scenario);
                component_buffer.size = elements_per_scenario[component] * (end_scenario - begin_scenario);
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

    void pack_root_dict() {
        packer_.pack_map(size_top_dict);
        packer_.pack("version");
        packer_.pack(version);
        packer_.pack("type");
        packer_.pack(dataset_->name);
        packer_.pack("is_batch");
        packer_.pack(is_batch_);
    }

    void pack_attributes() {
        packer_.pack("attributes");
        packer_.pack(attributes_);
    }

    void pack_data() {}
};

} // namespace power_grid_model::meta_data

#endif
