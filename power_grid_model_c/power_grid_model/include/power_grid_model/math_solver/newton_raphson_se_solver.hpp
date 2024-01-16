// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MATH_SOLVER_NEWTON_RAPHSON_SE_SOLVER_HPP
#define POWER_GRID_MODEL_MATH_SOLVER_NEWTON_RAPHSON_SE_SOLVER_HPP

/*
Newton Raphson state estimation solver
*/

#include "block_matrix.hpp"
#include "measured_values.hpp"
#include "y_bus.hpp"

#include "../calculation_parameters.hpp"
#include "../three_phase_tensor.hpp"

namespace power_grid_model::math_solver {

// hide implementation in inside namespace
namespace newton_raphson_se {

// block class for the unknown vector and/or right-hand side in state estimation equation
template <bool sym> struct NRSEUnknown : public Block<double, sym, false, 4> {
    template <int r, int c> using GetterType = typename Block<double, sym, false, 4>::template GetterType<r, c>;

    // eigen expression
    using Block<double, sym, false, 4>::Block;
    using Block<double, sym, false, 4>::operator=;

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
// [
//    [G, QH]
//    [Q, R ]
// ]
template <bool sym> class NRSEGainBlock : public Block<double, sym, true, 4> {
  public:
    template <int r, int c> using GetterType = typename Block<double, sym, true, 4>::template GetterType<r, c>;

    // eigen expression
    using Block<double, sym, true, 4>::Block;
    using Block<double, sym, true, 4>::operator=;

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
template <bool sym> class NewtonRaphsonSESolver {

    struct NRSEJacobian {
        struct i_side_block {};
        struct j_side_block {};

        RealTensor<sym> dP_dt{};
        RealTensor<sym> dP_dv{};
        RealTensor<sym> dQ_dt{};
        RealTensor<sym> dQ_dv{};
    };

  public:
    NewtonRaphsonSESolver(YBus<sym> const& y_bus, std::shared_ptr<MathModelTopology const> topo_ptr)
        : n_bus_{y_bus.size()},
          math_topo_{std::move(topo_ptr)},
          data_gain_(y_bus.nnz_lu()),
          del_x_rhs_(y_bus.size()),
          x_(y_bus.size()),
          sparse_solver_{y_bus.shared_indptr_lu(), y_bus.shared_indices_lu(), y_bus.shared_diag_lu()},
          perm_(y_bus.size()) {}

    MathOutput<sym> run_state_estimation(YBus<sym> const& y_bus, StateEstimationInput<sym> const& input, double err_tol,
                                         Idx max_iter, CalculationInfo& calculation_info) {
        // prepare
        Timer main_timer;
        Timer sub_timer;
        MathOutput<sym> output;
        output.u.resize(n_bus_);
        output.bus_injection.resize(n_bus_);
        double max_dev = std::numeric_limits<double>::max();

        main_timer = Timer(calculation_info, 2220, "Math solver");

        // preprocess measured value
        sub_timer = Timer(calculation_info, 2221, "Pre-process measured value");
        MeasuredValues<sym> const measured_values{y_bus.shared_topology(), input};

        // initialize voltage with initial angle
        sub_timer = Timer(calculation_info, 2223, "Initialize voltages");
        RealValue<sym> const mean_angle_shift = measured_values.mean_angle_shift();
        for (Idx bus = 0; bus != n_bus_; ++bus) {
            x_[bus].v() = 1.0;
            x_[bus].theta() = mean_angle_shift + math_topo_->phase_shift[bus];
            output.u[bus] = exp(1.0i * x_[bus].theta());
        }

        // loop to iterate
        Idx num_iter = 0;
        while (max_dev > err_tol || num_iter == 0) {
            if (num_iter++ == max_iter) {
                throw IterationDiverge{max_iter, max_dev, err_tol};
            }
            sub_timer = Timer(calculation_info, 2224, "Prepare LHS rhs");
            prepare_matrix_and_rhs(y_bus, measured_values, output.u);
            // solve with prefactorization
            sub_timer = Timer(calculation_info, 2225, "Solve sparse linear equation (pre-factorized)");
            sparse_solver_.solve_with_prefactorized_matrix(data_gain_, perm_, del_x_rhs_, del_x_rhs_);
            sub_timer = Timer(calculation_info, 2226, "Iterate unknown");
            max_dev = iterate_unknown(output.u);
        };

        // calculate math result
        sub_timer = Timer(calculation_info, 2227, "Calculate Math Result");
        // calculate_result(y_bus, measured_values, output);

        // Manually stop timers to avoid "Max number of iterations" to be included in the timing.
        sub_timer.stop();
        main_timer.stop();

        auto const key = Timer::make_key(2228, "Max number of iterations");
        calculation_info[key] = std::max(calculation_info[key], static_cast<double>(num_iter));

        return output;
    }

  private:
    Idx n_bus_;
    // shared topo data
    std::shared_ptr<MathModelTopology const> math_topo_;

    // data for gain matrix
    std::vector<NRSEGainBlock<sym>> data_gain_;
    // unknown and rhs
    std::vector<NRSERhs<sym>> del_x_rhs_;
    // voltage of current iteration
    std::vector<NRSERhs<sym>> x_;
    // solver
    SparseLUSolver<NRSEGainBlock<sym>, NRSERhs<sym>, NRSEUnknown<sym>> sparse_solver_;
    typename SparseLUSolver<NRSEGainBlock<sym>, NRSERhs<sym>, NRSEUnknown<sym>>::BlockPermArray perm_;

    void prepare_matrix_and_rhs(YBus<sym> const& /*y_bus*/, MeasuredValues<sym> const& /*measured_value*/,
                                ComplexValueVector<sym> const& /*current_u*/) {

        // prefactorize
        sparse_solver_.prefactorize(data_gain_, perm_);
    }

    double iterate_unknown(ComplexValueVector<sym>& u) {
        double max_dev = 0.0;

        for (Idx bus = 0; bus != n_bus_; ++bus) {
            // angle
            x_[bus].theta() += del_x_rhs_[bus].theta();
            // magnitude
            x_[bus].v() += x_[bus].v() * del_x_rhs_[bus].v();
            // phase offset to calculated voltage as normalized
            ComplexValue<sym> const u_tmp = x_[bus].v() * exp(1.0i * x_[bus].theta());
            // get dev of last iteration, get max
            double const dev = max_val(cabs(u_tmp - u[bus]));
            max_dev = std::max(dev, max_dev);
            // assign
            u[bus] = u_tmp;
        }
        return max_dev;
    }
};

template class NewtonRaphsonSESolver<true>;
template class NewtonRaphsonSESolver<false>;

} // namespace newton_raphson_se

using newton_raphson_se::NewtonRaphsonSESolver;

} // namespace power_grid_model::math_solver

#endif
