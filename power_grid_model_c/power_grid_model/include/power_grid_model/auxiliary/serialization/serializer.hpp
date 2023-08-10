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
          n_components_{n_components} {
        if (!is_batch_ && (batch_size_ != 1)) {
            throw SerializationError{"For non-batch dataset, batch size should be one!\n"};
        }
        store_buffers(components, elements_per_scenario, indptrs, data);
    }

  private:
    MetaDataset const* dataset_;
    bool is_batch_;
    Idx batch_size_;
    Idx n_components_;
    std::vector<ScenarioBuffer> buffers_;

    void store_buffers(char const** components, Idx const* elements_per_scenario, Idx const** indptrs,
                       void const** data) {
        buffers_.resize(batch_size_);
        for (Idx scenario = 0; scenario != batch_size_; ++scenario) {
            buffers_[scenario] =
                create_scenario_buffer_view(scenario, components, elements_per_scenario, indptrs, data);
        }
    }

    ScenarioBuffer create_scenario_buffer_view(Idx scenario, char const** components, Idx const* elements_per_scenario,
                                               Idx const** indptrs, void const** data) {
        ScenarioBuffer scenario_buffer{};
        for (Idx component = 0; component != n_components_; ++component) {
            ComponentBuffer component_buffer{};
            component_buffer.component = &dataset_->get_component(components[component]);
            if (elements_per_scenario[component] < 0) {
                component_buffer.data =
                    component_buffer.component->advance_ptr(data[component], indptrs[component][scenario]);
                component_buffer.size = indptrs[component][scenario + 1] - indptrs[component][scenario];
            } else {
                component_buffer.data = component_buffer.component->advance_ptr(
                    data[component], elements_per_scenario[component] * scenario);
                component_buffer.size = elements_per_scenario[component];
            }
            // only store the view if it is non-empty
            if (component_buffer.size > 0) {
                scenario_buffer.component_buffers.push_back(component_buffer);
            }
        }
        return scenario_buffer;
    }
};

} // namespace power_grid_model::meta_data

#endif
