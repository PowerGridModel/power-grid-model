// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_META_DATA_HPP
#define POWER_GRID_MODEL_CPP_META_DATA_HPP

#include "basics.hpp"
#include "handle.hpp"

#include "power_grid_model_c/meta_data.h"

namespace power_grid_model_cpp {
class MetaData {
  public:
    static Idx n_datasets() { return PGM_meta_n_datasets(nullptr); }

    static MetaDataset const* get_dataset_by_idx(Idx idx) {
        Handle const handle{};
        return handle.call_with(PGM_meta_get_dataset_by_idx, idx);
    }

    static MetaDataset const* get_dataset_by_name(std::string const& dataset) {
        Handle const handle{};
        return handle.call_with(PGM_meta_get_dataset_by_name, dataset.c_str());
    }

    static std::string dataset_name(MetaDataset const* dataset) {
        return std::string{PGM_meta_dataset_name(nullptr, dataset)};
    }

    static Idx n_components(MetaDataset const* dataset) { return PGM_meta_n_components(nullptr, dataset); }

    static MetaComponent const* get_component_by_idx(MetaDataset const* dataset, Idx idx) {
        Handle const handle{};
        return handle.call_with(PGM_meta_get_component_by_idx, dataset, idx);
    }

    static MetaComponent const* get_component_by_name(std::string const& dataset, std::string const& component) {
        Handle const handle{};
        return handle.call_with(PGM_meta_get_component_by_name, dataset.c_str(), component.c_str());
    }

    static std::string component_name(MetaComponent const* component) {
        return std::string{PGM_meta_component_name(nullptr, component)};
    }

    static size_t component_size(MetaComponent const* component) { return PGM_meta_component_size(nullptr, component); }

    static size_t component_alignment(MetaComponent const* component) {
        return PGM_meta_component_alignment(nullptr, component);
    }

    static Idx n_attributes(MetaComponent const* component) { return PGM_meta_n_attributes(nullptr, component); }

    static MetaAttribute const* get_attribute_by_idx(MetaComponent const* component, Idx idx) {
        Handle const handle{};
        return handle.call_with(PGM_meta_get_attribute_by_idx, component, idx);
    }

    static MetaAttribute const* get_attribute_by_name(std::string const& dataset, std::string const& component,
                                                      std::string const& attribute) {
        Handle const handle{};
        return handle.call_with(PGM_meta_get_attribute_by_name, dataset.c_str(), component.c_str(), attribute.c_str());
    }

    static std::string attribute_name(MetaAttribute const* attribute) {
        return std::string{PGM_meta_attribute_name(nullptr, attribute)};
    }

    static PGM_CType attribute_ctype(MetaAttribute const* attribute) {
        return static_cast<PGM_CType>(PGM_meta_attribute_ctype(nullptr, attribute));
    }

    static size_t attribute_offset(MetaAttribute const* attribute) {
        return PGM_meta_attribute_offset(nullptr, attribute);
    }

    static int is_little_endian() { return PGM_is_little_endian(nullptr); }
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_META_DATA_HPP
