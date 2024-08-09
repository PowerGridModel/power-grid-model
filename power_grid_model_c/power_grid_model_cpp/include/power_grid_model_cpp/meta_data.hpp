// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_META_DATA_HPP
#define POWER_GRID_MODEL_CPP_META_DATA_HPP

#include <memory>

#include "meta_data.h"

namespace power_grid_model_cpp {

class Meta {
public:
    static PGM_Idx n_datasets(PGM_Handle* handle) {
        return PGM_meta_n_datasets(handle);
    }
        
    static PGM_MetaDataset const* get_dataset_by_idx(PGM_Handle* handle, PGM_Idx idx) {
        return PGM_meta_get_dataset_by_idx(handle, idx);
    }

    static PGM_MetaDataset const* get_dataset_by_name(PGM_Handle* handle, char const* dataset) {
        return PGM_meta_get_dataset_by_name(handle, dataset);
    }

    static char const* dataset_name(PGM_Handle* handle, PGM_MetaDataset const* dataset) {
        return PGM_meta_dataset_name(handle, dataset);
    }

    static PGM_Idx n_components(PGM_Handle* handle, PGM_MetaDataset const* dataset) {
        return PGM_meta_n_components(handle, dataset);
    }

    static PGM_MetaComponent const* get_component_by_idx(PGM_Handle* handle, PGM_MetaDataset const* dataset,
                                                                PGM_Idx idx) {
        return PGM_meta_get_component_by_idx(handle, dataset, idx);
    }

    static PGM_MetaComponent const* get_component_by_name(PGM_Handle* handle, char const* dataset,
                                                                    char const* component) {
        return PGM_meta_get_component_by_name(handle, dataset, component);
    }

    static char const* component_name(PGM_Handle* handle, PGM_MetaComponent const* component) {
        return PGM_meta_component_name(handle, component);
    }

    static size_t component_size(PGM_Handle* handle, PGM_MetaComponent const* component) {
        return PGM_meta_component_size(handle, component);
    }

    static size_t component_alignment(PGM_Handle* handle, PGM_MetaComponent const* component) {
        return PGM_meta_component_alignment(handle, component);
    }

    static PGM_Idx n_attributes(PGM_Handle* handle, PGM_MetaComponent const* component) {
        return PGM_meta_n_attributes(handle, component);
    }

    static PGM_MetaAttribute const* get_attribute_by_idx(PGM_Handle* handle, PGM_MetaComponent const* component,
                                                                PGM_Idx idx) {
        return PGM_meta_get_attribute_by_idx(handle, component, idx);
    }

    static PGM_MetaAttribute const* get_attribute_by_name(PGM_Handle* handle, char const* dataset,
                                                                    char const* component, char const* attribute) {
        return PGM_meta_get_attribute_by_name(handle, dataset, component, attribute);
    }

    static char const* attribute_name(PGM_Handle* handle, PGM_MetaAttribute const* attribute) {
        return PGM_meta_attribute_name(handle, attribute);
    }

    static PGM_Idx attribute_ctype(PGM_Handle* handle, PGM_MetaAttribute const* attribute) {
        return PGM_meta_attribute_ctype(handle, attribute);
    }

    static size_t attribute_offset(PGM_Handle* handle, PGM_MetaAttribute const* attribute) {
        return PGM_meta_attribute_offset(handle, attribute);
    }

    static int is_little_endian(PGM_Handle* handle) {
        return PGM_is_little_endian(handle);
    }
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_META_DATA_HPP