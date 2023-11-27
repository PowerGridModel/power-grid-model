// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MATH_SOLVER_ITERATIVE_LINEAR_SE_SOLVER_HPP
#define POWER_GRID_MODEL_MATH_SOLVER_ITERATIVE_LINEAR_SE_SOLVER_HPP

/*
iterative linear state estimation solver
*/

#include "se_solver.hpp"

// Remove as not necessary
#include "block_matrix.hpp"
#include "sparse_lu_solver.hpp"
#include "y_bus.hpp"

#include "../calculation_parameters.hpp"
#include "../exception.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"
#include "../timer.hpp"

namespace power_grid_model {

// hide implementation in inside namespace
namespace math_model_impl {

// block class for the unknown vector and/or right-hand side in state estimation equation
template <bool sym> struct SEUnknownNR : public Block<DoubleComplex, sym, false, 3> {
    template <int r, int c> using GetterType = typename Block<DoubleComplex, sym, false, 2>::template GetterType<r, c>;

    // eigen expression
    using Block<DoubleComplex, sym, false, 2>::Block;
    using Block<DoubleComplex, sym, false, 2>::operator=;

    GetterType<0, 0> u() { return this->template get_val<0, 0>(); }
    GetterType<1, 0> phi() { return this->template get_val<1, 0>(); }

    GetterType<0, 0> eta() { return this->template get_val<0, 0>(); }
    GetterType<1, 0> tau() { return this->template get_val<1, 0>(); }
};

// block class for the right hand side in state estimation equation
template <bool sym> using SERhs = SEUnknown<sym>;

// class of 2*2 (6*6) se gain block
/*
[
   [G, QH]
   [Q, R ]
]
*/
template <bool sym> class SEGainBlock : public Block<DoubleComplex, sym, true, 2> {
  public:
    template <int r, int c> using GetterType = typename Block<DoubleComplex, sym, true, 2>::template GetterType<r, c>;

    // eigen expression
    using Block<DoubleComplex, sym, true, 2>::Block;
    using Block<DoubleComplex, sym, true, 2>::operator=;

    GetterType<0, 0> g() { return this->template get_val<0, 0>(); }
    GetterType<0, 1> qh() { return this->template get_val<0, 1>(); }
    GetterType<1, 0> q() { return this->template get_val<1, 0>(); }
    GetterType<1, 1> r() { return this->template get_val<1, 1>(); }
};

// solver
template <bool sym> class NewtonRaphsonSESolver : public SESolver<sym, NewtonRaphsonSESolver<sym>> {
    // block size 2 for symmetric, 6 for asym
    static constexpr Idx bsr_block_size_ = sym ? 3 : 9;

  public:
    NewtonRaphsonSESolver(YBus<sym> const& y_bus, std::shared_ptr<MathModelTopology const> topo_ptr)
        : SESolver{y_bus, topo_ptr} {} // add gain block info

  private:
    NRSEGainBlock fill_g_voltage();
    NRSEGainBlock fill_g_branch();
    NRSEGainBlock fill_g_shunt();
    NRSEGainBlock fill_q_injection();
    NRSEGainBlock fill_qh();

    NRSERhsBlock fill_eta_voltage();
    NRSERhsBlock fill_eta_branch();
    NRSERhsBlock fill_eta_shunt();
    NRSERhsBlock fill_tau_injection();

    double iterate_unknown(ComplexValueVector<sym>& u, bool has_angle_measurement) {
        double max_dev = 0.0;
        // phase shift anti offset of slack bus, phase a
        // if no angle measurement is present
        DoubleComplex const angle_offset = [&]() -> DoubleComplex {
            if (has_angle_measurement) {
                return 1.0;
            }
            if constexpr (sym) {
                return cabs(x_rhs_[math_topo_->slack_bus_].u()) / x_rhs_[math_topo_->slack_bus_].u();
            } else {
                return cabs(x_rhs_[math_topo_->slack_bus_].u()(0)) / x_rhs_[math_topo_->slack_bus_].u()(0);
            }
        }();

        for (Idx bus = 0; bus != n_bus_; ++bus) {
            // phase offset to calculated voltage as normalized
            ComplexValue<sym> const u_normalized = x_rhs_[bus].u() * angle_offset;
            // get dev of last iteration, get max
            double const dev = max_val(cabs(u_normalized - u[bus]));
            max_dev = std::max(dev, max_dev);
            // assign
            u[bus] = u_normalized;
        }
        return max_dev;
    }
};

template class NewtonRaphsonSESolver<true>;
template class NewtonRaphsonSESolver<false>;

} // namespace math_model_impl

template <bool sym> using NewtonRaphsonSESolver = math_model_impl::NewtonRaphsonSESolver<sym>;

} // namespace power_grid_model

#endif
