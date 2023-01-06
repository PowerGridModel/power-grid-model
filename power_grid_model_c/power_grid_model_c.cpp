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
auto call_with_bound(POWER_GRID_MODEL_Handle* handle, Functor func) -> std::result_of_t<Functor()> {
    try {
        return func();
    }
    catch (std::out_of_range& e) {
        handle->err_code = 1;
        handle->err_msg = std::string(e.what()) + "\n You supplied wrong name and/or index!\n";
        return std::result_of_t<Functor()>{};
    }
}

// create and destory handle
POWER_GRID_MODEL_Handle* POWER_GRID_MODEL_create_handle() {
    return new POWER_GRID_MODEL_Handle{};
}
void POWER_GRID_MODEL_destroy_handle(POWER_GRID_MODEL_Handle* handle) {
    delete handle;
}

// error handling
POWER_GRID_MODEL_Idx POWER_GRID_MODEL_err_code(POWER_GRID_MODEL_Handle const* handle) {
    return handle->err_code;
}
char const* POWER_GRID_MODEL_err_msg(POWER_GRID_MODEL_Handle const* handle) {
    return handle->err_msg.c_str();
}
void POWER_GRID_MODEL_clear_error(POWER_GRID_MODEL_Handle* handle) {
    *handle = POWER_GRID_MODEL_Handle{};
}

// retrieve meta data
// dataset
POWER_GRID_MODEL_Idx POWER_GRID_MODEL_meta_n_datasets(POWER_GRID_MODEL_Handle*) {
    return (Idx)meta_data::meta_data().size();
}
char const* POWER_GRID_MODEL_meta_dataset_name(POWER_GRID_MODEL_Handle* handle, POWER_GRID_MODEL_Idx idx) {
    static auto const dataset_list = list_of_datasets();
    return call_with_bound(handle, [&]() {
        return dataset_list.at(idx).c_str();
    });
}
// class
POWER_GRID_MODEL_Idx POWER_GRID_MODEL_meta_n_classes(POWER_GRID_MODEL_Handle* handle, char const* dataset) {
    return call_with_bound(handle, [&]() {
        return (Idx)meta_data::meta_data().at(dataset).size();
    });
}
char const* POWER_GRID_MODEL_meta_class_name(POWER_GRID_MODEL_Handle* handle, char const* dataset,
                                             POWER_GRID_MODEL_Idx idx) {
    static auto const class_list = list_of_classes();
    return call_with_bound(handle, [&]() {
        return class_list.at(dataset).at(idx).c_str();
    });
}
size_t POWER_GRID_MODEL_meta_class_size(POWER_GRID_MODEL_Handle* handle, char const* dataset, char const* class_name) {
    return call_with_bound(handle, [&]() {
        return meta_data::meta_data().at(dataset).at(class_name).size;
    });
}
size_t POWER_GRID_MODEL_meta_class_alignment(POWER_GRID_MODEL_Handle* handle, char const* dataset,
                                             char const* class_name) {
    return call_with_bound(handle, [&]() {
        return meta_data::meta_data().at(dataset).at(class_name).alignment;
    });
}
// attributes
POWER_GRID_MODEL_Idx POWER_GRID_MODEL_meta_n_attributes(POWER_GRID_MODEL_Handle* handle, char const* dataset,
                                                        char const* class_name) {
    return call_with_bound(handle, [&]() {
        return (Idx)meta_data::meta_data().at(dataset).at(class_name).attributes.size();
    });
}
char const* POWER_GRID_MODEL_meta_attribute_name(POWER_GRID_MODEL_Handle* handle, char const* dataset,
                                                 char const* class_name, POWER_GRID_MODEL_Idx idx) {
    return call_with_bound(handle, [&]() {
        return meta_data::meta_data().at(dataset).at(class_name).attributes.at(idx).name.c_str();
    });
}
char const* POWER_GRID_MODEL_meta_attribute_ctype(POWER_GRID_MODEL_Handle* handle, char const* dataset,
                                                  char const* class_name, char const* attribute) {
    return call_with_bound(handle, [&]() {
        return meta_data::meta_data().at(dataset).at(class_name).get_attr(attribute).ctype.c_str();
    });
}
size_t POWER_GRID_MODEL_meta_attribute_offset(POWER_GRID_MODEL_Handle* handle, char const* dataset,
                                              char const* class_name, char const* attribute) {
    return call_with_bound(handle, [&]() {
        return meta_data::meta_data().at(dataset).at(class_name).get_attr(attribute).offset;
    });
}
int POWER_GRID_MODEL_is_little_endian() {
    return meta_data::is_little_endian();
}

// buffer control
POWER_GRID_MODEL_API void* POWER_GRID_MODEL_create_buffer(POWER_GRID_MODEL_Handle* handle, char const* dataset,
                                                          char const* class_name, POWER_GRID_MODEL_Idx size) {
    auto const& data_class = call_with_bound(handle, [&]() {
        return meta_data::meta_data().at(dataset).at(class_name);
    });
    if (data_class.name == "") {
        return nullptr;
    }
#ifdef _WIN32
    return _aligned_malloc(data_class.size * size, data_class.alignment)
#else
    return std::aligned_alloc(data_class.alignment, data_class.size * size);
#endif
}
POWER_GRID_MODEL_API void POWER_GRID_MODEL_destroy_buffer(void* ptr) {
#ifdef _WIN32
    _aligned_free(ptr)
#else
    std::free(ptr);
#endif
}

// create model
POWER_GRID_MODEL_PowerGridModel* POWER_GRID_MODEL_create_model(POWER_GRID_MODEL_Handle* handle, double system_frequency,
                                                               POWER_GRID_MODEL_Idx n_input_types,
                                                               char const** type_names,
                                                               POWER_GRID_MODEL_Idx const* type_sizes,
                                                               void const** input_data) {
    POWER_GRID_MODEL_clear_error(handle);
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
void POWER_GRID_MODEL_destroy_model(POWER_GRID_MODEL_PowerGridModel* model) {
    delete model;
}

// construct and destroy model
