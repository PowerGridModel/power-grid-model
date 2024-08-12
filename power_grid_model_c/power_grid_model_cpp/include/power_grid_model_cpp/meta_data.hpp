// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_META_DATA_HPP
#define POWER_GRID_MODEL_CPP_META_DATA_HPP

#include "meta_data.h"

#include "basics.hpp"
#include "handle.hpp"

namespace power_grid_model_cpp {
class MetaData {
  public:
    MetaData() = default;

    ~MetaData() = default;

    static Idx n_datasets(Handle const& handle) {
        auto const n_datasets = PGM_meta_n_datasets(handle.get());
        handle.check_error();
        return n_datasets;
    }
    Idx n_datasets() const {
        auto const n_datasets = PGM_meta_n_datasets(handle_.get());
        handle_.check_error();
        return n_datasets;
    }

    static PGM_MetaDataset const* get_dataset_by_idx(Handle const& handle, Idx idx) {
        auto const dataset = PGM_meta_get_dataset_by_idx(handle.get(), idx);
        handle.check_error();
        return dataset;
    }
    PGM_MetaDataset const* get_dataset_by_idx(Idx idx) const {
        auto const dataset = PGM_meta_get_dataset_by_idx(handle_.get(), idx);
        handle_.check_error();
        return dataset;
    }

    static PGM_MetaDataset const* get_dataset_by_name(Handle const& handle, std::string const& dataset) {
        auto const dataset_aux = PGM_meta_get_dataset_by_name(handle.get(), dataset.c_str());
        handle.check_error();
        return dataset_aux;
    }
    PGM_MetaDataset const* get_dataset_by_name(std::string const& dataset) const {
        auto const dataset_aux = PGM_meta_get_dataset_by_name(handle_.get(), dataset.c_str());
        handle_.check_error();
        return dataset_aux;
    }

    static std::string const dataset_name(Handle const& handle, PGM_MetaDataset const* dataset) {
        auto const name = std::string(PGM_meta_dataset_name(handle.get(), dataset));
        handle.check_error();
        return name;
    }
    std::string const dataset_name(PGM_MetaDataset const* dataset) const {
        auto const name = std::string(PGM_meta_dataset_name(handle_.get(), dataset));
        handle_.check_error();
        return name;
    }

    static Idx n_components(Handle const& handle, PGM_MetaDataset const* dataset) {
        auto const n_components = PGM_meta_n_components(handle.get(), dataset);
        handle.check_error();
        return n_components;
    }
    Idx n_components(PGM_MetaDataset const* dataset) const {
        auto const n_components = PGM_meta_n_components(handle_.get(), dataset);
        handle_.check_error();
        return n_components;
    }

    static PGM_MetaComponent const* get_component_by_idx(Handle const& handle, PGM_MetaDataset const* dataset,
                                                         Idx idx) {
        auto const component = PGM_meta_get_component_by_idx(handle.get(), dataset, idx);
        handle.check_error();
        return component;
    }
    PGM_MetaComponent const* get_component_by_idx(PGM_MetaDataset const* dataset, Idx idx) const {
        auto const component = PGM_meta_get_component_by_idx(handle_.get(), dataset, idx);
        handle_.check_error();
        return component;
    }

    static PGM_MetaComponent const* get_component_by_name(Handle const& handle, std::string const& dataset,
                                                          std::string const& component) {
        auto const component_aux = PGM_meta_get_component_by_name(handle.get(), dataset.c_str(), component.c_str());
        handle.check_error();
        return component_aux;
    }
    PGM_MetaComponent const* get_component_by_name(std::string const& dataset, std::string const& component) const {
        auto const component_aux = PGM_meta_get_component_by_name(handle_.get(), dataset.c_str(), component.c_str());
        handle_.check_error();
        return component_aux;
    }

    static std::string const component_name(Handle const& handle, PGM_MetaComponent const* component) {
        auto const component_aux = std::string(PGM_meta_component_name(handle.get(), component));
        handle.check_error();
        return component_aux;
    }
    std::string const component_name(PGM_MetaComponent const* component) const {
        auto const component_aux = std::string(PGM_meta_component_name(handle_.get(), component));
        handle_.check_error();
        return component_aux;
    }

    static size_t component_size(Handle const& handle, PGM_MetaComponent const* component) {
        auto const size = PGM_meta_component_size(handle.get(), component);
        handle.check_error();
        return size;
    }
    size_t component_size(PGM_MetaComponent const* component) const {
        auto const size = PGM_meta_component_size(handle_.get(), component);
        handle_.check_error();
        return size;
    }

    static size_t component_alignment(Handle const& handle, PGM_MetaComponent const* component) {
        auto const alignment = PGM_meta_component_alignment(handle.get(), component);
        handle.check_error();
        return alignment;
    }
    size_t component_alignment(PGM_MetaComponent const* component) const {
        auto const alignment = PGM_meta_component_alignment(handle_.get(), component);
        handle_.check_error();
        return alignment;
    }

    static Idx n_attributes(Handle const& handle, PGM_MetaComponent const* component) {
        auto const n_attributes = PGM_meta_n_attributes(handle.get(), component);
        handle.check_error();
        return n_attributes;
    }
    Idx n_attributes(PGM_MetaComponent const* component) const {
        auto const n_attributes = PGM_meta_n_attributes(handle_.get(), component);
        handle_.check_error();
        return n_attributes;
    }

    static PGM_MetaAttribute const* get_attribute_by_idx(Handle const& handle, PGM_MetaComponent const* component,
                                                         Idx idx) {
        auto const attribute = PGM_meta_get_attribute_by_idx(handle.get(), component, idx);
        handle.check_error();
        return attribute;
    }
    PGM_MetaAttribute const* get_attribute_by_idx(PGM_MetaComponent const* component, Idx idx) const {
        auto const attribute = PGM_meta_get_attribute_by_idx(handle_.get(), component, idx);
        handle_.check_error();
        return attribute;
    }

    static PGM_MetaAttribute const* get_attribute_by_name(Handle const& handle, std::string const& dataset,
                                                          std::string const& component, std::string const& attribute) {
        auto const attribute_aux =
            PGM_meta_get_attribute_by_name(handle.get(), dataset.c_str(), component.c_str(), attribute.c_str());
        handle.check_error();
        return attribute_aux;
    }
    PGM_MetaAttribute const* get_attribute_by_name(std::string const& dataset, std::string const& component,
                                                   std::string const& attribute) const {
        auto const attribute_aux =
            PGM_meta_get_attribute_by_name(handle_.get(), dataset.c_str(), component.c_str(), attribute.c_str());
        handle_.check_error();
        return attribute_aux;
    }

    static std::string const attribute_name(Handle const& handle, PGM_MetaAttribute const* attribute) {
        auto const attribute_aux = std::string(PGM_meta_attribute_name(handle.get(), attribute));
        handle.check_error();
        return attribute_aux;
    }
    std::string const attribute_name(PGM_MetaAttribute const* attribute) const {
        auto const attribute_aux = std::string(PGM_meta_attribute_name(handle_.get(), attribute));
        handle_.check_error();
        return attribute_aux;
    }

    static Idx attribute_ctype(Handle const& handle, PGM_MetaAttribute const* attribute) {
        auto const ctype = PGM_meta_attribute_ctype(handle.get(), attribute);
        handle.check_error();
        return ctype;
    }
    Idx attribute_ctype(PGM_MetaAttribute const* attribute) const {
        auto const ctype = PGM_meta_attribute_ctype(handle_.get(), attribute);
        handle_.check_error();
        return ctype;
    }

    static size_t attribute_offset(Handle const& handle, PGM_MetaAttribute const* attribute) {
        auto const offset = PGM_meta_attribute_offset(handle.get(), attribute);
        handle.check_error();
        return offset;
    }
    size_t attribute_offset(PGM_MetaAttribute const* attribute) const {
        auto const offset = PGM_meta_attribute_offset(handle_.get(), attribute);
        handle_.check_error();
        return offset;
    }

    static int is_little_endian(Handle const& handle) {
        auto const is_little_endian = PGM_is_little_endian(handle.get());
        handle.check_error();
        return is_little_endian;
    }
    int is_little_endian() const {
        auto const is_little_endian = PGM_is_little_endian(handle_.get());
        handle_.check_error();
        return is_little_endian;
    }

  private:
    Handle handle_{};
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_META_DATA_HPP