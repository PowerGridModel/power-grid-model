// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_DLL_EXPORTS
#include "forward_declarations.hpp"

#include "power_grid_model_c/serialization.h"

#include "get_meta_data.hpp"
#include "handle.hpp"
#include "input_sanitization.hpp"

#include "power_grid_model_c/basics.h"
#include "power_grid_model_c/handle.h"

#include <power_grid_model/auxiliary/serialization/deserializer.hpp>
#include <power_grid_model/auxiliary/serialization/serializer.hpp>

namespace {
using namespace power_grid_model::meta_data;

using power_grid_model_c::call_with_catch;
using power_grid_model_c::safe_bool;
using power_grid_model_c::safe_enum;
using power_grid_model_c::safe_ptr;
using power_grid_model_c::safe_ptr_get;
using power_grid_model_c::safe_size;
using power_grid_model_c::to_c_size;

struct SerializationExceptionHandler : public power_grid_model_c::DefaultExceptionHandler {
    void operator()(PGM_Handle& handle) const noexcept { handle_all_errors(handle, PGM_serialization_error); }
};

constexpr SerializationExceptionHandler serialization_exception_handler{};
} // namespace

PGM_Deserializer* PGM_create_deserializer_from_binary_buffer(PGM_Handle* handle, char const* data, PGM_Idx size,
                                                             PGM_Idx serialization_format) {
    return call_with_catch(
        handle,
        [data, size, serialization_format] {
            return new PGM_Deserializer{// NOSONAR(S5025)
                                        from_buffer,
                                        {safe_ptr(data), safe_size<size_t>(size)},
                                        safe_enum<power_grid_model::SerializationFormat>(serialization_format),
                                        get_meta_data()};
        },
        serialization_exception_handler);
}

PGM_Deserializer* PGM_create_deserializer_from_null_terminated_string(PGM_Handle* handle, char const* data_string,
                                                                      PGM_Idx serialization_format) {
    return call_with_catch(
        handle,
        [data_string, serialization_format] {
            return new PGM_Deserializer{// NOSONAR(S5025)
                                        from_string, safe_ptr(data_string),
                                        safe_enum<power_grid_model::SerializationFormat>(serialization_format),
                                        get_meta_data()};
        },
        serialization_exception_handler);
}

PGM_WritableDataset* PGM_deserializer_get_dataset(PGM_Handle* handle, PGM_Deserializer* deserializer) {
    return call_with_catch(handle, [deserializer] { return &safe_ptr_get(deserializer).get_dataset_info(); });
}

void PGM_deserializer_parse_to_buffer(PGM_Handle* handle, PGM_Deserializer* deserializer) {
    call_with_catch(handle, [deserializer] { safe_ptr_get(deserializer).parse(); }, serialization_exception_handler);
}

// false warning from clang-tidy
// NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDelete)
void PGM_destroy_deserializer(PGM_Deserializer* deserializer) {
    delete deserializer; // NOSONAR(S5025)
}

PGM_Serializer* PGM_create_serializer(PGM_Handle* handle, PGM_ConstDataset const* dataset,
                                      PGM_Idx serialization_format) {
    return call_with_catch(
        handle,
        [dataset, serialization_format] {
            return new PGM_Serializer{// NOSONAR(S5025)
                                      safe_ptr_get(dataset),
                                      safe_enum<power_grid_model::SerializationFormat>(serialization_format)};
        },
        serialization_exception_handler);
}

void PGM_serializer_get_to_binary_buffer(PGM_Handle* handle, PGM_Serializer* serializer, PGM_Idx use_compact_list,
                                         char const** data, PGM_Idx* size) {
    call_with_catch(
        handle,
        [serializer, use_compact_list, data, size] {
            auto const buffer_data = safe_ptr_get(serializer).get_binary_buffer(safe_bool(use_compact_list));
            *data = buffer_data.data();
            *size = to_c_size<PGM_Idx>(std::ssize(buffer_data));
        },
        serialization_exception_handler);
}

char const* PGM_serializer_get_to_zero_terminated_string(PGM_Handle* handle, PGM_Serializer* serializer,
                                                         PGM_Idx use_compact_list, PGM_Idx indent) {
    return call_with_catch(
        handle,
        [serializer, use_compact_list, indent] {
            return safe_ptr_get(serializer).get_string(safe_bool(use_compact_list), indent).c_str();
        },
        serialization_exception_handler);
}

void PGM_destroy_serializer(PGM_Serializer* serializer) {
    delete serializer; // NOSONAR(S5025)
}
