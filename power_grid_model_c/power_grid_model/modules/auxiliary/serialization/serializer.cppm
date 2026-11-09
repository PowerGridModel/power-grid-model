// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <Eigen/Core>
#include <Eigen/Dense>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <format>
#include <iomanip>
#include <limits>
#include <map>
#include <msgpack.hpp>
#include <nlohmann/json.hpp>
#include <span>
#include <sstream>
#include <stack>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

import power_grid_model.auxiliary.serialization.common;
import power_grid_model.auxiliary.dataset;
import power_grid_model.auxiliary.dataset_fwd;
import power_grid_model.auxiliary.meta_data;
import power_grid_model.common.common;
import power_grid_model.common.enums;
import power_grid_model.common.exception;
import power_grid_model.common.three_phase_tensor;
import power_grid_model.common.counting_iterator;
import power_grid_model.common.iterator_facade;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/auxiliary/serialization/serializer.hpp>

export module power_grid_model.auxiliary.serialization.serializer;

export namespace power_grid_model::meta_data {
using power_grid_model::meta_data::Serializer;
} // namespace power_grid_model::meta_data
