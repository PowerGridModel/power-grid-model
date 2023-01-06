// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include "power_grid_model_c.hpp"

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

// retrieve meta data
// dataset
POWER_GRID_MODEL_Idx POWER_GRID_MODEL_meta_n_datasets(POWER_GRID_MODEL_Handle*) {
    return (Idx)meta_data::meta_data().size();
}
char const* POWER_GRID_MODEL_meta_dataset_name(POWER_GRID_MODEL_Handle* handle, POWER_GRID_MODEL_Idx idx) {
    static auto const dataset_list = list_of_datasets();
    try {
        return dataset_list.at(idx).c_str();
    }
    catch (std::out_of_range& e) {
        handle->err_code = 1;
        handle->err_msg = e.what();
        return nullptr;
    }
}
// class
POWER_GRID_MODEL_Idx POWER_GRID_MODEL_meta_n_classes(POWER_GRID_MODEL_Handle* handle, char const* dataset) {
    try {
        return (Idx)meta_data::meta_data().at(dataset).size();
    }
    catch (std::out_of_range& e) {
        handle->err_code = 1;
        handle->err_msg = e.what();
        return 0;
    }
}
char const* POWER_GRID_MODEL_meta_class_name(POWER_GRID_MODEL_Handle* handle, char const* dataset,
                                             POWER_GRID_MODEL_Idx idx) {
    static auto const class_list = list_of_classes();
    try {
        return class_list.at(dataset).at(idx).c_str();
    }
    catch (std::out_of_range& e) {
        handle->err_code = 1;
        handle->err_msg = e.what();
        return nullptr;
    }
}
size_t POWER_GRID_MODEL_meta_class_size(POWER_GRID_MODEL_Handle* handle, char const* dataset, char const* class_name) {
    try {
        return (Idx)meta_data::meta_data().at(dataset).at(class_name).size;
    }
    catch (std::out_of_range& e) {
        handle->err_code = 1;
        handle->err_msg = e.what();
        return 0;
    }
}
size_t POWER_GRID_MODEL_meta_class_alignment(POWER_GRID_MODEL_Handle* handle, char const* dataset,
                                             char const* class_name) {
    try {
        return (Idx)meta_data::meta_data().at(dataset).at(class_name).alignment;
    }
    catch (std::out_of_range& e) {
        handle->err_code = 1;
        handle->err_msg = e.what();
        return 0;
    }
}
// attributes
POWER_GRID_MODEL_Idx POWER_GRID_MODEL_meta_n_attributes(POWER_GRID_MODEL_Handle* handle, char const* dataset,
                                                        char const* class_name) {
    try {
        return (Idx)meta_data::meta_data().at(dataset).at(class_name).attributes.size();
    }
    catch (std::out_of_range& e) {
        handle->err_code = 1;
        handle->err_msg = e.what();
        return 0;
    }
}
char const* POWER_GRID_MODEL_meta_attribute_name(POWER_GRID_MODEL_Handle* handle, char const* dataset,
                                                 char const* class_name, POWER_GRID_MODEL_Idx idx) {
    try {
        return meta_data::meta_data().at(dataset).at(class_name).attributes[idx].name.c_str();
    }
    catch (std::out_of_range& e) {
        handle->err_code = 1;
        handle->err_msg = e.what();
        return nullptr;
    }
}
char const* POWER_GRID_MODEL_meta_attribute_ctype(POWER_GRID_MODEL_Handle* handle, char const* dataset,
                                                  char const* class_name, char const* attribute) {
    try {
        return meta_data::meta_data().at(dataset).at(class_name).get_attr(attribute).ctype.c_str();
    }
    catch (std::out_of_range& e) {
        handle->err_code = 1;
        handle->err_msg = e.what();
        return nullptr;
    }
}
size_t POWER_GRID_MODEL_meta_attribute_offset(POWER_GRID_MODEL_Handle* handle, char const* dataset,
                                              char const* class_name, char const* attribute) {
    try {
        return meta_data::meta_data().at(dataset).at(class_name).get_attr(attribute).offset;
    }
    catch (std::out_of_range& e) {
        handle->err_code = 1;
        handle->err_msg = e.what();
        return 0;
    }
}
int POWER_GRID_MODEL_is_little_endian() {
    return meta_data::is_little_endian();
}

// construct and destroy model
