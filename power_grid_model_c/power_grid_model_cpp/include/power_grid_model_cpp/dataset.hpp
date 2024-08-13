// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_DATASET_HPP
#define POWER_GRID_MODEL_CPP_DATASET_HPP

#include "dataset.h"

#include "basics.hpp"
#include "handle.hpp"

namespace power_grid_model_cpp {
class Dataset {
  public:
    virtual ~Dataset() = default;

    static std::string const info_name(Handle const& handle, PGM_DatasetInfo const* info) {
        auto const name = std::string(PGM_dataset_info_name(handle.get(), info));
        handle.check_error();
        return name;
    }
    std::string const info_name(PGM_DatasetInfo const* info) const {
        auto const name = std::string(PGM_dataset_info_name(handle_.get(), info));
        handle_.check_error();
        return name;
    }

    static Idx info_is_batch(Handle const& handle, PGM_DatasetInfo const* info) {
        auto const is_batch = PGM_dataset_info_is_batch(handle.get(), info);
        handle.check_error();
        return is_batch;
    }
    Idx info_is_batch(PGM_DatasetInfo const* info) const {
        auto const is_batch = PGM_dataset_info_is_batch(handle_.get(), info);
        handle_.check_error();
        return is_batch;
    }

    static Idx info_batch_size(Handle const& handle, PGM_DatasetInfo const* info) {
        auto const batch_size = PGM_dataset_info_batch_size(handle.get(), info);
        handle.check_error();
        return batch_size;
    }
    Idx info_batch_size(PGM_DatasetInfo const* info) const {
        auto const batch_size = PGM_dataset_info_batch_size(handle_.get(), info);
        handle_.check_error();
        return batch_size;
    }

    static Idx info_n_components(Handle const& handle, PGM_DatasetInfo const* info) {
        auto const n_components = PGM_dataset_info_n_components(handle.get(), info);
        handle.check_error();
        return n_components;
    }
    Idx info_n_components(PGM_DatasetInfo const* info) const {
        auto const n_components = PGM_dataset_info_n_components(handle_.get(), info);
        handle_.check_error();
        return n_components;
    }

    static std::string const info_component_name(Handle const& handle, PGM_DatasetInfo const* info, Idx component_idx) {
        auto const component_name = std::string(PGM_dataset_info_component_name(handle.get(), info, component_idx));
        handle.check_error();
        return component_name;
    }
    std::string const info_component_name(PGM_DatasetInfo const* info, Idx component_idx) const {
        auto const component_name = std::string(PGM_dataset_info_component_name(handle_.get(), info, component_idx));
        handle_.check_error();
        return component_name;
    }

    static Idx info_elements_per_scenario(Handle const& handle, PGM_DatasetInfo const* info, Idx component_idx) {
        auto const elements_per_scenario = PGM_dataset_info_elements_per_scenario(handle.get(), info, component_idx);
        handle.check_error();
        return elements_per_scenario;
    }
    Idx info_elements_per_scenario(PGM_DatasetInfo const* info, Idx component_idx) const {
        auto const elements_per_scenario = PGM_dataset_info_elements_per_scenario(handle_.get(), info, component_idx);
        handle_.check_error();
        return elements_per_scenario;
    }

    static Idx info_total_elements(Handle const& handle, PGM_DatasetInfo const* info, Idx component_idx) {
        auto const total_elements = PGM_dataset_info_total_elements(handle.get(), info, component_idx);
        handle.check_error();
        return total_elements;
    }
    Idx info_total_elements(PGM_DatasetInfo const* info, Idx component_idx) const {
        auto const total_elements = PGM_dataset_info_total_elements(handle_.get(), info, component_idx);
        handle_.check_error();
        return total_elements;
    }

  protected:
    Dataset() = default;
    Handle handle_{};
};

class DatasetConst : public Dataset {
  public:
    DatasetConst(std::string const& dataset, Idx is_batch, Idx batch_size)
        : Dataset(), dataset_{PGM_create_dataset_const(handle_.get(), dataset.c_str(), is_batch, batch_size)} {}
    DatasetConst(PGM_WritableDataset const* writable_dataset)
        : Dataset(), dataset_{PGM_create_dataset_const_from_writable(handle_.get(), writable_dataset)} {}
    DatasetConst(PGM_MutableDataset const* mutable_dataset)
        : Dataset(), dataset_{PGM_create_dataset_const_from_mutable(handle_.get(), mutable_dataset)} {}

    ~DatasetConst() = default;

    static void add_buffer(Handle const& handle, PGM_ConstDataset* dataset, std::string const& component,
                           Idx elements_per_scenario, Idx total_elements, Idx const* indptr, void const* data) {
        PGM_dataset_const_add_buffer(handle.get(), dataset, component.c_str(), elements_per_scenario, total_elements,
                                     indptr, data);
        handle.check_error();
    }
    void add_buffer(std::string const& component, Idx elements_per_scenario, Idx total_elements, Idx const* indptr,
                    void const* data) {
        PGM_dataset_const_add_buffer(handle_.get(), dataset_.get(), component.c_str(), elements_per_scenario,
                                     total_elements, indptr, data);
        handle_.check_error();
    }

    static void get_info(Handle const& handle, PGM_ConstDataset const* dataset) {
        PGM_dataset_const_get_info(handle.get(), dataset);
        handle.check_error();
    }
    void get_info() const {
        PGM_dataset_const_get_info(handle_.get(), dataset_.get());
        handle_.check_error();
    }

  private:
    UniquePtr<PGM_ConstDataset, PGM_destroy_dataset_const> dataset_;
};

class DatasetWritable : public Dataset {
  public:
    DatasetWritable() : Dataset() {}

    ~DatasetWritable() = default;

    static PGM_DatasetInfo const* get_info(Handle const& handle, PGM_WritableDataset const* dataset) {
        return PGM_dataset_writable_get_info(handle.get(), dataset);
        handle.check_error();
    }
    PGM_DatasetInfo const* get_info(PGM_WritableDataset const* dataset) const {
        return PGM_dataset_writable_get_info(handle_.get(), dataset);
        handle_.check_error();
    }

    static void set_buffer(Handle const& handle, PGM_WritableDataset* dataset, std::string const& component,
                           Idx* indptr, void* data) {
        PGM_dataset_writable_set_buffer(handle.get(), dataset, component.c_str(), indptr, data);
        handle.check_error();
    }
    void set_buffer(PGM_WritableDataset* dataset, std::string const& component, Idx* indptr, void* data) {
        PGM_dataset_writable_set_buffer(handle_.get(), dataset, component.c_str(), indptr, data);
        handle_.check_error();
    }
};

class DatasetMutable : public Dataset {
    DatasetMutable(std::string const& dataset, Idx is_batch, Idx batch_size)
        : Dataset(), dataset_{PGM_create_dataset_mutable(handle_.get(), dataset.c_str(), is_batch, batch_size)} {}

    ~DatasetMutable() = default;

    static void add_buffer(Handle const& handle, PGM_MutableDataset* dataset, std::string const& component,
                           Idx elements_per_scenario, Idx total_elements, Idx const* indptr, void* data) {
        PGM_dataset_mutable_add_buffer(handle.get(), dataset, component.c_str(), elements_per_scenario, total_elements,
                                       indptr, data);
        handle.check_error();
    }
    void add_buffer(std::string const& component, Idx elements_per_scenario, Idx total_elements, Idx const* indptr,
                    void* data) {
        PGM_dataset_mutable_add_buffer(handle_.get(), dataset_.get(), component.c_str(), elements_per_scenario,
                                       total_elements, indptr, data);
        handle_.check_error();
    }

    static PGM_DatasetInfo const* get_info(Handle const& handle, PGM_MutableDataset const* dataset) {
        PGM_dataset_mutable_get_info(handle.get(), dataset);
        handle.check_error();
    }
    PGM_DatasetInfo const* get_info() const {
        PGM_dataset_mutable_get_info(handle_.get(), dataset_.get());
        handle_.check_error();
    }

  private:
    UniquePtr<PGM_MutableDataset, PGM_destroy_dataset_mutable> dataset_;
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_DATASET_HPP