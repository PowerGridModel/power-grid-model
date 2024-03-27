// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

// handle dataset and buffer related stuff

#include "../common/common.hpp"
#include "../common/exception.hpp"
#include "dataset.hpp"
#include "meta_data.hpp"
#include "meta_data_gen.hpp"

#include <span>
#include <string_view>

namespace power_grid_model::meta_data {

template <dataset_handler_tag T>
constexpr bool is_data_mutable_v = std::same_as<T, mutable_dataset_t> || std::same_as<T, writable_dataset_t>;
template <dataset_handler_tag T> constexpr bool is_indptr_mutable_v = std::same_as<T, writable_dataset_t>;

static_assert(dataset_handler_tag<const_dataset_t>);
static_assert(dataset_handler_tag<mutable_dataset_t>);
static_assert(dataset_handler_tag<writable_dataset_t>);
static_assert(!is_data_mutable_v<const_dataset_t>);
static_assert(is_data_mutable_v<mutable_dataset_t>);
static_assert(is_data_mutable_v<writable_dataset_t>);
static_assert(!is_indptr_mutable_v<const_dataset_t>);
static_assert(!is_indptr_mutable_v<mutable_dataset_t>);
static_assert(is_indptr_mutable_v<writable_dataset_t>);

struct ComponentInfo {
    MetaComponent const* component;
    // for non-uniform component, this is -1, we use indptr to describe the elements per scenario
    Idx elements_per_scenario;
    Idx total_elements;
};

struct DatasetInfo {
    bool is_batch;
    Idx batch_size; // for single dataset, the batch size is one
    MetaDataset const* dataset;
    std::vector<ComponentInfo> component_info;
};

template <dataset_handler_tag dataset_handler_type_> class DatasetHandler {
    struct immutable_t {};
    struct mutable_t {};

  public:
    using dataset_handler_type = dataset_handler_type_;

    using Data = std::conditional_t<is_data_mutable_v<dataset_handler_type>, void, void const>;
    using Indptr = std::conditional_t<is_indptr_mutable_v<dataset_handler_type>, Idx, Idx const>;

    struct Buffer {
        Data* data;
        // for uniform buffer, indptr is empty
        std::span<Indptr> indptr;
    };

    DatasetHandler(bool is_batch, Idx batch_size, std::string_view dataset)
        : dataset_info_{.is_batch = is_batch,
                        .batch_size = batch_size,
                        .dataset = &meta_data.get_dataset(dataset),
                        .component_info = {}} {
        if (!dataset_info_.is_batch && (dataset_info_.batch_size != 1)) {
            throw DatasetError{"For non-batch dataset, batch size should be one!\n"};
        }
    }

    // implicit conversion constructor to const
    template <dataset_handler_tag other_dataset_handler_type>
        requires(is_data_mutable_v<other_dataset_handler_type> && !is_data_mutable_v<dataset_handler_type>)
    DatasetHandler(DatasetHandler<other_dataset_handler_type> const& other) : dataset_info_{other.get_description()} {
        for (Idx i{}; i != other.n_components(); ++i) {
            auto const& buffer = other.get_buffer(i);
            buffers_.push_back(Buffer{.data = buffer.data, .indptr = buffer.indptr});
        }
    }

    template <dataset_type_tag dataset_type>
        requires(is_const_dataset_v<dataset_type> || is_data_mutable_v<dataset_handler_type>)
    std::map<std::string, DataPointer<dataset_type>> export_dataset(Idx scenario = -1) const {
        if (!is_batch() && scenario > 0) {
            throw DatasetError{"Cannot export a single dataset with multiple scenarios!\n"};
        }
        std::map<std::string, DataPointer<dataset_type>> dataset;
        for (Idx i{}; i != n_components(); ++i) {
            ComponentInfo const& component = get_component_info(i);
            Buffer const& buffer = get_buffer(i);
            if (scenario < 0) {
                dataset[component.component->name] = DataPointer<dataset_type>{
                    buffer.data, buffer.indptr.data(), batch_size(), component.elements_per_scenario};
            } else {
                if (component.elements_per_scenario < 0) {
                    dataset[component.component->name] = DataPointer<dataset_type>{
                        component.component->advance_ptr(buffer.data, buffer.indptr[scenario]),
                        buffer.indptr[scenario + 1] - buffer.indptr[scenario]};
                } else {
                    dataset[component.component->name] = DataPointer<dataset_type>{
                        component.component->advance_ptr(buffer.data, component.elements_per_scenario * scenario),
                        component.elements_per_scenario};
                }
            }
        }
        return dataset;
    }

    bool is_batch() const { return dataset_info_.is_batch; }
    Idx batch_size() const { return dataset_info_.batch_size; }
    MetaDataset const& dataset() const { return *dataset_info_.dataset; }
    Idx n_components() const { return static_cast<Idx>(buffers_.size()); }
    DatasetInfo const& get_description() const { return dataset_info_; }
    ComponentInfo const& get_component_info(Idx i) const { return dataset_info_.component_info[i]; }
    Buffer const& get_buffer(std::string_view component) const { return get_buffer(find_component(component, true)); }
    Buffer const& get_buffer(Idx i) const { return buffers_[i]; }

    Idx find_component(std::string_view component, bool required = false) const {
        auto const found = std::ranges::find_if(dataset_info_.component_info, [component](ComponentInfo const& x) {
            return x.component->name == component;
        });
        if (found == dataset_info_.component_info.cend()) {
            if (required) {
                using namespace std::string_literals;
                throw DatasetError{"Cannot find component '"s + std::string{component} + "'!\n"s};
            }
            return -1;
        }
        return std::distance(dataset_info_.component_info.cbegin(), found);
    }

    ComponentInfo const& get_component_info(std::string_view component) const {
        return dataset_info_.component_info[find_component(component, true)];
    }

    void add_component_info(std::string_view component, Idx elements_per_scenario, Idx total_elements)
        requires is_indptr_mutable_v<dataset_handler_type>
    {
        add_component_info_impl(component, elements_per_scenario, total_elements);
    }

    void add_buffer(std::string_view component, Idx elements_per_scenario, Idx total_elements, Indptr* indptr,
                    Data* data)
        requires(!is_indptr_mutable_v<dataset_handler_type>)
    {
        check_non_uniform_integrity<immutable_t>(elements_per_scenario, total_elements, indptr);
        add_component_info_impl(component, elements_per_scenario, total_elements);
        buffers_.back().data = data;
        if (indptr) {
            buffers_.back().indptr = {indptr, static_cast<size_t>(batch_size() + 1)};
        } else {
            buffers_.back().indptr = {};
        }
    }

    void set_buffer(std::string_view component, Indptr* indptr, Data* data)
        requires is_indptr_mutable_v<dataset_handler_type>
    {
        Idx const idx = find_component(component, true);
        ComponentInfo const& info = dataset_info_.component_info[idx];
        check_non_uniform_integrity<mutable_t>(info.elements_per_scenario, info.total_elements, indptr);
        buffers_[idx].data = data;
        if (indptr) {
            buffers_[idx].indptr = {indptr, static_cast<size_t>(batch_size() + 1)};
        } else {
            buffers_[idx].indptr = {};
        }
    }

  private:
    DatasetInfo dataset_info_;
    std::vector<Buffer> buffers_;

    void check_uniform_integrity(Idx elements_per_scenario, Idx total_elements) {
        if ((elements_per_scenario >= 0) && (elements_per_scenario * batch_size() != total_elements)) {
            throw DatasetError{
                "For a uniform buffer, total_elements should be equal to elements_per_scenario * batch_size !\n"};
        }
    }

    template <typename check_indptr_content>
        requires std::same_as<check_indptr_content, mutable_t> || std::same_as<check_indptr_content, immutable_t>
    void check_non_uniform_integrity(Idx elements_per_scenario, Idx total_elements, Indptr* indptr) {
        if (elements_per_scenario < 0) {
            if (!indptr) {
                throw DatasetError{"For a non-uniform buffer, indptr should be supplied !\n"};
            }
            if constexpr (std::same_as<check_indptr_content, immutable_t>) {
                if (indptr[0] != 0 || indptr[batch_size()] != total_elements) {
                    throw DatasetError{
                        "For a non-uniform buffer, indptr should begin with 0 and end with total_elements !\n"};
                }
            }
        } else {
            if (indptr) {
                throw DatasetError{"For a uniform buffer, indptr should be nullptr !\n"};
            }
        }
    }

    void add_component_info_impl(std::string_view component, Idx elements_per_scenario, Idx total_elements) {
        if (find_component(component) >= 0) {
            throw DatasetError{"Cannot have duplicated components!\n"};
        }
        check_uniform_integrity(elements_per_scenario, total_elements);
        dataset_info_.component_info.push_back(
            {&dataset_info_.dataset->get_component(component), elements_per_scenario, total_elements});
        buffers_.push_back(Buffer{});
    }
};

using ConstDatasetHandler = DatasetHandler<const_dataset_t>;
using MutableDatasetHandler = DatasetHandler<mutable_dataset_t>;
using WritableDatasetHandler = DatasetHandler<writable_dataset_t>;

} // namespace power_grid_model::meta_data
