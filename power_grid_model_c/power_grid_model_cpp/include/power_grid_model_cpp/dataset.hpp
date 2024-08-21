// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_DATASET_HPP
#define POWER_GRID_MODEL_CPP_DATASET_HPP

#include "basics.hpp"
#include "handle.hpp"
#include "serialization.hpp"

#include "power_grid_model_c/dataset.h"

namespace power_grid_model_cpp {
class DatasetInfo {
  public:
    DatasetInfo(RawDatasetInfo const* info) : info_{info} {}

    static std::string const name(DatasetInfo const& info) {
        return std::string{info.handle_.call_with(PGM_dataset_info_name, info.info_.get())};
    }
    std::string const name() const { return name(*this); }

    static Idx is_batch(DatasetInfo const& info) {
        return info.handle_.call_with(PGM_dataset_info_is_batch, info.info_.get());
    }
    Idx is_batch() const { return is_batch(*this); }

    static Idx batch_size(DatasetInfo const& info) {
        return info.handle_.call_with(PGM_dataset_info_batch_size, info.info_.get());
    }
    Idx batch_size() const { return batch_size(*this); }

    static Idx n_components(DatasetInfo const& info) {
        return info.handle_.call_with(PGM_dataset_info_n_components, info.info_.get());
    }
    Idx n_components() const { return n_components(*this); }

    static std::string component_name(DatasetInfo const& info, Idx component_idx) {
        return std::string{info.handle_.call_with(PGM_dataset_info_component_name, info.info_.get(), component_idx)};
    }
    std::string component_name(Idx component_idx) const { return component_name(*this, component_idx); }

    static Idx component_elements_per_scenario(DatasetInfo const& info, Idx component_idx) {
        return info.handle_.call_with(PGM_dataset_info_elements_per_scenario, info.info_.get(), component_idx);
    }
    Idx component_elements_per_scenario(Idx component_idx) const {
        return component_elements_per_scenario(*this, component_idx);
    }

    static Idx component_total_elements(DatasetInfo const& info, Idx component_idx) {
        return info.handle_.call_with(PGM_dataset_info_total_elements, info.info_.get(), component_idx);
    }
    Idx component_total_elements(Idx component_idx) const { return component_total_elements(*this, component_idx); }

  private:
    Handle handle_{};
    std::unique_ptr<PGM_DatasetInfo const> info_;
};

class DatasetConst {
  public:
    DatasetConst(std::string const& dataset, Idx is_batch, Idx batch_size)
        : dataset_{handle_.call_with(PGM_create_dataset_const, dataset.c_str(), is_batch, batch_size)} {}
    DatasetConst(DatasetWritable const& writable_dataset)
        : dataset_{handle_.call_with(PGM_create_dataset_const_from_writable, writable_dataset.get())} {}
    DatasetConst(DatasetMutable const& mutable_dataset)
        : dataset_{handle_.call_with(PGM_create_dataset_const_from_mutable, mutable_dataset.get())} {}

    RawConstDataset* get() const { return dataset_.get(); }

    static void add_buffer(DatasetConst& dataset, std::string const& component, Idx elements_per_scenario,
                           Idx total_elements, Idx const* indptr, RawDataConstPtr data) {
        dataset.handle_.call_with(PGM_dataset_const_add_buffer, dataset.dataset_.get(), component.c_str(),
                                  elements_per_scenario, total_elements, indptr, data);
    }
    void add_buffer(std::string const& component, Idx elements_per_scenario, Idx total_elements, Idx const* indptr,
                    RawDataConstPtr data) {
        add_buffer(*this, component, elements_per_scenario, total_elements, indptr, data);
    }

    static DatasetInfo get_info(DatasetConst const& dataset) {
        return DatasetInfo{dataset.handle_.call_with(PGM_dataset_const_get_info, dataset.dataset_.get())};
    }
    DatasetInfo get_info() const { return get_info(*this); }

  private:
    Handle handle_{};
    detail::UniquePtr<RawConstDataset, PGM_destroy_dataset_const> dataset_;
};

class DatasetWritable {
  public:
    DatasetWritable(RawWritableDataset* dataset) : dataset_{dataset} { assert(dataset != nullptr); }

    RawWritableDataset* get() const { return dataset_.get(); }

    static DatasetInfo get_info(DatasetWritable const& dataset) {
        return DatasetInfo{dataset.handle_.call_with(PGM_dataset_writable_get_info, dataset.dataset_.get())};
    }
    DatasetInfo get_info() const { return get_info(*this); }

    static void set_buffer(DatasetWritable const& dataset, std::string const& component, Idx* indptr, RawDataPtr data) {
        dataset.handle_.call_with(PGM_dataset_writable_set_buffer, dataset.dataset_.get(), component.c_str(), indptr,
                                  data);
    }
    void set_buffer(std::string const& component, Idx* indptr, RawDataPtr data) {
        set_buffer(*this, component.c_str(), indptr, data);
    }

  private:
    Handle handle_{};
    std::unique_ptr<RawWritableDataset> dataset_;
};

class DatasetMutable {
  public:
    DatasetMutable(std::string const& dataset, Idx is_batch, Idx batch_size)
        : dataset_{handle_.call_with(PGM_create_dataset_mutable, dataset.c_str(), is_batch, batch_size)} {}

    RawMutableDataset* get() const { return dataset_.get(); }

    static void add_buffer(DatasetMutable& dataset, std::string const& component, Idx elements_per_scenario,
                           Idx total_elements, Idx const* indptr, RawDataPtr data) {
        dataset.handle_.call_with(PGM_dataset_mutable_add_buffer, dataset.dataset_.get(), component.c_str(),
                                  elements_per_scenario, total_elements, indptr, data);
    }
    void add_buffer(std::string const& component, Idx elements_per_scenario, Idx total_elements, Idx const* indptr,
                    RawDataPtr data) {
        add_buffer(*this, component.c_str(), elements_per_scenario, total_elements, indptr, data);
    }

    static DatasetInfo get_info(DatasetMutable const& dataset) {
        return DatasetInfo{dataset.handle_.call_with(PGM_dataset_mutable_get_info, dataset.dataset_.get())};
    }
    DatasetInfo get_info() const { return get_info(*this); }

  private:
    Handle handle_{};
    detail::UniquePtr<RawMutableDataset, PGM_destroy_dataset_mutable> dataset_;
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_DATASET_HPP
