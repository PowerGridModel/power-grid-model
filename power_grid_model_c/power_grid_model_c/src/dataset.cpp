// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_DLL_EXPORTS
#include "forward_declaration.hpp"

#include "power_grid_model_c/dataset.h"

#include <power_grid_model/auxiliary/dataset_handler.hpp>
#include <power_grid_model/auxiliary/meta_data.hpp>

char const* PGM_dataset_info_name(PGM_Handle*, PGM_DatasetInfo const* info) { return info->dataset->name.c_str(); }

PGM_Idx PGM_dataset_info_is_batch(PGM_Handle*, PGM_DatasetInfo const* info) { return info->is_batch; }

PGM_Idx PGM_dataset_info_batch_size(PGM_Handle*, PGM_DatasetInfo const* info) { return info->batch_size; }

PGM_Idx PGM_dataset_info_n_components(PGM_Handle*, PGM_DatasetInfo const* info) {
    return info->dataset->n_components();
}

char const* PGM_dataset_info_component_name(PGM_Handle*, PGM_DatasetInfo const* info, PGM_Idx component_idx) {
    return info->component_info[component_idx].component->name.c_str();
}

PGM_Idx PGM_dataset_info_elements_per_scenario(PGM_Handle*, PGM_DatasetInfo const* info, PGM_Idx component_idx) {
    return info->component_info[component_idx].elements_per_scenario;
}

PGM_Idx PGM_dataset_info_total_elements(PGM_Handle*, PGM_DatasetInfo const* info, PGM_Idx component_idx) {
    return info->component_info[component_idx].total_elements;
}
