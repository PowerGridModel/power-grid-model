// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <Eigen/Core>
#include <Eigen/Dense>
#include <array>
#include <cstddef>

export module power_grid_model.auxiliary.meta_gen.input;

export import power_grid_model.auxiliary.meta_gen.gen_getters;
export import power_grid_model.auxiliary.meta_data;
export import power_grid_model.common.common;
export import power_grid_model.auxiliary.input;

#define PGM_CXX_MODULE_PURVIEW

export {
#include <power_grid_model/auxiliary/meta_gen/input.hpp>
} // export
