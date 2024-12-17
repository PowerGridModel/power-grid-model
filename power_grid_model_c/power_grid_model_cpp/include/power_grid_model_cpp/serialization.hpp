// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_SERIALIZATION_HPP
#define POWER_GRID_MODEL_CPP_SERIALIZATION_HPP

#include "basics.hpp"
#include "dataset.hpp"
#include "handle.hpp"
#include "meta_data.hpp"

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

inline OwningDataset create_owning_dataset(DatasetWritable& writable_dataset) {
    auto const& info = writable_dataset.get_info();
    bool const is_batch = info.is_batch();
    Idx const batch_size = info.batch_size();
    auto const& dataset_name = info.name();
    DatasetMutable dataset_mutable{dataset_name, is_batch, batch_size};
    OwningMemory storage{};

    for (Idx component_idx{}; component_idx < info.n_components(); ++component_idx) {
        auto const& component_name = info.component_name(component_idx);
        auto const& component_meta = MetaData::get_component_by_name(dataset_name, component_name);
        Idx const component_size = info.component_total_elements(component_idx);
        Idx const elements_per_scenario = info.component_elements_per_scenario(component_idx);

        auto& current_indptr = storage.indptrs.emplace_back(elements_per_scenario < 0 ? batch_size + 1 : 0);
        if (!current_indptr.empty()) {
            current_indptr.at(0) = 0;
            current_indptr.at(batch_size) = component_size;
        }
        Idx* const indptr = current_indptr.empty() ? nullptr : current_indptr.data();
        auto& current_buffer = storage.buffers.emplace_back(component_meta, component_size);
        writable_dataset.set_buffer(component_name, indptr, current_buffer);
        dataset_mutable.add_buffer(component_name, elements_per_scenario, component_size, indptr, current_buffer);
    }
    return OwningDataset{.dataset = std::move(dataset_mutable), .storage = std::move(storage)};
}
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_SERIALIZATION_HPP
