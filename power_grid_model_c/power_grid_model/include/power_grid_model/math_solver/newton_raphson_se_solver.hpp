// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

// Newton Raphson state estimation solver

#include "block_matrix.hpp"
#include "common_solver_functions.hpp"
#include "measured_values.hpp"
#include "observability.hpp"
#include "y_bus.hpp"

#include "../calculation_parameters.hpp"
#include "../common/three_phase_tensor.hpp"
#include "../common/timer.hpp"

namespace power_grid_model::math_solver {

// hide implementation in inside namespace
namespace newton_raphson_se {

// block class for the unknown vector and/or right-hand side in state estimation equation
template <symmetry_tag sym> struct NRSEUnknown : public Block<double, sym, false, 4> {
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
template <symmetry_tag sym> using NRSERhs = NRSEUnknown<sym>;

// class of 4*4 (12*12) se gain block
// [
//    [G, Q^T]
//    [Q, R  ]
// ]
template <symmetry_tag sym> class NRSEGainBlock : public Block<double, sym, true, 4> {
  public:
    template <int r, int c> using GetterType = typename Block<double, sym, true, 4>::template GetterType<r, c>;
    template <int r, int c, int r_size, int c_size>
    using BlockGetterType = typename Block<double, sym, true, 4>::template BlockGetterType<r, c, r_size, c_size>;

    // eigen expression
    using Block<double, sym, true, 4>::Block;
    using Block<double, sym, true, 4>::operator=;

    GetterType<0, 0> g_P_theta() { return this->template get_val<0, 0>(); }
    GetterType<0, 1> g_P_v() { return this->template get_val<0, 1>(); }
    GetterType<1, 0> g_Q_theta() { return this->template get_val<1, 0>(); }
    GetterType<1, 1> g_Q_v() { return this->template get_val<1, 1>(); }

    // (Q^T)(X, Y)
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

    BlockGetterType<0, 0, 2, 2> g() { return this->template get_block_val<0, 0, 2, 2>(); }
    BlockGetterType<0, 1, 2, 2> qt() { return this->template get_block_val<0, 1, 2, 2>(); }
    BlockGetterType<1, 0, 2, 2> q() { return this->template get_block_val<1, 0, 2, 2>(); }
    BlockGetterType<1, 1, 2, 2> r() { return this->template get_block_val<1, 1, 2, 2>(); }
};

// solver
template <symmetry_tag sym> class NewtonRaphsonSESolver {
    enum class Order { row_major = 0, column_major = 1 };

    struct NRSEVoltageState {
        ComplexTensor<sym> ui_ui_conj{};
        ComplexTensor<sym> uj_uj_conj{};
        ComplexTensor<sym> ui_uj_conj{};
        ComplexTensor<sym> uj_ui_conj{};
        ComplexValue<sym> ui{};
        ComplexValue<sym> uj{};
        RealDiagonalTensor<sym> abs_ui_inv{};
        RealDiagonalTensor<sym> abs_uj_inv{};

        auto const& u_chi_u_chi_conj(Order ij_voltage_order) const {
            return ij_voltage_order == Order::row_major ? ui_ui_conj : uj_uj_conj;
        }
        auto const& u_chi_u_psi_conj(Order ij_voltage_order) const {
            return ij_voltage_order == Order::row_major ? ui_uj_conj : uj_ui_conj;
        }
        auto const& abs_u_chi_inv(Order ij_voltage_order) const {
            return ij_voltage_order == Order::row_major ? abs_ui_inv : abs_uj_inv;
        }
        auto const& abs_u_psi_inv(Order ij_voltage_order) const {
            return ij_voltage_order == Order::row_major ? abs_uj_inv : abs_ui_inv;
        }
    };

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

    SolverOutput<sym> run_state_estimation(YBus<sym> const& y_bus, StateEstimationInput<sym> const& input,
                                           double err_tol, Idx max_iter, CalculationInfo& calculation_info) {
        // prepare
        Timer main_timer;
        Timer sub_timer;
        SolverOutput<sym> output;
        output.u.resize(n_bus_);
        output.bus_injection.resize(n_bus_);
        double max_dev = std::numeric_limits<double>::max();

        main_timer = Timer(calculation_info, 2220, "Math solver");

        // preprocess measured value
        sub_timer = Timer(calculation_info, 2221, "Pre-process measured value");
        MeasuredValues<sym> const measured_values{y_bus.shared_topology(), input};
        necessary_observability_check(measured_values, y_bus.shared_topology());

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
            sub_timer = Timer(calculation_info, 2225, "Solve sparse linear equation");
            sparse_solver_.prefactorize_and_solve(data_gain_, perm_, delta_x_rhs_, delta_x_rhs_);
            sub_timer = Timer(calculation_info, 2226, "Iterate unknown");
            max_dev = iterate_unknown(output.u, measured_values);
        };

        // calculate math result
        sub_timer = Timer(calculation_info, 2227, "Calculate math result");
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

    void initialize_unknown(ComplexValueVector<sym>& initial_u, MeasuredValues<sym> const& measured_values) {
        reset_unknown();
        RealValue<sym> const mean_angle_shift = measured_values.mean_angle_shift();
        for (Idx bus = 0; bus != n_bus_; ++bus) {
            auto& estimated_result = x_[bus];

            estimated_result.theta() = mean_angle_shift + math_topo_->phase_shift[bus];
            if (measured_values.has_voltage(bus)) {
                if (measured_values.has_angle_measurement(bus)) {
                    estimated_result.theta() = arg(measured_values.voltage(bus));
                }
                estimated_result.v() = detail::cabs_or_real<sym>(measured_values.voltage(bus));
            }
            initial_u[bus] = estimated_result.v() * exp(1.0i * estimated_result.theta());
        }
    }

    void reset_unknown() {
        auto const default_unknown = [] {
            NRSERhs<sym> x;
            x.v() = 1.0;
            x.theta() = 0.0;
            x.phi_p() = 0.0;
            x.phi_q() = 0.0;
            return x;
        }();
        std::ranges::fill(x_, default_unknown);
    }

    void prepare_matrix_and_rhs(YBus<sym> const& y_bus, MeasuredValues<sym> const& measured_values,
                                ComplexValueVector<sym> const& current_u) {
        MathModelParam<sym> const& param = y_bus.math_model_param();
        IdxVector const& row_indptr = y_bus.row_indptr_lu();
        IdxVector const& col_indices = y_bus.col_indices_lu();
        IdxVector const& lu_diag = y_bus.lu_diag();

        // loop data index, all rows and columns
        for (Idx row = 0; row != n_bus_; ++row) {
            NRSERhs<sym>& rhs_block = delta_x_rhs_[row];
            NRSEGainBlock<sym>& diag_block = data_gain_[lu_diag[row]];

            rhs_block.clear();
            diag_block.clear();

            NRSEVoltageState u_state{};
            u_state.ui = current_u[row];
            u_state.abs_ui_inv = diagonal_inverse(x_[row].v());
            u_state.ui_ui_conj = vector_outer_product(u_state.ui, conj(u_state.ui));

            for (Idx data_idx_lu = row_indptr[row]; data_idx_lu != row_indptr[row + 1]; ++data_idx_lu) {
                // get data idx of y bus,
                Idx const data_idx = y_bus.map_lu_y_bus()[data_idx_lu];
                Idx const col = col_indices[data_idx_lu];

                // get a reference and reset block to zero
                NRSEGainBlock<sym>& block = data_gain_[data_idx_lu];
                if (row == col) {
                    // fill block with voltage measurement, only diagonal
                    process_voltage_measurements(diag_block, rhs_block, measured_values, row);
                } else {
                    // Diagonal block is being cleared outside this loop
                    block.clear();
                }

                // skip anything else for a fill-in
                if (data_idx == -1) {
                    continue;
                }

                u_state.uj = current_u[col];
                u_state.abs_uj_inv = diagonal_inverse(x_[col].v());
                u_state.uj_uj_conj = vector_outer_product(u_state.uj, conj(u_state.uj));
                u_state.ui_uj_conj = vector_outer_product(u_state.ui, conj(u_state.uj));
                u_state.uj_ui_conj = vector_outer_product(u_state.uj, conj(u_state.ui));

                // fill block with branch, shunt measurement
                for (Idx element_idx = y_bus.y_bus_entry_indptr()[data_idx];
                     element_idx != y_bus.y_bus_entry_indptr()[data_idx + 1]; ++element_idx) {
                    Idx const obj = y_bus.y_bus_element()[element_idx].idx;
                    YBusElementType const type = y_bus.y_bus_element()[element_idx].element_type;

                    switch (type) {
                    case YBusElementType::shunt:
                        if (measured_values.has_shunt(obj)) {
                            auto const& ys = param.shunt_param[obj];
                            auto const& measured_power = measured_values.shunt_power(obj);
                            process_shunt_measurement(block, rhs_block, ys, u_state, measured_power);
                        }
                        break;
                    case YBusElementType::bft:
                        [[fallthrough]];
                    case YBusElementType::btf: {
                        auto const& y_branch = param.branch_param[obj];
                        if (measured_values.has_branch_from(obj)) {
                            auto const ij_voltage_order =
                                (type == YBusElementType::bft) ? Order::row_major : Order::column_major;
                            process_branch_measurement(block, diag_block, rhs_block, y_branch.yff(), y_branch.yft(),
                                                       u_state, ij_voltage_order,
                                                       measured_values.branch_from_power(obj));
                        }
                        if (measured_values.has_branch_to(obj)) {
                            auto const ij_voltage_order =
                                (type == YBusElementType::btf) ? Order::row_major : Order::column_major;
                            process_branch_measurement(block, diag_block, rhs_block, y_branch.ytt(), y_branch.ytf(),
                                                       u_state, ij_voltage_order, measured_values.branch_to_power(obj));
                        }
                        break;
                    }
                    default:
                        assert(type == YBusElementType::bff || type == YBusElementType::btt);
                        break;
                    }
                }

                // fill block with injection measurement constraints
                if (measured_values.has_bus_injection(row)) {
                    auto const& yij = y_bus.admittance()[data_idx];
                    process_injection_row(block, diag_block, rhs_block, yij, u_state);
                    if (row == col) {
                        process_injection_diagonal(block, rhs_block, measured_values.bus_injection(row));
                    }
                } else if (row == col) {
                    virtually_remove_constraints(block);
                }
            }
        }

        fill_qt(y_bus);
        process_lagrange_multiplier(y_bus);
    }

    /// Q_ij = 0
    /// R_ii = -1.0, only diagonal
    /// assign -1.0 to diagonal of 3x3 tensor, for asym
    void virtually_remove_constraints(NRSEGainBlock<sym>& block) const {
        block.r_P_theta() = RealTensor<sym>{-1.0};
        block.r_Q_v() = RealTensor<sym>{-1.0};
    }

    /// R_ii = -variance, only diagonal
    /// assign variance to diagonal of 3x3 tensor, for asym
    void process_injection_diagonal(NRSEGainBlock<sym>& block, NRSERhs<sym>& rhs_block, auto const& injection) const {
        rhs_block.tau_p() += injection.value.real();
        rhs_block.tau_q() += injection.value.imag();
        block.r_P_theta() = RealTensor<sym>{RealValue<sym>{-injection.p_variance}};
        block.r_Q_v() = RealTensor<sym>{RealValue<sym>{-injection.q_variance}};
    }

    /// @brief Processes common part of all elements to fill from an injection measurement.
    ///
    /// Also includes zero injection constraint.
    /// This would be H, N, M, L at (row, col) block and partially the second part of the (row, row) block using the
    /// same H and M but multiplied by abs_ui_inv.
    ///
    /// @param block LHS(r, c)
    /// @param diag_block LHS(r, r)
    /// @param rhs_block RHS(r)
    /// @param yij admittance of (row with injection, c)
    /// @param u_state Voltage state of iteration
    void process_injection_row(NRSEGainBlock<sym>& block, NRSEGainBlock<sym>& diag_block, NRSERhs<sym>& rhs_block,
                               auto const& yij, auto const& u_state) const {
        auto const hm_ui_uj_yij = hm_complex_form(yij, u_state.ui_uj_conj);
        auto const nl_ui_uj_yij = dot(hm_ui_uj_yij, u_state.abs_uj_inv);
        auto const injection_jac = calculate_jacobian(hm_ui_uj_yij, nl_ui_uj_yij);
        add_injection_jacobian(block, injection_jac);

        // add paritial sum to the diagonal block and subtract from rhs for current row
        auto const f_x_complex_row = sum_row(hm_ui_uj_yij);
        auto const f_x_complex_abs_ui_inv_row = dot(u_state.abs_ui_inv, f_x_complex_row);
        auto const injection_jac_diagonal = jacobian_diagonal_component(f_x_complex_abs_ui_inv_row, f_x_complex_row);
        add_injection_jacobian(diag_block, injection_jac_diagonal);
        rhs_block.tau_p() -= real(f_x_complex_row);
        rhs_block.tau_q() -= imag(f_x_complex_row);
    }

    /// @brief Adds contribution of G(i, i) form the shunt.
    /// Note: The sign of Y_s is inverted per injection direction.
    ///
    /// @param block LHS(i, i)
    /// @param rhs_block RHS(i)
    /// @param ys shunt admittance related to the shunt measurement
    /// @param u_state Voltage state of iteration Voltage state of iteration
    /// @param measured_power measured shunt power
    void process_shunt_measurement(NRSEGainBlock<sym>& block, NRSERhs<sym>& rhs_block, auto const& ys,
                                   auto const& u_state, auto const& measured_power) {
        auto const hm_ui_ui_ys = hm_complex_form(-ys, u_state.ui_ui_conj);
        auto const nl_ui_ui_ys = dot(hm_ui_ui_ys, u_state.abs_ui_inv);
        auto const f_x_complex = sum_row(hm_ui_ui_ys);
        auto const f_x_complex_abs_ui_inv = sum_row(nl_ui_ui_ys);

        auto jac_block = calculate_jacobian(hm_ui_ui_ys, nl_ui_ui_ys);
        jac_block += jacobian_diagonal_component(f_x_complex_abs_ui_inv, f_x_complex);
        auto const& block_F_T_k_w = transpose_multiply_weight(jac_block, measured_power);
        multiply_add_jacobian_blocks_lhs(block, block_F_T_k_w, jac_block);
        multiply_add_jacobian_blocks_rhs(rhs_block, block_F_T_k_w, measured_power, f_x_complex);
    }

    /// @brief Adds contribution of branch measurements to the G(r, r), G(r, c) and eta_r blocks,
    ///  given the iteration passes through (r, c) ie. row, col
    ///
    /// When iterating via (row, col), have 4 cases regarding branch measurements:
    ///      if y_type == yft
    ///          if from_measurement,
    ///              xi = f, mu = t, chi = row, psi = col
    ///          if to_measurement,
    ///              xi = t, mu = f, chi = col, psi = row
    ///      if y_type == ytf &
    ///          if from_measurement,
    ///              xi = f, mu = t, chi = col, psi = row
    ///          if to_measurement,
    ///              xi = t, mu = f, chi = row, psi = col
    ///
    /// f_P_(x) = -M(U_chi, U_chi, Y_xi_xi) - M(U_chi, U_psi, Y_xi_mu)
    /// f_Q_(x) = H(U_chi, U_chi, Y_xi_xi) + H(U_chi, U_psi, Y_xi_mu)
    ///
    ///
    /// @param block G(r, c)
    /// @param diag_block G(r, r)
    /// @param rhs_block RHS(r)
    /// @param y_xi_xi shunt admittance near to branch measurement
    /// @param y_xi_mu admittance from the branch measurement to other bus
    /// @param u_state Voltage state of iteration voltage state vector
    /// @param order Order enum to determine if (chi, psi) = (row, col) or (col, row)
    /// @param measured_power
    void process_branch_measurement(NRSEGainBlock<sym>& block, NRSEGainBlock<sym>& diag_block, NRSERhs<sym>& rhs_block,
                                    const auto& y_xi_xi, const auto& y_xi_mu, const auto& u_state, Order const order,
                                    const auto& measured_power) {
        auto const hm_u_chi_u_chi_y_xi_xi = hm_complex_form(y_xi_xi, u_state.u_chi_u_chi_conj(order));
        auto const nl_u_chi_u_chi_y_xi_xi = dot(hm_u_chi_u_chi_y_xi_xi, u_state.abs_u_chi_inv(order));

        auto const hm_u_chi_u_psi_y_xi_mu = hm_complex_form(y_xi_mu, u_state.u_chi_u_psi_conj(order));
        auto const nl_u_chi_u_psi_y_xi_mu = dot(hm_u_chi_u_psi_y_xi_mu, u_state.abs_u_psi_inv(order));

        auto const f_x_complex = sum_row(hm_u_chi_u_chi_y_xi_xi + hm_u_chi_u_psi_y_xi_mu);
        auto const f_x_complex_u_chi_inv = dot(u_state.abs_u_chi_inv(order), f_x_complex);

        auto block_rr_or_cc = calculate_jacobian(hm_u_chi_u_chi_y_xi_xi, nl_u_chi_u_chi_y_xi_xi);
        block_rr_or_cc += jacobian_diagonal_component(f_x_complex_u_chi_inv, f_x_complex);
        auto const block_rc_or_cr = calculate_jacobian(hm_u_chi_u_psi_y_xi_mu, nl_u_chi_u_psi_y_xi_mu);

        if (order == Order::row_major) {
            multiply_add_branch_blocks(block, diag_block, rhs_block, block_rr_or_cc, block_rc_or_cr, measured_power,
                                       f_x_complex);
        } else {
            multiply_add_branch_blocks(block, diag_block, rhs_block, block_rc_or_cr, block_rr_or_cc, measured_power,
                                       f_x_complex);
        }
    }

    void multiply_add_branch_blocks(NRSEGainBlock<sym>& block, NRSEGainBlock<sym>& diag_block, NRSERhs<sym>& rhs_block,
                                    auto& left_block, const auto& right_block, const auto& measured_power,
                                    const auto& f_x_complex) {
        auto const& block_F_T_k_w = transpose_multiply_weight(left_block, measured_power);

        multiply_add_jacobian_blocks_lhs(diag_block, block_F_T_k_w, left_block);
        multiply_add_jacobian_blocks_lhs(block, block_F_T_k_w, right_block);
        multiply_add_jacobian_blocks_rhs(rhs_block, block_F_T_k_w, measured_power, f_x_complex);
    }

    /// @brief Fill Q^T(j,i) of LHS(i, j) from the Q(j, i) of LHS(j, i).
    ///
    /// @param y_bus
    void fill_qt(YBus<sym> const& y_bus) {
        iterate_matrix_skip_fills(
            [this](Idx /* row */, Idx /* col */, Idx data_idx, Idx data_idx_transpose) {
                data_gain_[data_idx].qt() = hermitian_transpose(data_gain_[data_idx_transpose].q());
            },
            y_bus);
    }

    /// @brief Process Lagrange multiplier eta_i(i) = sum( Q^T(j,i) * phi(j) ) for j = 1 to n_bus
    ///
    /// @param y_bus
    void process_lagrange_multiplier(YBus<sym> const& y_bus) {
        iterate_matrix_skip_fills(
            [this](Idx row, Idx col, Idx data_idx, Idx /* data_idx_transpose */) {
                auto& block = data_gain_[data_idx];
                auto& rhs_block = delta_x_rhs_[row];

                rhs_block.eta_theta() +=
                    dot(block.qt_P_theta(), x_[col].phi_p()) + dot(block.qt_P_v(), x_[col].phi_q());
                rhs_block.eta_v() += dot(block.qt_Q_theta(), x_[col].phi_p()) + dot(block.qt_Q_v(), x_[col].phi_q());
            },
            y_bus);
    }

    template <typename Func>
        requires std::invocable<Func, Idx /*row*/, Idx /*col*/, Idx /*data_idx*/, Idx /*data_idx_transpose*/>
    void iterate_matrix_skip_fills(Func func, YBus<sym> const& y_bus) const {
        auto const& row_indptr = y_bus.row_indptr_lu();
        // loop data index, all rows and columns
        for (Idx row = 0; row != n_bus_; ++row) {
            for (Idx data_idx = row_indptr[row]; data_idx != row_indptr[row + 1]; ++data_idx) {
                // skip for fill-in
                if (y_bus.map_lu_y_bus()[data_idx] == -1) {
                    continue;
                }

                Idx const col = y_bus.col_indices_lu()[data_idx];
                Idx const data_idx_transpose = y_bus.lu_transpose_entry()[data_idx];

                func(row, col, data_idx, data_idx_transpose);
            }
        }
    }

    /// @brief G(row, row) += w_k
    /// eta(row) += w_k . (z_k - f_k(x))
    ///
    /// In case there is no angle measurement, the slack bus or arbitray bus measurement is considered to have a virtual
    /// angle measurement of zero. w_theta = 1.0 by default for all measurements
    ///
    /// @param block LHS(row, col), ie. LHS(row, row)
    /// @param rhs_block RHS(row)
    /// @param measured_values
    /// @param bus bus with voltage measurement
    void process_voltage_measurements(NRSEGainBlock<sym>& block, NRSERhs<sym>& rhs_block,
                                      MeasuredValues<sym> const& measured_values, Idx const& bus) {
        if (!measured_values.has_voltage(bus)) {
            return;
        }

        // G += 1.0 / variance
        // for 3x3 tensor, fill diagonal
        auto const w_v = RealTensor<sym>{1.0 / measured_values.voltage_var(bus)};
        auto const abs_measured_v = detail::cabs_or_real<sym>(measured_values.voltage(bus));
        auto const delta_v = abs_measured_v - x_[bus].v();

        auto const virtual_angle_measurement_bus = measured_values.has_voltage(math_topo_->slack_bus)
                                                       ? math_topo_->slack_bus
                                                       : measured_values.first_voltage_measurement();

        RealTensor<sym> w_theta{};
        RealValue<sym> delta_theta{};
        if (measured_values.has_angle_measurement(bus)) {
            delta_theta = RealValue<sym>{arg(measured_values.voltage(bus))} - RealValue<sym>{x_[bus].theta()};
            w_theta = RealTensor<sym>{1.0};
        } else if (bus == virtual_angle_measurement_bus && !measured_values.has_angle()) {
            delta_theta = arg(ComplexValue<sym>{1.0}) - RealValue<sym>{x_[bus].theta()};
            w_theta = RealTensor<sym>{1.0};
        }

        block.g_P_theta() += w_theta;
        block.g_Q_v() += w_v;
        rhs_block.eta_theta() += dot(w_theta, delta_theta);
        rhs_block.eta_v() += dot(w_v, delta_v);
    }

    /// @brief The second part to add to the F_k(u1, u1, y11) block for shunt flow.
    /// The members are -D[Q], D[P] . D[V]^-1, D[P], D[Q] . D[V]^-1,
    ///
    /// @param f_x_complex_v_inv (P_i + j * Q_i) / abs(u1)
    /// @param f_x_complex P_i + j * Q_i
    /// @return  second part of F_k block
    static NRSEJacobian jacobian_diagonal_component(ComplexValue<sym> const& f_x_complex_v_inv,
                                                    ComplexValue<sym> const& f_x_complex) {
        NRSEJacobian jacobian{};
        jacobian.dP_dt = RealTensor<sym>{RealValue<sym>{-imag(f_x_complex)}};
        jacobian.dP_dv = RealTensor<sym>{RealValue<sym>{real(f_x_complex_v_inv)}};
        jacobian.dQ_dt = RealTensor<sym>{RealValue<sym>{real(f_x_complex)}};
        jacobian.dQ_dv = RealTensor<sym>{RealValue<sym>{imag(f_x_complex_v_inv)}};
        return jacobian;
    }

    /// @brief Calculate F_k(u1, u2, y12)^T . W_k. Hence first transpose, then dot product.
    /// where W_k = [[p_variance, 0], [0, q_variance]]
    ///
    /// @param jac_block F_k(u1, u2, y12)
    /// @param power_sensor object with members p_variance and q_variance
    /// @return  F_k(u1, u2, y12)^T . W
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

    /// @brief Matrix multiply F_{k,1}^T . w_k and F_{k,2}^T.
    /// Then add the product to G of LHS(row, col)
    ///
    /// @param lhs_block LHS(row, col)
    /// @param f_T_k_w F_{k,1}^T . w_k
    /// @param f_i_or_j F_{k,2}^T
    static void multiply_add_jacobian_blocks_lhs(NRSEGainBlock<sym>& lhs_block, NRSEJacobian const& f_T_k_w,
                                                 NRSEJacobian const& f_i_or_j) {
        lhs_block.g_P_theta() += dot(f_T_k_w.dP_dt, f_i_or_j.dP_dt) + dot(f_T_k_w.dP_dv, f_i_or_j.dQ_dt);
        lhs_block.g_P_v() += dot(f_T_k_w.dP_dt, f_i_or_j.dP_dv) + dot(f_T_k_w.dP_dv, f_i_or_j.dQ_dv);
        lhs_block.g_Q_theta() += dot(f_T_k_w.dQ_dt, f_i_or_j.dP_dt) + dot(f_T_k_w.dQ_dv, f_i_or_j.dQ_dt);
        lhs_block.g_Q_v() += dot(f_T_k_w.dQ_dt, f_i_or_j.dP_dv) + dot(f_T_k_w.dQ_dv, f_i_or_j.dQ_dv);
    }

    /// @brief Matrix multiply F_{k,1}^T . w_k and (z_k - f_k(x)).
    /// Then add the product to eta of RHS(row)
    ///
    /// @param rhs_block RHS(row)
    /// @param f_T_k_w F_{k,1}^T . w_k
    /// @param power_sensor measurement
    /// @param f_x_complex calculated power
    static void multiply_add_jacobian_blocks_rhs(NRSERhs<sym>& rhs_block, NRSEJacobian const& f_T_k_w,
                                                 PowerSensorCalcParam<sym> const& power_sensor,
                                                 ComplexValue<sym> const& f_x_complex) {
        auto const delta_power = power_sensor.value - f_x_complex;
        rhs_block.eta_theta() += dot(f_T_k_w.dP_dt, real(delta_power)) + dot(f_T_k_w.dP_dv, imag(delta_power));
        rhs_block.eta_v() += dot(f_T_k_w.dQ_dt, real(delta_power)) + dot(f_T_k_w.dQ_dv, imag(delta_power));
    }

    static void add_injection_jacobian(NRSEGainBlock<sym>& block, NRSEJacobian const& jacobian_block) {
        block.q_P_theta() += jacobian_block.dP_dt;
        block.q_P_v() += jacobian_block.dP_dv;
        block.q_Q_theta() += jacobian_block.dQ_dt;
        block.q_Q_v() += jacobian_block.dQ_dv;
    }

    /// @brief Construct the F_k(u1, u2, y12) block using helper function of hnml complex form
    /// The 4 members are H, N, M, L in the order.
    ///
    /// @param hm_complex hm_complex
    /// @param nl_complex hm_complex / abs(u2)
    /// @return  F_k(u1, u2, y12)
    static NRSEJacobian calculate_jacobian(ComplexTensor<sym> const& hm_complex, ComplexTensor<sym> const& nl_complex) {
        NRSEJacobian jacobian{};
        jacobian.dP_dt = imag(hm_complex);
        jacobian.dP_dv = real(nl_complex);
        jacobian.dQ_dt = -real(hm_complex);
        jacobian.dQ_dv = imag(nl_complex);
        return jacobian;
    }

    /// @brief Helper function for all G cos and B sin calculations
    ///
    /// @param yij admittance y12
    /// @param ui_uj_conj vector outer product of u1 and conj(u2)
    /// @return  -M(u1, u2, y12) + j * H(u1, u2, y12)
    static ComplexTensor<sym> hm_complex_form(ComplexTensor<sym> const& yij, ComplexTensor<sym> const& ui_uj_conj) {
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
            auto const& theta = x_[math_topo_->slack_bus].theta() + delta_x_rhs_[math_topo_->slack_bus].theta();
            if constexpr (is_symmetric_v<sym>) {
                return theta;
            } else {
                return theta(0);
            }
        }();

        for (Idx bus = 0; bus != n_bus_; ++bus) {
            auto& estimated_result = x_[bus];
            auto& estimated_delta = delta_x_rhs_[bus];

            // accumulate the unknown variable
            estimated_result.theta() += estimated_delta.theta() - RealValue<sym>{angle_offset};
            estimated_result.v() += estimated_delta.v();
            if (measured_values.has_bus_injection(bus)) {
                if (all_zero(measured_values.bus_injection(bus).p_variance)) {
                    estimated_result.phi_p() += estimated_delta.phi_p();
                }
                if (all_zero(measured_values.bus_injection(bus).q_variance)) {
                    estimated_result.phi_q() += estimated_delta.phi_q();
                }
            }

            auto const old_u = u[bus];
            u[bus] = estimated_result.v() * exp(1.0i * estimated_result.theta());
            // get dev of last iteration, get max
            double const dev = max_val(cabs(u[bus] - old_u));
            max_dev = std::max(dev, max_dev);
        }
        return max_dev;
    }

    static auto diagonal_inverse(RealValue<sym> const& value) {
        return RealDiagonalTensor<sym>{static_cast<RealValue<sym>>(RealValue<sym>{1.0} / value)};
    }
};

template class NewtonRaphsonSESolver<symmetric_t>;
template class NewtonRaphsonSESolver<asymmetric_t>;

} // namespace newton_raphson_se

using newton_raphson_se::NewtonRaphsonSESolver;

} // namespace power_grid_model::math_solver
