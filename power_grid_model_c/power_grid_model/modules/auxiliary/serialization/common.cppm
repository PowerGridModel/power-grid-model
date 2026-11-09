// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <Eigen/Core>
#include <Eigen/Dense>
#include <algorithm>
#include <cassert>
#include <concepts>
#include <span>
#include <vector>

export module power_grid_model.auxiliary.serialization.common;

export import power_grid_model.auxiliary.dataset;
export import power_grid_model.auxiliary.meta_data;

#define PGM_CXX_MODULE_PURVIEW

export {
#include <power_grid_model/auxiliary/serialization/common.hpp>
} // export
