// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef PGM_DLL_EXPORTS
#define PGM_DLL_EXPORTS
#endif

#ifdef PGM_USE_CXX_MODULES
import power_grid_model.math_solver.math_solver_dispatch;
#else
#include <power_grid_model/math_solver/math_solver_dispatch.hpp>
#endif

namespace power_grid_model_c {
using power_grid_model::MathSolverDispatcher;

MathSolverDispatcher const& get_math_solver_dispatcher();
} // namespace power_grid_model_c
