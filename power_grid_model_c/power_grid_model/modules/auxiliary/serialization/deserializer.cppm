// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <Eigen/Core>
#include <Eigen/Dense>
#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <format>
#include <functional>
#include <limits>
#include <map>
#include <msgpack.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <numeric>
#include <set>
#include <span>
#include <sstream>
#include <stack>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

import power_grid_model.auxiliary.serialization.common;
import power_grid_model.common.common;
import power_grid_model.common.enums;
import power_grid_model.common.exception;
import power_grid_model.common.three_phase_tensor;
import power_grid_model.common.typing;
import power_grid_model.auxiliary.dataset;
import power_grid_model.auxiliary.meta_data;
import power_grid_model.auxiliary.dataset_fwd;
import power_grid_model.common.counting_iterator;
import power_grid_model.common.iterator_facade;

#define PGM_CXX_MODULE_PURVIEW
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
