// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include "power_grid_model_c.hpp"

#include <cstdlib>

#include "power_grid_model/auxiliary/meta_data_gen.hpp"

using namespace power_grid_model;
using power_grid_model::meta_data::meta_data;

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
        handle->err_code = 1;
        handle->err_msg = std::string(e.what()) + "\n You supplied wrong name and/or index!\n";
        return std::invoke_result_t<Functor>{};
    }
}

// create and destory handle
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
PGM_Idx PGM_meta_n_classes(PGM_Handle* handle, char const* dataset) {
    return call_with_bound(handle, [&]() {
        return (Idx)meta_data::meta_data().at(dataset).size();
    });
}
char const* PGM_meta_class_name(PGM_Handle* handle, char const* dataset, PGM_Idx idx) {
    static auto const class_list = list_of_classes();
    return call_with_bound(handle, [&]() {
        return class_list.at(dataset).at(idx).c_str();
    });
}
size_t PGM_meta_class_size(PGM_Handle* handle, char const* dataset, char const* class_name) {
    return call_with_bound(handle, [&]() {
        return meta_data::meta_data().at(dataset).at(class_name).size;
    });
}
size_t PGM_meta_class_alignment(PGM_Handle* handle, char const* dataset, char const* class_name) {
    return call_with_bound(handle, [&]() {
        return meta_data::meta_data().at(dataset).at(class_name).alignment;
    });
}
// attributes
PGM_Idx PGM_meta_n_attributes(PGM_Handle* handle, char const* dataset, char const* class_name) {
    return call_with_bound(handle, [&]() {
        return (Idx)meta_data::meta_data().at(dataset).at(class_name).attributes.size();
    });
}
char const* PGM_meta_attribute_name(PGM_Handle* handle, char const* dataset, char const* class_name, PGM_Idx idx) {
    return call_with_bound(handle, [&]() {
        return meta_data::meta_data().at(dataset).at(class_name).attributes.at(idx).name.c_str();
    });
}
char const* PGM_meta_attribute_ctype(PGM_Handle* handle, char const* dataset, char const* class_name,
                                     char const* attribute) {
    return call_with_bound(handle, [&]() {
        return meta_data::meta_data().at(dataset).at(class_name).get_attr(attribute).ctype.c_str();
    });
}
size_t PGM_meta_attribute_offset(PGM_Handle* handle, char const* dataset, char const* class_name,
                                 char const* attribute) {
    return call_with_bound(handle, [&]() {
        return meta_data::meta_data().at(dataset).at(class_name).get_attr(attribute).offset;
    });
}
int PGM_is_little_endian(PGM_Handle*) {
    return meta_data::is_little_endian();
}

// buffer control
void* PGM_create_buffer(PGM_Handle* handle, char const* dataset, char const* class_name, PGM_Idx size) {
    auto const& data_class = call_with_bound(handle, [&]() {
        return meta_data::meta_data().at(dataset).at(class_name);
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
void PGM_destroy_buffer(PGM_Handle*, void* ptr) {
#ifdef _WIN32
    _aligned_free(ptr);
#else
    std::free(ptr);
#endif
}

// create model
PGM_PowerGridModel* PGM_create_model(PGM_Handle* handle, double system_frequency, PGM_Idx n_input_types,
                                     char const** type_names, PGM_Idx const* type_sizes, void const** input_data) {
    PGM_clear_error(handle);
    ConstDataset dataset{};
    for (Idx i = 0; i != n_input_types; ++i) {
        dataset[type_names[i]] = ConstDataPointer(input_data[i], type_sizes[i]);
    }
    try {
        return new MainModel{system_frequency, dataset, 0};
    }
    catch (std::exception& e) {
        handle->err_code = 1;
        handle->err_msg = e.what();
        return nullptr;
    }
}

// destory model
void PGM_destroy_model(PGM_Handle*, PGM_PowerGridModel* model) {
    delete model;
}

// construct and destroy model
