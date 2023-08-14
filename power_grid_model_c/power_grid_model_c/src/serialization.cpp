// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_DLL_EXPORTS
#include "power_grid_model_c/serialization.h"
#include "handle.hpp"
#include "power_grid_model_c/basics.h"

#include <power_grid_model/auxiliary/serialization/deserializer.hpp>
#include <power_grid_model/auxiliary/serialization/serializer.hpp>

using namespace power_grid_model::meta_data;

struct PGM_Deserializer : public Deserializer {
    using Deserializer::Deserializer;
};

PGM_Deserializer* PGM_create_deserializer_from_msgpack(PGM_Handle* handle, char const* data, PGM_Idx size) {
    try {
        return new PGM_Deserializer{from_msgpack, {data, static_cast<size_t>(size)}};
    } catch (std::exception const& e) {
        handle->err_code = PGM_serialization_error;
        handle->err_msg = e.what();
        return nullptr;
    }
}

PGM_Deserializer* PGM_create_deserializer_from_json(PGM_Handle* handle, char const* json_string) {
    try {
        return new PGM_Deserializer{from_json, json_string};
    } catch (std::exception const& e) {
        handle->err_code = PGM_serialization_error;
        handle->err_msg = e.what();
        return nullptr;
    }
}

char const* PGM_deserializer_dataset_name(PGM_Handle*, PGM_Deserializer* deserializer) {
    return deserializer->dataset_name().c_str();
}

PGM_Idx PGM_deserializer_is_batch(PGM_Handle* handle, PGM_Deserializer* deserializer) {
    return deserializer->is_batch();
}

PGM_Idx PGM_deserializer_batch_size(PGM_Handle* handle, PGM_Deserializer* deserializer) {
    return deserializer->batch_size();
}

PGM_Idx PGM_deserializer_n_components(PGM_Handle* handle, PGM_Deserializer* deserializer) {
    return deserializer->n_components();
}

char const* PGM_deserializer_component_name(PGM_Handle* handle, PGM_Deserializer* deserializer, PGM_Idx component_idx) {
    return deserializer->get_buffer_info(component_idx).component->name.c_str();
}

PGM_Idx PGM_deserializer_component_elements_per_scenario(PGM_Handle* handle, PGM_Deserializer* deserializer,
                                                         PGM_Idx component_idx) {
    return deserializer->get_buffer_info(component_idx).elements_per_scenario;
}

PGM_Idx PGM_deserializer_component_total_elements(PGM_Handle* handle, PGM_Deserializer* deserializer,
                                                  PGM_Idx component_idx) {
    return deserializer->get_buffer_info(component_idx).total_elements;
}
