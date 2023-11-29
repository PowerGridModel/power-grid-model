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
#include "measured_values.hpp"
#include "se_solver.hpp"
#include "y_bus.hpp"

#include "../calculation_parameters.hpp"
#include "../three_phase_tensor.hpp"

namespace power_grid_model {

// hide implementation in inside namespace
namespace math_model_impl {

// block class for the unknown vector and/or right-hand side in state estimation equation
template <bool sym> struct NRSEUnknown : public Block<DoubleComplex, sym, false, 4> {
    template <int r, int c> using GetterType = typename Block<DoubleComplex, sym, false, 4>::template GetterType<r, c>;

    // eigen expression
    using Block<DoubleComplex, sym, false, 4>::Block;
    using Block<DoubleComplex, sym, false, 4>::operator=;

    GetterType<0, 0> theta() { return this->template get_val<0, 0>(); }
    GetterType<1, 0> v() { return this->template get_val<1, 0>(); }
    GetterType<2, 0> phi_theta() { return this->template get_val<2, 0>(); }
    GetterType<3, 0> phi_v() { return this->template get_val<3, 0>(); }

    GetterType<0, 0> eta_theta() { return this->template get_val<0, 0>(); }
    GetterType<1, 0> eta_v() { return this->template get_val<1, 0>(); }
    GetterType<2, 0> tau_theta() { return this->template get_val<2, 0>(); }
    GetterType<3, 0> tau_v() { return this->template get_val<3, 0>(); }
};

// block class for the right hand side in state estimation equation
template <bool sym> using NRSERhs = NRSEUnknown<sym>;

// class of 4*4 (12*12) se gain block
/*
[
   [G, QH]
   [Q, R ]
]
*/
template <bool sym> class SEGainBlock : public Block<DoubleComplex, sym, true, 4> {
  public:
    template <int r, int c> using GetterType = typename Block<DoubleComplex, sym, true, 4>::template GetterType<r, c>;

    // eigen expression
    using Block<DoubleComplex, sym, true, 4>::Block;
    using Block<DoubleComplex, sym, true, 4>::operator=;

    GetterType<0, 0> g_P_theta() { return this->template get_val<0, 0>(); }
    GetterType<0, 1> g_P_v() { return this->template get_val<0, 1>(); }
    GetterType<1, 0> g_Q_theta() { return this->template get_val<1, 0>(); }
    GetterType<1, 1> g_Q_v() { return this->template get_val<1, 1>(); }

    GetterType<0, 2> qh_P_theta() { return this->template get_val<0, 2>(); }
    GetterType<0, 3> qh_P_v() { return this->template get_val<0, 3>(); }
    GetterType<1, 2> qh_Q_theta() { return this->template get_val<1, 2>(); }
    GetterType<1, 3> qh_Q_v() { return this->template get_val<1, 3>(); }

    GetterType<2, 0> q_P_theta() { return this->template get_val<2, 0>(); }
    GetterType<2, 1> q_P_v() { return this->template get_val<2, 1>(); }
    GetterType<3, 0> q_Q_theta() { return this->template get_val<3, 0>(); }
    GetterType<3, 1> q_Q_v() { return this->template get_val<3, 1>(); }

    GetterType<2, 2> r_P_theta() { return this->template get_val<2, 2>(); }
    GetterType<2, 3> r_P_v() { return this->template get_val<2, 3>(); }
    GetterType<3, 2> r_Q_theta() { return this->template get_val<3, 2>(); }
    GetterType<3, 3> r_Q_v() { return this->template get_val<3, 3>(); }
};

// solver
template <bool sym> class NewtonRaphsonSESolver : public SESolver<sym, NewtonRaphsonSESolver<sym>> {

  public:
    NewtonRaphsonSESolver(YBus<sym> const& y_bus, std::shared_ptr<MathModelTopology const> topo_ptr)
        : SESolver{y_bus, topo_ptr} {}

  private:
    std::vector<NRSEUnknown<sym>> x_;

    // TODO Fill below equations:
    // `weight` is already inverted variance
    // use voltage from x_[bus]

    // 6.3.1 equation 57
    NRSEGainBlock fill_g_voltage(double weight);
    // 6.3.1 equation 58
    NRSEGainBlock fill_g_branch(double weight, ComplexTensor<sym> const& yij, Idx const& from_bus, Idx const& to_bus);
    // 6.3.1 equation 59
    NRSEGainBlock fill_g_shunt(double weight, ComplexTensor<sym> const& yii, Idx const& bus);
    // 6.3.2 equation 61
    NRSEGainBlock fill_q_injection(ComplexTensor yij, Idx const& from_bus, Idx const& to_bus);
    // this is simple transpose of q, skip this one
    NRSEGainBlock fill_qh(); // transpose

    // 6.3.1 equation 57
    NRSERhsBlock fill_eta_voltage(ComplexValue<sym> measured_voltage, Idx const& bus);
    // 6.3.1 equation 58
    NRSERhsBlock fill_eta_branch(ComplexValue<sym> measured_power, ComplexTensor yii, Idx const& bus);
    // 6.3.1 equation 59
    NRSERhsBlock fill_eta_shunt(ComplexValue<sym> measured_power, ComplexTensor yii, Idx const& bus);
    // 6.3.2 equation 60
    NRSERhsBlock fill_tau_injection(ComplexValue<sym> measured_power, ComplexTensor yii, Idx const& bus);

    // Approach 1: Calculate and store previous power values separately for each measurement
    // Approach 2: Somehow incrementally subtract them

    ComplexTensor<sym> calculate_flow();
    ComplexTensor<sym> calculate_injection();

    double iterate_unknown(ComplexValueVector<sym>& u, bool has_angle_measurement) {
        // TODO Add logic for u += x_rhs_
        return find_max_deviation(u, has_angle_measurement);
    }
};

template class NewtonRaphsonSESolver<true>;
template class NewtonRaphsonSESolver<false>;

} // namespace math_model_impl

template <bool sym> using NewtonRaphsonSESolver = math_model_impl::NewtonRaphsonSESolver<sym>;

} // namespace power_grid_model

#endif
