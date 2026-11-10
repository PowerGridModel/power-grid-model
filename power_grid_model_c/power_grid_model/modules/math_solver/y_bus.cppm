// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <numeric>
#include <ranges>
#include <utility>
#include <vector>

import power_grid_model.calculation_parameters;
import power_grid_model.common.common;
import power_grid_model.common.enums;
import power_grid_model.common.grouped_index_vector;
import power_grid_model.common.counting_iterator;
import power_grid_model.common.three_phase_tensor;
import power_grid_model.common.iterator_facade;
import power_grid_model.common.maybe_owning_view;
import power_grid_model.common.small_vector;
import power_grid_model.common.statistics;
import power_grid_model.common.typing;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/math_solver/y_bus.hpp>

export module power_grid_model.math_solver.y_bus;

export namespace power_grid_model {
namespace math_solver {
using power_grid_model::math_solver::counting_sort_element;
using power_grid_model::math_solver::MatrixPos;
using power_grid_model::math_solver::YBus;
using power_grid_model::math_solver::YBusElementMap;
using power_grid_model::math_solver::YBusStructure;
} // namespace math_solver

using power_grid_model::YBus;
} // namespace power_grid_model

template class std::vector<std::function<void(bool)>>;
