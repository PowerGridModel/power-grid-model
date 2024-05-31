// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_DLL_EXPORTS
#include "forward_declarations.hpp"

#include "get_meta_data.hpp"
#include "handle.hpp"
#include "power_grid_model_c/dataset.h"

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/auxiliary/meta_data.hpp>

using namespace power_grid_model;
using namespace power_grid_model::meta_data;

// dataset info

char const* PGM_dataset_info_name(PGM_Handle* /*unused*/, PGM_DatasetInfo const* info) { return info->dataset->name; }

PGM_Idx PGM_dataset_info_is_batch(PGM_Handle* /*unused*/, PGM_DatasetInfo const* info) {
    return static_cast<PGM_Idx>(info->is_batch);
}

PGM_Idx PGM_dataset_info_batch_size(PGM_Handle* /*unused*/, PGM_DatasetInfo const* info) { return info->batch_size; }

PGM_Idx PGM_dataset_info_n_components(PGM_Handle* /*unused*/, PGM_DatasetInfo const* info) {
    return static_cast<PGM_Idx>(info->component_info.size());
}

char const* PGM_dataset_info_component_name(PGM_Handle* /*unused*/, PGM_DatasetInfo const* info,
                                            PGM_Idx component_idx) {
    return info->component_info[component_idx].component->name;
}

PGM_Idx PGM_dataset_info_elements_per_scenario(PGM_Handle* /*unused*/, PGM_DatasetInfo const* info,
                                               PGM_Idx component_idx) {
    return info->component_info[component_idx].elements_per_scenario;
}

PGM_Idx PGM_dataset_info_total_elements(PGM_Handle* /*unused*/, PGM_DatasetInfo const* info, PGM_Idx component_idx) {
    return info->component_info[component_idx].total_elements;
}

// const dataset

PGM_ConstDataset* PGM_create_dataset_const(PGM_Handle* handle, char const* dataset, PGM_Idx is_batch,
                                           PGM_Idx batch_size) {
    return call_with_catch(
        handle,
        [dataset, is_batch, batch_size]() {
            return new ConstDataset{static_cast<bool>(is_batch), batch_size, dataset, get_meta_data()};
        },
        PGM_regular_error);
}

PGM_ConstDataset* PGM_create_dataset_const_from_writable(PGM_Handle* handle,
                                                         PGM_WritableDataset const* writable_dataset) {
    return call_with_catch(
        handle, [writable_dataset]() { return new ConstDataset{*writable_dataset}; }, PGM_regular_error);
}

PGM_ConstDataset* PGM_create_dataset_const_from_mutable(PGM_Handle* handle, PGM_MutableDataset const* mutable_dataset) {
    return call_with_catch(
        handle, [mutable_dataset]() { return new ConstDataset{*mutable_dataset}; }, PGM_regular_error);
}

void PGM_destroy_dataset_const(PGM_ConstDataset* dataset) { delete dataset; }

void PGM_dataset_const_add_buffer(PGM_Handle* handle, PGM_ConstDataset* dataset, char const* component,
                                  PGM_Idx elements_per_scenario, PGM_Idx total_elements, PGM_Idx const* indptr,
                                  void const* data) {
    call_with_catch(
        handle,
        [dataset, component, elements_per_scenario, total_elements, indptr, data]() {
            dataset->add_buffer(component, elements_per_scenario, total_elements, indptr, data);
        },
        PGM_regular_error);
}

PGM_DatasetInfo const* PGM_dataset_const_get_info(PGM_Handle* /*unused*/, PGM_ConstDataset const* dataset) {
    return &dataset->get_description();
}

// writable dataset

PGM_DatasetInfo const* PGM_dataset_writable_get_info(PGM_Handle* /*unused*/, PGM_WritableDataset const* dataset) {
    return &dataset->get_description();
}

void PGM_dataset_writable_set_buffer(PGM_Handle* handle, PGM_WritableDataset* dataset, char const* component,
                                     PGM_Idx* indptr, void* data) {
    call_with_catch(
        handle, [dataset, component, indptr, data]() { dataset->set_buffer(component, indptr, data); },
        PGM_regular_error);
}

// mutable dataset

PGM_MutableDataset* PGM_create_dataset_mutable(PGM_Handle* handle, char const* dataset, PGM_Idx is_batch,
                                               PGM_Idx batch_size) {
    return call_with_catch(
        handle,
        [dataset, is_batch, batch_size]() {
            return new MutableDataset{static_cast<bool>(is_batch), batch_size, dataset, get_meta_data()};
        },
        PGM_regular_error);
}

void PGM_destroy_dataset_mutable(PGM_MutableDataset* dataset) { delete dataset; }

void PGM_dataset_mutable_add_buffer(PGM_Handle* handle, PGM_MutableDataset* dataset, char const* component,
                                    PGM_Idx elements_per_scenario, PGM_Idx total_elements, PGM_Idx const* indptr,
                                    void* data) {
    call_with_catch(
        handle,
        [dataset, component, elements_per_scenario, total_elements, indptr, data]() {
            dataset->add_buffer(component, elements_per_scenario, total_elements, indptr, data);
        },
        PGM_regular_error);
}

PGM_DatasetInfo const* PGM_dataset_mutable_get_info(PGM_Handle* /*unused*/, PGM_MutableDataset const* dataset) {
    return &dataset->get_description();
}
