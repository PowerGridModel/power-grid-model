// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_SERIALIZATION_HPP
#define POWER_GRID_MODEL_CPP_SERIALIZATION_HPP

#include "power_grid_model_c/serialization.h"

#include "basics.hpp"
#include "handle.hpp"

namespace power_grid_model_cpp {
class Deserializer {
  public:
    Deserializer(std::vector<std::byte> const& data, Idx serialization_format)
        : deserializer_{PGM_create_deserializer_from_binary_buffer(
              handle_.get(), reinterpret_cast<char const*>(data.data()), data.size(), serialization_format)} {}
    Deserializer(std::string const& data_string, Idx serialization_format)
        : deserializer_{PGM_create_deserializer_from_null_terminated_string(handle_.get(), data_string.c_str(),
                                                                            serialization_format)} {}

    PGM_Deserializer* get() const { return deserializer_.get(); }

    static PGM_WritableDataset* get_dataset(Handle const& handle, Deserializer const& deserializer) {
        auto dataset = PGM_deserializer_get_dataset(handle.get(), deserializer.get());
        handle.check_error();
        return dataset;
    }
    PGM_WritableDataset* get_dataset() const { return get_dataset(handle_, *this); }

    static void parse_to_buffer(Handle const& handle, Deserializer& deserializer) {
        PGM_deserializer_parse_to_buffer(handle.get(), deserializer.get());
        handle.check_error();
    }
    void parse_to_buffer() { return parse_to_buffer(handle_, *this); }

  private:
    Handle handle_{};
    UniquePtr<PGM_Deserializer, PGM_destroy_deserializer> deserializer_;
};

class Serializer {
  public:
    Serializer(PGM_ConstDataset const* dataset, Idx serialization_format)
        : serializer_{PGM_create_serializer(handle_.get(), dataset, serialization_format)} {}

    PGM_Serializer* get() const { return serializer_.get(); }

    static void get_to_binary_buffer(Handle const& handle, Serializer& serializer, Idx use_compact_list,
                                     std::vector<std::byte>& data, Idx* size) {
        char* temp_data = nullptr;
        PGM_serializer_get_to_binary_buffer(handle.get(), serializer.get(), use_compact_list, &temp_data, size);
        handle.check_error();
        if (temp_data != nullptr) {
            data.resize(*size);
            std::memcpy(data.data(), temp_data, *size);
        }
    }
    void get_to_binary_buffer(Idx use_compact_list, std::vector<std::byte>& data, Idx* size) {
        get_to_binary_buffer(handle_, *this, use_compact_list, data, size);
    }

    static std::string const get_to_zero_terminated_string(Handle const& handle, Serializer& serializer,
                                                           Idx use_compact_list, Idx indent) {
        auto str = std::string(
            PGM_serializer_get_to_zero_terminated_string(handle.get(), serializer.get(), use_compact_list, indent));
        handle.check_error();
        return str;
    }
    std::string const get_to_zero_terminated_string(Idx use_compact_list, Idx indent) {
        return get_to_zero_terminated_string(handle_, *this, use_compact_list, indent);
    }

  private:
    power_grid_model_cpp::Handle handle_{};
    UniquePtr<PGM_Serializer, PGM_destroy_serializer> serializer_;
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_SERIALIZATION_HPP
