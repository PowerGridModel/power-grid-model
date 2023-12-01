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
    // array selection function pointer
    static constexpr std::array has_branch_{&MeasuredValues<sym>::has_branch_from, &MeasuredValues<sym>::has_branch_to};
    static constexpr std::array branch_power_{&MeasuredValues<sym>::branch_from_power,
                                              &MeasuredValues<sym>::branch_to_power};

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

    void prepare_matrix_and_rhs(YBus<sym> const& y_bus, MeasuredValues<sym> const& measured_value) {
        MathModelParam<sym> const& param = y_bus.math_model_param();
        IdxVector const& row_indptr = y_bus.row_indptr_lu();
        IdxVector const& col_indices = y_bus.col_indices_lu();

        // loop data index, all rows and columns
        for (Idx row = 0; row != n_bus_; ++row) {
            for (Idx data_idx_lu = row_indptr[row]; data_idx_lu != row_indptr[row + 1]; ++data_idx_lu) {
                Idx const col = col_indices[data_idx_lu];
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

                    // TODO split complex for NR
                    block.g() += ComplexTensor<sym>{1.0 / measured_value.voltage_var(row)};
                    block.eta() += (u[bus] - x_[bus]) / measured_value.voltage_var(bus);
                }
                // fill block with branch, shunt measurement
                for (Idx element_idx = y_bus.y_bus_entry_indptr()[data_idx];
                     element_idx != y_bus.y_bus_entry_indptr()[data_idx + 1]; ++element_idx) {
                    Idx const obj = y_bus.y_bus_element()[element_idx].idx;
                    YBusElementType const type = y_bus.y_bus_element()[element_idx].element_type;
                    // shunt
                    if (type == YBusElementType::shunt) {
                        if (measured_value.has_shunt(obj)) {
                            // G += Ys^H * (variance^-1) * Ys
                            auto const& yii = param.shunt_param[obj];
                            auto const& shunt_addtion = calculate_shunt_block(yii, ui);
                            auto const& shunt_addtion_transpose_weight =
                                dot(hermitian_transpose(shunt_addition),
                                    diagonal_inverse(shunt_power.p_variance + shunt_power.q_variance));
                            block.g() += dot(shunt_addtion_transpose_weight, shunt_addition);

                            auto del_shunt_power = measured_value.shunt_power(obj) -calculate_shunt_power(yii, ui);
                            block.eta() += dot(shunt_addtion_transpose_weight, del_shunt_power);
                        }
                    }
                    // branch
                    else {
                        // branch from- and to-side index at 0, and 1 position
                        IntS const b0 = static_cast<IntS>(type) / 2;
                        IntS const b1 = static_cast<IntS>(type) % 2;
                        // measured at from-side: 0, to-side: 1
                        for (IntS const measured_side : std::array<IntS, 2>{0, 1}) {
                            // has measurement
                            if (std::invoke(has_branch_[measured_side], measured_value, obj)) {
                                // G += Y{side, b0}^H * (variance^-1) * Y{side, b1}
                                auto const& y_branch_start = param.branch_param[obj].value[measured_side * 2 + b0];
                                auto const& y_branch_end = param.branch_param[obj].value[measured_side * 2 + b1];
                                auto const& branch_addition = calculate_branch_block(yij, ui, uj);
                                auto const& power = std::invoke(branch_power_[measured_side], measured_value, obj);
                                block.g() +=
                                    dot(hermitian_transpose(param.branch_param[obj].value[measured_side * 2 + b0]),
                                        diagonal_inverse(power.p_variance + power.q_variance),
                                        param.branch_param[obj].value[measured_side * 2 + b1]);
                                block.q() += 0;   // TODO Fill RHS incrementally some combination of block.g()
                                block.eta() += 0; // TODO Fill RHS incrementally some combination of block.g()
                            }
                        }
                    }
                    Idx const k = bus_entry[row];
                    // diagonal correction
                    // del_pq has negative injection
                    // H += (-Q)
                    add_diag(data_jac_[k].h(), del_x_pq_[row].q());
                    // N -= (-P)
                    add_diag(data_jac_[k].n(), -del_x_pq_[row].p());
                    // M -= (-P)
                    add_diag(data_jac_[k].m(), -del_x_pq_[row].p());
                    // L -= (-Q)
                    add_diag(data_jac_[k].l(), -del_x_pq_[row].q());
                }
                // fill block with injection measurement
                // injection measurement exist
                if (measured_value.has_bus_injection(row)) {
                    // Q_ij = Y_bus_ij
                    block.q() = y_bus.admittance()[data_idx];
                    // R_ii = -variance, only diagonal
                    if (row == col) {
                        // assign variance to diagonal of 3x3 tensor, for asym
                        auto const& injection = measured_value.bus_injection(row);
                        block.r() = ComplexTensor<sym>{
                            static_cast<ComplexValue<sym>>(-(injection.p_variance + injection.q_variance))};
                    }
                }
                // injection measurement not exist
                else {
                    // Q_ij = 0
                    // R_ii = -1.0, only diagonal
                    // assign -1.0 to diagonal of 3x3 tensor, for asym
                    if (row == col) {
                        block.r() = ComplexTensor<sym>{-1.0};
                    }
                }
            }
        }

        // loop all transpose entry for QH
        // assign the hermitian transpose of the transpose entry of Q
        for (Idx data_idx_lu = 0; data_idx_lu != y_bus.nnz_lu(); ++data_idx_lu) {
            // skip for fill-in
            if (y_bus.map_lu_y_bus()[data_idx_lu] == -1) {
                continue;
            }
            Idx const data_idx_tranpose = y_bus.lu_transpose_entry()[data_idx_lu];
            data_gain_[data_idx_lu].qh() = hermitian_transpose(data_gain_[data_idx_tranpose].q());
        }
        // prefactorize
        sparse_solver_.prefactorize(data_gain_, perm_);
    }

    void prepare_rhs(YBus<sym> const& y_bus, MeasuredValues<sym> const& measured_value,
                     ComplexValueVector<sym> const& current_u) {
        MathModelParam<sym> const& param = y_bus.math_model_param();
        std::vector<BranchIdx> const branch_bus_idx = y_bus.math_topology().branch_bus_idx;
        // get generated (measured/estimated) voltage phasor
        // with current result voltage angle
        ComplexValueVector<sym> u = measured_value.voltage(current_u);

        // loop all bus to fill rhs
        for (Idx bus = 0; bus != n_bus_; ++bus) {
            Idx const data_idx = y_bus.bus_entry()[bus];
            // reset rhs block to fill values
            NRSERhs<sym>& rhs_block = x_rhs_[bus];
            rhs_block = NRSERhs<sym>{};
            // fill block with voltage measurement
            if (measured_value.has_voltage(bus)) {
                // eta += u / variance
                rhs_block.eta() += u[bus] / measured_value.voltage_var(bus);
            }
            // fill block with branch, shunt measurement, need to convert to current
            for (Idx element_idx = y_bus.y_bus_entry_indptr()[data_idx];
                 element_idx != y_bus.y_bus_entry_indptr()[data_idx + 1]; ++element_idx) {
                Idx const obj = y_bus.y_bus_element()[element_idx].idx;
                YBusElementType const type = y_bus.y_bus_element()[element_idx].element_type;
                // shunt
                if (type == YBusElementType::shunt) {
                    if (measured_value.has_shunt(obj)) {
                        PowerSensorCalcParam<sym> const& m = measured_value.shunt_power(obj);
                        // eta -= Ys^H * (variance^-1) * i_shunt
                        rhs_block.eta() -= dot(hermitian_transpose(param.shunt_param[obj]),
                                               diagonal_inverse(m.p_variance + m.q_variance), conj(m.value / u[bus]));
                    }
                }
                // branch
                else {
                    // branch is either ff or tt
                    IntS const b = static_cast<IntS>(type) / 2;
                    assert(b == static_cast<IntS>(type) % 2);
                    // measured at from-side: 0, to-side: 1
                    for (IntS const measured_side : std::array<IntS, 2>{0, 1}) {
                        // has measurement
                        if (std::invoke(has_branch_[measured_side], measured_value, obj)) {
                            PowerSensorCalcParam<sym> const& m =
                                std::invoke(branch_power_[measured_side], measured_value, obj);
                            // the current needs to be calculated with the voltage of the measured bus side
                            // NOTE: not the current bus!
                            Idx const measured_bus = branch_bus_idx[obj][measured_side];
                            // eta += Y{side, b}^H * (variance^-1) * i_branch_{f, t}
                            rhs_block.eta() +=
                                dot(hermitian_transpose(param.branch_param[obj].value[measured_side * 2 + b]),
                                    diagonal_inverse(m.p_variance + m.q_variance), conj(m.value / u[measured_bus]));
                        }
                    }
                }
            }
            // fill block with injection measurement, need to convert to current
            if (measured_value.has_bus_injection(bus)) {
                rhs_block.tau() = conj(measured_value.bus_injection(bus).value / u[bus]);
            }
        }
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
