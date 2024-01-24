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
#include "common_solver_functions.hpp"
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
        RealTensor<sym> dP_dt{};
        RealTensor<sym> dP_dv{};
        RealTensor<sym> dQ_dt{};
        RealTensor<sym> dQ_dv{};

        NRSEJacobian& operator+=(NRSEJacobian const& other) {
            this->dP_dt += other.dP_dt;
            this->dP_dv += other.dP_dv;
            this->dQ_dt += other.dQ_dt;
            this->dQ_dv += other.dQ_dv;
            return *this;
        }
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
            x_[bus].phi_p() = 0.0;
            x_[bus].phi_q() = 0.0;
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
        detail::calculate_se_result<sym>(y_bus, measured_values, output);

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
        IdxVector const& lu_diag = y_bus.lu_diag();

        // loop data index, all rows and columns
        for (Idx row = 0; row != n_bus_; ++row) {
            auto const& ui = current_u[row];
            auto const& abs_ui_inv = diagonal_inverse(x_[row].v());
            auto const ui_ui_conj = vector_outer_product(ui, conj(ui));

            NRSERhs<sym>& rhs_block = del_x_rhs_[row];
            rhs_block.clear();

            // get a reference and reset block to zero
            NRSEGainBlock<sym>& diag_block = data_gain_[lu_diag[row]];
            diag_block.clear();

            for (Idx data_idx_lu = row_indptr[row]; data_idx_lu != row_indptr[row + 1]; ++data_idx_lu) {
                Idx const col = col_indices[data_idx_lu];
                auto const& uj = current_u[col];
                auto const ui_uj_conj = vector_outer_product(ui, conj(uj));

                RealDiagonalTensor<sym> const& abs_uj_inv = diagonal_inverse(x_[col].v());
                // get a reference and reset block to zero
                NRSEGainBlock<sym>& block = data_gain_[data_idx_lu];
                // Diagonal block is being cleared outside this loop
                if (row != col) {
                    block.clear();
                }
                // get data idx of y bus,
                // skip for a fill-in
                Idx const data_idx = y_bus.map_lu_y_bus()[data_idx_lu];
                if (data_idx == -1) {
                    continue;
                }
                // fill block with voltage measurement, only diagonal
                if (row == col) {
                    add_voltage_measurements(block, rhs_block, measured_value, row);
                }
                // fill block with branch, shunt measurement
                for (Idx element_idx = y_bus.y_bus_entry_indptr()[data_idx];
                     element_idx != y_bus.y_bus_entry_indptr()[data_idx + 1]; ++element_idx) {
                    Idx const obj = y_bus.y_bus_element()[element_idx].idx;
                    YBusElementType const type = y_bus.y_bus_element()[element_idx].element_type;
                    if (type == YBusElementType::shunt && measured_value.has_shunt(obj)) {
                        auto const& yii = param.shunt_param[obj];
                        auto const& measured_power = measured_value.shunt_power(obj);
                        auto const jac_complex = jac_complex_intermediate_form(yii, ui_uj_conj);
                        auto const jac_complex_unit_other = dot(jac_complex, abs_ui_inv);
                        auto const calculated_power = sum_row(jac_complex);
                        auto const calculated_power_unit_self = sum_row(jac_complex_unit_other);

                        auto block_i = calculate_jacobian(jac_complex, jac_complex_unit_other);
                        block_i += jacobian_diagonal_component(calculated_power_unit_self, calculated_power);
                        multiply_add_jacobian_blocks(block, rhs_block, block_i, block_i, measured_power,
                                                     calculated_power);

                    } else if ((type == YBusElementType::bff || type == YBusElementType::bft) &&
                               measured_value.has_branch_from(obj)) {
                        auto const& y_xi_xi = param.branch_param[obj].yff();
                        auto const& y_xi_mu = param.branch_param[obj].yft();

                        auto const hnml_u_chi_u_chi_y_xi_xi = jac_complex_intermediate_form(y_xi_xi, ui_ui_conj);
                        auto const hnml_u_chi_u_psi_y_xi_mu = jac_complex_intermediate_form(y_xi_mu, ui_uj_conj);

                        auto const calculated_power = sum_row(hnml_u_chi_u_chi_y_xi_xi + hnml_u_chi_u_psi_y_xi_mu);
                        auto const calculated_power_unit_self = calculated_power / ui;
                        auto const& measured_power = measured_value.branch_from_power(obj);

                        auto const hnml_u_chi_u_chi_y_xi_xi_u_chi_inv = dot(hnml_u_chi_u_chi_y_xi_xi, abs_ui_inv);
                        auto block_i = calculate_jacobian(hnml_u_chi_u_chi_y_xi_xi, hnml_u_chi_u_chi_y_xi_xi_u_chi_inv);
                        // auto const hnml_u_chi_u_chi_y_xi_mu_u_chi_inv = dot(hnml_u_chi_u_psi_y_xi_mu, abs_ui_inv);
                        block_i += jacobian_diagonal_component(calculated_power_unit_self, calculated_power);

                        if (type == YBusElementType::bff) {
                            multiply_add_jacobian_blocks(block, rhs_block, block_i, block_i, measured_power,
                                                         calculated_power);
                        } else if (type == YBusElementType::bft) {
                            auto const hnml_u_chi_u_psi_y_xi_mu_u_psi_inv = dot(hnml_u_chi_u_psi_y_xi_mu, abs_uj_inv);
                            auto const block_j =
                                calculate_jacobian(hnml_u_chi_u_psi_y_xi_mu, hnml_u_chi_u_psi_y_xi_mu_u_psi_inv);
                            multiply_add_jacobian_blocks(block, rhs_block, block_i, block_j, measured_power,
                                                         calculated_power);
                        }
                    } else if ((type == YBusElementType::btt || type == YBusElementType::btf) &&
                               measured_value.has_branch_to(obj)) {
                        auto const& y_xi_xi = param.branch_param[obj].ytt();
                        auto const& y_xi_mu = param.branch_param[obj].ytf();

                        auto const hnml_u_chi_u_chi_y_xi_xi = jac_complex_intermediate_form(y_xi_xi, ui_ui_conj);
                        auto const hnml_u_chi_u_psi_y_xi_mu = jac_complex_intermediate_form(y_xi_mu, conj(ui_uj_conj));

                        auto const calculated_power = sum_row(hnml_u_chi_u_chi_y_xi_xi + hnml_u_chi_u_psi_y_xi_mu);
                        auto const calculated_power_unit_self = calculated_power / uj;
                        auto const& measured_power = measured_value.branch_to_power(obj);

                        auto const hnml_u_chi_u_chi_y_xi_xi_u_chi_inv = dot(hnml_u_chi_u_chi_y_xi_xi, abs_ui_inv);
                        auto block_j = calculate_jacobian(hnml_u_chi_u_chi_y_xi_xi, hnml_u_chi_u_chi_y_xi_xi_u_chi_inv);
                        // auto const hnml_u_chi_u_chi_y_xi_mu_u_chi_inv = dot(hnml_u_chi_u_psi_y_xi_mu, abs_ui_inv);
                        block_j += jacobian_diagonal_component(calculated_power_unit_self, calculated_power);

                        if (type == YBusElementType::btt) {
                            multiply_add_jacobian_blocks(block, rhs_block, block_j, block_j, measured_power,
                                                         calculated_power);

                        } else if (type == YBusElementType::btf) {
                            auto const hnml_u_chi_u_psi_y_xi_mu_u_psi_inv = dot(hnml_u_chi_u_psi_y_xi_mu, abs_uj_inv);
                            auto const block_i =
                                calculate_jacobian(hnml_u_chi_u_psi_y_xi_mu, hnml_u_chi_u_psi_y_xi_mu_u_psi_inv);
                            multiply_add_jacobian_blocks(block, rhs_block, block_j, block_i, measured_power,
                                                         calculated_power);
                        }
                    }
                }

                // fill block with injection measurement
                // injection measurement exist
                if (measured_value.has_bus_injection(row)) {
                    auto const& yij = y_bus.admittance()[data_idx];
                    auto const jac_complex_ft = jac_complex_intermediate_form(yij, ui_uj_conj);
                    auto const jac_complex_unit_other_ft = dot(jac_complex_ft, abs_uj_inv);
                    auto const jac_complex_unit_self_ft = dot(jac_complex_ft, abs_ui_inv);
                    auto const calculated_power = sum_row(jac_complex_ft);
                    auto const calculated_power_unit_self = sum_row(jac_complex_unit_self_ft);

                    auto const injection_jac = calculate_jacobian(jac_complex_ft, jac_complex_unit_other_ft);
                    add_injection_jacobian(block, injection_jac);

                    // add summed component to the diagonal block
                    auto const injection_jac_diagonal =
                        jacobian_diagonal_component(calculated_power_unit_self, calculated_power);
                    add_injection_jacobian(diag_block, injection_jac_diagonal);
                    // subtract f(x) incrementally
                    rhs_block.tau_p() -= real(calculated_power);
                    rhs_block.tau_q() -= imag(calculated_power);

                    // R_ii = -variance, only diagonal
                    if (row == col) {
                        // assign variance to diagonal of 3x3 tensor, for asym
                        auto const& injection = measured_value.bus_injection(row);
                        rhs_block.tau_p() += injection.value.real();
                        rhs_block.tau_q() += injection.value.imag();

                        block.r_P_theta() -=
                            RealTensor<sym>{RealValue<sym>{RealValue<sym>{1.0} / injection.p_variance}};
                        block.r_Q_v() -= RealTensor<sym>{RealValue<sym>{RealValue<sym>{1.0} / injection.q_variance}};
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
        make_symmetric_from_lower_triangle(y_bus);

        // prefactorize
        sparse_solver_.prefactorize(data_gain_, perm_);
    }

    void make_symmetric_from_lower_triangle(YBus<sym> const& y_bus) {
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
    }

    void add_voltage_measurements(NRSEGainBlock<sym>& block, NRSERhs<sym>& rhs_block,
                                  MeasuredValues<sym> const& measured_value, Idx const& bus) {
        if (!measured_value.has_voltage(bus)) {
            return;
        }

        // G += 1.0 / variance
        // for 3x3 tensor, fill diagonal
        auto const w_v = RealTensor<sym>{1.0 / measured_value.voltage_var(bus)};
        auto const abs_measured_v = detail::cabs_or_real<sym>(measured_value.voltage(bus));
        auto const del_v = abs_measured_v - x_[bus].v();

        auto w_angle = RealTensor<sym>{1.0};
        auto del_theta = RealValue<sym>{-x_[bus].theta()};

        auto const virtual_angle_measurement_bus = measured_value.has_angle_measurement(math_topo_->slack_bus)
                                                       ? math_topo_->slack_bus
                                                       : measured_value.first_voltage_measurement();

        if (measured_value.has_angle() || bus != virtual_angle_measurement_bus) {
            if (!measured_value.has_angle_measurement(bus)) {
                w_angle = RealTensor<sym>{0.0};
            }
            del_theta += RealValue<sym>{arg(measured_value.voltage(bus))};
        } else {
            del_theta += phase_shifted_zero_angle();
        }

        block.g_P_theta() += w_angle;
        block.g_Q_v() += w_v;
        rhs_block.eta_theta() += dot(w_angle, del_theta);
        rhs_block.eta_v() += dot(w_v, del_v);
    }

    NRSEJacobian jacobian_diagonal_component(ComplexValue<sym> calculated_power_unit_self,
                                             ComplexValue<sym> calculated_power) {
        NRSEJacobian jacobian{};
        jacobian.dP_dt -= RealTensor<sym>{RealValue<sym>{imag(calculated_power)}};
        jacobian.dP_dv += RealTensor<sym>{RealValue<sym>{real(calculated_power_unit_self)}};
        jacobian.dQ_dt += RealTensor<sym>{RealValue<sym>{real(calculated_power)}};
        jacobian.dQ_dv += RealTensor<sym>{RealValue<sym>{imag(calculated_power_unit_self)}};
        return jacobian;
    }

    NRSEJacobian calculate_jacobian(ComplexTensor<sym> const& jac_complex, ComplexTensor<sym> jac_complex_unit_other) {
        NRSEJacobian jacobian{};
        jacobian.dP_dt = imag(jac_complex);
        jacobian.dP_dv = real(jac_complex_unit_other);
        jacobian.dQ_dt = -real(jac_complex);
        jacobian.dQ_dv = imag(jac_complex_unit_other);
        return jacobian;
    }

    void multiply_add_jacobian_blocks(NRSEGainBlock<sym>& block, NRSERhs<sym>& rhs_block, NRSEJacobian const& block_1,
                                      NRSEJacobian const& block_2, PowerSensorCalcParam<sym> const& power_sensor,
                                      ComplexValue<sym> const& calculated_power) {
        auto const w_p = diagonal_inverse(power_sensor.p_variance);
        auto const w_q = diagonal_inverse(power_sensor.q_variance);
        auto const del_power = calculated_power - power_sensor.value;

        // matrix multiplication of F_k^T . w_k
        NRSEJacobian temp_product{};
        temp_product.dP_dt = dot(w_p, block_1.dP_dt);
        temp_product.dP_dv = dot(w_p, block_1.dP_dv);
        temp_product.dQ_dt = dot(w_q, block_1.dQ_dt);
        temp_product.dQ_dv = dot(w_q, block_1.dQ_dv);

        // matrix multiplication of F_k^T . w_k . F_k
        block.g_P_theta() += dot(temp_product.dP_dt, block_2.dP_dt) + dot(temp_product.dQ_dt, block_2.dQ_dt);
        block.g_P_v() += dot(temp_product.dP_dv, block_2.dP_dv) + dot(temp_product.dQ_dv, block_2.dQ_dv);
        block.g_Q_theta() += dot(temp_product.dP_dt, block_2.dP_dv) + dot(temp_product.dQ_dt, block_2.dQ_dv);
        block.g_Q_v() += dot(temp_product.dP_dv, block_2.dP_dt) + dot(temp_product.dQ_dv, block_2.dQ_dt);

        // matrix multiplication of F_k^T . w_k . (z - f(x))
        rhs_block.eta_theta() += dot(temp_product.dP_dt, real(del_power)) + dot(temp_product.dQ_dt, imag(del_power));
        rhs_block.eta_v() += dot(temp_product.dP_dv, real(del_power)) + dot(temp_product.dQ_dv, imag(del_power));
    }

    void add_injection_jacobian(NRSEGainBlock<sym>& block, NRSEJacobian const& jacobian_block) {
        block.q_P_theta() += jacobian_block.dP_dt;
        block.q_P_v() += jacobian_block.dP_dv;
        block.q_Q_theta() += jacobian_block.dQ_dt;
        block.q_Q_v() += jacobian_block.dQ_dv;
    }

    ComplexTensor<sym> jac_complex_intermediate_form(ComplexTensor<sym> const& yij,
                                                     ComplexTensor<sym> const& ui_conj_uj) {
        return conj(yij) * ui_conj_uj;
    }

    double iterate_unknown(ComplexValueVector<sym>& u) {
        double max_dev = 0.0;
        for (Idx bus = 0; bus != n_bus_; ++bus) {
            // accumulate the unknown variable
            auto const old_abs_u = x_[bus].v();
            x_[bus].theta() += del_x_rhs_[bus].theta();
            x_[bus].v() += del_x_rhs_[bus].v();
            x_[bus].phi_p() += del_x_rhs_[bus].phi_p();
            x_[bus].phi_q() += del_x_rhs_[bus].phi_q();

            // phase offset to calculated voltage as normalized
            u[bus] = x_[bus].v() * exp(1.0i * x_[bus].theta());
            // get dev of last iteration, get max
            double const dev = max_val(x_[bus].v() - old_abs_u);
            max_dev = std::max(dev, max_dev);
        }
        return max_dev;
    }

    auto diagonal_inverse(RealValue<sym> const& value) {
        return RealDiagonalTensor<sym>{static_cast<RealValue<sym>>(RealValue<sym>{1.0} / value)};
    }

    auto phase_shifted_zero_angle() {
        if constexpr (sym) {
            return 0.0;
        } else {
            return RealValue<false>{0.0, deg_240, deg_120};
        }
    }
};

template class NewtonRaphsonSESolver<true>;
template class NewtonRaphsonSESolver<false>;

} // namespace newton_raphson_se

using newton_raphson_se::NewtonRaphsonSESolver;

} // namespace power_grid_model::math_solver

#endif
