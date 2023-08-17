// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_DATASET_HANDLER_HPP
#define POWER_GRID_MODEL_AUXILIARY_DATASET_HANDLER_HPP

// handle dataset and buffer related stuff

#include "../exception.hpp"
#include "../power_grid_model.hpp"
#include "meta_data.hpp"

#include <span>
#include <string_view>

namespace power_grid_model::meta_data {

struct ComponentInfo {
    MetaComponent const* component;
    // for non-uniform component, this is -1, we use indptr to describe the elements per scenario
    Idx elements_per_scenario;
    Idx total_elements;
};

struct DatasetDescription {
    bool is_batch;
    Idx batch_size; // for single dataset, the batch size is one
    MetaDataset const* dataset;
    std::vector<ComponentInfo> component_info;
};

template <bool data_mutable, bool indptr_mutable>
    requires(data_mutable || !indptr_mutable)
struct DatasetHandler {
    using Data = std::conditional_t<data_mutable, void, void const>;
    using Indptr = std::conditional_t<indptr_mutable, Idx, Idx const>;
    struct Buffer {
        Data* data;
        // for uniform buffer, indptr is empty
        std::span<Indptr> indptr;
    };

    DatasetDescription description{};
    std::vector<Buffer> buffers;

    Idx n_components() const { return static_cast<Idx>(buffers.size()); }

    Idx find_component(std::string_view component, bool throw_not_found = false) const {
        auto const found = std::find_if(description.component_info.cbegin(), description.component_info.cend(),
                                        [component](ComponentInfo const& x) { return x.component->name == component; });
        if (found == description.component_info.cend()) {
            if (throw_not_found) {
                throw DatasetError{"Cannot find component!\n"};
            } else {
                return -1;
            }
        }
        return std::distance(description.component_info.cbegin(), found);
    }

    ComponentInfo const& get_component_info(std::string_view component) const {
        return description.component_info[find_component(component, true)];
    }

    Buffer const& get_buffer(std::string_view component) const { return buffers[find_component(component, true)]; }

    void add_component_info(std::string_view component, Idx elements_per_scenario, Idx total_elements) {
        if (find_component(component) >= 0) {
            throw DatasetError{"Cannot have duplicated components!\n"};
        }
        description.component_info.push_back(
            {&description.dataset->get_component(component), elements_per_scenario, total_elements});
        buffers.push_back(Buffer{});
    }

    void add_buffer(std::string_view component, Idx elements_per_scenario, Idx total_elements, Indptr* indptr,
                    Data* data) {
        add_component_info(component, elements_per_scenario, total_elements);
        buffers.back().data = data;
        buffers.back().indptr = {indptr, static_cast<size_t>(description.batch_size + 1)};
    }

    void set_buffer(std::string_view component, Indptr* indptr, Data* data) {
        Idx const idx = find_component(component, true);
        buffers[idx].data = data;
        buffers[idx].indptr = {indptr, static_cast<size_t>(description.batch_size + 1)};
    }
};

template struct DatasetHandler<true, true>;

using ConstDatasetHandler = DatasetHandler<false, false>;
using MutableDatasetHandler = DatasetHandler<true, false>;
using WritableDatasetHandler = DatasetHandler<true, true>;

} // namespace power_grid_model::meta_data

#endif