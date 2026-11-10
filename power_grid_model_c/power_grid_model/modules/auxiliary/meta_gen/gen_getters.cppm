// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>

export module power_grid_model.auxiliary.meta_gen.gen_getters;

export import power_grid_model.common.common;
export import power_grid_model.common.component_list;
export import power_grid_model.common.counting_iterator;
export import power_grid_model.common.three_phase_tensor;
export import power_grid_model.auxiliary.meta_data;

#define PGM_CXX_MODULE_PURVIEW

export {
#include <power_grid_model/auxiliary/meta_gen/gen_getters.hpp>
} // export
