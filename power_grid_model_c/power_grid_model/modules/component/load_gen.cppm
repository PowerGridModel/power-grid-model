// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <Eigen/Core>
#include <Eigen/Dense>
#include <boost/container/small_vector.hpp>
#include <cassert>
#include <complex>
#include <string>
#include <type_traits>

import power_grid_model.component.appliance;
import power_grid_model.component.base;
import power_grid_model.auxiliary.input;
import power_grid_model.common.common;
import power_grid_model.common.enums;
import power_grid_model.common.three_phase_tensor;
import power_grid_model.auxiliary.output;
import power_grid_model.auxiliary.update;
import power_grid_model.calculation_parameters;
import power_grid_model.common.grouped_index_vector;
import power_grid_model.common.counting_iterator;
import power_grid_model.common.iterator_facade;
import power_grid_model.common.typing;
import power_grid_model.common.maybe_owning_view;
import power_grid_model.common.small_vector;
import power_grid_model.common.statistics;
import power_grid_model.common.exception;
import power_grid_model.component.component;

#define PGM_CXX_MODULE_PURVIEW
#include <power_grid_model/component/load_gen.hpp>

export module power_grid_model.component.load_gen;

export namespace power_grid_model {
using power_grid_model::AsymGenerator;
using power_grid_model::AsymLoad;
using power_grid_model::GenericGenerator;
using power_grid_model::GenericLoad;
using power_grid_model::GenericLoadGen;
using power_grid_model::LoadGen;
using power_grid_model::SymGenerator;
using power_grid_model::SymLoad;
} // namespace power_grid_model
