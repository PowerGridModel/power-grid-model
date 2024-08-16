// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_META_DATA_HPP
#define POWER_GRID_MODEL_CPP_META_DATA_HPP

#include "power_grid_model_c/meta_data.h"

#include "basics.hpp"
#include "handle.hpp"

namespace power_grid_model_cpp {
class MetaData {
  public:
    static Idx _n_datasets() {
        Handle handle{};
        auto const n_datasets = PGM_meta_n_datasets(handle.get());
        handle.check_error();
        return n_datasets;
    }

    static MetaDataset const* _get_dataset_by_idx(Idx idx) {
        Handle handle{};
        auto const dataset = PGM_meta_get_dataset_by_idx(handle.get(), idx);
        handle.check_error();
        return dataset;
    }

    static MetaDataset const* _get_dataset_by_name(std::string const& dataset) {
        Handle handle{};
        auto const dataset_aux = PGM_meta_get_dataset_by_name(handle.get(), dataset.c_str());
        handle.check_error();
        return dataset_aux;
    }

    static std::string const _dataset_name(MetaDataset const* dataset) {
        Handle handle{};
        auto const name = std::string(PGM_meta_dataset_name(handle.get(), dataset));
        handle.check_error();
        return name;
    }

    static Idx _n_components(MetaDataset const* dataset) {
        Handle handle{};
        auto const n_components = PGM_meta_n_components(handle.get(), dataset);
        handle.check_error();
        return n_components;
    }

    static MetaComponent const* _get_component_by_idx(MetaDataset const* dataset, Idx idx) {
        Handle handle{};
        auto const component = PGM_meta_get_component_by_idx(handle.get(), dataset, idx);
        handle.check_error();
        return component;
    }

    static MetaComponent const* _get_component_by_name(std::string const& dataset, std::string const& component) {
        Handle handle{};
        auto const component_aux = PGM_meta_get_component_by_name(handle.get(), dataset.c_str(), component.c_str());
        handle.check_error();
        return component_aux;
    }

    static std::string const _component_name(MetaComponent const* component) {
        Handle handle{};
        auto const component_aux = std::string(PGM_meta_component_name(handle.get(), component));
        handle.check_error();
        return component_aux;
    }

    static size_t _component_size(MetaComponent const* component) {
        Handle handle{};
        auto const size = PGM_meta_component_size(handle.get(), component);
        handle.check_error();
        return size;
    }

    static size_t _component_alignment(MetaComponent const* component) {
        Handle handle{};
        auto const alignment = PGM_meta_component_alignment(handle.get(), component);
        handle.check_error();
        return alignment;
    }

    static Idx _n_attributes(MetaComponent const* component) {
        Handle handle{};
        auto const n_attributes = PGM_meta_n_attributes(handle.get(), component);
        handle.check_error();
        return n_attributes;
    }

    static MetaAttribute const* _get_attribute_by_idx(MetaComponent const* component, Idx idx) {
        Handle handle{};
        auto const attribute = PGM_meta_get_attribute_by_idx(handle.get(), component, idx);
        handle.check_error();
        return attribute;
    }

    static MetaAttribute const* _get_attribute_by_name(std::string const& dataset, std::string const& component,
                                                       std::string const& attribute) {
        Handle handle{};
        auto const attribute_aux =
            PGM_meta_get_attribute_by_name(handle.get(), dataset.c_str(), component.c_str(), attribute.c_str());
        handle.check_error();
        return attribute_aux;
    }

    static std::string const _attribute_name(MetaAttribute const* attribute) {
        Handle handle{};
        auto const attribute_aux = std::string(PGM_meta_attribute_name(handle.get(), attribute));
        handle.check_error();
        return attribute_aux;
    }

    static Idx _attribute_ctype(MetaAttribute const* attribute) {
        Handle handle{};
        auto const ctype = PGM_meta_attribute_ctype(handle.get(), attribute);
        handle.check_error();
        return ctype;
    }

    static size_t _attribute_offset(MetaAttribute const* attribute) {
        Handle handle{};
        auto const offset = PGM_meta_attribute_offset(handle.get(), attribute);
        handle.check_error();
        return offset;
    }

    static int _is_little_endian() {
        Handle handle{};
        auto const is_little_endian = PGM_is_little_endian(handle.get());
        handle.check_error();
        return is_little_endian;
    }

    Handle handle_{};
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_META_DATA_HPP
