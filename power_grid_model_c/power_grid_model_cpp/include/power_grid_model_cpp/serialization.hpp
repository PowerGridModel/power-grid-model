// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_SERIALIZATION_HPP
#define POWER_GRID_MODEL_CPP_SERIALIZATION_HPP

#include "basics.hpp"
#include "dataset.hpp"
#include "handle.hpp"

#include "power_grid_model_c/serialization.h"

namespace power_grid_model_cpp {
class Deserializer {
  public:
    Deserializer(std::vector<std::byte> const& data, Idx serialization_format)
        : deserializer_{handle_.call_with(PGM_create_deserializer_from_binary_buffer,
                                          reinterpret_cast<char const*>(data.data()), data.size(),
                                          serialization_format)} {}
    Deserializer(std::string const& data_string, Idx serialization_format)
        : deserializer_{handle_.call_with(PGM_create_deserializer_from_null_terminated_string, data_string.c_str(),
                                          serialization_format)} {}

    RawDeserializer* get() const { return deserializer_.get(); }

    static DatasetWritable get_dataset(Deserializer const& deserializer) {
        return DatasetWritable{deserializer.handle_.call_with(PGM_deserializer_get_dataset, deserializer.get())};
    }
    DatasetWritable get_dataset() const { return get_dataset(*this); }

    static void parse_to_buffer(Deserializer& deserializer) {
        deserializer.handle_.call_with(PGM_deserializer_parse_to_buffer, deserializer.get());
    }
    void parse_to_buffer() { parse_to_buffer(*this); }

  private:
    Handle handle_{};
    detail::UniquePtr<RawDeserializer, PGM_destroy_deserializer> deserializer_;
};

class Serializer {
  public:
    Serializer(DatasetConst const& dataset, Idx serialization_format)
        : serializer_{handle_.call_with(PGM_create_serializer, dataset.get(), serialization_format)} {}

    RawSerializer* get() const { return serializer_.get(); }

    static void get_to_binary_buffer(Serializer& serializer, Idx use_compact_list, std::vector<std::byte>& data,
                                     Idx* size) {
        char* temp_data = nullptr;
        serializer.handle_.call_with(PGM_serializer_get_to_binary_buffer, serializer.get(), use_compact_list,
                                     &temp_data, size);
        if (temp_data != nullptr) {
            data.resize(*size);
            std::memcpy(data.data(), temp_data, *size);
        }
    }
    void get_to_binary_buffer(Idx use_compact_list, std::vector<std::byte>& data, Idx* size) {
        get_to_binary_buffer(*this, use_compact_list, data, size);
    }

    static std::string get_to_zero_terminated_string(Serializer& serializer, Idx use_compact_list, Idx indent) {
        return std::string{serializer.handle_.call_with(PGM_serializer_get_to_zero_terminated_string, serializer.get(),
                                                        use_compact_list, indent)};
    }
    std::string get_to_zero_terminated_string(Idx use_compact_list, Idx indent) {
        return get_to_zero_terminated_string(*this, use_compact_list, indent);
    }

  private:
    power_grid_model_cpp::Handle handle_{};
    detail::UniquePtr<RawSerializer, PGM_destroy_serializer> serializer_;
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_SERIALIZATION_HPP