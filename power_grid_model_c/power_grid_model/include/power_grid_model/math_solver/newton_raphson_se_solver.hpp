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
template <bool sym> class NRSEGainBlock : public Block<DoubleComplex, sym, true, 4> {
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
template <bool sym> class NewtonRaphsonSESolver {

  public:
    NewtonRaphsonSESolver(YBus<sym> const& y_bus, std::shared_ptr<MathModelTopology const> topo_ptr)
        : n_bus_{y_bus.size()},
          math_topo_{std::move(topo_ptr)},
          data_gain_(y_bus.nnz_lu()),
          x_rhs_(y_bus.size()),
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
            output.u[bus] = exp(1.0i * (mean_angle_shift + math_topo_->phase_shift[bus]));
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
            sparse_solver_.solve_with_prefactorized_matrix(data_gain_, perm_, x_rhs_, x_rhs_);
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
    std::vector<NRSERhs<sym>> x_rhs_;
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
            ComplexValue<sym> const ui = current_u[row];
            RealValue<sym> const abs_ui = cabs(ui);
            NRSERhs<sym>& rhs_block = x_rhs_[row];
            rhs_block = NRSERhs<sym>{};
            for (Idx data_idx_lu = row_indptr[row]; data_idx_lu != row_indptr[row + 1]; ++data_idx_lu) {
                Idx const col = col_indices[data_idx_lu];
                ComplexValue<sym> const uj = current_u[col];
                RealValue<sym> const abs_uj = cabs(uj);
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

                    auto const& weight = ComplexTensor<sym>{1.0 / measured_value.voltage_var(row)};
                    auto const& del_u = measured_u[row] - current_u[row];
                    // block.g_P_theta() += weight;
                    block.g_P_v() += weight;
                    // block.g_Q_theta() += weight;
                    block.g_Q_v() += weight;
                    // block.eta_theta() += dot(weight, cabs(del_u));
                    rhs_block.eta_v() += dot(weight, (del_u / cabs(del_u)));
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
                            auto const& shunt_power = measured_value.shunt_power(obj);
                            shunt_measurement_contribution(yii, ui, shunt_power, block, rhs_block);
                        }
                    }
                    else if (type == YBusElementType::bff){
                        if (measured_value.has_branch_from(obj)) {
                            auto const& yii = param.branch_param[obj].value[type];
                            auto const& branch_from_power = measured_value.branch_from_power(obj);
                            shunt_measurement_contribution(yii, ui, branch_from_power, block, rhs_block);
                        } 
                    }
                    else if (type == YBusElementType::btt)  {
                        if (measured_value.has_branch_to(obj)) {
                            auto const& yii = param.shunt_param[obj].value[type];
                            auto const& branch_to_power = measured_value.branch_to_power(obj);
                            shunt_measurement_contribution(yii, ui, branch_to_power, block, rhs_block);
                        }
                    }
                    else if (type == YBusElementType::bft)  {
                        if (measured_value.has_branch_from(obj)) {
                            auto const& yii = param.branch_param[obj].value[YBusElementType::bff];
                            auto const& yij = param.branch_param[obj].value[YBusElementType::bft];
                            auto const& branch_from_power = measured_value.branch_from_power(obj);
                            branch_measurement_contribution(branch_from_power, yii, yij, ui, uj, block, rhs_block);
                        }
                    }
                    else {
                        if (measured_value.has_branch_to(obj)) {
                            auto const& yii = param.branch_param[obj].value[YBusElementType::btt];
                            auto const& yij = param.branch_param[obj].value[YBusElementType::btf];
                            auto const& branch_to_power = measured_value.branch_to_power(obj);
                            branch_measurement_contribution(branch_to_power, yii, yij, uj, ui, block, rhs_block);
                        }
                    }
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
                        block.q_P_theta() += abs_ui * abs_ui * yij.imag();
                        block.q_P_v() += -abs_ui * yij.real();
                        block.q_Q_theta() += abs_ui * abs_ui * yij.real();
                        block.q_Q_v() += abs_ui * yij.imag();

                        rhs_block.tau_P() += injection.value.real();
                        rhs_block.tau_Q() += injection.value.imag();

                        block.r_P_theta() = -injection.p_variance * injection.p_variance;
                        block.r_Q_V() = -injection.q_variance * injection.q_variance;
                    } else {
                        block.q_P_theta() += g_sin_minus_b_cos(yij, ui, uj);
                        block.q_P_v() += g_cos_plus_b_sin(yij, ui, uj) / abs_uj;
                        block.q_Q_theta() += -g_cos_plus_b_sin(yij, ui, uj);
                        block.q_Q_v() += -g_sin_minus_b_cos(yij, ui, uj) / abs_uj;

                        rhs_block.tau_P() += -g_cos_plus_b_sin(yij, ui, uj);
                        rhs_block.tau_Q() += -g_sin_minus_b_cos(yij, ui, uj);
                    }
                }
                // injection measurement not exist
                else {
                    // Q_ij = 0
                    // R_ii = -1.0, only diagonal
                    // assign -1.0 to diagonal of 3x3 tensor, for asym
                    if (row == col) {
                        block.r_P_theta() = RealValue<sym>{-1.0};
                        block.r_Q_V() = RealValue<sym>{-1.0};
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
            data_gain_[data_idx_lu].qh_P_V() = data_gain_[data_idx_tranpose].q_Q_theta();
            data_gain_[data_idx_lu].qh_Q_theta() = data_gain_[data_idx_tranpose].q_P_V();
            data_gain_[data_idx_lu].qh_Q_V() = data_gain_[data_idx_tranpose].q_Q_V();
        }
        // prefactorize
        sparse_solver_.prefactorize(data_gain_, perm_);
    }

    void branch_measurement_contribution(const auto& branch_power, const auto& yii, const auto& yij,
                                         const ComplexValue<sym>& ui, const ComplexValue<sym>& uj,
                                         NRSEGainBlock<sym>& block, NRSERhs<sym>& rhs_block) {
        RealValue<sym> const abs_ui = cabs(ui);
        RealValue<sym> const abs_uj = cabs(uj);
        ComplexDiagonalTensor<sym> const w_p = diagonal_inverse(branch_power.p_variance);
        ComplexDiagonalTensor<sym> const w_q = diagonal_inverse(branch_power.q_variance);

        RealValue<sym> const gs_minus_bc = g_sin_minus_b_cos(yij, ui, uj);
        RealValue<sym> const gc_plus_bs = g_cos_plus_b_sin(yij, ui, uj);

        RealValue<sym> const dP_dt_i = -gs_minus_bc;
        RealValue<sym> const dP_dV_i = gc_plus_bs / abs_ui;
        RealValue<sym> const dQ_dt_i = gc_plus_bs;
        RealValue<sym> const dQ_dV_i = gs_minus_bc / abs_ui;

        RealValue<sym> const dP_dt_j = gs_minus_bc;
        RealValue<sym> const dP_dV_j = gc_plus_bs / abs_uj;
        RealValue<sym> const dQ_dt_j = -gc_plus_bs;
        RealValue<sym> const dQ_dV_j = gs_minus_bc / abs_uj;

        block.g_P_theta() += w_p * dP_dt_i * dP_dt_j + w_q * dQ_dt_i * dQ_dt_j;
        block.g_Q_v() += w_p * dP_dV_i * dP_dV_j + w_q * dQ_dV_i * dQ_dV_j;

        block.g_P_v() += w_p * dP_dt_i * dP_dV_j + w_q * dQ_dt_i * dQ_dV_j;
        block.g_Q_theta() += w_p * dP_dt_j * dP_dV_i + w_q * dQ_dt_j * dQ_dV_i;

        auto const del_branch_power_p = branch_power.value - gc_plus_bs;
        auto const del_branch_power_q = branch_power.value - gs_minus_bc;

        rhs_block.eta_p() = w_p * dP_dt_i * del_branch_power_p + w_p * dQ_dt_i * del_branch_power_q;
        rhs_block.eta_q() = w_p * dP_dV_i * del_branch_power_p + w_p * dQ_dV_i * del_branch_power_q;
    }

    void shunt_measurement_contribution(const auto& yii, const ComplexValue<sym>& ui, const auto& shunt_power,
                                        NRSEGainBlock<sym>& block, NRSERhs<sym>& rhs_block) {

        auto const calculated_shunt_power = calculate_shunt_power(yii, ui);
        RealValue<sym> const abs_ui = cabs(ui);
        auto const dP_dVi = 2 * abs_ui * yii.real();
        auto const dQ_dVi = -2 * abs_ui * yii.imag();
        auto const w_p = diagonal_inverse(shunt_power.p_variance);
        auto const w_q = diagonal_inverse(shunt_power.q_variance);
        block.g_Q_v() += w_p * dP_dVi * dP_dVi + w_q * dQ_dVi * dQ_dVi;

        auto const del_shunt_power = shunt_power.value - ui * ui * yii;

        rhs_block.eta_q() += w_p * dP_dVi * del_shunt_power.real() + w_q * dQ_dVi * del_shunt_power.imag();
    }

    RealTensor<sym> cos_ij(ComplexValue<sym> ui, ComplexValue<sym> uj) {
        // diag(Vi) * cos(theta_ij) * diag(Vj)
        // Ui_r @* Uj_r + Ui_i @* Uj_i
        // = cij
        return vector_outer_product(real(ui), real(uj)) + vector_outer_product(imag(ui), imag(uj));
    }

    RealTensor<sym> sin_ij(ComplexValue<sym> ui, ComplexValue<sym> uj) {
        // diag(Vi) * sin(theta_ij) * diag(Vj)
        // = Ui_i @* Uj_r - Ui_r @* Uj_i
        // = sij
        return vector_outer_product(imag(ui), real(uj)) - vector_outer_product(real(ui), imag(uj));
    }

    auto g_sin_minus_b_cos(ComplexTensor<sym> yij, ComplexValue<sym> ui, ComplexValue<sym> uj) {
        return real(yij) * sin_ij(ui, uj) - imag(yij) * cos_ij(ui, uj);
    }
    auto g_cos_plus_b_sin(ComplexTensor<sym> yij, ComplexValue<sym> ui, ComplexValue<sym> uj) {
        return real(yij) * cos_ij(ui, uj) + imag(yij) * sin_ij(ui, uj);
    }

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

    void calculate_result(YBus<sym> const& y_bus, MeasuredValues<sym> const& measured_value, MathOutput<sym>& output) {
        // call y bus
        output.branch = y_bus.template calculate_branch_flow<BranchMathOutput<sym>>(output.u);
        output.shunt = y_bus.template calculate_shunt_flow<ApplianceMathOutput<sym>>(output.u);
        output.bus_injection = y_bus.calculate_injection(output.u);
        std::tie(output.load_gen, output.source) =
            measured_value.calculate_load_gen_source(output.u, output.bus_injection);
    }
};

template class IterativeLinearSESolver<true>;
template class IterativeLinearSESolver<false>;

} // namespace math_model_impl

template <bool sym> using IterativeLinearSESolver = math_model_impl::IterativeLinearSESolver<sym>;

} // namespace power_grid_model

#endif
