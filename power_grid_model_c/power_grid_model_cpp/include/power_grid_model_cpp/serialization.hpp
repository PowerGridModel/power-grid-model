// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_SERIALIZATION_HPP
#define POWER_GRID_MODEL_CPP_SERIALIZATION_HPP

#include "serialization.h"

#include "basics.hpp"
#include "handle.hpp"

namespace power_grid_model_cpp {
class Deserializer {
  public:
    power_grid_model_cpp::Handle handle;

    Deserializer(std::vector<std::byte> const& data, Idx serialization_format)
        : handle(),
          deserializer_{PGM_create_deserializer_from_binary_buffer(
              handle.get(), reinterpret_cast<char const*>(data.data()), data.size(), serialization_format)} {}
    Deserializer(std::string const& data_string, Idx serialization_format)
        : handle(),
          deserializer_{PGM_create_deserializer_from_null_terminated_string(handle.get(), data_string.c_str(),
                                                                            serialization_format)} {}

    ~Deserializer() = default;

    static PGM_WritableDataset* get_dataset(PGM_Handle* provided_handle, PGM_Deserializer* deserializer) {
        return PGM_deserializer_get_dataset(provided_handle, deserializer);
    }
    PGM_WritableDataset* get_dataset() const { return PGM_deserializer_get_dataset(handle.get(), deserializer_.get()); }

    static void parse_to_buffer(PGM_Handle* provided_handle, PGM_Deserializer* deserializer) {
        PGM_deserializer_parse_to_buffer(provided_handle, deserializer);
    }
    void parse_to_buffer() { PGM_deserializer_parse_to_buffer(handle.get(), deserializer_.get()); }

  private:
    UniquePtr<PGM_Deserializer, PGM_destroy_deserializer> deserializer_;
};

class Serializer {
  public:
    power_grid_model_cpp::Handle handle;

    Serializer(PGM_ConstDataset const* dataset, Idx serialization_format)
        : handle(), serializer_{PGM_create_serializer(handle.get(), dataset, serialization_format)} {}

    ~Serializer() = default;

    static void get_to_binary_buffer(PGM_Handle* provided_handle, PGM_Serializer* serializer, Idx use_compact_list,
                                     std::vector<std::byte>& data, Idx* size) {
        char* temp_data = nullptr;
        PGM_serializer_get_to_binary_buffer(provided_handle, serializer, use_compact_list, &temp_data, size);
        if (temp_data != nullptr) {
            data.resize(*size);
            std::memcpy(data.data(), temp_data, *size);
        }
    }
    void get_to_binary_buffer(Idx use_compact_list, std::vector<std::byte>& data, Idx* size) const {
        char* temp_data = nullptr;
        PGM_serializer_get_to_binary_buffer(handle.get(), serializer_.get(), use_compact_list, &temp_data, size);
        if (temp_data != nullptr) {
            data.resize(*size);
            std::memcpy(data.data(), temp_data, *size);
        }
    }

    static std::string const get_to_zero_terminated_string(PGM_Handle* provided_handle, PGM_Serializer* serializer,
                                                           Idx use_compact_list, Idx indent) {
        return std::string(
            PGM_serializer_get_to_zero_terminated_string(provided_handle, serializer, use_compact_list, indent));
    }
    std::string const get_to_zero_terminated_string(Idx use_compact_list, Idx indent) {
        return std::string(
            PGM_serializer_get_to_zero_terminated_string(handle.get(), serializer_.get(), use_compact_list, indent));
    }

  private:
    UniquePtr<PGM_Serializer, PGM_destroy_serializer> serializer_;
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_SERIALIZATION_HPP