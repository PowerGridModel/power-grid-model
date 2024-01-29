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
          delta_x_rhs_(y_bus.size()),
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
        initialize_unknown(output.u, measured_values);

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
            sparse_solver_.solve_with_prefactorized_matrix(data_gain_, perm_, delta_x_rhs_, delta_x_rhs_);
            sub_timer = Timer(calculation_info, 2226, "Iterate unknown");
            max_dev = iterate_unknown(output.u, measured_values);
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
    std::vector<NRSERhs<sym>> delta_x_rhs_;
    // voltage of current iteration
    std::vector<NRSERhs<sym>> x_;
    // solver
    SparseLUSolver<NRSEGainBlock<sym>, NRSERhs<sym>, NRSEUnknown<sym>> sparse_solver_;
    typename SparseLUSolver<NRSEGainBlock<sym>, NRSERhs<sym>, NRSEUnknown<sym>>::BlockPermArray perm_;

    // array selection function pointer
    static constexpr std::array has_branch_{&MeasuredValues<sym>::has_branch_from, &MeasuredValues<sym>::has_branch_to};
    static constexpr std::array branch_power_{&MeasuredValues<sym>::branch_from_power,
                                              &MeasuredValues<sym>::branch_to_power};

    void initialize_unknown(ComplexValueVector<sym>& initial_u, MeasuredValues<sym> const& measured_values) {
        reset_unknown();
        RealValue<sym> const mean_angle_shift = measured_values.mean_angle_shift();
        for (Idx bus = 0; bus != n_bus_; ++bus) {
            x_[bus].theta() = mean_angle_shift + math_topo_->phase_shift[bus];
            if (measured_values.has_voltage(bus)) {
                if (measured_values.has_angle_measurement(bus)) {
                    x_[bus].theta() = arg(measured_values.voltage(bus));
                }
                x_[bus].v() = detail::cabs_or_real<sym>(measured_values.voltage(bus));
            }
            initial_u[bus] = x_[bus].v() * exp(1.0i * x_[bus].theta());
        }
    }

    void reset_unknown() {
        static auto const default_unknown = [] {
            NRSERhs<sym> x;
            x.v() = 1.0;
            x.theta() = 0.0;
            x.phi_p() = 0.0;
            x.phi_q() = 0.0;
            return x;
        }();
        std::ranges::fill(x_, default_unknown);
    }

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

            NRSERhs<sym>& rhs_block = delta_x_rhs_[row];
            rhs_block.clear();

            // get a reference and reset block to zero
            NRSEGainBlock<sym>& diag_block = data_gain_[lu_diag[row]];
            diag_block.clear();

            for (Idx data_idx_lu = row_indptr[row]; data_idx_lu != row_indptr[row + 1]; ++data_idx_lu) {
                Idx const col = col_indices[data_idx_lu];
                auto const& uj = current_u[col];
                auto const ui_uj_conj = vector_outer_product(ui, conj(uj));
                auto const uj_uj_conj = vector_outer_product(uj, conj(uj));

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
                    process_voltage_measurements(block, rhs_block, measured_value, row);
                }
                // fill block with branch, shunt measurement
                for (Idx element_idx = y_bus.y_bus_entry_indptr()[data_idx];
                     element_idx != y_bus.y_bus_entry_indptr()[data_idx + 1]; ++element_idx) {
                    Idx const obj = y_bus.y_bus_element()[element_idx].idx;
                    YBusElementType const type = y_bus.y_bus_element()[element_idx].element_type;
                    if (type == YBusElementType::shunt) {
                        if (measured_value.has_shunt(obj)) {
                            auto const& yii = param.shunt_param[obj];
                            auto const& measured_power = measured_value.shunt_power(obj);
                            process_shunt_measurement(block, rhs_block, yii, ui_ui_conj, abs_ui_inv, measured_power);
                        }
                    } else if (type == YBusElementType::bft || type == YBusElementType::btf) {
                        // measured at from-side: 0, to-side: 1
                        for (IntS const measured_side : std::array<IntS, 2>{0, 1}) {
                            // has measurement
                            if (std::invoke(has_branch_[measured_side], measured_value, obj)) {
                                // branch from- and to-side index at 0, and 1 position
                                IntS const type_ft_or_tf = static_cast<IntS>(type) / 2;
                                // G += Y{side, b0}^H * (variance^-1) * Y{side, b1}
                                auto const& power = std::invoke(branch_power_[measured_side], measured_value, obj);
                                auto const& y_xi_xi = param.branch_param[obj].value[2 * measured_side];
                                auto const& y_xi_mu = param.branch_param[obj].value[2 * measured_side + 1];
                                if (type == YBusElementType::bft) {
                                    process_branch_measurement(block, diag_block, rhs_block, y_xi_xi, y_xi_mu,
                                                               ui_ui_conj, ui_uj_conj, abs_ui_inv, abs_uj_inv, power,
                                                               true);
                                } else {
                                    process_branch_measurement(block, diag_block, rhs_block, y_xi_xi, y_xi_mu,
                                                               uj_uj_conj, conj(ui_uj_conj), abs_uj_inv, abs_ui_inv,
                                                               power, false);
                                }
                            }
                        }
                    } else {
                        assert(type == YBusElementType::bff || type == YBusElementType::btt);
                    }
                }

                // fill block with injection measurement constraints
                if (measured_value.has_bus_injection(row)) {
                    auto const& yij = y_bus.admittance()[data_idx];
                    process_injection_row(block, diag_block, rhs_block, yij, ui_uj_conj, abs_ui_inv, abs_uj_inv);

                    // R_ii = -variance, only diagonal
                    if (row == col) {
                        // assign variance to diagonal of 3x3 tensor, for asym
                        auto const& injection = measured_value.bus_injection(row);
                        rhs_block.tau_p() += injection.value.real();
                        rhs_block.tau_q() += injection.value.imag();

                        block.r_P_theta() = RealTensor<sym>{RealValue<sym>{-injection.p_variance}};
                        block.r_Q_v() = RealTensor<sym>{RealValue<sym>{-injection.q_variance}};
                    }
                } else {
                    // virtually remove constraints from equation
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

    void process_injection_row(NRSEGainBlock<sym>& block, NRSEGainBlock<sym>& diag_block, NRSERhs<sym>& rhs_block,
                               auto const& yij, auto const& ui_uj_conj, auto const& abs_ui_iv,
                               RealDiagonalTensor<sym> const& abs_uj_inv) {
        auto const hnml_complex_ft = hnml_complex_form(yij, ui_uj_conj);
        auto const hnml_complex_abs_uj_inv_ft = dot(hnml_complex_ft, abs_uj_inv);
        auto const f_x_complex_row = sum_row(hnml_complex_ft);
        auto const f_x_complex_abs_uj_inv_row = dot(abs_ui_iv, f_x_complex_row);

        auto const injection_jac = calculate_jacobian(hnml_complex_ft, hnml_complex_abs_uj_inv_ft);
        add_injection_jacobian(block, injection_jac);

        // add paritial sum to the diagonal block and subtract from rhs for current row
        auto const injection_jac_diagonal = jacobian_diagonal_component(f_x_complex_abs_uj_inv_row, f_x_complex_row);
        add_injection_jacobian(diag_block, injection_jac_diagonal);
        rhs_block.tau_p() -= real(f_x_complex_row);
        rhs_block.tau_q() -= imag(f_x_complex_row);
    }

    void process_shunt_measurement(NRSEGainBlock<sym>& block, NRSERhs<sym>& rhs_block, auto const& yii,
                                   auto const& ui_ui_conj, auto const& abs_ui_inv, auto const& measured_power) {
        auto const hnml_ui_ui_yii = hnml_complex_form(yii, ui_ui_conj);
        auto const hnml_ui_ui_yii_abs_ui_inv = dot(hnml_ui_ui_yii, abs_ui_inv);
        auto const f_x_complex = sum_row(hnml_ui_ui_yii);
        auto const f_x_complex_abs_ui_inv = sum_row(hnml_ui_ui_yii_abs_ui_inv);

        auto jac_block = calculate_jacobian(hnml_ui_ui_yii, hnml_ui_ui_yii_abs_ui_inv);
        jac_block += jacobian_diagonal_component(f_x_complex_abs_ui_inv, f_x_complex);
        auto const& block_f_T_k_w = transpose_multiply_weight(jac_block, measured_power);
        multiply_add_jacobian_blocks_lhs(block, block_f_T_k_w, jac_block);
        multiply_add_jacobian_blocks_rhs(rhs_block, block_f_T_k_w, measured_power, f_x_complex);
    }

    void process_branch_measurement(NRSEGainBlock<sym>& block, NRSEGainBlock<sym>& diag_block, NRSERhs<sym>& rhs_block,
                                    const auto& y_xi_xi, const auto& y_xi_mu, const auto& u_chi_u_chi_conj,
                                    const auto& u_chi_u_psi_conj, const auto& abs_u_chi_inv, const auto& abs_u_psi_inv,
                                    const auto& measured_power, bool multiply_with_i_transpose) {
        auto const hnml_u_chi_u_chi_y_xi_xi = hnml_complex_form(y_xi_xi, u_chi_u_chi_conj);
        auto const hnml_u_chi_u_psi_y_xi_mu = hnml_complex_form(y_xi_mu, u_chi_u_psi_conj);

        auto const f_x_complex = sum_row(hnml_u_chi_u_chi_y_xi_xi + hnml_u_chi_u_psi_y_xi_mu);
        auto const f_x_complex_u_chi_inv = dot(abs_u_chi_inv, f_x_complex);

        auto const hnml_u_chi_u_chi_y_xi_xi_u_chi_inv = dot(hnml_u_chi_u_chi_y_xi_xi, abs_u_chi_inv);
        auto const hnml_u_chi_u_psi_y_xi_mu_u_psi_inv = dot(hnml_u_chi_u_psi_y_xi_mu, abs_u_psi_inv);

        auto block_ii_or_jj = calculate_jacobian(hnml_u_chi_u_chi_y_xi_xi, hnml_u_chi_u_chi_y_xi_xi_u_chi_inv);
        block_ii_or_jj += jacobian_diagonal_component(f_x_complex_u_chi_inv, f_x_complex);
        auto const block_ij_or_ji = calculate_jacobian(hnml_u_chi_u_psi_y_xi_mu, hnml_u_chi_u_psi_y_xi_mu_u_psi_inv);

        if (multiply_with_i_transpose) {
            multiply_add_branch_blocks(block, diag_block, rhs_block, block_ii_or_jj, block_ij_or_ji, measured_power,
                                       f_x_complex);
        } else {
            multiply_add_branch_blocks(block, diag_block, rhs_block, block_ij_or_ji, block_ii_or_jj, measured_power,
                                       f_x_complex);
        }
    }

    void multiply_add_branch_blocks(NRSEGainBlock<sym>& block, NRSEGainBlock<sym>& diag_block, NRSERhs<sym>& rhs_block,
                                    auto& left_block, const auto& right_block, const auto& measured_power,
                                    const auto& f_x_complex) {
        auto const& block_f_T_k_w = transpose_multiply_weight(left_block, measured_power);

        multiply_add_jacobian_blocks_lhs(diag_block, block_f_T_k_w, left_block);
        multiply_add_jacobian_blocks_rhs(rhs_block, block_f_T_k_w, measured_power, f_x_complex);

        multiply_add_jacobian_blocks_lhs(block, block_f_T_k_w, right_block);
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

    void process_voltage_measurements(NRSEGainBlock<sym>& block, NRSERhs<sym>& rhs_block,
                                      MeasuredValues<sym> const& measured_value, Idx const& bus) {
        if (!measured_value.has_voltage(bus)) {
            return;
        }

        // G += 1.0 / variance
        // for 3x3 tensor, fill diagonal
        auto const w_v = RealTensor<sym>{1.0 / measured_value.voltage_var(bus)};
        auto const abs_measured_v = detail::cabs_or_real<sym>(measured_value.voltage(bus));
        auto const delta_v = abs_measured_v - x_[bus].v();

        auto const virtual_angle_measurement_bus = measured_value.has_angle_measurement(math_topo_->slack_bus)
                                                       ? math_topo_->slack_bus
                                                       : measured_value.first_voltage_measurement();

        RealTensor<sym> w_angle{};
        RealValue<sym> delta_theta{};
        if (measured_value.has_angle_measurement(bus)) {
            delta_theta = RealValue<sym>{arg(measured_value.voltage(bus))} - RealValue<sym>{x_[bus].theta()};
            w_angle = RealTensor<sym>{1.0};
        } else if (bus == virtual_angle_measurement_bus && !measured_value.has_angle()) {
            delta_theta = phase_shifted_zero_angle() - RealValue<sym>{x_[bus].theta()};
            w_angle = RealTensor<sym>{1.0};
        }

        block.g_P_theta() += w_angle;
        block.g_Q_v() += w_v;
        rhs_block.eta_theta() += dot(w_angle, delta_theta);
        rhs_block.eta_v() += dot(w_v, delta_v);
    }

    /**
     * @brief The second part to add to the F_k(u1, u1, y11) block for shunt flow.
     * The members are -D[Q], D[P] . D[V]^-1, D[P], D[Q] . D[V]^-1,
     *
     * @param f_x_complex_v_inv (P_i + j * Q_i) / abs(u1)
     * @param f_x_complex P_i + j * Q_i
     * @return  second part of F_k block
     */
    static NRSEJacobian jacobian_diagonal_component(ComplexValue<sym> const& f_x_complex_v_inv,
                                                    ComplexValue<sym> const& f_x_complex) {
        NRSEJacobian jacobian{};
        jacobian.dP_dt = -RealTensor<sym>{RealValue<sym>{imag(f_x_complex)}};
        jacobian.dP_dv = RealTensor<sym>{RealValue<sym>{real(f_x_complex_v_inv)}};
        jacobian.dQ_dt = RealTensor<sym>{RealValue<sym>{real(f_x_complex)}};
        jacobian.dQ_dv = RealTensor<sym>{RealValue<sym>{imag(f_x_complex_v_inv)}};
        return jacobian;
    }

    /**
     * @brief Calculate F_k(u1, u2, y12)^T . W_k. Hence first transpose, then dot product.
     * where W_k = [[p_variance, 0], [0, q_variance]]
     *
     * @param jac_block F_k(u1, u2, y12)
     * @param power_sensor object with members p_variance and q_variance
     * @return  F_k(u1, u2, y12)^T . W
     */
    NRSEJacobian transpose_multiply_weight(NRSEJacobian const& jac_block,
                                           PowerSensorCalcParam<sym> const& power_sensor) {
        auto const w_p = diagonal_inverse(power_sensor.p_variance);
        auto const w_q = diagonal_inverse(power_sensor.q_variance);

        NRSEJacobian product{};
        product.dP_dt = dot(w_p, jac_block.dP_dt);
        product.dP_dv = dot(w_q, jac_block.dQ_dt);
        product.dQ_dt = dot(w_p, jac_block.dP_dv);
        product.dQ_dv = dot(w_q, jac_block.dQ_dv);
        return product;
    }

    /**
     * @brief Matrix multiply F_{k,1}^T . w_k and F_{k,2}^T.
     * Then add the product to G of gain block
     *
     * @param lhs_block Gain block, G of the LHS
     * @param f_T_k_w F_{k,1}^T . w_k
     * @param f_i_or_j F_{k,2}^T
     */
    static void multiply_add_jacobian_blocks_lhs(NRSEGainBlock<sym>& lhs_block, NRSEJacobian const& f_T_k_w,
                                                 NRSEJacobian const& f_i_or_j) {
        lhs_block.g_P_theta() += dot(f_T_k_w.dP_dt, f_i_or_j.dP_dt) + dot(f_T_k_w.dP_dv, f_i_or_j.dQ_dt);
        lhs_block.g_P_v() += dot(f_T_k_w.dP_dt, f_i_or_j.dP_dv) + dot(f_T_k_w.dP_dv, f_i_or_j.dQ_dv);
        lhs_block.g_Q_theta() += dot(f_T_k_w.dQ_dt, f_i_or_j.dP_dt) + dot(f_T_k_w.dQ_dv, f_i_or_j.dQ_dt);
        lhs_block.g_Q_v() += dot(f_T_k_w.dQ_dt, f_i_or_j.dP_dv) + dot(f_T_k_w.dQ_dv, f_i_or_j.dQ_dv);
    }

    static void multiply_add_jacobian_blocks_rhs(NRSERhs<sym>& rhs_block, NRSEJacobian const& block_f_T_k_w,
                                                 PowerSensorCalcParam<sym> const& power_sensor,
                                                 ComplexValue<sym> const& f_x_complex) {
        auto const delta_power = power_sensor.value - f_x_complex;

        // matrix multiplication of F_k^T . w_k . (z - f(x))
        rhs_block.eta_theta() +=
            dot(block_f_T_k_w.dP_dt, real(delta_power)) + dot(block_f_T_k_w.dP_dv, imag(delta_power));
        rhs_block.eta_v() += dot(block_f_T_k_w.dQ_dt, real(delta_power)) + dot(block_f_T_k_w.dQ_dv, imag(delta_power));
    }

    static void add_injection_jacobian(NRSEGainBlock<sym>& block, NRSEJacobian const& jacobian_block) {
        block.q_P_theta() += jacobian_block.dP_dt;
        block.q_P_v() += jacobian_block.dP_dv;
        block.q_Q_theta() += jacobian_block.dQ_dt;
        block.q_Q_v() += jacobian_block.dQ_dv;
    }

    /**
     * @brief Construct the F_k(u1, u2, y12) block using helper function of hnml complex form
     * The 4 members are H, N, M, L in the order.
     *
     * @param hnml_complex hnml_complex
     * @param hnml_complex_v_inv hnml_complex / abs(u2)
     * @return  F_k(u1, u2, y12)
     */
    static NRSEJacobian calculate_jacobian(ComplexTensor<sym> const& hnml_complex,
                                           ComplexTensor<sym> const& hnml_complex_v_inv) {
        NRSEJacobian jacobian{};
        jacobian.dP_dt = imag(hnml_complex);
        jacobian.dP_dv = real(hnml_complex_v_inv);
        jacobian.dQ_dt = -real(hnml_complex);
        jacobian.dQ_dv = imag(hnml_complex_v_inv);
        return jacobian;
    }

    /**
     * @brief Helper function for all G cos and B sin calculations
     *
     * @param yij admittance y12
     * @param ui_uj_conj vector outer product of u1 and conj(u2)
     * @return  -M(u1, u2, y12) + j * H(u1, u2, y12)
     */
    static ComplexTensor<sym> hnml_complex_form(ComplexTensor<sym> const& yij, ComplexTensor<sym> const& ui_uj_conj) {
        return conj(yij) * ui_uj_conj;
    }

    double iterate_unknown(ComplexValueVector<sym>& u, MeasuredValues<sym> measured_values) {
        double max_dev = 0.0;
        // phase shift anti offset of slack bus, phase a
        // if no angle measurement is present
        double const angle_offset = [&]() -> double {
            if (measured_values.has_angle()) {
                return 0.0;
            }
            auto const& theta = x_[math_topo_->slack_bus].theta();
            if constexpr (sym) {
                return theta;
            } else {
                return theta(0);
            }
        }();

        for (Idx bus = 0; bus != n_bus_; ++bus) {
            // accumulate the unknown variable
            x_[bus].theta() += delta_x_rhs_[bus].theta() - RealValue<sym>{angle_offset};
            x_[bus].v() += delta_x_rhs_[bus].v();
            if (measured_values.has_bus_injection(bus) && any_zero(measured_values.bus_injection(bus).p_variance)) {
                x_[bus].phi_p() += delta_x_rhs_[bus].phi_p();
            }
            if (measured_values.has_bus_injection(bus) && any_zero(measured_values.bus_injection(bus).q_variance)) {
                x_[bus].phi_q() += delta_x_rhs_[bus].phi_q();
            }

            auto const old_u = u[bus];
            u[bus] = x_[bus].v() * exp(1.0i * x_[bus].theta());
            // get dev of last iteration, get max
            double const dev = max_val(cabs(u[bus] - old_u));
            max_dev = std::max(dev, max_dev);
        }
        return max_dev;
    }

    static auto diagonal_inverse(RealValue<sym> const& value) {
        return RealDiagonalTensor<sym>{static_cast<RealValue<sym>>(RealValue<sym>{1.0} / value)};
    }

    static auto phase_shifted_zero_angle() {
        if constexpr (sym) {
            return 0.0;
        } else {
            return RealValue<false>{0.0, -deg_120, deg_120};
        }
    }
};

template class NewtonRaphsonSESolver<true>;
template class NewtonRaphsonSESolver<false>;

} // namespace newton_raphson_se

using newton_raphson_se::NewtonRaphsonSESolver;

} // namespace power_grid_model::math_solver

#endif
