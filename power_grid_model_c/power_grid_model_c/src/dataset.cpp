// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_DLL_EXPORTS
#include "forward_declarations.hpp"

#include "power_grid_model_c/dataset.h"

#include "get_meta_data.hpp"
#include "handle.hpp"
#include "input_sanitization.hpp"

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/auxiliary/meta_data.hpp>
#include <power_grid_model/common/typing.hpp>

using namespace power_grid_model;
using namespace power_grid_model::meta_data;
using power_grid_model_c::call_with_catch;
using power_grid_model_c::safe_bool;
using power_grid_model_c::safe_ptr;
using power_grid_model_c::safe_ptr_get;
using power_grid_model_c::safe_ptr_maybe_nullptr;
using power_grid_model_c::safe_str_view;
using power_grid_model_c::to_c_bool;
using power_grid_model_c::to_c_size;

// dataset info

char const* PGM_dataset_info_name(PGM_Handle* handle, PGM_DatasetInfo const* info) {
    return call_with_catch(handle, [info] { return safe_ptr_get(safe_ptr(info)->dataset).name; });
}

PGM_Idx PGM_dataset_info_is_batch(PGM_Handle* handle, PGM_DatasetInfo const* info) {
    return call_with_catch(handle, [info] { return to_c_bool<PGM_Idx>(safe_ptr_get(info).is_batch); });
}

PGM_Idx PGM_dataset_info_batch_size(PGM_Handle* handle, PGM_DatasetInfo const* info) {
    return call_with_catch(handle, [info] { return safe_ptr_get(info).batch_size; });
}

PGM_Idx PGM_dataset_info_n_components(PGM_Handle* handle, PGM_DatasetInfo const* info) {
    return call_with_catch(handle, [info] { return to_c_size(std::ssize(safe_ptr_get(info).component_info)); });
}

char const* PGM_dataset_info_component_name(PGM_Handle* handle, PGM_DatasetInfo const* info, PGM_Idx component_idx) {
    return call_with_catch(handle, [info, component_idx] {
        return safe_ptr_get(safe_ptr_get(info).component_info.at(component_idx).component).name;
    });
}

PGM_Idx PGM_dataset_info_elements_per_scenario(PGM_Handle* handle, PGM_DatasetInfo const* info, PGM_Idx component_idx) {
    return call_with_catch(handle, [info, component_idx] {
        return safe_ptr_get(info).component_info.at(component_idx).elements_per_scenario;
    });
}

PGM_Idx PGM_dataset_info_total_elements(PGM_Handle* handle, PGM_DatasetInfo const* info, PGM_Idx component_idx) {
    return call_with_catch(
        handle, [info, component_idx] { return safe_ptr_get(info).component_info.at(component_idx).total_elements; });
}

PGM_Idx PGM_dataset_info_has_attribute_indications(PGM_Handle* handle, PGM_DatasetInfo const* info,
                                                   PGM_Idx component_idx) {
    return call_with_catch(handle, [info, component_idx] {
        return to_c_bool<PGM_Idx>(safe_ptr_get(info).component_info.at(component_idx).has_attribute_indications);
    });
}

PGM_Idx PGM_dataset_info_n_attribute_indications(PGM_Handle* handle, PGM_DatasetInfo const* info,
                                                 PGM_Idx component_idx) {
    return call_with_catch(handle, [info, component_idx] {
        return to_c_size(std::ssize(safe_ptr_get(info).component_info.at(component_idx).attribute_indications));
    });
}

char const* PGM_dataset_info_attribute_name(PGM_Handle* handle, PGM_DatasetInfo const* info, PGM_Idx component_idx,
                                            PGM_Idx attribute_idx) {
    return call_with_catch(handle, [info, component_idx, attribute_idx] {
        return safe_ptr_get(safe_ptr_get(info).component_info.at(component_idx).attribute_indications.at(attribute_idx))
            .name;
    });
}

// const dataset

PGM_ConstDataset* PGM_create_dataset_const(PGM_Handle* handle, char const* dataset, PGM_Idx is_batch,
                                           PGM_Idx batch_size) {
    return call_with_catch(handle, [dataset, is_batch, batch_size] {
        return new ConstDataset{// NOSONAR(S5025)
                                safe_bool(is_batch), batch_size, safe_str_view(dataset), get_meta_data()};
    });
}

PGM_ConstDataset* PGM_create_dataset_const_from_writable(PGM_Handle* handle,
                                                         PGM_WritableDataset const* writable_dataset) {
    return call_with_catch(handle, [writable_dataset] {
        return new ConstDataset{safe_ptr_get(writable_dataset)}; // NOSONAR(S5025)
    });
}

PGM_ConstDataset* PGM_create_dataset_const_from_mutable(PGM_Handle* handle, PGM_MutableDataset const* mutable_dataset) {
    return call_with_catch(handle, [mutable_dataset] {
        return new ConstDataset{safe_ptr_get(mutable_dataset)}; // NOSONAR(S5025)
    });
}

void PGM_destroy_dataset_const(PGM_ConstDataset* dataset) {
    delete dataset; // NOSONAR(S5025)
}

void PGM_dataset_const_add_buffer(PGM_Handle* handle, PGM_ConstDataset* dataset, char const* component,
                                  PGM_Idx elements_per_scenario, PGM_Idx total_elements, PGM_Idx const* indptr,
                                  void const* data) {
    call_with_catch(handle, [dataset, component, elements_per_scenario, total_elements, indptr, data] {
        safe_ptr_get(dataset).add_buffer(safe_str_view(component), elements_per_scenario, total_elements,
                                         safe_ptr_maybe_nullptr(indptr), safe_ptr_maybe_nullptr(data));
    });
}

void PGM_dataset_const_add_attribute_buffer(PGM_Handle* handle, PGM_ConstDataset* dataset, char const* component,
                                            char const* attribute, void const* data) {
    call_with_catch(handle, [dataset, component, attribute, data] {
        safe_ptr_get(dataset).add_attribute_buffer(safe_str_view(component), safe_str_view(attribute), safe_ptr(data));
    });
}
void PGM_dataset_const_set_next_cartesian_product_dimension(PGM_Handle* handle, PGM_ConstDataset* dataset,
                                                            PGM_ConstDataset const* next_dataset) {
    call_with_catch(handle, [dataset, next_dataset] {
        safe_ptr_get(dataset).set_next_cartesian_product_dimension(safe_ptr(next_dataset));
    });
}

PGM_DatasetInfo const* PGM_dataset_const_get_info(PGM_Handle* handle, PGM_ConstDataset const* dataset) {
    return call_with_catch(handle, [dataset] { return &safe_ptr_get(dataset).get_description(); });
}

// writable dataset

PGM_DatasetInfo const* PGM_dataset_writable_get_info(PGM_Handle* handle, PGM_WritableDataset const* dataset) {
    return call_with_catch(handle, [dataset] { return &safe_ptr_get(dataset).get_description(); });
}

void PGM_dataset_writable_set_buffer(PGM_Handle* handle, PGM_WritableDataset* dataset, char const* component,
                                     PGM_Idx* indptr, void* data) {
    call_with_catch(handle, [dataset, component, indptr, data] {
        safe_ptr_get(dataset).set_buffer(safe_str_view(component), safe_ptr_maybe_nullptr(indptr),
                                         safe_ptr_maybe_nullptr(data));
    });
}

void PGM_dataset_writable_set_attribute_buffer(PGM_Handle* handle, PGM_WritableDataset* dataset, char const* component,
                                               char const* attribute, void* data) {
    call_with_catch(handle, [dataset, component, attribute, data] {
        safe_ptr_get(dataset).set_attribute_buffer(safe_str_view(component), safe_str_view(attribute), safe_ptr(data));
    });
}

// mutable dataset

PGM_MutableDataset* PGM_create_dataset_mutable(PGM_Handle* handle, char const* dataset, PGM_Idx is_batch,
                                               PGM_Idx batch_size) {
    return call_with_catch(handle, [dataset, is_batch, batch_size] {
        return new MutableDataset{// NOSONAR(S5025)
                                  safe_bool(is_batch), batch_size, safe_str_view(dataset), get_meta_data()};
    });
}

void PGM_destroy_dataset_mutable(PGM_MutableDataset* dataset) {
    delete dataset; // NOSONAR(S5025)
}

void PGM_dataset_mutable_add_buffer(PGM_Handle* handle, PGM_MutableDataset* dataset, char const* component,
                                    PGM_Idx elements_per_scenario, PGM_Idx total_elements, PGM_Idx const* indptr,
                                    void* data) {
    call_with_catch(handle, [dataset, component, elements_per_scenario, total_elements, indptr, data] {
        safe_ptr_get(dataset).add_buffer(safe_str_view(component), elements_per_scenario, total_elements,
                                         safe_ptr_maybe_nullptr(indptr), safe_ptr_maybe_nullptr(data));
    });
}

void PGM_dataset_mutable_add_attribute_buffer(PGM_Handle* handle, PGM_MutableDataset* dataset, char const* component,
                                              char const* attribute, void* data) {
    call_with_catch(handle, [dataset, component, attribute, data] {
        safe_ptr_get(dataset).add_attribute_buffer(safe_str_view(component), safe_str_view(attribute), safe_ptr(data));
    });
}

PGM_DatasetInfo const* PGM_dataset_mutable_get_info(PGM_Handle* handle, PGM_MutableDataset const* dataset) {
    return call_with_catch(handle, [dataset] { return &safe_ptr_get(dataset).get_description(); });
}
