// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_DATASET_HPP
#define POWER_GRID_MODEL_CPP_DATASET_HPP

#include <memory>

#include "dataset.h"
#include "handle.hpp"

namespace power_grid_model_cpp {
class Dataset {
public:
    power_grid_model_cpp::Handle handle;

    virtual ~Dataset() = default;

    static char const* info_name(PGM_Handle* provided_handle, PGM_DatasetInfo const* info) {
        return PGM_dataset_info_name(provided_handle, info);
    }
    char const* info_name(PGM_DatasetInfo const* info) const {
        return PGM_dataset_info_name(handle.get(), info);
    }
    
    static PGM_Idx info_is_batch(PGM_Handle* provided_handle, PGM_DatasetInfo const* info) {
        return PGM_dataset_info_is_batch(provided_handle, info);
    }
    PGM_Idx info_is_batch(PGM_DatasetInfo const* info) const {
        return PGM_dataset_info_is_batch(handle.get(), info);
    }

    static PGM_Idx info_batch_size(PGM_Handle* provided_handle, PGM_DatasetInfo const* info){
        return PGM_dataset_info_batch_size(provided_handle, info);
    }
    PGM_Idx info_batch_size(PGM_DatasetInfo const* info) const {
        return PGM_dataset_info_batch_size(handle.get(), info);
    }

    static PGM_Idx info_n_components(PGM_Handle* provided_handle, PGM_DatasetInfo const* info){
        return PGM_dataset_info_n_components(provided_handle, info);
    }
    PGM_Idx info_n_components(PGM_DatasetInfo const* info) const {
        return PGM_dataset_info_n_components(handle.get(), info);
    }

    static char const* info_component_name(PGM_Handle* provided_handle, PGM_DatasetInfo const* info,
                                                    PGM_Idx component_idx){
        return PGM_dataset_info_component_name(provided_handle, info, component_idx);
    }
    char const* info_component_name(PGM_DatasetInfo const* info, PGM_Idx component_idx) const {
        return PGM_dataset_info_component_name(handle.get(), info, component_idx);
    }

    static PGM_Idx info_elements_per_scenario(PGM_Handle* provided_handle, PGM_DatasetInfo const* info,
                                                    PGM_Idx component_idx){
        return PGM_dataset_info_elements_per_scenario(provided_handle, info, component_idx);
    }
    PGM_Idx info_elements_per_scenario(PGM_DatasetInfo const* info, PGM_Idx component_idx) const {
        return PGM_dataset_info_elements_per_scenario(handle.get(), info, component_idx);
    }

    static PGM_Idx info_total_elements(PGM_Handle* provided_handle, PGM_DatasetInfo const* info,
                                                    PGM_Idx component_idx){
        return PGM_dataset_info_total_elements(provided_handle, info, component_idx);
    }
    PGM_Idx info_total_elements(PGM_DatasetInfo const* info, PGM_Idx component_idx) const {
        return PGM_dataset_info_total_elements(handle.get(), info, component_idx);
    }
protected:
    Dataset() : handle() {};
};

class DatasetConst : public Dataset {
public:
    DatasetConst(char const* dataset, PGM_Idx is_batch, PGM_Idx batch_size)
        : Dataset(),
          dataset_{PGM_create_dataset_const(handle.get(), dataset, is_batch, batch_size), details::DeleterFunctor<&PGM_destroy_dataset_const>()} {}
    DatasetConst(PGM_WritableDataset const* writable_dataset)
        : Dataset(),
          dataset_{PGM_create_dataset_const_from_writable(handle.get(), writable_dataset), details::DeleterFunctor<&PGM_destroy_dataset_const>()} {}
    DatasetConst(PGM_MutableDataset const* mutable_dataset)
        : Dataset(),
          dataset_{PGM_create_dataset_const_from_mutable(handle.get(), mutable_dataset), details::DeleterFunctor<&PGM_destroy_dataset_const>()} {}

    ~DatasetConst() = default;

    static void add_buffer(PGM_Handle* provided_handle, PGM_ConstDataset* dataset, char const* component,
                            PGM_Idx elements_per_scenario, PGM_Idx total_elements, PGM_Idx const* indptr,
                            void const* data) {
        PGM_dataset_const_add_buffer(provided_handle, dataset, component,
                            elements_per_scenario, total_elements, indptr, data);
    }
    void add_buffer(char const* component, PGM_Idx elements_per_scenario, PGM_Idx total_elements, PGM_Idx const* indptr,
                            void const* data) {
        PGM_dataset_const_add_buffer(handle.get(), dataset_.get(), component,
                            elements_per_scenario, total_elements, indptr, data);
    }

    static void get_info(PGM_Handle* provided_handle, PGM_ConstDataset const* dataset) {
        PGM_dataset_const_get_info(provided_handle, dataset);
    }
    void get_info() const {
        PGM_dataset_const_get_info(handle.get(), dataset_.get());
    }

private:
    std::unique_ptr<PGM_ConstDataset, details::DeleterFunctor<&PGM_destroy_dataset_const>> dataset_;

};

class DatasetWritable : public Dataset {
public:
    DatasetWritable() : Dataset() {}

    ~DatasetWritable() = default;

    static PGM_DatasetInfo const* get_info(PGM_Handle* provided_handle, PGM_WritableDataset const* dataset) {
        return PGM_dataset_writable_get_info(provided_handle, dataset);
    }
    PGM_DatasetInfo const* get_info() const {
        return PGM_dataset_writable_get_info(handle.get(), dataset_.get());
    }

    static void set_buffer(PGM_Handle* provided_handle, PGM_WritableDataset* dataset, char const* component, PGM_Idx* indptr, void* data) {
        PGM_dataset_writable_set_buffer(provided_handle, dataset, component, indptr, data);
    }
    void set_buffer(char const* component, PGM_Idx* indptr, void* data) {
        PGM_dataset_writable_set_buffer(handle.get(), dataset_.get(), component, indptr, data);
    }

    void set_dataset(PGM_WritableDataset* dataset) { // Why isn't there a constructor/destructor in the C api?
        dataset_ = std::unique_ptr<PGM_WritableDataset>(dataset);
    }

private:
    std::unique_ptr<PGM_WritableDataset> dataset_;

};

class DatasetMutable : public Dataset {
    DatasetMutable(char const* dataset, PGM_Idx is_batch, PGM_Idx batch_size)
        : Dataset(), dataset_{
            PGM_create_dataset_mutable(handle.get(), dataset, is_batch, batch_size),
            details::DeleterFunctor<&PGM_destroy_dataset_mutable>()} {}

    ~DatasetMutable() = default;

    static void add_buffer(PGM_Handle* provided_handle, PGM_MutableDataset* dataset, char const* component,
                                            PGM_Idx elements_per_scenario, PGM_Idx total_elements,
                                            PGM_Idx const* indptr, void* data) {
        PGM_dataset_mutable_add_buffer(provided_handle, dataset, component, elements_per_scenario,
                                        total_elements, indptr, data);
    }
    void add_buffer(char const* component, PGM_Idx elements_per_scenario, PGM_Idx total_elements,
                    PGM_Idx const* indptr, void* data) {
        PGM_dataset_mutable_add_buffer(handle.get(), dataset_.get(), component, elements_per_scenario,
                                        total_elements, indptr, data);
    }

    static PGM_DatasetInfo const* get_info(PGM_Handle* provided_handle, PGM_MutableDataset const* dataset){
        PGM_dataset_mutable_get_info(provided_handle, dataset);
    }
    PGM_DatasetInfo const* get_info() const {
        PGM_dataset_mutable_get_info(handle.get(), dataset_.get());
    }

private:
    std::unique_ptr<PGM_MutableDataset, details::DeleterFunctor<&PGM_destroy_dataset_mutable>> dataset_;
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_DATASET_HPP