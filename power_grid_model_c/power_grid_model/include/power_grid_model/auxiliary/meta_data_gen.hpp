// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "input.hpp"
#include "meta_data.hpp"
#include "meta_gen/gen_getters.hpp"
#include "meta_gen/input.hpp"
#include "meta_gen/output.hpp"
#include "meta_gen/update.hpp"
#include "output.hpp"
#include "update.hpp"

#include "../all_components.hpp"
#include "../common/common.hpp"

#include <map>
#include <string>

// generate of meta data
namespace power_grid_model::meta_data::meta_data_gen {

// generate meta data
constexpr MetaData meta_data = get_meta_data<AllComponents, // all components list
                                             dataset_mark<[] { return "input"; }, input_getter_s>,
                                             dataset_mark<[] { return "update"; }, update_getter_s>,
                                             dataset_mark<[] { return "sym_output"; }, sym_output_getter_s>,
                                             dataset_mark<[] { return "asym_output"; }, asym_output_getter_s>,
                                             dataset_mark<[] { return "sc_output"; }, sc_output_getter_s>
                                             // end list of all marks
                                             >::value;

} // namespace power_grid_model::meta_data::meta_data_gen
