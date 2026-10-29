// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <power_grid_model/auxiliary/serialization/deserializer.hpp>

export module power_grid_model.auxiliary.serialization.deserializer;

export namespace power_grid_model {
namespace meta_data {
using power_grid_model::meta_data::Deserializer;
using power_grid_model::meta_data::from_buffer;
using power_grid_model::meta_data::from_buffer_t;
using power_grid_model::meta_data::from_json;
using power_grid_model::meta_data::from_json_t;
using power_grid_model::meta_data::from_msgpack;
using power_grid_model::meta_data::from_msgpack_t;
using power_grid_model::meta_data::from_string;
using power_grid_model::meta_data::from_string_t;

namespace detail {
using power_grid_model::meta_data::detail::DefaultErrorVisitor;
using power_grid_model::meta_data::detail::JsonSAXVisitor;
} // namespace detail
} // namespace meta_data

using power_grid_model::SerializationError;
} // namespace power_grid_model
