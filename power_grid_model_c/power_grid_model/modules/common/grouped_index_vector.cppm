// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <power_grid_model/common/grouped_index_vector.hpp>

export module power_grid_model.common.grouped_index_vector;

export namespace power_grid_model {
using power_grid_model::DenseGroupedIdxVector;
using power_grid_model::enumerated_zip_sequence;
using power_grid_model::from_dense;
using power_grid_model::from_dense_t;
using power_grid_model::from_sparse;
using power_grid_model::from_sparse_t;
using power_grid_model::grouped_idx_vector_type;
using power_grid_model::SparseGroupedIdxVector;

namespace detail {
using power_grid_model::detail::sparse_decode;
using power_grid_model::detail::sparse_encode;
} // namespace detail
} // namespace power_grid_model
