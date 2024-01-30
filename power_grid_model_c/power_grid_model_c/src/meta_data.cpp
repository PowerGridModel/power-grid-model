// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_DLL_EXPORTS
#include "forward_declarations.hpp"

#include "power_grid_model_c/meta_data.h"

#include "handle.hpp"

#include <power_grid_model/auxiliary/meta_data_gen.hpp>
#include <power_grid_model/auxiliary/static_asserts/input.hpp>
#include <power_grid_model/auxiliary/static_asserts/output.hpp>
#include <power_grid_model/auxiliary/static_asserts/update.hpp>

namespace {
using namespace power_grid_model;

// assert index type
static_assert(std::is_same_v<PGM_Idx, Idx>);
static_assert(std::is_same_v<PGM_ID, ID>);

constexpr std::string_view bound_error_msg = "\n You supplied wrong name and/or index!\n";

auto const meta_catch = [](PGM_Handle* handle, auto func) -> decltype(auto) {
    return call_with_catch<std::out_of_range>(handle, func, PGM_regular_error, bound_error_msg);
};

} // namespace

// retrieve meta data
// dataset
PGM_Idx PGM_meta_n_datasets(PGM_Handle* /* handle */) { return meta_data::meta_data.n_datasets(); }
PGM_MetaDataset const* PGM_meta_get_dataset_by_idx(PGM_Handle* handle, PGM_Idx idx) {
    return meta_catch(handle, [idx]() {
        if (idx >= meta_data::meta_data.n_datasets()) {
            throw std::out_of_range{"Index out of range!\n"};
        }
        return &meta_data::meta_data.datasets[idx];
    });
}
PGM_MetaDataset const* PGM_meta_get_dataset_by_name(PGM_Handle* handle, char const* dataset) {
    return meta_catch(handle, [dataset]() { return &meta_data::meta_data.get_dataset(dataset); });
}
char const* PGM_meta_dataset_name(PGM_Handle* /* handle */, PGM_MetaDataset const* dataset) { return dataset->name; }
// component
PGM_Idx PGM_meta_n_components(PGM_Handle* /* handle */, PGM_MetaDataset const* dataset) {
    return dataset->n_components();
}
PGM_MetaComponent const* PGM_meta_get_component_by_idx(PGM_Handle* handle, PGM_MetaDataset const* dataset,
                                                       PGM_Idx idx) {
    return meta_catch(handle, [idx, dataset]() {
        if (idx >= dataset->n_components()) {
            throw std::out_of_range{"Index out of range!\n"};
        }
        return &dataset->components[idx];
    });
}
PGM_MetaComponent const* PGM_meta_get_component_by_name(PGM_Handle* handle, char const* dataset,
                                                        char const* component) {
    return meta_catch(
        handle, [component, dataset]() { return &meta_data::meta_data.get_dataset(dataset).get_component(component); });
}
char const* PGM_meta_component_name(PGM_Handle* /* handle */, PGM_MetaComponent const* component) {
    return component->name;
}
size_t PGM_meta_component_size(PGM_Handle* /* handle */, PGM_MetaComponent const* component) { return component->size; }
size_t PGM_meta_component_alignment(PGM_Handle* /* handle */, PGM_MetaComponent const* component) {
    return component->alignment;
}
// attributes
PGM_Idx PGM_meta_n_attributes(PGM_Handle* /* handle */, PGM_MetaComponent const* component) {
    return component->n_attributes();
}
PGM_MetaAttribute const* PGM_meta_get_attribute_by_idx(PGM_Handle* handle, PGM_MetaComponent const* component,
                                                       PGM_Idx idx) {
    return meta_catch(handle, [idx, component]() {
        if (idx >= component->n_attributes()) {
            throw std::out_of_range{"Index out of range!\n"};
        }
        return &component->attributes[idx];
    });
}
PGM_MetaAttribute const* PGM_meta_get_attribute_by_name(PGM_Handle* handle, char const* dataset, char const* component,
                                                        char const* attribute) {
    return meta_catch(handle, [component, dataset, attribute]() {
        return &meta_data::meta_data.get_dataset(dataset).get_component(component).get_attribute(attribute);
    });
}
char const* PGM_meta_attribute_name(PGM_Handle* /* handle */, PGM_MetaAttribute const* attribute) {
    return attribute->name;
}
PGM_Idx PGM_meta_attribute_ctype(PGM_Handle* /* handle */, PGM_MetaAttribute const* attribute) {
    return static_cast<PGM_Idx>(attribute->ctype);
}
size_t PGM_meta_attribute_offset(PGM_Handle* /* handle */, PGM_MetaAttribute const* attribute) {
    return attribute->offset;
}
int PGM_is_little_endian(PGM_Handle* /* handle */) { return static_cast<int>(meta_data::is_little_endian()); }
