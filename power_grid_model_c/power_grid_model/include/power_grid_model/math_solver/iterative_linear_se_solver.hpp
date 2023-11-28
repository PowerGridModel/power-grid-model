// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MATH_SOLVER_ITERATIVE_LINEAR_SE_SOLVER_HPP
#define POWER_GRID_MODEL_MATH_SOLVER_ITERATIVE_LINEAR_SE_SOLVER_HPP

/*
iterative linear state estimation solver
*/

#include "block_matrix.hpp"
#include "measured_values.hpp"
#include "sparse_lu_solver.hpp"
#include "y_bus.hpp"

#include "../calculation_parameters.hpp"
#include "../three_phase_tensor.hpp"

namespace power_grid_model {

// hide implementation in inside namespace
namespace math_model_impl {

// block class for the unknown vector and/or right-hand side in state estimation equation
template <bool sym> struct ILSEUnknown : public Block<DoubleComplex, sym, false, 2> {
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
template <bool sym> using ILSERhs = ILSEUnknown<sym>;

// class of 2*2 (6*6) se gain block
/*
[
   [G, QH]
   [Q, R ]
]
*/
template <bool sym> class ILSEGainBlock : public Block<DoubleComplex, sym, true, 2> {
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

template <bool sym> class IterativeLinearSESolver : public SESolver<sym, IterativeLinearSESolver<sym>> {

  public:
    IterativeLinearSESolver(YBus<sym> const& y_bus, std::shared_ptr<MathModelTopology const> topo_ptr)
        : : SESolver{y_bus, topo_ptr} {}

  private:
    ILSEGainBlock fill_g_voltage(double weight) { return weight; }
    ILSEGainBlock fill_g_branch(double weight, ComplexTensor<sym> const& yij, ComplexValue<sym> const&,
                                ComplexValue<sym> const&) {
        return dot(hermitian_transpose(yij), weight, yij);
    }
    ILSEGainBlock fill_g_shunt(double weight, ComplexTensor<sym> const& yii, ComplexValue<sym> const& ui);
    ILSEGainBlock fill_q_injection(ComplexTensor yij, ComplexValue<sym> const& ui, ComplexValue<sym> const& uj);
    ILSEGainBlock fill_qh(); // transpose

    ILSERhsBlock fill_eta_voltage(ComplexValue<sym> measured_voltage, ComplexValue<sym> const& ui);
    ILSERhsBlock fill_eta_branch(ComplexValue<sym> measured_power, ComplexTensor yii, ComplexValue<sym> const& ui);
    ILSERhsBlock fill_eta_shunt(ComplexValue<sym> measured_power, ComplexTensor yii, ComplexValue<sym> const& ui);
    ILSERhsBlock fill_tau_injection(ComplexValue<sym> measured_power, ComplexTensor yii, ComplexValue<sym> const& ui);

    double iterate_unknown() { return find_max_deviation(ComplexValueVector<sym> & u, bool has_angle_measurement); }
};

template class IterativeLinearSESolver<true>;
template class IterativeLinearSESolver<false>;

} // namespace math_model_impl

template <bool sym> using IterativeLinearSESolver = math_model_impl::IterativeLinearSESolver<sym>;

} // namespace power_grid_model

#endif
