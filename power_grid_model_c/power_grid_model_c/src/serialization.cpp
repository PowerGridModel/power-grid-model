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

struct PGM_Serializer : public Serializer {
    using Serializer::Serializer;
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

void PGM_deserializer_parse_to_buffer(PGM_Handle* handle, PGM_Deserializer* deserializer, char const** components,
                                      void** data, PGM_Idx** indptrs) {
    try {
        deserializer->set_buffer(components, data, indptrs);
        deserializer->parse();
    } catch (std::exception const& e) {
        handle->err_code = PGM_serialization_error;
        handle->err_msg = e.what();
    }
}

void PGM_destroy_deserializer(PGM_Deserializer* deserializer) { delete deserializer; }

PGM_Serializer* PGM_create_serializer(PGM_Handle* handle, char const* dataset, PGM_Idx is_batch, PGM_Idx batch_size,
                                      PGM_Idx n_components, char const** components,
                                      PGM_Idx const* elements_per_scenario, PGM_Idx const** indptrs,
                                      void const** data) {
    try {
        return new PGM_Serializer{dataset,    static_cast<bool>(is_batch), batch_size, n_components,
                                  components, elements_per_scenario,       indptrs,    data};
    } catch (std::exception const& e) {
        handle->err_code = PGM_serialization_error;
        handle->err_msg = e.what();
        return nullptr;
    }
}

void PGM_get_msgpack(PGM_Handle* handle, PGM_Serializer* serializer, PGM_Idx use_compact_list, char const** data,
                     PGM_Idx* size) {
    try {
        auto const msgpack_data = serializer->get_msgpack(static_cast<bool>(use_compact_list));
        *data = msgpack_data.data();
        *size = static_cast<PGM_Idx>(msgpack_data.size());
    } catch (std::exception const& e) {
        handle->err_code = PGM_serialization_error;
        handle->err_msg = e.what();
    }
}

char const* PGM_get_json(PGM_Handle* handle, PGM_Serializer* serializer, PGM_Idx use_compact_list, PGM_Idx indent) {
    try {
        return serializer->get_json(static_cast<bool>(use_compact_list), indent).c_str();
    } catch (std::exception const& e) {
        handle->err_code = PGM_serialization_error;
        handle->err_msg = e.what();
        return nullptr;
    }
}

void PGM_destroy_serializer(PGM_Serializer* serializer) { delete serializer; }