// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_DLL_EXPORTS
#include "forward_declaration.hpp"

#include "handle.hpp"
#include "power_grid_model_c/basics.h"
#include "power_grid_model_c/handle.h"
#include "power_grid_model_c/serialization.h"

#include <power_grid_model/auxiliary/serialization/deserializer.hpp>
#include <power_grid_model/auxiliary/serialization/serializer.hpp>

using namespace power_grid_model::meta_data;

PGM_Deserializer* PGM_create_deserializer_from_msgpack(PGM_Handle* handle, char const* data, PGM_Idx size) {
    return call_with_catch(
        handle,
        [&] {
            return new PGM_Deserializer{from_msgpack, {data, static_cast<size_t>(size)}};
        },
        PGM_serialization_error);
}

PGM_Deserializer* PGM_create_deserializer_from_json(PGM_Handle* handle, char const* json_string) {
    return call_with_catch(
        handle,
        [&] {
            return new PGM_Deserializer{from_json, json_string};
        },
        PGM_serialization_error);
}

PGM_WritableDataset* PGM_deserializer_get_dataset(PGM_Handle* /*unused*/, PGM_Deserializer* deserializer) {
    return &deserializer->get_dataset_info();
}

void PGM_deserializer_parse_to_buffer(PGM_Handle* handle, PGM_Deserializer* deserializer) {
    call_with_catch(
        handle, [&] { deserializer->parse(); }, PGM_serialization_error);
}

// false warning from clang-tidy
// NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDelete)
void PGM_destroy_deserializer(PGM_Deserializer* deserializer) { delete deserializer; }

PGM_Serializer* PGM_create_serializer(PGM_Handle* handle, PGM_ConstDataset const* dataset) {
    return call_with_catch(
        handle, [&] { return new PGM_Serializer{*dataset}; }, PGM_serialization_error);
}

void PGM_get_msgpack(PGM_Handle* handle, PGM_Serializer* serializer, PGM_Idx use_compact_list, char const** data,
                     PGM_Idx* size) {
    call_with_catch(
        handle,
        [&] {
            auto const msgpack_data = serializer->get_msgpack(static_cast<bool>(use_compact_list));
            *data = msgpack_data.data();
            *size = static_cast<PGM_Idx>(msgpack_data.size());
        },
        PGM_serialization_error);
}

char const* PGM_get_json(PGM_Handle* handle, PGM_Serializer* serializer, PGM_Idx use_compact_list, PGM_Idx indent) {
    return call_with_catch(
        handle, [&] { return serializer->get_json(static_cast<bool>(use_compact_list), indent).c_str(); },
        PGM_serialization_error);
}

void PGM_destroy_serializer(PGM_Serializer* serializer) { delete serializer; }
