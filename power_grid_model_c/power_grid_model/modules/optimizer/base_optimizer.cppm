// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <Eigen/Core>
#include <Eigen/Dense>
#include <boost/container/small_vector.hpp>
#include <concepts>
#include <memory>
#include <type_traits>
#include <vector>

import power_grid_model.calculation_parameters;
import power_grid_model.common.common;
import power_grid_model.common.enums;
import power_grid_model.common.grouped_index_vector;
import power_grid_model.common.counting_iterator;
import power_grid_model.common.iterator_facade;
import power_grid_model.common.typing;
import power_grid_model.common.maybe_owning_view;
import power_grid_model.common.small_vector;
import power_grid_model.common.statistics;
import power_grid_model.common.three_phase_tensor;
import power_grid_model.main_core.state;
import power_grid_model.container_fwd;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/optimizer/base_optimizer.hpp>

export module power_grid_model.optimizer.base_optimizer;

export namespace power_grid_model::optimizer {
using power_grid_model::optimizer::NoOptimizer;
using power_grid_model::optimizer::optimizer_c;
} // namespace power_grid_model::optimizer

export namespace power_grid_model::optimizer::detail {
using power_grid_model::optimizer::detail::BaseOptimizer;
using power_grid_model::optimizer::detail::state_calculator_c;
using power_grid_model::optimizer::detail::steady_state_calculator_c;
} // namespace power_grid_model::optimizer::detail
