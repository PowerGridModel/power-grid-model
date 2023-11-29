// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MATH_SOLVER_SE_SOLVER_HPP
#define POWER_GRID_MODEL_MATH_SOLVER_SE_SOLVER_HPP

/*
iterative linear state estimation solver
*/

#include "block_matrix.hpp"
#include "iterative_linear_se_solver.hpp"
#include "measured_values.hpp"
#include "newton_raphson_se_solver.hpp"
#include "sparse_lu_solver.hpp"
#include "y_bus.hpp"

#include "../calculation_parameters.hpp"
#include "../exception.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"
#include "../timer.hpp"

namespace power_grid_model {

// hide implementation in inside namespace
namespace math_model_impl {

// solver
template <bool sym, derived_solver_type DerivedSolver> class SESolver {

  public:
    SESolver(YBus<sym> const& y_bus, std::shared_ptr<MathModelTopology const> topo_ptr)
        : n_bus_{y_bus.size()},
          math_topo_{std::move(topo_ptr)},
          data_gain_(y_bus.nnz_lu()),
          x_rhs_(y_bus.size()),
          sparse_solver_{y_bus.shared_indptr_lu(), y_bus.shared_indices_lu(), y_bus.shared_diag_lu()},
          perm_(y_bus.size()),
          is_nr_solver_(std::is_same<DerivedSolver, NewtonRaphsonSESolver>::value;) {}

    friend DerivedSolver;

    using SEGainBlock = typename std::conditional<is_nr_solver_, NRSEGainBlock, ILSEGainBlock>::type;
    using SEUnknown = typename std::conditional<is_nr_solver_, NRSESEUnknown, ILSEUnknown>::type;
    using SERhs = typename std::conditional<is_nr_solver_, NRSESERhs, ILSERhs>::type;

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

        // prepare matrix, including pre-factorization
        sub_timer = Timer(calculation_info, 2222, "Prepare matrix, including pre-factorization");
        prepare_matrix(y_bus, measured_values, output.u);

        // loop to iterate
        Idx num_iter = 0;
        while (max_dev > err_tol || num_iter == 0) {
            if (num_iter++ == max_iter) {
                throw IterationDiverge{max_iter, max_dev, err_tol};
            }
            sub_timer = Timer(calculation_info, 2224, "Update matrices");
            if constexpr (is_nr_solver_) {
                prepare_matrix(y_bus, measured_values, output.u);
            }
            prepare_rhs(y_bus, measured_values, output.u);
            // solve with prefactorization
            sub_timer = Timer(calculation_info, 2225, "Solve sparse linear equation");
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

    // solver type
    bool is_nr_solver_;

    // data for gain matrix
    std::vector<SEGainBlock<sym>> data_gain_;
    // unknown and rhs
    std::vector<SERhs<sym>> x_rhs_;
    // solver
    SparseLUSolver<SEGainBlock<sym>, SERhs<sym>, SEUnknown<sym>> sparse_solver_;
    typename SparseLUSolver<SEGainBlock<sym>, SERhs<sym>, SEUnknown<sym>>::BlockPermArray perm_;

    auto diagonal_inverse(RealValue<sym> const& value) {
        return ComplexDiagonalTensor<sym>{static_cast<ComplexValue<sym>>(RealValue<sym>{1.0} / value)};
    }

    void prepare_matrix(YBus<sym> const& y_bus, MeasuredValues<sym> const& measured_value) {
        MathModelParam<sym> const& param = y_bus.math_model_param();
        IdxVector const& row_indptr = y_bus.row_indptr_lu();
        IdxVector const& col_indices = y_bus.col_indices_lu();

        // loop data index, all rows and columns
        for (Idx row = 0; row != n_bus_; ++row) {
            for (Idx data_idx_lu = row_indptr[row]; data_idx_lu != row_indptr[row + 1]; ++data_idx_lu) {
                Idx const col = col_indices[data_idx_lu];
                // get a reference and reset block to zero
                SEGainBlock<sym>& block = data_gain_[data_idx_lu];
                block = SEGainBlock<sym>{};
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
                    block.g() += ComplexTensor<sym>{1.0 / measured_value.voltage_var(row)};
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
                            auto const& shunt_power = measured_value.shunt_power(obj);
                            block.g() += dot(hermitian_transpose(param.shunt_param[obj]),
                                             diagonal_inverse(shunt_power.p_variance + shunt_power.q_variance),
                                             param.shunt_param[obj]);
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
                                auto const& power = std::invoke(branch_power_[measured_side], measured_value, obj);
                                block.g() +=
                                    dot(hermitian_transpose(param.branch_param[obj].value[measured_side * 2 + b0]),
                                        diagonal_inverse(power.p_variance + power.q_variance),
                                        param.branch_param[obj].value[measured_side * 2 + b1]);
                            }
                        }
                    }
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

    void update_matrices(YBus<sym> const& y_bus, MeasuredValues<sym> const& measured_value,
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
            SERhs<sym>& rhs_block = x_rhs_[bus];
            rhs_block = SERhs<sym>{};
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

    double find_max_deviation(ComplexValueVector<sym>& u, bool has_angle_measurement) {
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

} // namespace math_model_impl

} // namespace power_grid_model

#endif
