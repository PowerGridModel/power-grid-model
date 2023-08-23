// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_DLL_EXPORTS
#include "forward_declaration.hpp"

#include "handle.hpp"
#include "power_grid_model_c/dataset.h"

#include <power_grid_model/auxiliary/dataset_handler.hpp>
#include <power_grid_model/auxiliary/meta_data.hpp>

using namespace power_grid_model::meta_data;

char const* PGM_dataset_info_name(PGM_Handle* /*unused*/, PGM_DatasetInfo const* info) {
    return info->dataset->name.c_str();
}

PGM_Idx PGM_dataset_info_is_batch(PGM_Handle* /*unused*/, PGM_DatasetInfo const* info) {
    return static_cast<PGM_Idx>(info->is_batch);
}

PGM_Idx PGM_dataset_info_batch_size(PGM_Handle* /*unused*/, PGM_DatasetInfo const* info) { return info->batch_size; }

PGM_Idx PGM_dataset_info_n_components(PGM_Handle* /*unused*/, PGM_DatasetInfo const* info) {
    return static_cast<PGM_Idx>(info->component_info.size());
}

char const* PGM_dataset_info_component_name(PGM_Handle* /*unused*/, PGM_DatasetInfo const* info,
                                            PGM_Idx component_idx) {
    return info->component_info[component_idx].component->name.c_str();
}

PGM_Idx PGM_dataset_info_elements_per_scenario(PGM_Handle* /*unused*/, PGM_DatasetInfo const* info,
                                               PGM_Idx component_idx) {
    return info->component_info[component_idx].elements_per_scenario;
}

PGM_Idx PGM_dataset_info_total_elements(PGM_Handle* /*unused*/, PGM_DatasetInfo const* info, PGM_Idx component_idx) {
    return info->component_info[component_idx].total_elements;
}

PGM_ConstDataset* PGM_create_const_dataset(PGM_Handle* handle, char const* dataset, PGM_Idx is_batch,
                                           PGM_Idx batch_size) {
    return call_with_catch(
        handle,
        [&]() {
            return new ConstDatasetHandler{static_cast<bool>(is_batch), batch_size, dataset};
        },
        PGM_regular_error);
}

void PGM_destroy_const_dataset(PGM_ConstDataset* dataset) { delete dataset; }

void PGM_const_dataset_add_buffer(PGM_Handle* handle, PGM_ConstDataset* dataset, char const* component,
                                  PGM_Idx elements_per_scenario, PGM_Idx total_elements, PGM_Idx const* indptr,
                                  void const* data) {
    call_with_catch(
        handle, [&]() { dataset->add_buffer(component, elements_per_scenario, total_elements, indptr, data); },
        PGM_regular_error);
}

PGM_DatasetInfo const* PGM_const_dataset_get_info(PGM_Handle* /*unused*/, PGM_ConstDataset const* dataset) {
    return &dataset->get_description();
}

PGM_DatasetInfo const* PGM_writable_dataset_get_info(PGM_Handle* /*unused*/, PGM_WritableDataset const* dataset) {
    return &dataset->get_description();
}

void PGM_writable_dataset_set_buffer(PGM_Handle* handle, PGM_WritableDataset* dataset, char const* component,
                                     PGM_Idx* indptr, void* data) {
    call_with_catch(
        handle, [&]() { dataset->set_buffer(component, indptr, data); }, PGM_regular_error);
}
