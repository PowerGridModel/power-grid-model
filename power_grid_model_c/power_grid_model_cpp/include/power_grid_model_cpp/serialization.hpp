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

#include <cstring>

namespace power_grid_model_cpp {
class Deserializer {
  public:
    Deserializer(std::vector<std::byte> const& data, Idx serialization_format)
        : deserializer_{handle_.call_with(PGM_create_deserializer_from_binary_buffer,
                                          reinterpret_cast<const char*>(data.data()), static_cast<Idx>(data.size()),
                                          serialization_format)},
          dataset_{handle_.call_with(PGM_deserializer_get_dataset, get())} {}
    Deserializer(std::vector<char> const& data, Idx serialization_format)
        : deserializer_{handle_.call_with(PGM_create_deserializer_from_binary_buffer, data.data(),
                                          static_cast<Idx>(data.size()), serialization_format)},
          dataset_{handle_.call_with(PGM_deserializer_get_dataset, get())} {}
    Deserializer(std::string const& data_string, Idx serialization_format)
        : deserializer_{handle_.call_with(PGM_create_deserializer_from_null_terminated_string, data_string.c_str(),
                                          serialization_format)},
          dataset_{handle_.call_with(PGM_deserializer_get_dataset, get())} {}

    RawDeserializer* get() { return deserializer_.get(); }
    RawDeserializer const* get() const { return deserializer_.get(); }

    DatasetWritable& get_dataset() { return dataset_; }

    void parse_to_buffer() { handle_.call_with(PGM_deserializer_parse_to_buffer, get()); }

  private:
    Handle handle_{};
    detail::UniquePtr<RawDeserializer, &PGM_destroy_deserializer> deserializer_;
    DatasetWritable dataset_;
};

class Serializer {
  public:
    Serializer(DatasetConst const& dataset, Idx serialization_format)
        : serializer_{handle_.call_with(PGM_create_serializer, dataset.get(), serialization_format)} {}

    RawSerializer* get() { return serializer_.get(); }
    RawSerializer const* get() const { return serializer_.get(); }

    std::string_view get_to_binary_buffer(Idx use_compact_list) {
        char const* temp_data{};
        Idx buffer_size{};
        handle_.call_with(PGM_serializer_get_to_binary_buffer, get(), use_compact_list, &temp_data, &buffer_size);
        if (temp_data == nullptr) {
            return std::string_view{};
        } // empty data
        return std::string_view{temp_data, static_cast<size_t>(buffer_size)};
    }

    void get_to_binary_buffer(Idx use_compact_list, std::vector<std::byte>& data) {
        auto temp_data = get_to_binary_buffer(use_compact_list);
        if (!temp_data.empty()) {
            data.resize(temp_data.size());
            std::memcpy(data.data(), temp_data.data(), temp_data.size());
        } else {
            data.resize(0); // empty data
        }
    }

    void get_to_binary_buffer(Idx use_compact_list, std::vector<char>& data) {
        auto temp_data = get_to_binary_buffer(use_compact_list);
        if (!temp_data.empty()) {
            data.assign(temp_data.begin(), temp_data.end());
        } else {
            data.resize(0); // empty data
        }
    }

    std::string get_to_zero_terminated_string(Idx use_compact_list, Idx indent) {
        return std::string{
            handle_.call_with(PGM_serializer_get_to_zero_terminated_string, get(), use_compact_list, indent)};
    }

  private:
    power_grid_model_cpp::Handle handle_{};
    detail::UniquePtr<RawSerializer, &PGM_destroy_serializer> serializer_;
};
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_SERIALIZATION_HPP
