// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_SHARED_SOLVER_FUNCTIONS_HPP
#define POWER_GRID_MODEL_SHARED_SOLVER_FUNCTIONS_HPP

#include "y_bus.hpp"

#include "../calculation_parameters.hpp"

namespace power_grid_model::shared_solver_functions {

template <bool sym>
void add_sources(IdxVector const& source_bus_indptr, Idx const& bus_number, YBus<sym> const& y_bus,
                 ComplexVector const& u_source_vector, ComplexTensor<sym>& diagonal_element, ComplexValue<sym>& u_bus) {
    for (Idx source_number = source_bus_indptr[bus_number]; source_number != source_bus_indptr[bus_number + 1];
         ++source_number) {
        ComplexTensor<sym> const y_source = y_bus.math_model_param().source_param[source_number];
        diagonal_element += y_source; // add y_source to the diagonal of Ybus
        u_bus += dot(y_source, ComplexValue<sym>{u_source_vector[source_number]}); // rhs += Y_source * U_source
    }
}

} // namespace power_grid_model::shared_solver_functions

#endif