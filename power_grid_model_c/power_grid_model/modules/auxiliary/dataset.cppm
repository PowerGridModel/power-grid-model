// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <functional>
#include <iterator>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

import power_grid_model.auxiliary.dataset_fwd;
import power_grid_model.auxiliary.meta_data;
import power_grid_model.common.common;
import power_grid_model.common.exception;
import power_grid_model.common.counting_iterator;
import power_grid_model.common.iterator_facade;
import power_grid_model.common.three_phase_tensor;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/auxiliary/dataset.hpp>

export module power_grid_model.auxiliary.dataset;

export namespace power_grid_model::meta_data {
using power_grid_model::meta_data::AttributeBuffer;
using power_grid_model::meta_data::columnar_attribute_range;
using power_grid_model::meta_data::ColumnarAttributeRange;
using power_grid_model::meta_data::ComponentInfo;
using power_grid_model::meta_data::const_range_object;
using power_grid_model::meta_data::Dataset;
using power_grid_model::meta_data::DatasetInfo;
using power_grid_model::meta_data::get_attribute_span;
using power_grid_model::meta_data::is_data_mutable_v;
using power_grid_model::meta_data::mutable_range_object;
using power_grid_model::meta_data::row_based_range;
} // namespace power_grid_model::meta_data

export namespace power_grid_model {
using power_grid_model::ConstDataset;
using power_grid_model::Dataset;
using power_grid_model::MutableDataset;
using power_grid_model::WritableDataset;
} // namespace power_grid_model
