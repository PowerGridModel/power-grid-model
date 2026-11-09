// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <Eigen/Core>
#include <Eigen/Dense>
#include <boost/container/small_vector.hpp>
#include <concepts>
#include <memory>
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
import power_grid_model.container_fwd;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/main_core/state.hpp>

export module power_grid_model.main_core.state;

export namespace power_grid_model::main_core {
using power_grid_model::main_core::main_model_state_c;
using power_grid_model::main_core::MainModelState;
using power_grid_model::main_core::model_component_state_c;
} // namespace power_grid_model::main_core
