// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef PGM_DLL_EXPORTS
#define PGM_DLL_EXPORTS
#endif

#include "forward_declarations.hpp"

#include <power_grid_model/math_solver/math_solver_dispatch.hpp>

namespace power_grid_model {

MathSolverDispatcher const& get_math_solver_dispatcher();

} // namespace power_grid_model
