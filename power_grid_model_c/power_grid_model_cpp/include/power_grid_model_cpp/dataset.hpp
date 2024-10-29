// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_DATASET_HPP
#define POWER_GRID_MODEL_CPP_DATASET_HPP

#include "basics.hpp"
#include "buffer.hpp"
#include "handle.hpp"

#include "power_grid_model_c/dataset.h"
namespace power_grid_model_cpp {

class DatasetInfo {

  public:
    DatasetInfo(RawDatasetInfo const* info) noexcept : info_{info} {}
    DatasetInfo(DatasetInfo&&) = default;
    DatasetInfo& operator=(DatasetInfo&&) = default;
    DatasetInfo(const DatasetInfo&) = delete;            // No copy constructor
    DatasetInfo& operator=(const DatasetInfo&) = delete; // No copy assignment
    ~DatasetInfo() = default;

    std::string name() const { return std::string{handle_.call_with(PGM_dataset_info_name, info_)}; }

    Idx is_batch() const { return handle_.call_with(PGM_dataset_info_is_batch, info_); }

    Idx batch_size() const { return handle_.call_with(PGM_dataset_info_batch_size, info_); }

    Idx n_components() const { return handle_.call_with(PGM_dataset_info_n_components, info_); }

    std::string component_name(Idx component_idx) const {
        return std::string{handle_.call_with(PGM_dataset_info_component_name, info_, component_idx)};
    }

    Idx component_elements_per_scenario(Idx component_idx) const {
        return handle_.call_with(PGM_dataset_info_elements_per_scenario, info_, component_idx);
    }

    Idx component_total_elements(Idx component_idx) const {
        return handle_.call_with(PGM_dataset_info_total_elements, info_, component_idx);
    }

  private:
    Handle handle_{};
    RawDatasetInfo const* info_;
};

class DatasetWritable {
  public:
    DatasetWritable(RawWritableDataset* dataset)
        : dataset_{dataset}, info_{handle_.call_with(PGM_dataset_writable_get_info, dataset_)} {}
    DatasetWritable(DatasetWritable&&) = default;
    DatasetWritable& operator=(DatasetWritable&&) = default;
    DatasetWritable(const DatasetWritable&) = delete;            // No copy constructor
    DatasetWritable& operator=(const DatasetWritable&) = delete; // No copy assignment
    ~DatasetWritable() = default;

    RawWritableDataset const* get() const { return dataset_; }
    RawWritableDataset* get() { return dataset_; }

    DatasetInfo const& get_info() const { return info_; }

    void set_buffer(std::string const& component, Idx* indptr, RawDataPtr data) {
        handle_.call_with(PGM_dataset_writable_set_buffer, get(), component.c_str(), indptr, data);
    }

    void set_buffer(std::string const& component, Idx* indptr, Buffer& data) {
        handle_.call_with(PGM_dataset_writable_set_buffer, get(), component.c_str(), indptr, data.get());
    }

    void set_attribute_buffer(std::string const& component, std::string const& attribute, RawDataPtr data) {
        handle_.call_with(PGM_dataset_writable_set_attribute_buffer, get(), component.c_str(), attribute.c_str(), data);
    }

    void set_attribute_buffer(std::string const& component, std::string const& attribute, Buffer& data) {
        handle_.call_with(PGM_dataset_writable_set_attribute_buffer, get(), component.c_str(), attribute.c_str(),
                          data.get());
    }

  private:
    Handle handle_{};
    RawWritableDataset* dataset_;
    DatasetInfo info_;
};

class DatasetMutable {
  public:
    DatasetMutable(std::string const& dataset, Idx is_batch, Idx batch_size)
        : dataset_{handle_.call_with(PGM_create_dataset_mutable, dataset.c_str(), is_batch, batch_size)},
          info_{handle_.call_with(PGM_dataset_mutable_get_info, get())} {}

    RawMutableDataset const* get() const { return dataset_.get(); }
    RawMutableDataset* get() { return dataset_.get(); }

    void add_buffer(std::string const& component, Idx elements_per_scenario, Idx total_elements, Idx const* indptr,
                    RawDataPtr data) {
        handle_.call_with(PGM_dataset_mutable_add_buffer, get(), component.c_str(), elements_per_scenario,
                          total_elements, indptr, data);
    }

    void add_buffer(std::string const& component, Idx elements_per_scenario, Idx total_elements, Idx const* indptr,
                    Buffer& data) {
        handle_.call_with(PGM_dataset_mutable_add_buffer, get(), component.c_str(), elements_per_scenario,
                          total_elements, indptr, data.get());
    }

    void add_attribute_buffer(std::string const& component, std::string const& attribute, RawDataPtr data) {
        handle_.call_with(PGM_dataset_mutable_add_attribute_buffer, get(), component.c_str(), attribute.c_str(), data);
    }

    void add_attribute_buffer(std::string const& component, std::string const& attribute, Buffer& data) {
        handle_.call_with(PGM_dataset_mutable_add_attribute_buffer, get(), component.c_str(), attribute.c_str(),
                          data.get());
    }

    DatasetInfo const& get_info() const { return info_; }

  private:
    Handle handle_{};
    detail::UniquePtr<RawMutableDataset, &PGM_destroy_dataset_mutable> dataset_;
    DatasetInfo info_;
};

class DatasetConst {
  public:
    DatasetConst(std::string const& dataset, Idx is_batch, Idx batch_size)
        : dataset_{handle_.call_with(PGM_create_dataset_const, dataset.c_str(), is_batch, batch_size)},
          info_{handle_.call_with(PGM_dataset_const_get_info, get())} {}
    DatasetConst(DatasetWritable const& writable_dataset)
        : dataset_{handle_.call_with(PGM_create_dataset_const_from_writable, writable_dataset.get())},
          info_{handle_.call_with(PGM_dataset_const_get_info, get())} {}
    DatasetConst(DatasetMutable const& mutable_dataset)
        : dataset_{handle_.call_with(PGM_create_dataset_const_from_mutable, mutable_dataset.get())},
          info_{handle_.call_with(PGM_dataset_const_get_info, get())} {}

    RawConstDataset const* get() const { return dataset_.get(); }
    RawConstDataset* get() { return dataset_.get(); }

    void add_buffer(std::string const& component, Idx elements_per_scenario, Idx total_elements, Idx const* indptr,
                    RawDataConstPtr data) {
        handle_.call_with(PGM_dataset_const_add_buffer, get(), component.c_str(), elements_per_scenario, total_elements,
                          indptr, data);
    }

    void add_buffer(std::string const& component, Idx elements_per_scenario, Idx total_elements, Idx const* indptr,
                    Buffer const& data) {
        handle_.call_with(PGM_dataset_const_add_buffer, get(), component.c_str(), elements_per_scenario, total_elements,
                          indptr, data.get());
    }

    void add_attribute_buffer(std::string const& component, std::string const& attribute, RawDataConstPtr data) {
        handle_.call_with(PGM_dataset_const_add_attribute_buffer, get(), component.c_str(), attribute.c_str(), data);
    }

    void add_attribute_buffer(std::string const& component, std::string const& attribute, Buffer const& data) {
        handle_.call_with(PGM_dataset_const_add_attribute_buffer, get(), component.c_str(), attribute.c_str(),
                          data.get());
    }

    DatasetInfo const& get_info() const { return info_; }

  private:
    Handle handle_{};
    detail::UniquePtr<RawConstDataset, &PGM_destroy_dataset_const> dataset_;
    DatasetInfo info_;
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_DATASET_HPP
