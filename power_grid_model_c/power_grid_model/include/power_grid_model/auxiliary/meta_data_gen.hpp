// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

// Umbrella header for meta data generation

#include "input.hpp" // IWYU pragma: keep
#include "meta_data.hpp"
#include "meta_gen/gen_getters.hpp"
#include "meta_gen/input.hpp"  // IWYU pragma: keep
#include "meta_gen/output.hpp" // IWYU pragma: keep
#include "meta_gen/update.hpp" // IWYU pragma: keep
#include "output.hpp"          // IWYU pragma: keep
#include "update.hpp"          // IWYU pragma: keep

#include "../all_components.hpp"

// generate of meta data
namespace power_grid_model::meta_data::meta_data_gen {

// generate meta data
constexpr MetaData meta_data =
    get_meta_data<AllComponents, // all components list
                  input_getter_s, update_getter_s, sym_output_getter_s, asym_output_getter_s, sc_output_getter_s
                  // end list of all marks
                  >::value;

} // namespace power_grid_model::meta_data::meta_data_gen
