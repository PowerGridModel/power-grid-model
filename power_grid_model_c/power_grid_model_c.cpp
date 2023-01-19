// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include "power_grid_model_c.hpp"

#include <cstdlib>

#include "power_grid_model/auxiliary/meta_data_gen.hpp"

using namespace power_grid_model;
using power_grid_model::meta_data::meta_data;

// context handle
struct PGM_Handle {
    Idx err_code;
    std::string err_msg;
    IdxVector failed_scenarios;
    std::vector<std::string> batch_errs;
    mutable std::vector<char const*> batch_errs_c_str;
};

// options
struct PGM_Options {
    Idx calculation_type{PGM_power_flow};
    Idx calculation_method{PGM_newton_raphson};
    Idx symmetric{1};
    double err_tol{1e-8};
    Idx max_iter{20};
    Idx threading{-1};
};

// helper functions
std::vector<std::string> list_of_datasets() {
    std::vector<std::string> res;
    auto const& meta = meta_data::meta_data();
    std::transform(meta.cbegin(), meta.cend(), std::back_inserter(res), [](auto const& x) {
        return x.first;
    });
    return res;
}
std::map<std::string, std::vector<std::string>> list_of_classes() {
    std::map<std::string, std::vector<std::string>> res;
    for (auto const& [key, val] : meta_data::meta_data()) {
        std::vector<std::string> vec;
        std::transform(val.cbegin(), val.cend(), std::back_inserter(vec), [](auto const& x) {
            return x.first;
        });
        res[key] = vec;
    }
    return res;
}
template <class Functor>
auto call_with_bound(PGM_Handle* handle, Functor func) -> std::invoke_result_t<Functor> {
    try {
        return func();
    }
    catch (std::out_of_range& e) {
        handle->err_code = PGM_regular_error;
        handle->err_msg = std::string(e.what()) + "\n You supplied wrong name and/or index!\n";
        return std::invoke_result_t<Functor>{};
    }
}

// create and destroy handle
PGM_Handle* PGM_create_handle() {
    return new PGM_Handle{};
}
void PGM_destroy_handle(PGM_Handle* handle) {
    delete handle;
}

// error handling
PGM_Idx PGM_err_code(PGM_Handle const* handle) {
    return handle->err_code;
}
char const* PGM_err_msg(PGM_Handle const* handle) {
    return handle->err_msg.c_str();
}
PGM_Idx PGM_n_failed_scenarios(PGM_Handle const* handle) {
    return (Idx)handle->failed_scenarios.size();
}
PGM_Idx const* PGM_failed_scenarios(PGM_Handle const* handle) {
    return handle->failed_scenarios.data();
}
char const** PGM_batch_errs(PGM_Handle const* handle) {
    handle->batch_errs_c_str.clear();
    std::transform(handle->batch_errs.begin(), handle->batch_errs.end(), std::back_inserter(handle->batch_errs_c_str),
                   [](auto const& x) {
                       return x.c_str();
                   });
    return handle->batch_errs_c_str.data();
}
void PGM_clear_error(PGM_Handle* handle) {
    *handle = PGM_Handle{};
}

// retrieve meta data
// dataset
PGM_Idx PGM_meta_n_datasets(PGM_Handle*) {
    return (Idx)meta_data::meta_data().size();
}
char const* PGM_meta_dataset_name(PGM_Handle* handle, PGM_Idx idx) {
    static auto const dataset_list = list_of_datasets();
    return call_with_bound(handle, [&]() {
        return dataset_list.at(idx).c_str();
    });
}
// class
PGM_Idx PGM_meta_n_components(PGM_Handle* handle, char const* dataset) {
    return call_with_bound(handle, [&]() {
        return (Idx)meta_data::meta_data().at(dataset).size();
    });
}
char const* PGM_meta_component_name(PGM_Handle* handle, char const* dataset, PGM_Idx idx) {
    static auto const class_list = list_of_classes();
    return call_with_bound(handle, [&]() {
        return class_list.at(dataset).at(idx).c_str();
    });
}
size_t PGM_meta_component_size(PGM_Handle* handle, char const* dataset, char const* component) {
    return call_with_bound(handle, [&]() {
        return meta_data::meta_data().at(dataset).at(component).size;
    });
}
size_t PGM_meta_component_alignment(PGM_Handle* handle, char const* dataset, char const* component) {
    return call_with_bound(handle, [&]() {
        return meta_data::meta_data().at(dataset).at(component).alignment;
    });
}
// attributes
PGM_Idx PGM_meta_n_attributes(PGM_Handle* handle, char const* dataset, char const* component) {
    return call_with_bound(handle, [&]() {
        return (Idx)meta_data::meta_data().at(dataset).at(component).attributes.size();
    });
}
char const* PGM_meta_attribute_name(PGM_Handle* handle, char const* dataset, char const* component, PGM_Idx idx) {
    return call_with_bound(handle, [&]() {
        return meta_data::meta_data().at(dataset).at(component).attributes.at(idx).name.c_str();
    });
}
char const* PGM_meta_attribute_ctype(PGM_Handle* handle, char const* dataset, char const* component,
                                     char const* attribute) {
    return call_with_bound(handle, [&]() {
        return meta_data::meta_data().at(dataset).at(component).get_attr(attribute).ctype.c_str();
    });
}
size_t PGM_meta_attribute_offset(PGM_Handle* handle, char const* dataset, char const* component,
                                 char const* attribute) {
    return call_with_bound(handle, [&]() {
        return meta_data::meta_data().at(dataset).at(component).get_attr(attribute).offset;
    });
}
int PGM_is_little_endian(PGM_Handle*) {
    return meta_data::is_little_endian();
}

// buffer control
void* PGM_create_buffer(PGM_Handle* handle, char const* dataset, char const* component, PGM_Idx size) {
    auto const& data_class = call_with_bound(handle, [&]() {
        return meta_data::meta_data().at(dataset).at(component);
    });
    if (data_class.name == "") {
        return nullptr;
    }
#ifdef _WIN32
    return _aligned_malloc(data_class.size * size, data_class.alignment);
#else
    return std::aligned_alloc(data_class.alignment, data_class.size * size);
#endif
}
void PGM_destroy_buffer(void* ptr) {
#ifdef _WIN32
    _aligned_free(ptr);
#else
    std::free(ptr);
#endif
}
void PGM_buffer_set_nan(PGM_Handle* handle, char const* dataset, char const* component, void* ptr, PGM_Idx size) {
    auto const& data_class = call_with_bound(handle, [&]() {
        return meta_data::meta_data().at(dataset).at(component);
    });
    if (data_class.name == "") {
        return;
    }
    for (Idx i = 0; i != size; ++i) {
        data_class.set_nan(ptr, i);
    }
}
// template for get and set attribute
template <bool is_get, class BufferPtr, class ValuePtr>
void buffer_get_set_value(PGM_Handle* handle, char const* dataset, char const* component, char const* attribute,
                          BufferPtr buffer_ptr, ValuePtr value_ptr, PGM_Idx size, PGM_Idx stride) {
    auto const& data_class = call_with_bound(handle, [&]() {
        return meta_data::meta_data().at(dataset).at(component);
    });
    auto const& attr = call_with_bound(handle, [&]() {
        return data_class.get_attr(attribute);
    });
    if (attr.name == "") {
        return;
    }
    // if stride is negative, use the size of the attributes as stride
    if (stride < 0) {
        stride = attr.size;
    }
    for (Idx i = 0; i != size; ++i) {
        ValuePtr const shifted_value_ptr =
            reinterpret_cast<std::conditional_t<is_get, char*, char const*>>(value_ptr) + stride * i;
        if constexpr (is_get) {
            data_class.get_attr(buffer_ptr, shifted_value_ptr, attr, i);
        }
        else {
            data_class.set_attr(buffer_ptr, shifted_value_ptr, attr, i);
        }
    }
}
void PGM_buffer_set_value(PGM_Handle* handle, char const* dataset, char const* component, char const* attribute,
                          void* buffer_ptr, void const* src_ptr, PGM_Idx size, PGM_Idx src_stride) {
    buffer_get_set_value<false>(handle, dataset, component, attribute, buffer_ptr, src_ptr, size, src_stride);
}
void PGM_buffer_get_value(PGM_Handle* handle, char const* dataset, char const* component, char const* attribute,
                          void const* buffer_ptr, void* dest_ptr, PGM_Idx size, PGM_Idx dest_stride) {
    buffer_get_set_value<true>(handle, dataset, component, attribute, buffer_ptr, dest_ptr, size, dest_stride);
}

// options
PGM_Options* PGM_create_options(PGM_Handle*) {
    return new PGM_Options{};
}
void PGM_destroy_options(PGM_Options* opt) {
    delete opt;
}
void PGM_set_calculation_type(PGM_Handle*, PGM_Options* opt, PGM_Idx type) {
    opt->calculation_type = type;
}
void PGM_set_calculation_method(PGM_Handle*, PGM_Options* opt, PGM_Idx method) {
    opt->calculation_method = method;
}
void PGM_set_symmetric(PGM_Handle*, PGM_Options* opt, PGM_Idx sym) {
    opt->symmetric = sym;
}
void PGM_set_err_tol(PGM_Handle*, PGM_Options* opt, double err_tol) {
    opt->err_tol = err_tol;
}
void PGM_set_max_iter(PGM_Handle*, PGM_Options* opt, PGM_Idx max_iter) {
    opt->max_iter = max_iter;
}
void PGM_set_threading(PGM_Handle*, PGM_Options* opt, PGM_Idx threading) {
    opt->threading = threading;
}

// create model
PGM_PowerGridModel* PGM_create_model(PGM_Handle* handle, double system_frequency, PGM_Idx n_input_types,
                                     char const** type_names, PGM_Idx const* type_sizes, void const** input_data) {
    PGM_clear_error(handle);
    ConstDataset dataset{};
    for (Idx i = 0; i != n_input_types; ++i) {
        dataset[type_names[i]] = ConstDataPointer{input_data[i], type_sizes[i]};
    }
    try {
        return new MainModel{system_frequency, dataset, 0};
    }
    catch (std::exception& e) {
        handle->err_code = PGM_regular_error;
        handle->err_msg = e.what();
        return nullptr;
    }
}

// update model
void PGM_update_model(PGM_Handle* handle, PGM_PowerGridModel* model, PGM_Idx n_update_types, char const** type_names,
                      PGM_Idx const* type_sizes, void const** update_data) {
    PGM_clear_error(handle);
    ConstDataset dataset{};
    for (Idx i = 0; i != n_update_types; ++i) {
        dataset[type_names[i]] = ConstDataPointer{update_data[i], type_sizes[i]};
    }
    try {
        model->update_component(dataset);
    }
    catch (std::exception& e) {
        handle->err_code = PGM_regular_error;
        handle->err_msg = e.what();
    }
}

// copy model
PGM_PowerGridModel* PGM_copy_model(PGM_Handle* handle, PGM_PowerGridModel const* model) {
    try {
        return new MainModel{*model};
    }
    catch (std::exception& e) {
        handle->err_code = PGM_regular_error;
        handle->err_msg = e.what();
        return nullptr;
    }
}

// get indexer
void PGM_get_indexer(PGM_Handle* handle, PGM_PowerGridModel const* model, char const* component_type, PGM_Idx size,
                     PGM_ID const* ids, PGM_Idx* indexer) {
    try {
        model->get_indexer(component_type, ids, size, indexer);
    }
    catch (std::exception& e) {
        handle->err_code = PGM_regular_error;
        handle->err_msg = e.what();
    }
}

// run calculation
void PGM_calculate(PGM_Handle* handle, PGM_PowerGridModel* model, PGM_Options const* opt, PGM_Idx n_output_types,
                   char const** output_type_names, void** output_data, PGM_Idx n_batch, PGM_Idx n_update_types,
                   char const** update_type_names, PGM_Idx const* sizes_per_batch, PGM_Idx const** indptrs_per_type,
                   void const** update_data) {
    PGM_clear_error(handle);
    std::map<std::string, Idx> const n_component = model->all_component_count();
    // prepare output dataset
    Dataset output_dataset{};
    // set n_output_batch to one for single calculation
    Idx const n_output_batch = std::max((Idx)1, n_batch);
    for (Idx i = 0; i != n_output_types; ++i) {
        auto const found = n_component.find(output_type_names[i]);
        if (found != n_component.cend()) {
            output_dataset[output_type_names[i]] = MutableDataPointer{output_data[i], n_output_batch, found->second};
        }
    }
    // prepare update dataset
    ConstDataset update_dataset{};
    for (Idx i = 0; i != n_update_types; ++i) {
        if (sizes_per_batch[i] < 0) {
            // use indptr as sparse batch
            update_dataset[update_type_names[i]] = ConstDataPointer(update_data[i], indptrs_per_type[i], n_batch);
        }
        else {
            // use dense batch
            update_dataset[update_type_names[i]] = ConstDataPointer(update_data[i], n_batch, sizes_per_batch[i]);
        }
    }
    // call calculation
    try {
        switch (opt->calculation_type) {
            case PGM_power_flow:
                if (opt->symmetric) {
                    model->calculate_power_flow<true>(opt->err_tol, opt->max_iter,
                                                      (CalculationMethod)opt->calculation_method, output_dataset,
                                                      update_dataset, opt->threading);
                }
                else {
                    model->calculate_power_flow<false>(opt->err_tol, opt->max_iter,
                                                       (CalculationMethod)opt->calculation_method, output_dataset,
                                                       update_dataset, opt->threading);
                }
                break;
            case PGM_state_estimation:
                if (opt->symmetric) {
                    model->calculate_state_estimation<true>(opt->err_tol, opt->max_iter,
                                                            (CalculationMethod)opt->calculation_method, output_dataset,
                                                            update_dataset, opt->threading);
                }
                else {
                    model->calculate_state_estimation<false>(opt->err_tol, opt->max_iter,
                                                             (CalculationMethod)opt->calculation_method, output_dataset,
                                                             update_dataset, opt->threading);
                }
                break;
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
