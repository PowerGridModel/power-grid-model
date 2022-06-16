// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MATH_SOLVER_NEWTON_RAPHSON_PF_SOLVER_HPP
#define POWER_GRID_MODEL_MATH_SOLVER_NEWTON_RAPHSON_PF_SOLVER_HPP

/*
Newton Raphson Power Flow

****** Voltage
U_i = V_i * exp(1j * theta_i) = U_i_r + 1j * U_i_i
U_i_r = V_i * cos(theta_i)
U_i_i = V_i * sin(theta_i)

****** Admittance matrix
Yij = Gij + 1j * Bij

****** Object function:

f(theta, V) = PQ_sp - PQ_cal = del_pq = 0
sp = specified
cal = calculated

PQ_sp = [P_sp_0, Q_sp_0, P_sp_1, Q_sp_1, ...]^T
PQ_cal = [P_cal_0, Q_cal_0, P_cal_1, Q_cal_1, ...]^T


****** Solution: use Newton-Raphson iteration
The modified Jacobian derivative
Jf = [
     [Jf00, Jf01, Jf02, ..., ]
     [Jf10, Jf11, Jf12, ..., ],
     ...
    ]

J = -Jf
  J_ij =
     [[dP_cal_i/dtheta_j, dP_cal_i/dV_j * V_j],
      [dQ_cal_i/dtheta_j, dQ_cal_i/dV_j * V_j]]
    -
     [[dP_sp_i/dtheta_j, dP_sp_i/dV_j * V_j],
      [dQ_sp_i/dtheta_j, dQ_sp_i/dV_j * V_j]]

Iteration increment
del_x = [del_theta_0, del_V_0/V_0, del_theta_1, del_V_1/V_1, ...]^T = - (Jf)^-1 * del_pq = J^-1 * del_pq

theta_i_(k+1) = theta_i_(k) + del_theta_i
V_i_(k+1) = V_i_k + (del_V_i/V_i) * V_i


****** Calculation process
set J[...] = 0
set del_pq[...] = 0


*** intermediate variable H, N, M, L into J, as incomplete J

symbol @* : outer product of two vectors https://en.wikipedia.org/wiki/Outer_product
     x1 = [a, b]^T, x2 = [c, d]^T, x1 @* x2 = [[ac, ad], [bc, bd]]
symbol .* : elementwise multiply of two matrices/tensors or vector

theta_ij =
    for symmetric: theta_i - theta_j
    for asymmetric: [
                     [theta_i_a - theta_j_a, theta_i_a - theta_j_b, theta_i_a - theta_j_c],
                     [theta_i_b - theta_j_a, theta_i_b - theta_j_b, theta_i_b - theta_j_c],
                     [theta_i_c - theta_j_a, theta_i_c - theta_j_b, theta_i_c - theta_j_c]
                    ]
diag(Vi) * cos(theta_ij) * diag(Vj) = Ui_r @* Uj_r + Ui_i @* Uj_i = cij
diag(Vi) * sin(theta_ij) * diag(Vj) = Ui_i @* Uj_r - Ui_r @* Uj_i = sij

Hij = diag(Vi) * ( Gij .* sin(theta_ij) - Bij .* cos(theta_ij) ) * diag(Vj)
    = Gij .* sij - Bij .* cij
Nij = diag(Vi) * ( Gij .* cos(Theta_ij) + Bij .* sin(Theta_ij) ) * diag(Vj)
    = Gij .* cij + Bij .* sij
Mij = - Nij
Lij = Hij

save to J_ij = [
                   [Hij, Nij],
                   [Mij, Lij]
               ]

*** PQ_cal
P_cal_i = sum{j} (Nij * I)
Q_cal_i = sum{j} (Hij * I)
I =
    symmetric: 1
    asymmetric: [1, 1, 1]^T
set
del_pq_i = -[P_cal_i, Q_cal_i]

*** modify J into complete Jacobian for PQ_cal
correction for diagonal
Jii.H += diag(-Q_cal_i)
Jii.N -= diag(-P_cal_i)
Jii.M -= diag(-P_i)
Jii.L -= diag(-Q_cal_i)

*** calculate PQ_sp, and dPQ_sp/(dtheta, dV)

** for load/generation
PQ_sp =
    PQ_base for constant pq
    PQ_base * V for constant i
    PQ_base * V^2 for constant z
del_pq += PQ_sp

dPQ_sp/dtheta = 0
dPQ_sp/dV_i * V =
    0 for constant pq
    PQ_base * V for constant i (dPQ_sp/dV = PQ_base)
    PQ_base * 2 * V^2 for constant z (dPQ_sp/dV = PQ_base * 2 * V)
J.N -= diag(dP_sp/dV .* V)
J.L -= diag(dQ_sp/dV .* V)

** for source
A mini two bus equivalent system is built to solve the problem

bus_m (network) ---Y--- bus_s (voltage_source)
element_admittance matrix [[Y -Y], [-Y, Y]]
voltage at bus_s is known as U_ref
voltage at bus_m is U_m

The PQ_sp for the bus_m in the network, is in this case the negative of the power injection for this fictional 2-bus
network

* Calculate HNML for mm, ms, using the same formula, then calculate the other quantities
P_cal_m = (Nmm + Nms) * I
Q_cal_m = (Hmm + Hms) * I
dP_cal_m/dtheta = Hmm - diag(Q_cal_m)
dP_cal_m/dV = Nmm + diag(P_cal_m)
dQ_cal_m/dtheta = Mmm + diag(P_cal_m)
dQ_cal_m/dV = Lmm + diag(Q_cal_m)

* negate the value and add into the main matrices
PQ_sp = -PQ_cal_m
del_pq -= PQ_cal_m

J.H -= -dP_cal_m/dtheta
J.N -= -dP_cal_m/dV
J.M -= -dQ_cal_m/dtheta
J.L -= -dQ_cal_m/dV

*/

#include "../calculation_parameters.hpp"
#include "../exception.hpp"
#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"
#include "../timer.hpp"
#include "block_matrix.hpp"
#include "bsr_solver.hpp"
#include "iterative_pf_solver.hpp"
#include "y_bus.hpp"

namespace power_grid_model {

// hide implementation in inside namespace
namespace math_model_impl {

// class for phasor in polar coordinate
template <bool sym>
struct PolarPhasor {
    RealValue<sym> theta;
    RealValue<sym> v;
};
static_assert(sizeof(PolarPhasor<true>) == sizeof(double[2]));
static_assert(alignof(PolarPhasor<true>) == alignof(double[2]));
static_assert(std::is_standard_layout_v<PolarPhasor<true>>);
static_assert(sizeof(PolarPhasor<false>) == sizeof(double[6]));
static_assert(alignof(PolarPhasor<false>) == alignof(double[6]));
static_assert(std::is_standard_layout_v<PolarPhasor<false>>);
// class for complex power
template <bool sym>
struct ComplexPower {
    RealValue<sym> p;
    RealValue<sym> q;
};
static_assert(sizeof(ComplexPower<true>) == sizeof(double[2]));
static_assert(alignof(ComplexPower<true>) == alignof(double[2]));
static_assert(std::is_standard_layout_v<ComplexPower<true>>);
static_assert(sizeof(ComplexPower<false>) == sizeof(double[6]));
static_assert(alignof(ComplexPower<false>) == alignof(double[6]));
static_assert(std::is_standard_layout_v<ComplexPower<false>>);

// class of pf block
// block of incomplete power flow jacobian
// non-diagonal H, N, M, L
// [ [H = dP/dTheta, N = V * dP/dV],
// [M = dQ/dTheta = -N, L = V * dQ/dV = H] ]
// Hij = Gij .* sij - Bij .* cij = L
// Nij = Gij .* cij + Bij .* sij = -M
template <bool sym>
class PFJacBlock : public BlockEntry<double, sym> {
   public:
    using typename BlockEntry<double, sym>::GetterType;
    GetterType h() {
        return this->template get_val<0, 0>();
    }
    GetterType n() {
        return this->template get_val<0, 1>();
    }
    GetterType m() {
        return this->template get_val<1, 0>();
    }
    GetterType l() {
        return this->template get_val<1, 1>();
    }
};
constexpr block_entry_trait<PFJacBlock> jac_trait{};

// solver
template <bool sym>
class NewtonRaphsonPFSolver : public IterativePFSolver<sym> {
   private:
    // block size 2 for symmetric, 6 for asym
    static constexpr Idx bsr_block_size_ = sym ? 2 : 6;

   public:
    NewtonRaphsonPFSolver(YBus<sym> const& y_bus, std::shared_ptr<MathModelTopology const> const& topo_ptr)
        : IterativePFSolver<sym>{y_bus, topo_ptr},
          n_bus(y_bus.size()),
          data_jac_(y_bus.nnz()),
          x_(y_bus.size()),
          del_x_(y_bus.size()),
          del_pq_(y_bus.size()),
          bsr_solver_{y_bus.size(), bsr_block_size_, y_bus.shared_indptr(), y_bus.shared_indices()} {
    }

    MathOutput<sym> run_power_flow(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input, double err_tol,
                                   Idx max_iter, CalculationInfo& calculation_info) {
        std::vector<double> const& phase_shift = *get_phase_shift();    //*phase_shift_;
        IdxVector const& source_bus_indptr = *get_source_bus_indptr();  //*source_bus_indptr_;
        // Change n_bus_ name
        Idx n_bus = y_bus.size();
        // prepare
        MathOutput<sym> output;
        output.u.resize(n_bus);
        double max_dev = std::numeric_limits<double>::max();

        Timer main_timer(calculation_info, 2220, "Math solver");

        // initialize
        Timer sub_timer(calculation_info, 2221, "Initialize calculation");
        // average u_ref of all sources
        DoubleComplex const u_ref = [&]() {
            DoubleComplex sum_u_ref = 0.0;
            for (Idx bus = 0; bus != n_bus; ++bus) {
                for (Idx source = source_bus_indptr[bus]; source != source_bus_indptr[bus + 1]; ++source) {
                    sum_u_ref += input.source[source] * std::exp(1.0i * -phase_shift[bus]);  // offset phase shift
                }
            }
            return sum_u_ref / (double)input.source.size();
        }();
        // assign u_ref as flat start
        for (Idx i = 0; i != n_bus; ++i) {
            // consider phase shift
            output.u[i] = ComplexValue<sym>{u_ref * std::exp(1.0i * phase_shift[i])};
            x_[i].v = cabs(output.u[i]);
            x_[i].theta = arg(output.u[i]);
        }
        sub_timer.stop();

        // start calculation
        // iteration
        Idx num_iter = 0;
        while (max_dev > err_tol) {
            if (num_iter++ == max_iter) {
                throw IterationDiverge{max_iter, max_dev, err_tol};
            }
            sub_timer = Timer(calculation_info, 2222, "Calculate jacobian and rhs");
            calculate_jacobian_and_deviation(y_bus, input, output.u);
            sub_timer = Timer(calculation_info, 2223, "Solve sparse linear equation");
            bsr_solver_.solve(data_jac_.data(), del_pq_.data(), del_x_.data());
            sub_timer = Timer(calculation_info, 2224, "Iterate unknown");
            max_dev = iterate_unknown(output.u);
            sub_timer.stop();
        }

        // calculate math result
        sub_timer = Timer(calculation_info, 2225, "Calculate Math Result");
        calculate_result(y_bus, input, output);

        // Manually stop timers to avoid "Max number of iterations" to be included in the timing.
        sub_timer.stop();
        main_timer.stop();

        const auto key = Timer::make_key(2226, "Max number of iterations");
        calculation_info[key] = std::max(calculation_info[key], (double)num_iter);

        return output;
    }

   private:
    // data for jacobian
    std::vector<PFJacBlock<sym>> data_jac_;
    // calculation data
    std::vector<PolarPhasor<sym>> x_;      // unknown
    std::vector<PolarPhasor<sym>> del_x_;  // unknown iterative
    // this stores in different steps
    // 1. negative power injection: - p/q_calculated
    // 2. power unbalance: p/q_specified - p/q_calculated
    std::vector<ComplexPower<sym>> del_pq_;
    BSRSolver<double> bsr_solver_;
    // ! 2 instances of n_bus
    Idx n_bus;

    void calculate_jacobian_and_deviation(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input,
                                          ComplexValueVector<sym> const& u) {
        IdxVector const& load_gen_bus_indptr = *get_load_gen_bus_indptr();     //*load_gen_bus_indptr_;
        IdxVector const& source_bus_indptr = *get_source_bus_indptr();         //*source_bus_indptr_;
        std::vector<LoadGenType> const& load_gen_type = *get_load_gen_type();  //*load_gen_type_;
        ComplexTensorVector<sym> const& ydata = y_bus.admittance();
        IdxVector const& indptr = y_bus.row_indptr();
        IdxVector const& indices = y_bus.col_indices();
        IdxVector const& bus_entry = y_bus.bus_entry();

        // loop for row indices as i for whole matrix
        for (Idx i = 0; i != n_bus; ++i) {
            // reset power injection
            del_pq_[i].p = RealValue<sym>{0.0};
            del_pq_[i].q = RealValue<sym>{0.0};
            // loop for column for incomplete jacobian and injection
            // k as data indices
            // j as column indices
            for (Idx k = indptr[i]; k != indptr[i + 1]; ++k) {
                Idx const j = indices[k];
                // incomplete jacobian
                data_jac_[k] = calculate_hnml(ydata[k], u[i], u[j]);
                // accumulate negative power injection
                // -P = sum(-N)
                del_pq_[i].p -= sum_row(data_jac_[k].n());
                // -Q = sum (-H)
                del_pq_[i].q -= sum_row(data_jac_[k].h());
            }
            // correct diagonal part of jacobian
            Idx const k = bus_entry[i];
            // diagonal correction
            // del_pq has negative injection
            // H += (-Q)
            add_diag(data_jac_[k].h(), del_pq_[i].q);
            // N -= (-P)
            add_diag(data_jac_[k].n(), -del_pq_[i].p);
            // M -= (-P)
            add_diag(data_jac_[k].m(), -del_pq_[i].p);
            // L -= (-Q)
            add_diag(data_jac_[k].l(), -del_pq_[i].q);
        }

        // loop individual load/source, i as bus number, j as load/source number
        for (Idx i = 0; i != n_bus; ++i) {
            // k as data sequence number
            Idx const k = bus_entry[i];

            // loop load
            for (Idx j = load_gen_bus_indptr[i]; j != load_gen_bus_indptr[i + 1]; ++j) {
                // load type
                LoadGenType const type = load_gen_type[j];
                // modify jacobian and del_pq based on type
                switch (type) {
                    case LoadGenType::const_pq:
                        // PQ_sp = PQ_base
                        del_pq_[i].p += real(input.s_injection[j]);
                        del_pq_[i].q += imag(input.s_injection[j]);
                        // -dPQ_sp/dV * V = 0
                        break;
                    case LoadGenType::const_y:
                        // PQ_sp = PQ_base * V^2
                        del_pq_[i].p += real(input.s_injection[j]) * x_[i].v * x_[i].v;
                        del_pq_[i].q += imag(input.s_injection[j]) * x_[i].v * x_[i].v;
                        // -dPQ_sp/dV * V = -PQ_base * 2 * V^2
                        add_diag(data_jac_[k].n(), -real(input.s_injection[j]) * 2.0 * x_[i].v * x_[i].v);
                        add_diag(data_jac_[k].l(), -imag(input.s_injection[j]) * 2.0 * x_[i].v * x_[i].v);
                        break;
                    case LoadGenType::const_i:
                        // PQ_sp = PQ_base * V
                        del_pq_[i].p += real(input.s_injection[j]) * x_[i].v;
                        del_pq_[i].q += imag(input.s_injection[j]) * x_[i].v;
                        // -dPQ_sp/dV * V = -PQ_base * V
                        add_diag(data_jac_[k].n(), -real(input.s_injection[j]) * x_[i].v);
                        add_diag(data_jac_[k].l(), -imag(input.s_injection[j]) * x_[i].v);
                        break;
                    default:
                        throw MissingCaseForEnumError("Jacobian and deviation calculation", type);
                }
            }

            // loop source
            for (Idx j = source_bus_indptr[i]; j != source_bus_indptr[i + 1]; ++j) {
                ComplexTensor<sym> const y_ref = y_bus.math_model_param().source_param[j];
                ComplexValue<sym> const u_ref{input.source[j]};
                // calculate block, um = ui, us = uref
                PFJacBlock<sym> block_mm = calculate_hnml(y_ref, u[i], u[i]);
                PFJacBlock<sym> block_ms = calculate_hnml(-y_ref, u[i], u_ref);
                // P_cal_m = (Nmm + Nms) * I
                RealValue<sym> const p_cal = sum_row(block_mm.n() + block_ms.n());
                // Q_cal_m = (Hmm + Hms) * I
                RealValue<sym> const q_cal = sum_row(block_mm.h() + block_ms.h());
                // correct hnml for mm
                add_diag(block_mm.h(), -q_cal);
                add_diag(block_mm.n(), p_cal);
                add_diag(block_mm.m(), p_cal);
                add_diag(block_mm.l(), q_cal);
                // append to del_pq
                del_pq_[i].p -= p_cal;
                del_pq_[i].q -= q_cal;
                // append to jacobian block
                // hnml -= -dPQ_cal/(dtheta,dV)
                // hnml += dPQ_cal/(dtheta,dV)
                data_jac_[k].h() += block_mm.h();
                data_jac_[k].n() += block_mm.n();
                data_jac_[k].m() += block_mm.m();
                data_jac_[k].l() += block_mm.l();
            }
        }
    }

    double iterate_unknown(ComplexValueVector<sym>& u) {
        double max_dev = 0.0;
        // loop each bus as i
        for (Idx i = 0; i != n_bus; ++i) {
            // angle
            x_[i].theta += del_x_[i].theta;
            // magnitude
            x_[i].v += x_[i].v * del_x_[i].v;
            // temperary complex phasor
            // U = V * exp(1i*theta)
            ComplexValue<sym> const& u_tmp = x_[i].v * exp(1.0i * x_[i].theta);
            // get dev of last iteration, get max
            double const dev = max_val(cabs(u_tmp - u[i]));
            max_dev = std::max(dev, max_dev);
            // assign
            u[i] = u_tmp;
        }
        return max_dev;
    }

    static PFJacBlock<sym> calculate_hnml(ComplexTensor<sym> const& yij, ComplexValue<sym> const& ui,
                                          ComplexValue<sym> const& uj) {
        PFJacBlock<sym> block{};
        // real and imag of addmittance
        RealTensor<sym> const gij = real(yij);
        RealTensor<sym> const bij = imag(yij);
        // diag(Vi) * cos(theta_ij) * diag(Vj)
        // Ui_r @* Uj_r + Ui_i @* Uj_i
        // = cij
        RealTensor<sym> const c_ij =
            vector_outer_product(real(ui), real(uj)) + vector_outer_product(imag(ui), imag(uj));
        // diag(Vi) * sin(theta_ij) * diag(Vj)
        // = Ui_i @* Uj_r - Ui_r @* Uj_i
        // = sij
        RealTensor<sym> const s_ij =
            vector_outer_product(imag(ui), real(uj)) - vector_outer_product(real(ui), imag(uj));
        // calculate H, N, M, L
        // Hij = diag(Vi) * ( Gij .* sin(theta_ij) - Bij .* cos(theta_ij) ) * diag(Vj)
        // = Gij .* sij - Bij .* cij
        block.h() = gij * s_ij - bij * c_ij;
        // Nij = diag(Vi) * ( Gij .* cos(Theta_ij) + Bij .* sin(Theta_ij) ) * diag(Vj)
        // = Gij .* cij + Bij .* sij
        block.n() = gij * c_ij + bij * s_ij;
        // Mij = - Nij
        block.m() = -block.n();
        // Lij = Hij
        block.l() = block.h();
        return block;
    }
};

template class NewtonRaphsonPFSolver<true>;
template class NewtonRaphsonPFSolver<false>;

}  // namespace math_model_impl

template <bool sym>
using NewtonRaphsonPFSolver = math_model_impl::NewtonRaphsonPFSolver<sym>;

}  // namespace power_grid_model

#endif
