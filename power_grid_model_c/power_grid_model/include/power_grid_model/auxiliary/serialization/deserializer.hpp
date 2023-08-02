// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_AUXILIARY_SERIALIZTION_DESERIALIZER_HPP
#define POWER_GRID_MODEL_AUXILIARY_SERIALIZTION_DESERIALIZER_HPP

#include "../../power_grid_model.hpp"

#include <nlohmann/json.hpp>

#include <msgpack.hpp>

namespace power_grid_model::meta_data {

class Deserializer {
   public:
    void deserialize_from_json(char const* json_string) {
        nlohmann::json const json_document = nlohmann::json::parse(json_string);
        std::vector<char> msgpack_data;
        json_document.to_msgpack(msgpack_data);
        deserialize_from_msgpack(msgpack_data.data(), msgpack_data.size());
    }

    void deserialize_from_msgpack(char const* data, size_t length) {
        handle_ = msgpack::unpack(data, length);
        object_ = handle_.get();
    }

   private:
    msgpack::object_handle handle_;
    msgpack::object object_;
};

}  // namespace power_grid_model::meta_data

#endif