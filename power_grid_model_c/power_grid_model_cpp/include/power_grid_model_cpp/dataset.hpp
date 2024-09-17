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

    static std::string name(DatasetInfo const& info) {
        return std::string{info.handle_.call_with(PGM_dataset_info_name, info.info_)};
    }
    std::string name() const { return name(*this); }

    static Idx is_batch(DatasetInfo const& info) {
        return info.handle_.call_with(PGM_dataset_info_is_batch, info.info_);
    }
    Idx is_batch() const { return is_batch(*this); }

    static Idx batch_size(DatasetInfo const& info) {
        return info.handle_.call_with(PGM_dataset_info_batch_size, info.info_);
    }
    Idx batch_size() const { return batch_size(*this); }

    static Idx n_components(DatasetInfo const& info) {
        return info.handle_.call_with(PGM_dataset_info_n_components, info.info_);
    }
    Idx n_components() const { return n_components(*this); }

    static std::string component_name(DatasetInfo const& info, Idx component_idx) {
        return std::string{info.handle_.call_with(PGM_dataset_info_component_name, info.info_, component_idx)};
    }
    std::string component_name(Idx component_idx) const { return component_name(*this, component_idx); }

    static Idx component_elements_per_scenario(DatasetInfo const& info, Idx component_idx) {
        return info.handle_.call_with(PGM_dataset_info_elements_per_scenario, info.info_, component_idx);
    }
    Idx component_elements_per_scenario(Idx component_idx) const {
        return component_elements_per_scenario(*this, component_idx);
    }

    static Idx component_total_elements(DatasetInfo const& info, Idx component_idx) {
        return info.handle_.call_with(PGM_dataset_info_total_elements, info.info_, component_idx);
    }
    Idx component_total_elements(Idx component_idx) const { return component_total_elements(*this, component_idx); }

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

    RawWritableDataset* get() const { return dataset_; }

    static DatasetInfo const& get_info(DatasetWritable const& dataset) { return dataset.info_; }
    DatasetInfo const& get_info() const { return get_info(*this); }

    static void set_buffer(DatasetWritable& dataset, std::string const& component, Idx* indptr, RawDataPtr data) {
        dataset.handle_.call_with(PGM_dataset_writable_set_buffer, dataset.dataset_, component.c_str(), indptr, data);
    }
    void set_buffer(std::string const& component, Idx* indptr, RawDataPtr data) {
        set_buffer(*this, component, indptr, data);
    }

    static void set_buffer(DatasetWritable& dataset, std::string const& component, Idx* indptr, Buffer const& data) {
        dataset.handle_.call_with(PGM_dataset_writable_set_buffer, dataset.dataset_, component.c_str(), indptr,
                                  data.get());
    }
    void set_buffer(std::string const& component, Idx* indptr, Buffer const& data) {
        set_buffer(*this, component, indptr, data);
    }

    static void set_attribute_buffer(DatasetWritable& dataset, std::string const& component,
                                     std::string const& attribute, RawDataPtr data) {
        dataset.handle_.call_with(PGM_dataset_writable_set_attribute_buffer, dataset.dataset_, component.c_str(),
                                  attribute.c_str(), data);
    }
    void set_attribute_buffer(std::string const& component, std::string const& attribute, RawDataPtr data) {
        set_attribute_buffer(*this, component, attribute, data);
    }

    static void set_attribute_buffer(DatasetWritable& dataset, std::string const& component,
                                     std::string const& attribute, Buffer const& data) {
        dataset.handle_.call_with(PGM_dataset_writable_set_attribute_buffer, dataset.dataset_, component.c_str(),
                                  attribute.c_str(), data.get());
    }
    void set_attribute_buffer(std::string const& component, std::string const& attribute, Buffer const& data) {
        set_attribute_buffer(*this, component, attribute, data);
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
          info_{handle_.call_with(PGM_dataset_mutable_get_info, dataset_.get())} {}

    RawMutableDataset* get() const { return dataset_.get(); }

    static void add_buffer(DatasetMutable const& dataset, std::string const& component, Idx elements_per_scenario,
                           Idx total_elements, Idx const* indptr, RawDataPtr data) {
        dataset.handle_.call_with(PGM_dataset_mutable_add_buffer, dataset.dataset_.get(), component.c_str(),
                                  elements_per_scenario, total_elements, indptr, data);
    }
    void add_buffer(std::string const& component, Idx elements_per_scenario, Idx total_elements, Idx const* indptr,
                    RawDataPtr data) { // NOSONAR: no const
        add_buffer(*this, component, elements_per_scenario, total_elements, indptr, data);
    }

    static void add_buffer(DatasetMutable const& dataset, std::string const& component, Idx elements_per_scenario,
                           Idx total_elements, Idx const* indptr, Buffer const& data) {
        dataset.handle_.call_with(PGM_dataset_mutable_add_buffer, dataset.dataset_.get(), component.c_str(),
                                  elements_per_scenario, total_elements, indptr, data.get());
    }
    void add_buffer(std::string const& component, Idx elements_per_scenario, Idx total_elements, Idx const* indptr,
                    Buffer const& data) { // NOSONAR: no const
        add_buffer(*this, component, elements_per_scenario, total_elements, indptr, data);
    }

    static void add_attribute_buffer(DatasetMutable const& dataset, std::string const& component,
                                     std::string const& attribute, RawDataPtr data) {
        dataset.handle_.call_with(PGM_dataset_mutable_add_attribute_buffer, dataset.dataset_.get(), component.c_str(),
                                  attribute.c_str(), data);
    }
    void add_attribute_buffer(std::string const& component, std::string const& attribute,
                              RawDataPtr data) { // NOSONAR: no const
        add_attribute_buffer(*this, component, attribute, data);
    }

    static void add_attribute_buffer(DatasetMutable const& dataset, std::string const& component,
                                     std::string const& attribute, Buffer const& data) {
        dataset.handle_.call_with(PGM_dataset_mutable_add_attribute_buffer, dataset.dataset_.get(), component.c_str(),
                                  attribute.c_str(), data.get());
    }
    void add_attribute_buffer(std::string const& component, std::string const& attribute,
                              Buffer const& data) { // NOSONAR: no const: no const
        add_attribute_buffer(*this, component, attribute, data);
    }

    static DatasetInfo const& get_info(DatasetMutable const& dataset) { return dataset.info_; }
    DatasetInfo const& get_info() const { return get_info(*this); }

  private:
    Handle handle_{};
    detail::UniquePtr<RawMutableDataset, &PGM_destroy_dataset_mutable> dataset_;
    DatasetInfo info_;
};

class DatasetConst {
  public:
    DatasetConst(std::string const& dataset, Idx is_batch, Idx batch_size)
        : dataset_{handle_.call_with(PGM_create_dataset_const, dataset.c_str(), is_batch, batch_size)},
          info_{handle_.call_with(PGM_dataset_const_get_info, dataset_.get())} {}
    DatasetConst(DatasetWritable const& writable_dataset)
        : dataset_{handle_.call_with(PGM_create_dataset_const_from_writable, writable_dataset.get())},
          info_{handle_.call_with(PGM_dataset_const_get_info, dataset_.get())} {}
    DatasetConst(DatasetMutable const& mutable_dataset)
        : dataset_{handle_.call_with(PGM_create_dataset_const_from_mutable, mutable_dataset.get())},
          info_{handle_.call_with(PGM_dataset_const_get_info, dataset_.get())} {}

    RawConstDataset* get() const { return dataset_.get(); }

    static void add_buffer(DatasetConst const& dataset, std::string const& component, Idx elements_per_scenario,
                           Idx total_elements, Idx const* indptr, RawDataConstPtr data) {
        dataset.handle_.call_with(PGM_dataset_const_add_buffer, dataset.dataset_.get(), component.c_str(),
                                  elements_per_scenario, total_elements, indptr, data);
    }
    void add_buffer(std::string const& component, Idx elements_per_scenario, Idx total_elements, Idx const* indptr,
                    RawDataConstPtr data) { // NOSONAR: no const
        add_buffer(*this, component, elements_per_scenario, total_elements, indptr, data);
    }

    static void add_buffer(DatasetConst const& dataset, std::string const& component, Idx elements_per_scenario,
                           Idx total_elements, Idx const* indptr, Buffer const& data) {
        dataset.handle_.call_with(PGM_dataset_const_add_buffer, dataset.dataset_.get(), component.c_str(),
                                  elements_per_scenario, total_elements, indptr, data.get());
    }
    void add_buffer(std::string const& component, Idx elements_per_scenario, Idx total_elements, Idx const* indptr,
                    Buffer const& data) { // NOSONAR: no const
        add_buffer(*this, component, elements_per_scenario, total_elements, indptr, data);
    }

    static void add_attribute_buffer(DatasetConst const& dataset, std::string const& component,
                                     std::string const& attribute, RawDataConstPtr data) {
        dataset.handle_.call_with(PGM_dataset_const_add_attribute_buffer, dataset.dataset_.get(), component.c_str(),
                                  attribute.c_str(), data);
    }
    void add_attribute_buffer(std::string const& component, std::string const& attribute,
                              RawDataConstPtr data) { // NOSONAR: no const
        add_attribute_buffer(*this, component, attribute, data);
    }

    static void add_attribute_buffer(DatasetConst const& dataset, std::string const& component,
                                     std::string const& attribute, Buffer const& data) {
        dataset.handle_.call_with(PGM_dataset_const_add_attribute_buffer, dataset.dataset_.get(), component.c_str(),
                                  attribute.c_str(), data.get());
    }
    void add_attribute_buffer(std::string const& component, std::string const& attribute,
                              Buffer const& data) { // NOSONAR: no const
        add_attribute_buffer(*this, component, attribute, data);
    }

    static DatasetInfo const& get_info(DatasetConst const& dataset) { return dataset.info_; }
    DatasetInfo const& get_info() const { return get_info(*this); }

  private:
    Handle handle_{};
    detail::UniquePtr<RawConstDataset, &PGM_destroy_dataset_const> dataset_;
    DatasetInfo info_;
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_DATASET_HPP
