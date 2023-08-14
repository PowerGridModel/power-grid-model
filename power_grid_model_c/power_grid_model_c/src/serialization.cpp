// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_DLL_EXPORTS
#include "power_grid_model_c/serialization.h"
#include "handle.hpp"
#include "power_grid_model_c/basics.h"

#include <power_grid_model/auxiliary/serialization/deserializer.hpp>
#include <power_grid_model/auxiliary/serialization/serializer.hpp>

using namespace power_grid_model::meta_data;

struct PGM_Deserializer : public Deserializer {
    using Deserializer::Deserializer;
};

PGM_Deserializer* PGM_create_deserializer_from_msgpack(PGM_Handle* handle, char const* data, PGM_Idx size) {
    try {
        return new PGM_Deserializer{from_msgpack, {data, static_cast<size_t>(size)}};
    } catch (std::exception const& e) {
        handle->err_code = PGM_serialization_error;
        handle->err_msg = e.what();
        return nullptr;
    }
}

PGM_Deserializer* PGM_create_deserializer_from_json(PGM_Handle* handle, char const* json_string) {
    try {
        return new PGM_Deserializer{from_json, json_string};
    } catch (std::exception const& e) {
        handle->err_code = PGM_serialization_error;
        handle->err_msg = e.what();
        return nullptr;
    }
}
