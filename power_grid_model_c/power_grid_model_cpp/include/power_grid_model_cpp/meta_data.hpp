// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_META_DATA_HPP
#define POWER_GRID_MODEL_CPP_META_DATA_HPP

#include <memory>

#include "meta_data.h"

namespace power_grid_model_cpp {
    
PGM_Idx meta_n_datasets(PGM_Handle* handle) {
    return PGM_meta_n_datasets(handle);
}
    
PGM_MetaDataset const* meta_get_dataset_by_idx(PGM_Handle* handle, PGM_Idx idx) {
    return PGM_meta_get_dataset_by_idx(handle, idx);
}

PGM_MetaDataset const* meta_get_dataset_by_name(PGM_Handle* handle, char const* dataset) {
    return PGM_meta_get_dataset_by_name(handle, dataset);
}

char const* meta_dataset_name(PGM_Handle* handle, PGM_MetaDataset const* dataset) {
    return PGM_meta_dataset_name(handle, dataset);
}

PGM_Idx meta_n_components(PGM_Handle* handle, PGM_MetaDataset const* dataset) {
    return PGM_meta_n_components(handle, dataset);
}

PGM_MetaComponent const* meta_get_component_by_idx(PGM_Handle* handle, PGM_MetaDataset const* dataset,
                                                               PGM_Idx idx) {
    return PGM_meta_get_component_by_idx(handle, dataset, idx);
}

PGM_MetaComponent const* meta_get_component_by_name(PGM_Handle* handle, char const* dataset,
                                                                char const* component) {
    return PGM_meta_get_component_by_name(handle, dataset, component);
}

char const* meta_component_name(PGM_Handle* handle, PGM_MetaComponent const* component) {
    return PGM_meta_component_name(handle, component);
}

size_t meta_component_size(PGM_Handle* handle, PGM_MetaComponent const* component) {
    return PGM_meta_component_size(handle, component);
}

size_t meta_component_alignment(PGM_Handle* handle, PGM_MetaComponent const* component) {
    return PGM_meta_component_alignment(handle, component);
}

PGM_Idx meta_n_attributes(PGM_Handle* handle, PGM_MetaComponent const* component) {
    return PGM_meta_n_attributes(handle, component);
}

PGM_MetaAttribute const* meta_get_attribute_by_idx(PGM_Handle* handle, PGM_MetaComponent const* component,
                                                               PGM_Idx idx) {
    return PGM_meta_get_attribute_by_idx(handle, component, idx);
}

PGM_MetaAttribute const* meta_get_attribute_by_name(PGM_Handle* handle, char const* dataset,
                                                                char const* component, char const* attribute) {
    return PGM_meta_get_attribute_by_name(handle, dataset, component, attribute);
}

char const* meta_attribute_name(PGM_Handle* handle, PGM_MetaAttribute const* attribute) {
    return PGM_meta_attribute_name(handle, attribute);
}

PGM_Idx meta_attribute_ctype(PGM_Handle* handle, PGM_MetaAttribute const* attribute) {
    return PGM_meta_attribute_ctype(handle, attribute);
}

size_t meta_attribute_offset(PGM_Handle* handle, PGM_MetaAttribute const* attribute) {
    return PGM_meta_attribute_offset(handle, attribute);
}

int is_little_endian(PGM_Handle* handle) {
    return PGM_is_little_endian(handle);
}


} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_META_DATA_HPP