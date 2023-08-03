// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_DLL_EXPORTS
#include "power_grid_model_c/meta_data.h"

#include "handle.hpp"

#include <power_grid_model/auxiliary/meta_data_gen.hpp>

namespace {
using namespace power_grid_model;

// assert index type
static_assert(std::is_same_v<PGM_Idx, Idx>);
static_assert(std::is_same_v<PGM_ID, ID>);

template <class Functor>
auto call_with_bound(PGM_Handle* handle, Functor func) -> std::invoke_result_t<Functor> {
    static std::remove_cv_t<std::remove_reference_t<std::invoke_result_t<Functor>>> const empty{};
    try {
        return func();
    }
    catch (std::out_of_range& e) {
        handle->err_code = PGM_regular_error;
        handle->err_msg = std::string(e.what()) + "\n You supplied wrong name and/or index!\n";
        return empty;
    }
}
}  // namespace

// retrieve meta data
// dataset
PGM_Idx PGM_meta_n_datasets(PGM_Handle*) {
    return meta_data::meta_data().n_datasets();
}
PGM_MetaDataset const* PGM_meta_get_dataset_by_idx(PGM_Handle* handle, PGM_Idx idx) {
    return call_with_bound(handle, [idx]() -> decltype(auto) {
        return &meta_data::meta_data().datasets.at(idx);
    });
}
PGM_MetaDataset const* PGM_meta_get_dataset_by_name(PGM_Handle* handle, char const* dataset) {
    return call_with_bound(handle, [dataset]() -> decltype(auto) {
        return &meta_data::meta_data().get_dataset(dataset);
    });
}
char const* PGM_meta_dataset_name(PGM_Handle*, PGM_MetaDataset const* dataset) {
    return dataset->name.c_str();
}
// component
PGM_Idx PGM_meta_n_components(PGM_Handle*, PGM_MetaDataset const* dataset) {
    return dataset->n_components();
}
PGM_MetaComponent const* PGM_meta_get_component_by_idx(PGM_Handle* handle, PGM_MetaDataset const* dataset,
                                                       PGM_Idx idx) {
    return call_with_bound(handle, [idx, dataset]() -> decltype(auto) {
        return &dataset->components.at(idx);
    });
}
PGM_MetaComponent const* PGM_meta_get_component_by_name(PGM_Handle* handle, char const* dataset,
                                                        char const* component) {
    return call_with_bound(handle, [component, dataset]() -> decltype(auto) {
        return &meta_data::meta_data().get_dataset(dataset).get_component(component);
    });
}
char const* PGM_meta_component_name(PGM_Handle*, PGM_MetaComponent const* component) {
    return component->name.c_str();
}
size_t PGM_meta_component_size(PGM_Handle*, PGM_MetaComponent const* component) {
    return component->size;
}
size_t PGM_meta_component_alignment(PGM_Handle*, PGM_MetaComponent const* component) {
    return component->alignment;
}
// attributes
PGM_Idx PGM_meta_n_attributes(PGM_Handle*, PGM_MetaComponent const* component) {
    return component->n_attributes();
}
PGM_MetaAttribute const* PGM_meta_get_attribute_by_idx(PGM_Handle* handle, PGM_MetaComponent const* component,
                                                       PGM_Idx idx) {
    return call_with_bound(handle, [idx, component]() -> decltype(auto) {
        return &component->attributes.at(idx);
    });
}
PGM_MetaAttribute const* PGM_meta_get_attribute_by_name(PGM_Handle* handle, char const* dataset, char const* component,
                                                        char const* attribute) {
    return call_with_bound(handle, [component, dataset, attribute]() -> decltype(auto) {
        return &meta_data::meta_data().get_dataset(dataset).get_component(component).get_attribute(attribute);
    });
}
char const* PGM_meta_attribute_name(PGM_Handle*, PGM_MetaAttribute const* attribute) {
    return attribute->name.c_str();
}
PGM_Idx PGM_meta_attribute_ctype(PGM_Handle*, PGM_MetaAttribute const* attribute) {
    return static_cast<PGM_Idx>(attribute->ctype);
}
size_t PGM_meta_attribute_offset(PGM_Handle*, PGM_MetaAttribute const* attribute) {
    return attribute->offset;
}
int PGM_is_little_endian(PGM_Handle*) {
    return meta_data::is_little_endian();
}
