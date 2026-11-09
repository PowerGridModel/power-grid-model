// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <algorithm>
#include <cassert>
#include <cmath>
#include <concepts>
#include <iterator>
#include <numeric>
#include <utility>
#include <vector>

import power_grid_model.common.common;
import power_grid_model.common.counting_iterator;
import power_grid_model.common.grouped_index_vector;
import power_grid_model.common.iterator_facade;
import power_grid_model.common.typing;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/index_mapping.hpp>

export module power_grid_model.index_mapping;

export namespace power_grid_model {
using power_grid_model::build_dense_mapping;
using power_grid_model::build_sparse_mapping;
using power_grid_model::DenseIndexMapping;
using power_grid_model::SparseIndexMapping;

namespace index_mapping::detail {
using power_grid_model::index_mapping::detail::index_mapping_criterion_gcc;
} // namespace index_mapping::detail
} // namespace power_grid_model
