// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_DATASET_HPP
#define POWER_GRID_MODEL_CPP_DATASET_HPP

#include "power_grid_model_c/dataset.h"

#include "basics.hpp"
#include "handle.hpp"
#include "serialization.hpp"

namespace power_grid_model_cpp {
class Info {
  public:
    Info(RawDatasetInfo const* info) : info_{info} {}

    static std::string const dataset_name(Info const& info) {
        auto const name = std::string(PGM_dataset_info_name(info.handle_.get(), info.info_.get()));
        info.handle_.check_error();
        return name;
    }
    std::string const dataset_name() const { return dataset_name(*this); }

    static Idx dataset_is_batch(Info const& info) {
        auto const is_batch = PGM_dataset_info_is_batch(info.handle_.get(), info.info_.get());
        info.handle_.check_error();
        return is_batch;
    }
    Idx dataset_is_batch() const { return dataset_is_batch(*this); }

    static Idx dataset_batch_size(Info const& info) {
        auto const batch_size = PGM_dataset_info_batch_size(info.handle_.get(), info.info_.get());
        info.handle_.check_error();
        return batch_size;
    }
    Idx dataset_batch_size() const { return dataset_batch_size(*this); }

    static Idx dataset_n_components(Info const& info) {
        auto const n_components = PGM_dataset_info_n_components(info.handle_.get(), info.info_.get());
        info.handle_.check_error();
        return n_components;
    }
    Idx dataset_n_components() const { return dataset_n_components(*this); }

    static std::string const component_name(Info const& info, Idx component_idx) {
        auto const component_name =
            std::string(PGM_dataset_info_component_name(info.handle_.get(), info.info_.get(), component_idx));
        info.handle_.check_error();
        return component_name;
    }
    std::string const component_name(Idx component_idx) const { return component_name(*this, component_idx); }

    static Idx component_elements_per_scenario(Info const& info, Idx component_idx) {
        auto const elements_per_scenario =
            PGM_dataset_info_elements_per_scenario(info.handle_.get(), info.info_.get(), component_idx);
        info.handle_.check_error();
        return elements_per_scenario;
    }
    Idx component_elements_per_scenario(Idx component_idx) const {
        return component_elements_per_scenario(*this, component_idx);
    }

    static Idx component_total_elements(Info const& info, Idx component_idx) {
        auto const total_elements =
            PGM_dataset_info_total_elements(info.handle_.get(), info.info_.get(), component_idx);
        info.handle_.check_error();
        return total_elements;
    }
    Idx component_total_elements(Idx component_idx) const { return component_total_elements(*this, component_idx); }

  private:
    Handle handle_{};
    std::unique_ptr<PGM_DatasetInfo const> info_;
};

class DatasetConst {
  public:
    DatasetConst(std::string const& dataset, Idx is_batch, Idx batch_size)
        : dataset_{PGM_create_dataset_const(handle_.get(), dataset.c_str(), is_batch, batch_size)} {}
    DatasetConst(DatasetWritable const& writable_dataset)
        : dataset_{PGM_create_dataset_const_from_writable(handle_.get(), writable_dataset.get())} {}
    DatasetConst(DatasetMutable const& mutable_dataset)
        : dataset_{PGM_create_dataset_const_from_mutable(handle_.get(), mutable_dataset.get())} {}

    RawConstDataset* get() const { return dataset_.get(); }

    static void add_buffer(DatasetConst& dataset, std::string const& component, Idx elements_per_scenario,
                           Idx total_elements, Idx const* indptr, RawDataConstPtr data) {
        PGM_dataset_const_add_buffer(dataset.handle_.get(), dataset.dataset_.get(), component.c_str(),
                                     elements_per_scenario, total_elements, indptr, data);
        dataset.handle_.check_error();
    }
    void add_buffer(std::string const& component, Idx elements_per_scenario, Idx total_elements, Idx const* indptr,
                    RawDataConstPtr data) {
        return add_buffer(*this, component, elements_per_scenario, total_elements, indptr, data);
    }

    static Info const get_info(DatasetConst const& dataset) {
        auto const info = PGM_dataset_const_get_info(dataset.handle_.get(), dataset.dataset_.get());
        dataset.handle_.check_error();
        return Info(info);
    }
    Info const get_info() const { return get_info(*this); }

  private:
    Handle handle_{};
    detail::UniquePtr<RawConstDataset, PGM_destroy_dataset_const> dataset_;
};

class DatasetWritable {
  public:
    RawWritableDataset* get() const { return dataset_.get(); }

    static Info const get_info(DatasetWritable const& dataset) {
        auto const info = PGM_dataset_writable_get_info(dataset.handle_.get(), dataset.dataset_.get());
        dataset.handle_.check_error();
        return Info(info);
    }
    Info const get_info() const { return get_info(*this); }

    static void set_buffer(DatasetWritable const& dataset, std::string const& component, Idx* indptr, RawDataPtr data) {
        PGM_dataset_writable_set_buffer(dataset.handle_.get(), dataset.dataset_.get(), component.c_str(), indptr, data);
        dataset.handle_.check_error();
    }
    void set_buffer(std::string const& component, Idx* indptr, RawDataPtr data) {
        return set_buffer(*this, component.c_str(), indptr, data);
    }

  private:
    friend class Deserializer;
    DatasetWritable(RawWritableDataset* dataset) : dataset_{dataset} {}
    Handle handle_{};
    std::unique_ptr<RawWritableDataset> dataset_;
};

class DatasetMutable {
  public:
    DatasetMutable(std::string const& dataset, Idx is_batch, Idx batch_size)
        : dataset_{PGM_create_dataset_mutable(handle_.get(), dataset.c_str(), is_batch, batch_size)} {}

    RawMutableDataset* get() const { return dataset_.get(); }

    static void add_buffer(DatasetMutable& dataset, std::string const& component, Idx elements_per_scenario,
                           Idx total_elements, Idx const* indptr, RawDataPtr data) {
        PGM_dataset_mutable_add_buffer(dataset.handle_.get(), dataset.dataset_.get(), component.c_str(),
                                       elements_per_scenario, total_elements, indptr, data);
        dataset.handle_.check_error();
    }
    void add_buffer(std::string const& component, Idx elements_per_scenario, Idx total_elements, Idx const* indptr,
                    RawDataPtr data) {
        return add_buffer(*this, component.c_str(), elements_per_scenario, total_elements, indptr, data);
    }

    static Info const get_info(DatasetMutable const& dataset) {
        auto const info = PGM_dataset_mutable_get_info(dataset.handle_.get(), dataset.dataset_.get());
        dataset.handle_.check_error();
        return Info(info);
    }
    Info const get_info() const { return get_info(*this); }

  private:
    Handle handle_{};
    detail::UniquePtr<RawMutableDataset, PGM_destroy_dataset_mutable> dataset_;
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_DATASET_HPP
