// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_DLL_EXPORTS
#include "forward_declarations.hpp"

#include "get_meta_data.hpp"
#include "handle.hpp"
#include "power_grid_model_c/basics.h"
#include "power_grid_model_c/handle.h"
#include "power_grid_model_c/serialization.h"

#include <power_grid_model/auxiliary/serialization/deserializer.hpp>
#include <power_grid_model/auxiliary/serialization/serializer.hpp>

using namespace power_grid_model::meta_data;

PGM_Deserializer* PGM_create_deserializer_from_binary_buffer(PGM_Handle* handle, char const* data, PGM_Idx size,
                                                             PGM_Idx serialization_format) {
    return call_with_catch(
        handle,
        [data, size, serialization_format] {
            return new PGM_Deserializer{from_buffer,
                                        {data, static_cast<size_t>(size)},
                                        static_cast<power_grid_model::SerializationFormat>(serialization_format),
                                        get_meta_data()};
        },
        PGM_serialization_error);
}

PGM_Deserializer* PGM_create_deserializer_from_null_terminated_string(PGM_Handle* handle, char const* data_string,
                                                                      PGM_Idx serialization_format) {
    return call_with_catch(
        handle,
        [data_string, serialization_format] {
            return new PGM_Deserializer{from_string, data_string,
                                        static_cast<power_grid_model::SerializationFormat>(serialization_format),
                                        get_meta_data()};
        },
        PGM_serialization_error);
}

PGM_WritableDataset* PGM_deserializer_get_dataset(PGM_Handle* /*unused*/, PGM_Deserializer* deserializer) {
    return &deserializer->get_dataset_info();
}

void PGM_deserializer_parse_to_buffer(PGM_Handle* handle, PGM_Deserializer* deserializer) {
    call_with_catch(handle, [deserializer] { deserializer->parse(); }, PGM_serialization_error);
}

// false warning from clang-tidy
// NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDelete)
void PGM_destroy_deserializer(PGM_Deserializer* deserializer) { delete deserializer; }

PGM_Serializer* PGM_create_serializer(PGM_Handle* handle, PGM_ConstDataset const* dataset,
                                      PGM_Idx serialization_format) {
    return call_with_catch(
        handle,
        [dataset, serialization_format] {
            return new PGM_Serializer{*dataset,
                                      static_cast<power_grid_model::SerializationFormat>(serialization_format)};
        },
        PGM_serialization_error);
}

void PGM_serializer_get_to_binary_buffer(PGM_Handle* handle, PGM_Serializer* serializer, PGM_Idx use_compact_list,
                                         char const** data, PGM_Idx* size) {
    call_with_catch(
        handle,
        [serializer, use_compact_list, data, size] {
            auto const buffer_data = serializer->get_binary_buffer(static_cast<bool>(use_compact_list));
            *data = buffer_data.data();
            *size = static_cast<PGM_Idx>(buffer_data.size());
        },
        PGM_serialization_error);
}

char const* PGM_serializer_get_to_zero_terminated_string(PGM_Handle* handle, PGM_Serializer* serializer,
                                                         PGM_Idx use_compact_list, PGM_Idx indent) {
    return call_with_catch(
        handle,
        [serializer, use_compact_list, indent] {
            return serializer->get_string(static_cast<bool>(use_compact_list), indent).c_str();
        },
        PGM_serialization_error);
}

void PGM_destroy_serializer(PGM_Serializer* serializer) { delete serializer; }
