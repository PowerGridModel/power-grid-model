// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MATH_SOLVER_NEWTON_RAPHSON_SE_SOLVER_HPP
#define POWER_GRID_MODEL_MATH_SOLVER_NEWTON_RAPHSON_SE_SOLVER_HPP

/*
iterative linear state estimation solver
*/

#include "block_matrix.hpp"
#include "y_bus.hpp"

#include "../calculation_parameters.hpp"
#include "../three_phase_tensor.hpp"

namespace power_grid_model {

// hide implementation in inside namespace
namespace math_model_impl {

// solver
template <bool sym> class NewtonRaphsonSESolver {
  public:
    NewtonRaphsonSESolver(YBus<sym> const& /* y_bus */,
                          std::shared_ptr<MathModelTopology const> const& /* topo_ptr */) {}

    MathOutput<sym> run_state_estimation(YBus<sym> const& /* y_bus */, StateEstimationInput<sym> const& /* input */,
                                         double /* err_tol */, Idx /* max_iter */,
                                         CalculationInfo& /* calculation_info */) {
        // hide implementation
        return {};
    }
};

template class NewtonRaphsonSESolver<true>;
template class NewtonRaphsonSESolver<false>;

} // namespace math_model_impl

template <bool sym> using NewtonRaphsonSESolver = math_model_impl::NewtonRaphsonSESolver<sym>;

} // namespace power_grid_model

#endif
