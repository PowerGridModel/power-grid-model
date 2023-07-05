// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

// include the public header
#define PGM_DLL_EXPORTS
#include "power_grid_model_c.h"

#include "handle.hpp"
#include "options.hpp"

// include PGM header
#include <power_grid_model/auxiliary/meta_data_gen.hpp>
#include <power_grid_model/main_model.hpp>
#include <power_grid_model/power_grid_model.hpp>

// include private stl header
#include <cstdlib>

using namespace power_grid_model;

// aliases main class
struct PGM_PowerGridModel : public MainModel {
    using MainModel::MainModel;
};

namespace {
using meta_data::RawDataConstPtr;
using meta_data::RawDataPtr;

// global reference to meta data
meta_data::MetaData const& pgm_meta = meta_data::meta_data();
}  // namespace

// assert index type
static_assert(std::is_same_v<PGM_Idx, Idx>);
static_assert(std::is_same_v<PGM_ID, ID>);

namespace {
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
    return pgm_meta.n_datasets();
}
PGM_MetaDataset const* PGM_meta_get_dataset_by_idx(PGM_Handle* handle, PGM_Idx idx) {
    return call_with_bound(handle, [idx]() -> decltype(auto) {
        return &pgm_meta.datasets.at(idx);
    });
}
PGM_MetaDataset const* PGM_meta_get_dataset_by_name(PGM_Handle* handle, char const* dataset) {
    return call_with_bound(handle, [dataset]() -> decltype(auto) {
        return &pgm_meta.get_dataset(dataset);
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
        return &pgm_meta.get_dataset(dataset).get_component(component);
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
        return &pgm_meta.get_dataset(dataset).get_component(component).get_attribute(attribute);
    });
}
char const* PGM_meta_attribute_name(PGM_Handle*, PGM_MetaAttribute const* attribute) {
    return attribute->name.c_str();
}
char const* PGM_meta_attribute_ctype(PGM_Handle*, PGM_MetaAttribute const* attribute) {
    return attribute->ctype.c_str();
}
size_t PGM_meta_attribute_offset(PGM_Handle*, PGM_MetaAttribute const* attribute) {
    return attribute->offset;
}
int PGM_is_little_endian(PGM_Handle*) {
    return meta_data::is_little_endian();
}

// buffer control
RawDataPtr PGM_create_buffer(PGM_Handle*, PGM_MetaComponent const* component, PGM_Idx size) {
#ifdef _WIN32
    return _aligned_malloc(component->size * size, component->alignment);
#else
    return std::aligned_alloc(component->alignment, component->size * size);
#endif
}
void PGM_destroy_buffer(RawDataPtr ptr) {
#ifdef _WIN32
    _aligned_free(ptr);
#else
    std::free(ptr);
#endif
}
void PGM_buffer_set_nan(PGM_Handle*, PGM_MetaComponent const* component, void* ptr, PGM_Idx size) {
    component->set_nan(ptr, 0, size);
}

namespace {
// template for get and set attribute
template <bool is_get, class BufferPtr, class ValuePtr>
void buffer_get_set_value(PGM_MetaAttribute const* attribute, BufferPtr buffer_ptr, ValuePtr value_ptr, PGM_Idx size,
                          PGM_Idx stride) {
    // if stride is negative, use the size of the attributes as stride
    if (stride < 0) {
        stride = attribute->size;
    }
    for (Idx i = 0; i != size; ++i) {
        ValuePtr const shifted_value_ptr =
            reinterpret_cast<std::conditional_t<is_get, char*, char const*>>(value_ptr) + stride * i;
        if constexpr (is_get) {
            attribute->get_value(buffer_ptr, shifted_value_ptr, i);
        }
        else {
            attribute->set_value(buffer_ptr, shifted_value_ptr, i);
        }
    }
}
}  // namespace
void PGM_buffer_set_value(PGM_Handle*, PGM_MetaAttribute const* attribute, RawDataPtr buffer_ptr,
                          RawDataConstPtr src_ptr, PGM_Idx size, PGM_Idx src_stride) {
    buffer_get_set_value<false>(attribute, buffer_ptr, src_ptr, size, src_stride);
}
void PGM_buffer_get_value(PGM_Handle*, PGM_MetaAttribute const* attribute, RawDataConstPtr buffer_ptr,
                          RawDataPtr dest_ptr, PGM_Idx size, PGM_Idx dest_stride) {
    buffer_get_set_value<true>(attribute, buffer_ptr, dest_ptr, size, dest_stride);
}

// create model
PGM_PowerGridModel* PGM_create_model(PGM_Handle* handle, double system_frequency, PGM_Idx n_components,
                                     char const** components, PGM_Idx const* component_sizes,
                                     RawDataConstPtr* input_data) {
    PGM_clear_error(handle);
    ConstDataset dataset{};
    for (Idx i = 0; i != n_components; ++i) {
        dataset[components[i]] = ConstDataPointer{input_data[i], component_sizes[i]};
    }
    try {
        return new PGM_PowerGridModel{system_frequency, dataset, 0};
    }
    catch (std::exception& e) {
        handle->err_code = PGM_regular_error;
        handle->err_msg = e.what();
        return nullptr;
    }
}

// update model
void PGM_update_model(PGM_Handle* handle, PGM_PowerGridModel* model, PGM_Idx n_components, char const** components,
                      PGM_Idx const* component_sizes, RawDataConstPtr* update_data) {
    PGM_clear_error(handle);
    ConstDataset dataset{};
    for (Idx i = 0; i != n_components; ++i) {
        dataset[components[i]] = ConstDataPointer{update_data[i], component_sizes[i]};
    }
    try {
        model->update_component<MainModel::permanent_update_t>(dataset);
    }
    catch (std::exception& e) {
        handle->err_code = PGM_regular_error;
        handle->err_msg = e.what();
    }
}

// copy model
PGM_PowerGridModel* PGM_copy_model(PGM_Handle* handle, PGM_PowerGridModel const* model) {
    try {
        return new PGM_PowerGridModel{*model};
    }
    catch (std::exception& e) {
        handle->err_code = PGM_regular_error;
        handle->err_msg = e.what();
        return nullptr;
    }
}

// get indexer
void PGM_get_indexer(PGM_Handle* handle, PGM_PowerGridModel const* model, char const* component, PGM_Idx size,
                     PGM_ID const* ids, PGM_Idx* indexer) {
    try {
        model->get_indexer(component, ids, size, indexer);
    }
    catch (std::exception& e) {
        handle->err_code = PGM_regular_error;
        handle->err_msg = e.what();
    }
}

// run calculation
void PGM_calculate(PGM_Handle* handle, PGM_PowerGridModel* model, PGM_Options const* opt, PGM_Idx n_output_components,
                   char const** output_components, RawDataPtr* output_data, PGM_Idx n_scenarios,
                   PGM_Idx n_update_components, char const** update_components,
                   PGM_Idx const* n_component_elements_per_scenario, PGM_Idx const** indptrs_per_component,
                   RawDataConstPtr* update_data) {
    PGM_clear_error(handle);
    std::map<std::string, Idx> const n_component = model->all_component_count();
    // prepare output dataset
    Dataset output_dataset{};
    // set n_output_batch to one for single calculation
    Idx const n_output_scenarios = std::max(Idx{1}, n_scenarios);
    for (Idx i = 0; i != n_output_components; ++i) {
        auto const found = n_component.find(output_components[i]);
        if (found != n_component.cend()) {
            output_dataset[output_components[i]] =
                MutableDataPointer{output_data[i], n_output_scenarios, found->second};
        }
    }
    // prepare update dataset
    ConstDataset update_dataset{};
    for (Idx i = 0; i != n_update_components; ++i) {
        if (n_component_elements_per_scenario[i] < 0) {
            // use indptr as sparse batch
            update_dataset[update_components[i]] =
                ConstDataPointer(update_data[i], indptrs_per_component[i], n_scenarios);
        }
        else {
            // use dense batch
            update_dataset[update_components[i]] =
                ConstDataPointer(update_data[i], n_scenarios, n_component_elements_per_scenario[i]);
        }
    }
    // call calculation
    try {
        switch (opt->calculation_type) {
            case PGM_power_flow:
                if (opt->symmetric) {
                    handle->batch_parameter = model->calculate_power_flow<true>(
                        opt->err_tol, opt->max_iter, (CalculationMethod)opt->calculation_method, output_dataset,
                        update_dataset, opt->threading);
                }
                else {
                    handle->batch_parameter = model->calculate_power_flow<false>(
                        opt->err_tol, opt->max_iter, (CalculationMethod)opt->calculation_method, output_dataset,
                        update_dataset, opt->threading);
                }
                break;
            case PGM_state_estimation:
                if (opt->symmetric) {
                    handle->batch_parameter = model->calculate_state_estimation<true>(
                        opt->err_tol, opt->max_iter, (CalculationMethod)opt->calculation_method, output_dataset,
                        update_dataset, opt->threading);
                }
                else {
                    handle->batch_parameter = model->calculate_state_estimation<false>(
                        opt->err_tol, opt->max_iter, (CalculationMethod)opt->calculation_method, output_dataset,
                        update_dataset, opt->threading);
                }
                break;
            case PGM_short_circuit:
                [[fallthrough]];
            default:
                throw MissingCaseForEnumError{"CalculationType", opt->calculation_type};
        }
    }
    catch (BatchCalculationError& e) {
        handle->err_code = PGM_batch_error;
        handle->err_msg = e.what();
        handle->failed_scenarios = e.failed_scenarios();
        handle->batch_errs = e.err_msgs();
    }
    catch (std::exception& e) {
        handle->err_code = PGM_regular_error;
        handle->err_msg = e.what();
    }
    catch (...) {
        handle->err_code = PGM_regular_error;
        handle->err_msg = "Unknown error!\n";
    }
}

// destroy model
void PGM_destroy_model(PGM_PowerGridModel* model) {
    delete model;
}
