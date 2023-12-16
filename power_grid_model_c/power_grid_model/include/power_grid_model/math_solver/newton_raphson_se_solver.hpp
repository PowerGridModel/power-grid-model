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

namespace power_grid_model {

// hide implementation in inside namespace
namespace math_model_impl {

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
/*
[
   [G, QH]
   [Q, R ]
]
*/
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
            max_dev = iterate_unknown(output.u, measured_values.has_angle_measurement());
        };

        // calculate math result
        sub_timer = Timer(calculation_info, 2227, "Calculate Math Result");
        calculate_result(y_bus, measured_values, output);

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

    void prepare_matrix_and_rhs(YBus<sym> const& y_bus, MeasuredValues<sym> const& measured_value,
                                ComplexValueVector<sym> const& current_u) {
        MathModelParam<sym> const& param = y_bus.math_model_param();
        IdxVector const& row_indptr = y_bus.row_indptr_lu();
        IdxVector const& col_indices = y_bus.col_indices_lu();
        // get generated (measured/estimated) voltage phasor
        // with current result voltage angle
        // check if this is  the right way or not
        ComplexValueVector<sym> measured_u = measured_value.voltage(current_u);

        // loop data index, all rows and columns
        for (Idx row = 0; row != n_bus_; ++row) {
            auto const& ui = current_u[row];
            RealValue<sym> const& abs_ui = x_[row].v();
            auto const unit_ui = exp(1.0i * x_[row].theta());
            NRSERhs<sym>& rhs_block = del_x_rhs_[row];
            rhs_block = NRSERhs<sym>{};
            for (Idx data_idx_lu = row_indptr[row]; data_idx_lu != row_indptr[row + 1]; ++data_idx_lu) {
                Idx const col = col_indices[data_idx_lu];
                auto const& uj = current_u[col];
                RealValue<sym> const& abs_uj = x_[col].v();
                auto const unit_uj = exp(1.0i * x_[col].theta());
                // get a reference and reset block to zero
                NRSEGainBlock<sym>& block = data_gain_[data_idx_lu];
                block = NRSEGainBlock<sym>{};
                // get data idx of y bus,
                // skip for a fill-in
                Idx const data_idx = y_bus.map_lu_y_bus()[data_idx_lu];
                if (data_idx == -1) {
                    continue;
                }
                // fill block with voltage measurement, only diagonal
                if ((row == col) && measured_value.has_voltage(row)) {
                    // G += 1.0 / variance
                    // for 3x3 tensor, fill diagonal
                    // TODO Figure out angle measurement
                    auto const w_v = RealTensor<sym>{1.0 / measured_value.voltage_var(row)};
                    auto const del_u = measured_u[row] - current_u[row];
                    // block.g_P_theta() += weight_angle;
                    block.g_P_v() += w_v;
                    // block.g_Q_theta() += weight_angle;
                    block.g_Q_v() += w_v;
                    // block.eta_theta() += dot(weight_angle, cabs(del_u));
                    rhs_block.eta_v() += dot(w_v, cabs(del_u));
                }
                // fill block with branch, shunt measurement
                for (Idx element_idx = y_bus.y_bus_entry_indptr()[data_idx];
                     element_idx != y_bus.y_bus_entry_indptr()[data_idx + 1]; ++element_idx) {
                    Idx const obj = y_bus.y_bus_element()[element_idx].idx;
                    YBusElementType const type = y_bus.y_bus_element()[element_idx].element_type;
                    // shunt
                    if (type == YBusElementType::shunt) {
                        if (measured_value.has_shunt(obj)) {
                            auto const& yii = param.shunt_param[obj];
                            auto const calculated_power = calculated_shunt_power(yii, ui);
                            NRSEJacobian block_i = shunt_jacobian(ui, yii);
                            multiply_add_jacobian_blocks(block, rhs_block, block_i, block_i,
                                                         measured_value.branch_from_power(obj), calculated_power);
                        }
                    } else {
                        // TODO move this inside NRSEjacboian
                        auto const& yij = param.branch_param[obj].yft();
                        auto const gc_plus_bs = g_cos_plus_b_sin(yij, ui, uj);
                        auto const gs_minus_bc = g_sin_minus_b_cos(yij, ui, uj);
                        ComplexValue<sym> calculated_power_ij = calculated_branch_power(gc_plus_bs, gs_minus_bc);

                        if (type == YBusElementType::bff) {
                            if (measured_value.has_branch_from(obj)) {
                                auto const& yii = param.branch_param[obj].yff();
                                auto const calculated_power = calculated_power_ij + calculated_shunt_power(yii, ui);
                                NRSEJacobian block_i = branch_from_jacobian(gs_minus_bc, gc_plus_bs, abs_uj);
                                multiply_add_jacobian_blocks(block, rhs_block, block_i, block_i,
                                                             measured_value.branch_from_power(obj), calculated_power);
                            }
                        } else if (type == YBusElementType::bft) {
                            if (measured_value.has_branch_from(obj)) {
                                auto const& yii = param.branch_param[obj].yff();
                                auto const calculated_power = calculated_power_ij + calculated_shunt_power(yii, ui);
                                NRSEJacobian block_i = branch_from_jacobian(gs_minus_bc, gc_plus_bs, abs_uj);
                                NRSEJacobian block_j = branch_to_jacobian(gs_minus_bc, gc_plus_bs, abs_ui);
                                multiply_add_jacobian_blocks(block, rhs_block, block_i, block_j,
                                                             measured_value.branch_from_power(obj), calculated_power);
                            }
                        } else if (type == YBusElementType::btt) {
                            if (measured_value.has_branch_to(obj)) {
                                auto const& yii = param.branch_param[obj].ytt();
                                auto const calculated_power = calculated_power_ij + calculated_shunt_power(yii, uj);
                                NRSEJacobian block_j = branch_to_jacobian(gs_minus_bc, gc_plus_bs, abs_ui);
                                multiply_add_jacobian_blocks(block, rhs_block, block_j, block_j,
                                                             measured_value.branch_to_power(obj), calculated_power);
                            }
                        } else if (type == YBusElementType::btf) {
                            if (measured_value.has_branch_to(obj)) {
                                auto const& yii = param.branch_param[obj].ytt();
                                auto const calculated_power = calculated_power_ij + calculated_shunt_power(yii, uj);
                                NRSEJacobian block_i = branch_to_jacobian(gs_minus_bc, gc_plus_bs, abs_uj);
                                NRSEJacobian block_j = branch_from_jacobian(gs_minus_bc, gc_plus_bs, abs_ui);
                                multiply_add_jacobian_blocks(block, rhs_block, block_i, block_j,
                                                             measured_value.branch_to_power(obj), calculated_power);
                            }
                        }
                    }

                    // } else if (type == YBusElementType::bft) {
                    //     if (measured_value.has_branch_from(obj)) {
                    //         auto const& yij = param.branch_param[obj].yft();
                    //         auto const& yii = param.branch_param[obj].yff();
                    //         auto const gc_plus_bs = g_cos_plus_b_sin(yij, ui, uj);
                    //         auto const gs_minus_bc = g_sin_minus_b_cos(yij, ui, uj);
                    //         ComplexValue<sym> calculated_power = calculated_branch_power(gc_plus_bs, gs_minus_bc);
                    //         calculated_power += calculated_shunt_power(yii, ui);
                    //         // branch_measurement_contribution(
                    //         //     measured_value.branch_from_power(obj), param.branch_param[obj].yff(),
                    //         //     param.branch_param[obj].yft(), row, col, current_u, block, rhs_block);
                    //     }
                    // }
                }

                // fill block with injection measurement
                // injection measurement exist
                if (measured_value.has_bus_injection(row)) {
                    auto const& yij = y_bus.admittance()[data_idx];

                    // R_ii = -variance, only diagonal
                    if (row == col) {
                        // assign variance to diagonal of 3x3 tensor, for asym
                        auto const& injection = measured_value.bus_injection(row);
                        // add negative of the non-summation value then sign will be reversed when row is summed
                        auto const w_p = diagonal_inverse(injection.p_variance);
                        auto const w_q = diagonal_inverse(injection.q_variance);
                        // block.q_P_theta() += dot(imag(yij), abs_ui, abs_ui);
                        // block.q_P_v() += dot(real(yij), -abs_ui);
                        // block.q_Q_theta() += dot(real(yij), abs_ui, abs_ui);;
                        // block.q_Q_v() += dot(imag(yij), abs_ui);

                        rhs_block.tau_theta() += injection.value.real();
                        rhs_block.tau_v() += injection.value.imag();

                        // block.r_P_theta() += w_p * w_p;
                        // block.r_Q_v() += w_q * w_q;
                    } else {
                        block.q_P_theta() += g_sin_minus_b_cos(yij, ui, uj);
                        block.q_P_v() += g_cos_plus_b_sin(yij, ui, unit_uj);
                        block.q_Q_theta() += -g_cos_plus_b_sin(yij, ui, uj);
                        block.q_Q_v() += -g_sin_minus_b_cos(yij, ui, unit_uj);

                        // subtract f(x) incrementally
                        rhs_block.tau_theta() += -sum_row(g_cos_plus_b_sin(yij, ui, uj));
                        rhs_block.tau_v() += -sum_row(g_sin_minus_b_cos(yij, ui, uj));
                    }
                }
                // injection measurement not exist
                else {
                    // Q_ij = 0
                    // R_ii = -1.0, only diagonal
                    // assign -1.0 to diagonal of 3x3 tensor, for asym
                    if (row == col) {
                        block.r_P_theta() = RealTensor<sym>{-1.0};
                        block.r_Q_v() = RealTensor<sym>{-1.0};
                    }
                }
            }
            // extra component and sum of run
            // auto const& diag_block = data_gain[y_bus.lu_diag()[row]];
            // diag_block.q_P_theta() = -sum_gain_row();
            // diag_block.q_P_v() = -sum_gain_row();
            // diag_block.q_Q_theta() = -sum_gain_row();
            // diag_block.q_Q_v() = -sum_gain_row();
        }

        // loop all transpose entry for QH
        // assign the transpose of the transpose entry of Q
        for (Idx data_idx_lu = 0; data_idx_lu != y_bus.nnz_lu(); ++data_idx_lu) {
            // skip for fill-in
            if (y_bus.map_lu_y_bus()[data_idx_lu] == -1) {
                continue;
            }
            Idx const data_idx_tranpose = y_bus.lu_transpose_entry()[data_idx_lu];
            data_gain_[data_idx_lu].qh_P_theta() = data_gain_[data_idx_tranpose].q_P_theta();
            data_gain_[data_idx_lu].qh_P_v() = data_gain_[data_idx_tranpose].q_Q_theta();
            data_gain_[data_idx_lu].qh_Q_theta() = data_gain_[data_idx_tranpose].q_P_v();
            data_gain_[data_idx_lu].qh_Q_v() = data_gain_[data_idx_tranpose].q_Q_v();
        }
        // prefactorize
        sparse_solver_.prefactorize(data_gain_, perm_);
    }

    // void branch_measurement_contribution(PowerSensorCalcParam<sym> const& branch_power, ComplexTensor<sym> const&
    // yii,
    //                                      ComplexTensor<sym> const& yij, Idx const& row, Idx const& col,
    //                                      ComplexValueVector<sym> const& current_u, NRSEGainBlock<sym>& block,
    //                                      NRSERhs<sym>& rhs_block) {
    //     auto const& ui = current_u[row];
    //     auto const& uj = current_u[col];
    //     RealValue<sym> const& abs_ui = x_[row].v();
    //     RealValue<sym> const& abs_uj = x_[col].v();
    //     auto const unit_ui = exp(1.0i * x_[row].theta());
    //     auto const unit_uj = exp(1.0i * x_[col].theta());

    //     auto const w_p = diagonal_inverse(branch_power.p_variance);
    //     auto const w_q = diagonal_inverse(branch_power.q_variance);

    //     RealTensor<sym> const gs_minus_bc = g_sin_minus_b_cos(yij, ui, uj);
    //     RealTensor<sym> const gc_plus_bs = g_cos_plus_b_sin(yij, ui, uj);

    //     RealTensor<sym> const dP_dt_i = -gs_minus_bc;
    //     RealTensor<sym> const dP_dV_i =
    //         g_cos_plus_b_sin(yij, unit_ui, uj) - dot(real(yii), abs_ui, RealDiagonalTensor<sym>{2.0});
    //     RealTensor<sym> const dQ_dt_i = gc_plus_bs;
    //     RealTensor<sym> const dQ_dV_i =
    //         g_sin_minus_b_cos(yij, unit_ui, uj) + dot(imag(yii), abs_ui, RealDiagonalTensor<sym>{2.0});

    //     RealTensor<sym> const dP_dt_j = gs_minus_bc;
    //     RealTensor<sym> const dP_dV_j = g_cos_plus_b_sin(yij, ui, unit_uj);
    //     RealTensor<sym> const dQ_dt_j = -gc_plus_bs;
    //     RealTensor<sym> const dQ_dV_j = g_cos_plus_b_sin(yij, ui, unit_uj);

    //     RealValue<sym> const del_branch_power_p = real(branch_power.value) - sum_row(gc_plus_bs);
    //     RealValue<sym> const del_branch_power_q = imag(branch_power.value) - sum_row(gs_minus_bc);

    //     block.g_P_theta() += dot(w_p, dP_dt_i, dP_dt_j) + dot(w_q, dQ_dt_i, dQ_dt_j);
    //     block.g_Q_v() += dot(w_p, dP_dV_i, dP_dV_j) + dot(w_q, dQ_dV_i, dQ_dV_j);
    //     block.g_P_v() += dot(w_p, dP_dt_i, dP_dV_j) + dot(w_q, dQ_dt_i, dQ_dV_j);
    //     block.g_Q_theta() += dot(w_p, dP_dt_j, dP_dV_i) + dot(w_q, dQ_dt_j, dQ_dV_i);

    //     rhs_block.eta_theta() = dot(w_p, dP_dt_i, del_branch_power_p) + dot(w_q, dQ_dt_i, del_branch_power_q);
    //     rhs_block.eta_v() = dot(w_p, dP_dV_i, del_branch_power_p) + dot(w_q, dQ_dV_i, del_branch_power_q);
    // }

    // void shunt_measurement_contribution(PowerSensorCalcParam<sym> const& shunt_power, ComplexTensor<sym> const& yii,
    //                                     Idx const& row, ComplexValueVector<sym> const& current_u,
    //                                     NRSEGainBlock<sym>& block, NRSERhs<sym>& rhs_block) {

    //     auto const& ui = current_u[row];
    //     auto const& abs_ui = x_[row].v();
    //     RealTensor<sym> const dP_dVi = dot(real(yii), abs_ui, RealDiagonalTensor<sym>{2.0});
    //     RealTensor<sym> const dQ_dVi = dot(imag(yii), abs_ui, RealDiagonalTensor<sym>{-2.0});
    //     auto const w_p = diagonal_inverse(shunt_power.p_variance);
    //     auto const w_q = diagonal_inverse(shunt_power.q_variance);
    //     block.g_Q_v() += dot(w_p, dP_dVi, dP_dVi) + dot(w_q, dQ_dVi, dQ_dVi);

    //     ComplexValue<sym> const del_shunt_power = shunt_power.value - dot(yii, ui) * ui;

    //     rhs_block.eta_v() += dot(w_p, dP_dVi, real(del_shunt_power)) + dot(w_q, dQ_dVi, imag(del_shunt_power));
    // }

    NRSEJacobian shunt_jacobian(ComplexValue<sym> const& ui, ComplexTensor<sym> const& yii) {
        NRSEJacobian jacobian_sub_block{};
        jacobian_sub_block.dP_dv = dot(real(yii), cabs(ui), RealDiagonalTensor<sym>{2.0});
        jacobian_sub_block.dQ_dv = dot(real(yii), cabs(ui), RealDiagonalTensor<sym>{2.0});
        return jacobian_sub_block;
    }

    NRSEJacobian branch_from_jacobian(RealTensor<sym> const& gs_minus_bc, RealTensor<sym> const& gc_plus_bs,
                                      RealValue<sym> const& abs_u_other_side) {
        NRSEJacobian jacobian_sub_block{};
        jacobian_sub_block.dP_dt = gs_minus_bc;
        jacobian_sub_block.dP_dv = gc_plus_bs; // / abs_u_other_side;
        jacobian_sub_block.dQ_dt = -gc_plus_bs;
        jacobian_sub_block.dQ_dv = gs_minus_bc; // / abs_u_other_side;
        return jacobian_sub_block;
    }

    NRSEJacobian branch_to_jacobian(RealTensor<sym> const& gs_minus_bc, RealTensor<sym> const& gc_plus_bs,
                                    RealValue<sym> const& abs_u_other_side) {
        auto jacobian_sub_block = branch_from_jacobian(gs_minus_bc, gc_plus_bs, abs_u_other_side);
        jacobian_sub_block.dP_dt = -jacobian_sub_block.dP_dt;
        jacobian_sub_block.dQ_dt = -jacobian_sub_block.dQ_dt;
        return jacobian_sub_block;
    }

    // TODO Replace with branch
    NRSEJacobian injection_non_diagonal_jacobian(RealTensor<sym> const& gs_minus_bc, RealTensor<sym> const& gc_plus_bs,
                                                 RealValue<sym> const& abs_uj) {
        RealValue<sym> const abs_uj_inv = 1 / abs_uj;
        NRSEJacobian jacobian_sub_block{};
        jacobian_sub_block.dP_dt = gs_minus_bc;
        jacobian_sub_block.dP_dv = gc_plus_bs; // / abs_uj;
        jacobian_sub_block.dQ_dt = -gc_plus_bs;
        jacobian_sub_block.dQ_dv = gs_minus_bc; // / abs_uj;
        return jacobian_sub_block;
    }

    void multiply_add_jacobian_blocks(NRSEGainBlock<sym>& block, NRSERhs<sym>& rhs_block, NRSEJacobian block_1,
                                      NRSEJacobian block_2, PowerSensorCalcParam<sym> power_sensor,
                                      ComplexValue<sym> calculated_power) {
        auto const w_p = diagonal_inverse(power_sensor.p_variance);
        auto const w_q = diagonal_inverse(power_sensor.q_variance);
        auto const del_power = calculated_power - power_sensor.value;

        block.g_P_theta() += dot(w_p, block_1.dP_dt, block_2.dP_dt) + dot(w_q, block_1.dQ_dt, block_2.dQ_dt);
        block.g_P_v() += dot(w_p, block_1.dP_dv, block_2.dP_dv) + dot(w_q, block_1.dQ_dv, block_2.dQ_dv);
        block.g_Q_theta() += dot(w_p, block_1.dP_dt, block_2.dP_dv) + dot(w_q, block_1.dQ_dt, block_2.dQ_dv);
        block.g_Q_v() += dot(w_p, block_1.dP_dv, block_2.dP_dt) + dot(w_q, block_1.dQ_dv, block_2.dQ_dt);

        rhs_block.eta_theta() += dot(w_p, block_1.dP_dt, real(del_power)) + dot(w_q, block_1.dQ_dt, imag(del_power));
        rhs_block.eta_v() += dot(w_p, block_1.dP_dv, real(del_power)) + dot(w_q, block_1.dQ_dv, imag(del_power));
    }

    ComplexValue<sym> calculated_branch_power(RealTensor<sym> const& gs_minus_bc, RealTensor<sym> const& gc_plus_bs) {
        // ComplexValue<sym> power{};
        // power.real(sum_row(gc_plus_bs));
        // power.imag(sum_row(gs_minus_bc));
        return ComplexValue<sym>{};
    }

    ComplexValue<sym> calculated_shunt_power(ComplexTensor<sym> const& yii, ComplexValue<sym> const& ui) {
        return dot(yii, ui * ui);
    }

    RealTensor<sym> ui_uj_cos_ij(ComplexValue<sym> ui, ComplexValue<sym> uj) {
        return vector_outer_product(real(ui), real(uj)) + vector_outer_product(imag(ui), imag(uj));
    }

    RealTensor<sym> ui_uj_sin_ij(ComplexValue<sym> ui, ComplexValue<sym> uj) {
        return vector_outer_product(imag(ui), real(uj)) - vector_outer_product(real(ui), imag(uj));
    }

    auto g_sin_minus_b_cos(ComplexTensor<sym> yij, ComplexValue<sym> ui, ComplexValue<sym> uj) {
        return real(yij) * ui_uj_sin_ij(ui, uj) - imag(yij) * ui_uj_cos_ij(ui, uj);
    }
    auto g_cos_plus_b_sin(ComplexTensor<sym> yij, ComplexValue<sym> ui, ComplexValue<sym> uj) {
        return real(yij) * ui_uj_cos_ij(ui, uj) + imag(yij) * ui_uj_sin_ij(ui, uj);
    }

    double iterate_unknown(ComplexValueVector<sym>& u, bool /*has_angle_measurement*/) {
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

    void calculate_result(YBus<sym> const& y_bus, MeasuredValues<sym> const& measured_value, MathOutput<sym>& output) {
        // call y bus
        output.branch = y_bus.template calculate_branch_flow<BranchMathOutput<sym>>(output.u);
        output.shunt = y_bus.template calculate_shunt_flow<ApplianceMathOutput<sym>>(output.u);
        output.bus_injection = y_bus.calculate_injection(output.u);
        std::tie(output.load_gen, output.source) =
            measured_value.calculate_load_gen_source(output.u, output.bus_injection);
    }

    auto diagonal_inverse(RealValue<sym> const& value) {
        return RealDiagonalTensor<sym>{static_cast<RealValue<sym>>(RealValue<sym>{1.0} / value)};
    }
};

template class NewtonRaphsonSESolver<true>;
template class NewtonRaphsonSESolver<false>;

} // namespace math_model_impl

template <bool sym> using NewtonRaphsonSESolver = math_model_impl::NewtonRaphsonSESolver<sym>;

} // namespace power_grid_model

#endif
