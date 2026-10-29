// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <power_grid_model/auxiliary/meta_data.hpp>

export module power_grid_model.auxiliary.meta_data;

export namespace power_grid_model::meta_data {
using power_grid_model::meta_data::ctype_func_selector;
using power_grid_model::meta_data::ctype_t;
using power_grid_model::meta_data::ctype_v;
using power_grid_model::meta_data::get_attributes_list;
using power_grid_model::meta_data::is_little_endian;
using power_grid_model::meta_data::MetaAttribute;
using power_grid_model::meta_data::MetaComponent;
using power_grid_model::meta_data::MetaData;
using power_grid_model::meta_data::MetaDataset;
using power_grid_model::meta_data::nan_value;
using power_grid_model::meta_data::RawDataConstPtr;
using power_grid_model::meta_data::RawDataPtr;
using power_grid_model::meta_data::set_nan;

// list of all dataset names
using power_grid_model::meta_data::asym_output_getter_s;
using power_grid_model::meta_data::input_getter_s;
using power_grid_model::meta_data::sc_output_getter_s;
using power_grid_model::meta_data::sym_output_getter_s;
using power_grid_model::meta_data::update_getter_s;

} // namespace power_grid_model::meta_data
