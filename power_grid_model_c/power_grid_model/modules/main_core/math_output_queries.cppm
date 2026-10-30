// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <power_grid_model/main_core/math_output_queries.hpp>

export module power_grid_model.main_core.math_output_queries;

export namespace power_grid_model::main_core {
using power_grid_model::main_core::get_bus_injection_output_from_math_id;
using power_grid_model::main_core::get_bus_injection_output_from_topo_id;
using power_grid_model::main_core::get_component_output;
using power_grid_model::main_core::get_voltage_output;
} // namespace power_grid_model::main_core
