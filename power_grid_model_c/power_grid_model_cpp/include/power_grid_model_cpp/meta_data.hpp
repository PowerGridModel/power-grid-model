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
    static Idx n_datasets(Handle const& handle) {
        auto const n_datasets = PGM_meta_n_datasets(handle.get());
        handle.check_error();
        return n_datasets;
    }
    Idx n_datasets() const { return n_datasets(handle_); }

    static MetaDataset const* get_dataset_by_idx(Handle const& handle, Idx idx) {
        auto const dataset = PGM_meta_get_dataset_by_idx(handle.get(), idx);
        handle.check_error();
        return dataset;
    }
    MetaDataset const* get_dataset_by_idx(Idx idx) const { return get_dataset_by_idx(handle_, idx); }

    static MetaDataset const* get_dataset_by_name(Handle const& handle, std::string const& dataset) {
        auto const dataset_aux = PGM_meta_get_dataset_by_name(handle.get(), dataset.c_str());
        handle.check_error();
        return dataset_aux;
    }
    MetaDataset const* get_dataset_by_name(std::string const& dataset) const {
        return get_dataset_by_name(handle_, dataset.c_str());
    }

    static std::string const dataset_name(Handle const& handle, MetaDataset const* dataset) {
        auto const name = std::string(PGM_meta_dataset_name(handle.get(), dataset));
        handle.check_error();
        return name;
    }
    std::string const dataset_name(MetaDataset const* dataset) const { return dataset_name(handle_, dataset); }

    static Idx n_components(Handle const& handle, MetaDataset const* dataset) {
        auto const n_components = PGM_meta_n_components(handle.get(), dataset);
        handle.check_error();
        return n_components;
    }
    Idx n_components(MetaDataset const* dataset) const { return n_components(handle_, dataset); }

    static MetaComponent const* get_component_by_idx(Handle const& handle, MetaDataset const* dataset, Idx idx) {
        auto const component = PGM_meta_get_component_by_idx(handle.get(), dataset, idx);
        handle.check_error();
        return component;
    }
    MetaComponent const* get_component_by_idx(MetaDataset const* dataset, Idx idx) const {
        return get_component_by_idx(handle_, dataset, idx);
    }

    static MetaComponent const* get_component_by_name(Handle const& handle, std::string const& dataset,
                                                      std::string const& component) {
        auto const component_aux = PGM_meta_get_component_by_name(handle.get(), dataset.c_str(), component.c_str());
        handle.check_error();
        return component_aux;
    }
    MetaComponent const* get_component_by_name(std::string const& dataset, std::string const& component) const {
        return get_component_by_name(handle_, dataset.c_str(), component.c_str());
    }

    static std::string const component_name(Handle const& handle, MetaComponent const* component) {
        auto const component_aux = std::string(PGM_meta_component_name(handle.get(), component));
        handle.check_error();
        return component_aux;
    }
    std::string const component_name(MetaComponent const* component) const {
        return component_name(handle_, component);
    }

    static size_t component_size(Handle const& handle, MetaComponent const* component) {
        auto const size = PGM_meta_component_size(handle.get(), component);
        handle.check_error();
        return size;
    }
    size_t component_size(MetaComponent const* component) const { return component_size(handle_, component); }

    static size_t component_alignment(Handle const& handle, MetaComponent const* component) {
        auto const alignment = PGM_meta_component_alignment(handle.get(), component);
        handle.check_error();
        return alignment;
    }
    size_t component_alignment(MetaComponent const* component) const { return component_alignment(handle_, component); }

    static Idx n_attributes(Handle const& handle, MetaComponent const* component) {
        auto const n_attributes = PGM_meta_n_attributes(handle.get(), component);
        handle.check_error();
        return n_attributes;
    }
    Idx n_attributes(MetaComponent const* component) const { return n_attributes(handle_, component); }

    static MetaAttribute const* get_attribute_by_idx(Handle const& handle, MetaComponent const* component, Idx idx) {
        auto const attribute = PGM_meta_get_attribute_by_idx(handle.get(), component, idx);
        handle.check_error();
        return attribute;
    }
    MetaAttribute const* get_attribute_by_idx(MetaComponent const* component, Idx idx) const {
        return get_attribute_by_idx(handle_, component, idx);
    }

    static MetaAttribute const* get_attribute_by_name(Handle const& handle, std::string const& dataset,
                                                      std::string const& component, std::string const& attribute) {
        auto const attribute_aux =
            PGM_meta_get_attribute_by_name(handle.get(), dataset.c_str(), component.c_str(), attribute.c_str());
        handle.check_error();
        return attribute_aux;
    }
    MetaAttribute const* get_attribute_by_name(std::string const& dataset, std::string const& component,
                                               std::string const& attribute) const {
        return get_attribute_by_name(handle_, dataset.c_str(), component.c_str(), attribute.c_str());
    }

    static std::string const attribute_name(Handle const& handle, MetaAttribute const* attribute) {
        auto const attribute_aux = std::string(PGM_meta_attribute_name(handle.get(), attribute));
        handle.check_error();
        return attribute_aux;
    }
    std::string const attribute_name(MetaAttribute const* attribute) const {
        return attribute_name(handle_, attribute);
    }

    static Idx attribute_ctype(Handle const& handle, MetaAttribute const* attribute) {
        auto const ctype = PGM_meta_attribute_ctype(handle.get(), attribute);
        handle.check_error();
        return ctype;
    }
    Idx attribute_ctype(MetaAttribute const* attribute) const { return attribute_ctype(handle_, attribute); }

    static size_t attribute_offset(Handle const& handle, MetaAttribute const* attribute) {
        auto const offset = PGM_meta_attribute_offset(handle.get(), attribute);
        handle.check_error();
        return offset;
    }
    size_t attribute_offset(MetaAttribute const* attribute) const { return attribute_offset(handle_, attribute); }

    static int is_little_endian(Handle const& handle) {
        auto const is_little_endian = PGM_is_little_endian(handle.get());
        handle.check_error();
        return is_little_endian;
    }
    int is_little_endian() const { return is_little_endian(handle_); }

  private:
    Handle handle_{};
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_META_DATA_HPP
