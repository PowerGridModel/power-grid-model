// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <power_grid_model/main_core/update.hpp>

export module power_grid_model.main_core.update;

export namespace power_grid_model::main_core::update {
namespace independence {
using power_grid_model::main_core::update::independence::check_update_independence;
using power_grid_model::main_core::update::independence::UpdateCompProperties;
} // namespace independence
using power_grid_model::main_core::update::get_all_sequence_idx_map;
using power_grid_model::main_core::update::update_component;
using power_grid_model::main_core::update::update_inverse;
} // namespace power_grid_model::main_core::update
