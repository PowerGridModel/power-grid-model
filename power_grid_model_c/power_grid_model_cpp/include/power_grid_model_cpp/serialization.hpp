// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_SERIALIZATION_HPP
#define POWER_GRID_MODEL_CPP_SERIALIZATION_HPP

#include <memory>

#include "serialization.h"
#include "handle.hpp"

namespace power_grid_model_cpp {
class Deserializer {
public:
    power_grid_model_cpp::Handle handle;

    Deserializer(char const* data, PGM_Idx size, PGM_Idx serialization_format)
        : handle(), deserializer_{PGM_create_deserializer_from_binary_buffer(handle.get(), data, size, serialization_format), details::DeleterFunctor<&PGM_destroy_deserializer>()} {}
    Deserializer(char const* data_string, PGM_Idx serialization_format)
        : handle(), deserializer_{PGM_create_deserializer_from_null_terminated_string(handle.get(), data_string, serialization_format), details::DeleterFunctor<&PGM_destroy_deserializer>()} {}
    
    ~Deserializer() = default;

    static PGM_WritableDataset* deserializer_get_dataset(PGM_Handle* provided_handle, PGM_Deserializer* deserializer) {
        return PGM_deserializer_get_dataset(provided_handle, deserializer);
    }
    PGM_WritableDataset* deserializer_get_dataset() {
        return PGM_deserializer_get_dataset(handle.get(), deserializer_.get());
    }

    static void deserializer_parse_to_buffer(PGM_Handle* provided_handle, PGM_Deserializer* deserializer) {
        PGM_deserializer_parse_to_buffer(provided_handle, deserializer);
    }
    void deserializer_parse_to_buffer() {
        PGM_deserializer_parse_to_buffer(handle.get(), deserializer_.get());
    }

private:
    std::unique_ptr<PGM_Deserializer, details::DeleterFunctor<&PGM_destroy_deserializer>> deserializer_;
};

class Serializer {
public:
    power_grid_model_cpp::Handle handle;

    Serializer(PGM_ConstDataset const* dataset, PGM_Idx serialization_format)
        : handle(), serializer_{PGM_create_serializer(handle.get(), dataset, serialization_format), details::DeleterFunctor<&PGM_destroy_serializer>()} {}

    static void serializer_get_to_binary_buffer(PGM_Handle* provided_handle, PGM_Serializer* serializer,
                                                PGM_Idx use_compact_list, char const** data, PGM_Idx* size) {
        PGM_serializer_get_to_binary_buffer(provided_handle, serializer, use_compact_list, data, size);
    }
    void serializer_get_to_binary_buffer(PGM_Idx use_compact_list, char const** data, PGM_Idx* size) {
        PGM_serializer_get_to_binary_buffer(handle.get(), serializer_.get(), use_compact_list, data, size);
    }

    static char const* serializer_get_to_zero_terminated_string(PGM_Handle* provided_handle, PGM_Serializer* serializer,
                                                                PGM_Idx use_compact_list, PGM_Idx indent) {
        return PGM_serializer_get_to_zero_terminated_string(provided_handle, serializer, use_compact_list, indent);
    }
    char const* serializer_get_to_zero_terminated_string(PGM_Idx use_compact_list, PGM_Idx indent) {
        return PGM_serializer_get_to_zero_terminated_string(handle.get(), serializer_.get(), use_compact_list, indent);
    }

private:
    std::unique_ptr<PGM_Serializer, details::DeleterFunctor<&PGM_destroy_serializer>> serializer_;
};
} // namespace power_grid_model_cpp

#endif