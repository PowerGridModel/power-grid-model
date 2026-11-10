// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <cassert>
#include <complex>
#include <concepts>
#include <cstddef>

import power_grid_model.component.base;
import power_grid_model.auxiliary.input;
import power_grid_model.common.common;
import power_grid_model.common.enums;
import power_grid_model.common.three_phase_tensor;
import power_grid_model.auxiliary.output;
import power_grid_model.auxiliary.update;
import power_grid_model.calculation_parameters;
import power_grid_model.common.exception;
import power_grid_model.component.component;
import power_grid_model.common.counting_iterator;
import power_grid_model.common.grouped_index_vector;
import power_grid_model.common.iterator_facade;
import power_grid_model.common.maybe_owning_view;
import power_grid_model.common.small_vector;
import power_grid_model.common.statistics;
import power_grid_model.common.typing;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/component/edge.hpp>

export module power_grid_model.component.edge;

export namespace power_grid_model {
using power_grid_model::Edge;
} // namespace power_grid_model
