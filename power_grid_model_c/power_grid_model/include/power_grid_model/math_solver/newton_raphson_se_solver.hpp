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
    GetterType<2, 0> phi_p() { return this->template get_val<2, 0>(); }
    GetterType<3, 0> phi_q() { return this->template get_val<3, 0>(); }

    GetterType<0, 0> eta_theta() { return this->template get_val<0, 0>(); }
    GetterType<1, 0> eta_v() { return this->template get_val<1, 0>(); }
    GetterType<2, 0> tau_p() { return this->template get_val<2, 0>(); }
    GetterType<3, 0> tau_q() { return this->template get_val<3, 0>(); }
};

// block class for the right hand side in state estimation equation
template <bool sym> using NRSERhs = NRSEUnknown<sym>;

// class of 4*4 (12*12) se gain block
// [
//    [G, QT]
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

    GetterType<0, 2> qt_P_theta() { return this->template get_val<0, 2>(); }
    GetterType<0, 3> qt_P_v() { return this->template get_val<0, 3>(); }
    GetterType<1, 2> qt_Q_theta() { return this->template get_val<1, 2>(); }
    GetterType<1, 3> qt_Q_v() { return this->template get_val<1, 3>(); }

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
            auto const& abs_ui_inv = diagonal_inverse(x_[row].v());
            NRSERhs<sym>& rhs_block = del_x_rhs_[row];
            rhs_block = NRSERhs<sym>{};

            for (Idx data_idx_lu = row_indptr[row]; data_idx_lu != row_indptr[row + 1]; ++data_idx_lu) {
                Idx const col = col_indices[data_idx_lu];
                auto const& uj = current_u[col];
                RealDiagonalTensor<sym> const& abs_uj_inv = diagonal_inverse(x_[col].v());
                // get a reference and reset block to zero
                NRSEGainBlock<sym>& block = data_gain_[data_idx_lu];
                // Initialize the block (Diagonal block gets initialized outside this loop)
                if (row != col) {
                    block = NRSEGainBlock<sym>{};
                }
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
                            auto const gc_plus_bs_ii = g_cos_plus_b_sin(yii, ui, ui);
                            auto const gs_minus_bc_ii = g_sin_minus_b_cos(yii, ui, ui);

                            auto const calculated_p = sum_row(gc_plus_bs_ii);
                            auto const calculated_q = sum_row(gs_minus_bc_ii);

                            auto const measured_power = measured_value.shunt_power(obj);
                            auto const block_i = power_flow_jacobian_i(gs_minus_bc_ii, gc_plus_bs_ii, abs_ui_inv,
                                                                       calculated_p, calculated_q);
                            multiply_add_jacobian_blocks(block, rhs_block, block_i, block_i, measured_power,
                                                         calculated_p, calculated_q);
                        }
                    } else if (type == YBusElementType::bff || type == YBusElementType::bft) {
                        if (measured_value.has_branch_from(obj)) {
                            auto const& yii = param.branch_param[obj].yff();
                            auto const gc_plus_bs_ii = g_cos_plus_b_sin(yii, ui, ui);
                            auto const gs_minus_bc_ii = g_sin_minus_b_cos(yii, ui, ui);

                            auto const& yij = param.branch_param[obj].yft();
                            auto const gc_plus_bs_ij = g_cos_plus_b_sin(yij, ui, uj);
                            auto const gs_minus_bc_ij = g_sin_minus_b_cos(yij, ui, uj);

                            auto const calculated_p = sum_row(gc_plus_bs_ii + gc_plus_bs_ij);
                            auto const calculated_q = sum_row(gs_minus_bc_ii + gs_minus_bc_ij);

                            auto const measured_power = measured_value.branch_from_power(obj);
                            if (type == YBusElementType::bff) {
                                auto const block_i = power_flow_jacobian_i(gs_minus_bc_ii, gc_plus_bs_ii, abs_ui_inv,
                                                                           calculated_p, calculated_q);
                                multiply_add_jacobian_blocks(block, rhs_block, block_i, block_i, measured_power,
                                                             calculated_p, calculated_q);
                            } else if (type == YBusElementType::bft) {
                                auto const block_i = power_flow_jacobian_i(gs_minus_bc_ii, gc_plus_bs_ii, abs_ui_inv,
                                                                           calculated_p, calculated_q);
                                auto const block_j = power_flow_jacobian_j(gs_minus_bc_ij, gc_plus_bs_ij, abs_uj_inv);
                                multiply_add_jacobian_blocks(block, rhs_block, block_i, block_j, measured_power,
                                                             calculated_p, calculated_q);
                            }
                        }
                    } else if (type == YBusElementType::btt || type == YBusElementType::btf) {
                        if (measured_value.has_branch_to(obj)) {
                            auto const& yii = param.branch_param[obj].ytt();
                            auto const gc_plus_bs_jj = g_cos_plus_b_sin(yii, uj, uj);
                            auto const gs_minus_bc_jj = g_sin_minus_b_cos(yii, uj, uj);

                            auto const& yij = param.branch_param[obj].ytf();
                            auto const gc_plus_bs_ji = g_cos_plus_b_sin(yij, ui, uj);
                            auto const gs_minus_bc_ji = g_sin_minus_b_cos(yij, ui, uj);

                            auto const calculated_p = sum_row(gc_plus_bs_jj + gc_plus_bs_ji);
                            auto const calculated_q = sum_row(gs_minus_bc_jj + gs_minus_bc_ji);

                            auto const measured_power = measured_value.branch_to_power(obj);

                            if (type == YBusElementType::btt) {
                                auto const block_j = power_flow_jacobian_j(gs_minus_bc_jj, gc_plus_bs_jj, abs_uj_inv);
                                multiply_add_jacobian_blocks(block, rhs_block, block_j, block_j, measured_power,
                                                             calculated_p, calculated_q);
                            } else if (type == YBusElementType::btf) {
                                auto const block_i = power_flow_jacobian_i(gs_minus_bc_jj, gc_plus_bs_jj, abs_uj_inv,
                                                                           calculated_p, calculated_q);
                                auto const block_j = power_flow_jacobian_j(gs_minus_bc_ji, gc_plus_bs_ji, abs_uj_inv);
                                multiply_add_jacobian_blocks(block, rhs_block, block_j, block_i, measured_power,
                                                             calculated_p, calculated_q);
                            }
                        }
                    }
                }

                // Initialize diagonal block
                NRSEGainBlock<sym>& diag_block = data_gain_[y_bus.lu_diag()[row]];
                diag_block = NRSEGainBlock<sym>{};

                // fill block with injection measurement
                // injection measurement exist
                if (measured_value.has_bus_injection(row)) {
                    auto const& yij = y_bus.admittance()[data_idx];
                    auto const gc_plus_bs = g_cos_plus_b_sin(yij, ui, uj);
                    auto const gs_minus_bc = g_sin_minus_b_cos(yij, ui, uj);
                    auto const partial_calculated_p = sum_row(gc_plus_bs);
                    auto const partial_calculated_q = sum_row(gs_minus_bc);

                    // R_ii = -variance, only diagonal
                    if (row == col) {
                        // assign variance to diagonal of 3x3 tensor, for asym
                        auto const& injection = measured_value.bus_injection(row);
                        // add negative of the non-summation value then sign will be reversed when row is summed
                        auto const w_p = diagonal_inverse(injection.p_variance);
                        auto const w_q = diagonal_inverse(injection.q_variance);

                        auto const injection_jacobian = power_flow_jacobian_i(
                            gs_minus_bc, gc_plus_bs, abs_ui_inv, partial_calculated_p, partial_calculated_q);
                        add_injection_jacobian(block, injection_jacobian);

                        rhs_block.tau_p() += injection.value.real();
                        rhs_block.tau_q() += injection.value.imag();

                        // block.r_P_theta() += RealTensor<sym>{w_p * w_p};
                        // block.r_Q_v() += w_q * w_q;
                    } else {
                        auto const injection_jacobian = power_flow_jacobian_j(gs_minus_bc, gc_plus_bs, abs_uj_inv);
                        add_injection_jacobian(block, injection_jacobian);

                        // subtract f(x) incrementally
                        rhs_block.tau_p() -= partial_calculated_p;
                        rhs_block.tau_q() -= partial_calculated_q;
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

                // Lagrange Multiplier: eta_i += sum_j (q_ji^T . phi_j)
                rhs_block.eta_theta() +=
                    dot(block.q_P_theta(), x_[col].phi_p()) + dot(block.q_Q_theta(), x_[col].phi_q());
                rhs_block.eta_v() += dot(block.q_P_v(), x_[col].phi_p()) + dot(block.q_Q_v(), x_[col].phi_q());
            }
        }

        // loop all transpose entry for QH
        // assign the transpose of the transpose entry of Q
        for (Idx data_idx_lu = 0; data_idx_lu != y_bus.nnz_lu(); ++data_idx_lu) {
            // skip for fill-in
            if (y_bus.map_lu_y_bus()[data_idx_lu] == -1) {
                continue;
            }
            Idx const data_idx_tranpose = y_bus.lu_transpose_entry()[data_idx_lu];
            data_gain_[data_idx_lu].qt_P_theta() = data_gain_[data_idx_tranpose].q_P_theta();
            data_gain_[data_idx_lu].qt_P_v() = data_gain_[data_idx_tranpose].q_Q_theta();
            data_gain_[data_idx_lu].qt_Q_theta() = data_gain_[data_idx_tranpose].q_P_v();
            data_gain_[data_idx_lu].qt_Q_v() = data_gain_[data_idx_tranpose].q_Q_v();
        }
        // prefactorize
        sparse_solver_.prefactorize(data_gain_, perm_);
    }

    NRSEJacobian power_flow_jacobian_i(RealTensor<sym> const& gs_minus_bc, RealTensor<sym> const& gc_plus_bs,
                                       RealDiagonalTensor<sym> abs_ui_inv, RealValue<sym> calculated_p,
                                       RealValue<sym> calculated_q) {
        auto jacobian = power_flow_jacobian_j(gs_minus_bc, gc_plus_bs, abs_ui_inv);
        jacobian.dP_dt -= RealTensor<sym>{calculated_q};
        jacobian.dP_dv += dot(calculated_p, abs_ui_inv);
        jacobian.dQ_dt += RealTensor<sym>{calculated_p};
        jacobian.dQ_dv += dot(calculated_q, abs_ui_inv);
        return jacobian;
    }

    NRSEJacobian power_flow_jacobian_j(RealTensor<sym> const& gs_minus_bc, RealTensor<sym> const& gc_plus_bs,
                                       RealDiagonalTensor<sym> const& abs_uj_inv) {
        NRSEJacobian jacobian{};
        jacobian.dP_dt = gs_minus_bc;
        jacobian.dP_dv = -gc_plus_bs;
        jacobian.dQ_dt = dot(gc_plus_bs, abs_uj_inv);
        jacobian.dQ_dv = dot(gs_minus_bc, abs_uj_inv);
        return jacobian;
    }

    void multiply_add_jacobian_blocks(NRSEGainBlock<sym>& block, NRSERhs<sym>& rhs_block, NRSEJacobian block_1,
                                      NRSEJacobian block_2, PowerSensorCalcParam<sym> power_sensor,
                                      RealValue<sym> calculated_power_p, RealValue<sym> calculated_power_q) {
        auto const w_p = diagonal_inverse(power_sensor.p_variance);
        auto const w_q = diagonal_inverse(power_sensor.q_variance);
        auto const del_power_p = calculated_power_p - real(power_sensor.value);
        auto const del_power_q = calculated_power_q - imag(power_sensor.value);

        // matrix multiplication of F_k^T . w_k . F_k
        block.g_P_theta() += dot(w_p, block_1.dP_dt, block_2.dP_dt) + dot(w_q, block_1.dQ_dt, block_2.dQ_dt);
        block.g_P_v() += dot(w_p, block_1.dP_dv, block_2.dP_dv) + dot(w_q, block_1.dQ_dv, block_2.dQ_dv);
        block.g_Q_theta() += dot(w_p, block_1.dP_dt, block_2.dP_dv) + dot(w_q, block_1.dQ_dt, block_2.dQ_dv);
        block.g_Q_v() += dot(w_p, block_1.dP_dv, block_2.dP_dt) + dot(w_q, block_1.dQ_dv, block_2.dQ_dt);

        // matrix multiplication of F_k^T . w_k . (z - f(x))
        rhs_block.eta_theta() += dot(w_p, block_1.dP_dt, del_power_p) + dot(w_q, block_1.dQ_dt, del_power_q);
        rhs_block.eta_v() += dot(w_p, block_1.dP_dv, del_power_p) + dot(w_q, block_1.dQ_dv, del_power_q);
    }

    void add_injection_jacobian(NRSEGainBlock<sym>& block, NRSEJacobian jacobian_block) {
        block.q_P_theta() += jacobian_block.dP_dt;
        block.q_P_v() += jacobian_block.dP_dv;
        block.q_Q_theta() += jacobian_block.dQ_dt;
        block.q_Q_v() += jacobian_block.dQ_dv;
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
            // accumulate unknown
            x_[bus].theta() += del_x_rhs_[bus].theta();
            x_[bus].v() += x_[bus].v() * del_x_rhs_[bus].v();
            x_[bus].phi_p() += del_x_rhs_[bus].phi_p();
            x_[bus].phi_q() += del_x_rhs_[bus].phi_q();

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

} // namespace newton_raphson_se

using newton_raphson_se::NewtonRaphsonSESolver;

} // namespace power_grid_model::math_solver

#endif
