// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

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

#include "block_matrix.hpp"
#include "iterative_pf_solver.hpp"
#include "sparse_lu_solver.hpp"
#include "y_bus.hpp"

#include "../calculation_parameters.hpp"
#include "../common/common.hpp"
#include "../common/counting_iterator.hpp"
#include "../common/enum.hpp"
#include "../common/exception.hpp"
#include "../common/grouped_index_vector.hpp"
#include "../common/three_phase_tensor.hpp"

#include <algorithm>
#include <complex>
#include <functional>
#include <ranges>
#include <vector>

namespace power_grid_model::math_solver {

// hide implementation in inside namespace
namespace newton_raphson_pf {

constexpr Idx no_limit_check = -1;
constexpr Idx do_limit_check = 0;
constexpr Idx limit_check_at_iteration = 2; // TODO(frie-soptim): maybe consider making this a parameter in the future

// class for phasor in polar coordinate and/or complex power
template <symmetry_tag sym> struct PolarPhasor : public Block<double, sym, false, 2> {
    template <int r, int c> using GetterType = Block<double, sym, false, 2>::template GetterType<r, c>;

    // eigen expression
    using Block<double, sym, false, 2>::Block;
    using Block<double, sym, false, 2>::operator=;

    GetterType<0, 0> theta() { return this->template get_val<0, 0>(); }
    GetterType<1, 0> v() { return this->template get_val<1, 0>(); }

    GetterType<0, 0> p() { return this->template get_val<0, 0>(); }
    GetterType<1, 0> q() { return this->template get_val<1, 0>(); }

    // linear solve path getters: x0 = Ur, x1 = Ui and Eq0 = Real, Eq1 = Imag
    GetterType<0, 0> i_real() { return this->template get_val<0, 0>(); }
    GetterType<1, 0> i_imag() { return this->template get_val<1, 0>(); }
    GetterType<0, 0> u_real() { return this->template get_val<0, 0>(); }
    GetterType<1, 0> u_imag() { return this->template get_val<1, 0>(); }
};

// class for complex power
template <symmetry_tag sym> using ComplexPower = PolarPhasor<sym>;

// class of pf block
// block of incomplete power flow jacobian
// non-diagonal H, N, M, L
// [ [H = dP/dTheta, N = V * dP/dV],
// [M = dQ/dTheta = -N, L = V * dQ/dV = H] ]
// Hij = Gij .* sij - Bij .* cij = L
// Nij = Gij .* cij + Bij .* sij = -M
template <symmetry_tag sym> class PFJacBlock : public Block<double, sym, true, 2> {
  public:
    template <int r, int c> using GetterType = Block<double, sym, true, 2>::template GetterType<r, c>;

    // eigen expression
    using Block<double, sym, true, 2>::Block;
    using Block<double, sym, true, 2>::operator=;

    GetterType<0, 0> h() { return this->template get_val<0, 0>(); }
    GetterType<0, 1> n() { return this->template get_val<0, 1>(); }
    GetterType<1, 0> m() { return this->template get_val<1, 0>(); }
    GetterType<1, 1> l() { return this->template get_val<1, 1>(); }

    // linear solve path getters: x0 = Ur, x1 = Ui and Eq0 = Real, Eq1 = Imag
    // System: [[G, -B], [B, G]] * [Ur, Ui]^T = [Ir, Ii]^T
    GetterType<0, 0> real_real() { return this->template get_val<0, 0>(); }
    GetterType<0, 1> real_imag() { return this->template get_val<0, 1>(); }
    GetterType<1, 0> imag_real() { return this->template get_val<1, 0>(); }
    GetterType<1, 1> imag_imag() { return this->template get_val<1, 1>(); }
};

// solver
template <symmetry_tag sym_type>
class NewtonRaphsonPFSolver : public IterativePFSolver<sym_type, NewtonRaphsonPFSolver<sym_type>> {
  public:
    using sym = sym_type;

    using SparseSolverType = SparseLUSolver<PFJacBlock<sym>, ComplexPower<sym>, PolarPhasor<sym>>;
    using BlockPermArray = SparseLUSolver<PFJacBlock<sym>, ComplexPower<sym>, PolarPhasor<sym>>::BlockPermArray;

    static constexpr auto is_iterative = true;

    NewtonRaphsonPFSolver(YBus<sym> const& y_bus, MathModelTopology const& topo)
        : IterativePFSolver<sym, NewtonRaphsonPFSolver>{y_bus, topo},
          data_jac_(y_bus.nnz_lu()),
          x_(y_bus.size()),
          del_x_pq_(y_bus.size()),
          sparse_solver_{y_bus.row_indptr_lu(), y_bus.col_indices_lu(), y_bus.lu_diag()},
          perm_(y_bus.size()),
          bus_control_(y_bus.size()),
          voltage_regulators_per_load_gen_{std::ref(topo.voltage_regulators_per_load_gen)},
          clamped_regulators_per_load_gen_(topo.load_gen_type.size(), RealValue<sym>{nan}) {}

    // Initialize the unknown variable in polar form using a linear voltage guess solved in the real domain.
    // This implementation reuses the class-level Jacobian matrix (data_jac_) and RHS vector (del_x_pq_)
    // to eliminate temporary memory allocations.
    // The complex system (G + jB)(Ur + jUi) = Ir + jIi is mapped to the real system:
    // [[G, -B], [B, G]] [Ur, Ui]^T = [Ir, Ii]^T
    // This mapping ensures that G aligns with the N/M sub-blocks as in the NR Jacobian.
    void initialize_derived_solver(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input,
                                   SolverOutput<sym>& output) {
        // Reset reused buffers to zero
        std::ranges::fill(data_jac_, PFJacBlock<sym>{});
        std::ranges::fill(del_x_pq_, PolarPhasor<sym>{});

        // initialize bus state, in case solver instance is reused in batching
        std::ranges::fill(bus_control_, BusControlState{});

        const bool has_usable_limits = set_bus_types_and_q_limits(input);
        limit_check_countdown_ = has_usable_limits ? limit_check_at_iteration : no_limit_check;

        // Map network admittance to real-domain system
        IdxVector const& map_lu_y_bus = y_bus.map_lu_y_bus();
        ComplexTensorVector<sym> const& ydata = y_bus.admittance();
        for (Idx jac_idx = 0; jac_idx < std::ssize(data_jac_); ++jac_idx) {
            Idx const k_y_bus = map_lu_y_bus[jac_idx];
            if (k_y_bus != -1) {
                set_linear_block(data_jac_[jac_idx], ydata[k_y_bus]);
            }
        }

        IdxVector const& bus_entry = y_bus.lu_diag();
        auto const& load_gens_per_bus = this->load_gens_per_bus_.get();
        auto const& sources_per_bus = this->sources_per_bus_.get();
        for (auto const& [bus_number, load_gens, sources] :
             enumerated_zip_sequence(load_gens_per_bus, sources_per_bus)) {
            Idx const diagonal_position = bus_entry[bus_number];
            add_linear_initial_guess_loads(load_gens, data_jac_[diagonal_position], input);
            add_linear_initial_guess_sources(sources, data_jac_[diagonal_position], del_x_pq_[bus_number], y_bus,
                                             input);
        }

        // Solve: [Ur, Ui] result in [p, q]
        sparse_solver_.prefactorize_and_solve(data_jac_, perm_, del_x_pq_, del_x_pq_);

        for (Idx const i : std::views::iota(Idx{}, this->n_bus_)) {
            output.u[i] =
                ComplexValue<sym>{RealValue<sym>{del_x_pq_[i].u_real()}, RealValue<sym>{del_x_pq_[i].u_imag()}};
        }

        set_reference_voltage_for_pv_buses(output.u);

        // get magnitude and angle of start voltage
        for (Idx i = 0; i != this->n_bus_; ++i) {
            x_[i].v() = cabs(output.u[i]);
            x_[i].theta() = arg(output.u[i]);
        }
    }

    // Calculate the Jacobian and deviation
    void prepare_matrix_and_rhs(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input,
                                ComplexValueVector<sym> const& u) {
        build_jacobian_and_rhs(y_bus, input, u, false);

        if (limit_check_countdown_ > do_limit_check) {
            --limit_check_countdown_;
        }
        const bool buses_switched = (limit_check_countdown_ == do_limit_check) && enforce_q_limits(input);
        if (buses_switched) {
            // rebuild jacobian and del_pq after PV -> PQ switching
            build_jacobian_and_rhs(y_bus, input, u, true);
        }
        apply_pv_constraints(y_bus);
    }

    // Solve the linear Equations
    void solve_matrix() { sparse_solver_.prefactorize_and_solve(data_jac_, perm_, del_x_pq_, del_x_pq_); }

    // Get maximum deviation among all bus voltages
    double iterate_unknown(ComplexValueVector<sym>& u, double err_tol, bool cache_run) {
        double max_dev = 0.0;
        // loop each bus as i
        for (Idx i = 0; i != this->n_bus_; ++i) {
            // angle
            x_[i].theta() += del_x_pq_[i].theta();
            // magnitude
            x_[i].v() += x_[i].v() * del_x_pq_[i].v();
            // temporary complex phasor
            // U = V * exp(1i*theta)
            ComplexValue<sym> const& u_tmp = x_[i].v() * exp(1.0i * x_[i].theta());
            // get dev of last iteration, get max
            double const dev = max_val(cabs(u_tmp - u[i]));
            max_dev = std::max(dev, max_dev);
            // assign
            u[i] = u_tmp;
        }

        if (max_dev <= err_tol && (limit_check_countdown_ > do_limit_check) && !cache_run) {
            // force a limit check if it did not happen yet, but the solution has already converged
            limit_check_countdown_ = do_limit_check;
            return std::numeric_limits<double>::infinity();
        }
        return max_dev;
    }

    void finalize_result(PowerFlowInput<sym> const& /*input*/, SolverOutput<sym>& output) {
        // pass on values needed for distribution of reactive power among voltage regulators in common_solver_function
        output.bus.resize(this->n_bus_);
        for (Idx i = 0; i != this->n_bus_; ++i) {
            auto& bus_output = output.bus[i];
            auto const& q_limit = bus_control_[i].q_limit;

            bus_output.bus_type = bus_control_[i].type;
            bus_output.q_limit_violated = q_limit.limit_violated;
        }
    }

  private:
    // data for jacobian
    std::vector<PFJacBlock<sym>> data_jac_;
    // calculation data
    std::vector<PolarPhasor<sym>> x_; // unknown
    // this stores in different steps
    // 1. negative power injection: - p/q_calculated
    // 2. power unbalance: p/q_specified - p/q_calculated
    // 3. unknown iterative
    std::vector<PolarPhasor<sym>> del_x_pq_;

    SparseSolverType sparse_solver_;
    // permutation array
    BlockPermArray perm_;

    struct QLimitState {
        bool has_q_limits{false};                 // check before using limits
        bool recalc_after_limit_violation{false}; // re-calculate jacobian and del_x_pq_ after bus type switch if true
        LimitViolation limit_violated{LimitViolation::none};
        double bus_q_min{0}; // start with 0, unspecified limit on regulator should change this to NaN
        double bus_q_max{0};
    };

    struct BusControlState {
        BusType type{BusType::pq};
        DoubleComplex u_ref{};
        QLimitState q_limit{};
    };

    std::vector<BusControlState> bus_control_;
    std::reference_wrapper<DenseGroupedIdxVector const> voltage_regulators_per_load_gen_;

    // limit check control: -1 = no limit check, 0 = check limit, >0 = wait this many iterations before checking
    Idx limit_check_countdown_{-1};

    // store clamped Q-value for a load_gen in case of a limit violation to avoid recalculation in add_loads()
    std::vector<RealValue<sym>> clamped_regulators_per_load_gen_;

    auto set_bus_types_and_q_limits(PowerFlowInput<sym> const& input) {
        auto const& voltage_regulators_per_load_gen = voltage_regulators_per_load_gen_.get();

        auto has_usable_q_limits = false;
        for (auto const& [bus_idx, load_gens, sources] :
             enumerated_zip_sequence(this->load_gens_per_bus_.get(), this->sources_per_bus_.get())) {
            auto& bus_control = bus_control_[bus_idx];
            if (!sources.empty()) {
                bus_control.type = BusType::slack;
                // don't need to calculate bus limits for slack buses, as active sources and regulators on the same node
                // are disallowed
                continue;
            }

            for (Idx const load_gen_idx : load_gens) {
                if (input.load_gen_status.empty() || input.load_gen_status[load_gen_idx] == status_off) {
                    continue;
                }
                for (Idx const voltage_regulator_idx :
                     voltage_regulators_per_load_gen.get_element_range(load_gen_idx)) {
                    // TODO(figueroa1395): Unit test this
                    auto const& regulator = input.voltage_regulator[voltage_regulator_idx];
                    if (regulator.status != status_off) {

                        bus_control.type = BusType::pv;
                        bus_control.u_ref = regulator.u_ref; // all regulators on the bus must have the same u_ref

                        // unlimited load_gen/regulator on a bus makes the whole bus unlimited,
                        // because `bus_limit = limit_reg1 + NaN = NaN`
                        bus_control.q_limit.bus_q_min += regulator.q_min;
                        bus_control.q_limit.bus_q_max += regulator.q_max;
                    }
                }
            }

            if (bus_control.type == BusType::pv) {
                // bus has limits if q_min or q_max is not NaN
                bus_control.q_limit.has_q_limits =
                    !is_nan(bus_control.q_limit.bus_q_min) || !is_nan(bus_control.q_limit.bus_q_max);
                if (bus_control.q_limit.has_q_limits) {
                    has_usable_q_limits = true;
                }
            }
        }
        return has_usable_q_limits;
    }

    void set_reference_voltage_for_pv_buses(ComplexValueVector<sym>& u) {
        for (Idx i = 0; i != this->n_bus_; ++i) {
            if (bus_control_[i].type == BusType::pv) {
                u[i] = bus_control_[i].u_ref * phase_shift(u[i]);
            }
        }
    }

    /// @brief power_flow_ij = (ui @* conj(uj))  .* conj(yij)
    /// Hij = diag(Vi) * ( Gij .* sin(theta_ij) - Bij .* cos(theta_ij) ) * diag(Vj)
    /// = imaginary(power_flow_ij)
    /// Nij = diag(Vi) * ( Gij .* cos(theta_ij) + Bij .* sin(theta_ij) ) * diag(Vj)
    /// = real(power_flow_ij)
    /// Mij = -Nij
    /// Lij = Hij
    static PFJacBlock<sym> calculate_hnml(ComplexTensor<sym> const& yij, ComplexValue<sym> const& ui,
                                          ComplexValue<sym> const& uj) {
        PFJacBlock<sym> block{};
        ComplexTensor<sym> const power_flow_ij = vector_outer_product(ui, conj(uj)) * conj(yij);
        block.h() = imag(power_flow_ij);
        block.n() = real(power_flow_ij);
        block.m() = -block.n();
        block.l() = block.h();
        return block;
    }

    void prepare_matrix_and_rhs_from_network_perspective(YBus<sym> const& y_bus, ComplexValueVector<sym> const& u,
                                                         IdxVector const& bus_entry, bool partial_rebuild) {
        IdxVector const& indptr = y_bus.row_indptr_lu();
        IdxVector const& indices = y_bus.col_indices_lu();
        IdxVector const& map_lu_y_bus = y_bus.map_lu_y_bus();
        ComplexTensorVector<sym> const& ydata = y_bus.admittance();

        for (Idx row = 0; row != this->n_bus_; ++row) {
            if (partial_rebuild && !bus_control_[row].q_limit.recalc_after_limit_violation) {
                // only re-calculate rows in jacobian for buses that have switched from PV to PQ due to Q-limit
                // violation
                continue;
            }
            // reset power injection
            del_x_pq_[row].p() = RealValue<sym>{0.0};
            del_x_pq_[row].q() = RealValue<sym>{0.0};
            // loop for column for incomplete jacobian and injection
            // k as data indices
            // j as column indices
            for (Idx k = indptr[row]; k != indptr[row + 1]; ++k) {
                // set to zero and skip if it is a fill-in
                Idx const k_y_bus = map_lu_y_bus[k];
                if (k_y_bus == -1) {
                    data_jac_[k] = PFJacBlock<sym>{};
                    continue;
                }
                Idx const j = indices[k];
                // incomplete jacobian
                data_jac_[k] = calculate_hnml(ydata[k_y_bus], u[row], u[j]);
                // accumulate negative power injection
                // -P = sum(-N)
                del_x_pq_[row].p() -= sum_row(data_jac_[k].n());
                // -Q = sum (-H)
                del_x_pq_[row].q() -= sum_row(data_jac_[k].h());
            }
            // correct diagonal part of jacobian
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
    }

    void build_jacobian_and_rhs(YBus<sym> const& y_bus, PowerFlowInput<sym> const& input,
                                ComplexValueVector<sym> const& u, bool partial_rebuild) {
        std::vector<LoadGenType> const& load_gen_type = this->load_gen_type_.get();
        IdxVector const& bus_entry = y_bus.lu_diag();

        auto const& load_gens_per_bus = this->load_gens_per_bus_.get();
        auto const& sources_per_bus = this->sources_per_bus_.get();

        prepare_matrix_and_rhs_from_network_perspective(y_bus, u, bus_entry, partial_rebuild);

        for (auto const& [bus_number, load_gens, sources] :
             enumerated_zip_sequence(load_gens_per_bus, sources_per_bus)) {

            if (partial_rebuild) {
                if (!bus_control_[bus_number].q_limit.recalc_after_limit_violation) {
                    // add only for buses that have switched from PV to PQ due to Q-limit violation
                    continue;
                }
                bus_control_[bus_number].q_limit.recalc_after_limit_violation = false;
            }
            Idx const diagonal_position = bus_entry[bus_number];
            add_loads(load_gens, bus_number, diagonal_position, input, load_gen_type);
            add_sources(sources, bus_number, diagonal_position, y_bus, input, u);
        }
    }

    void apply_pv_constraints(YBus<sym> const& y_bus) {
        // Apply PV constraints after calculation of Jacobian and potential recalculation of Jacobian
        // in case of PV/PQ switching due to Q-limit violation, but before solving the linear equation.

        IdxVector const& indptr = y_bus.row_indptr_lu();
        IdxVector const& indices = y_bus.col_indices_lu();

        // PV-bus voltage identity row:
        // For PV buses, the Q-equation is replaced by the algebraic constraint
        //     rPV = V - Vset = 0.
        // Since the state vector uses relative voltage increments deltaV_rel = deltaV / V,
        // the derivative drPV/dDeltaV_rel equals the current voltage magnitude V.
        // Therefore:
        //   - all off-diagonal voltage derivatives (L block) are zero,
        //   - all Theta-derivatives (M block) are zero,
        //   - only the diagonal L entry is set to V.
        // This removes the Q-equation and enforces the PV voltage constraint in the NR step.
        for (Idx row = 0; row != this->n_bus_; ++row) {
            if (bus_control_[row].type != BusType::pv) {
                continue;
            }
            for (Idx k = indptr[row]; k != indptr[row + 1]; ++k) {
                Idx const column = indices[k];
                data_jac_[k].m() = RealTensor<sym>{0.0};
                if (row == column) {
                    auto const& v = x_[column].v();
                    if constexpr (is_symmetric_v<sym>) {
                        data_jac_[k].l() = v;
                    } else {
                        data_jac_[k].l() = RealTensor<asymmetric_t>{{v[0], v[1], v[2]}};
                    }
                } else {
                    data_jac_[k].l() = RealTensor<sym>{0.0};
                }
            }
            // Replace the reactive-power mismatch with the voltage-magnitude constraint.
            del_x_pq_[row].q() = 0.0;
        }
    }

    /**
     * Calculate total specified Q from regulated load/generators
     *
     * For voltage-regulated generators, the "specified Q" is typically ~0 because the regulator
     * controls voltage and provides whatever Q is needed. For const_i/const_y types, we scale
     * by voltage to get the voltage-dependent specified value.
     */
    RealValue<sym> specified_q(Idx load_gen, Idx bus, PowerFlowInput<sym> const& input) {
        RealValue<sym> const q = imag(input.s_injection[load_gen]);
        switch (this->load_gen_type_.get()[load_gen]) {
            using enum LoadGenType;
        case const_pq:
            return q;
        case const_i:
            return q * x_[bus].v();
        case const_y:
            return q * x_[bus].v() * x_[bus].v();
        default:
            throw MissingCaseForEnumError("Reactive power limit calculation", this->load_gen_type_.get()[load_gen]);
        }
    }

    static LimitViolation violated_limit(RealValue<sym> const& q, double const& q_min, double const& q_max) {
        double q_total;
        if constexpr (is_symmetric_v<sym>) {
            q_total = q;
        } else {
            q_total = q(0) + q(1) + q(2);
        }

        if (!is_nan(q_max) && q_total > q_max + numerical_tolerance) {
            return LimitViolation::upper;
        }
        if (!is_nan(q_min) && q_total < q_min - numerical_tolerance) {
            return LimitViolation::lower;
        }
        return LimitViolation::none;
    }

    bool enforce_q_limits(PowerFlowInput<sym> const& input) {
        bool switched = false;
        auto const& regulators_per_load_gen = voltage_regulators_per_load_gen_.get();

        for (auto const& [bus, load_gens] : enumerated_zip_sequence(this->load_gens_per_bus_.get())) {
            if (bus_control_[bus].type != BusType::pv || !bus_control_[bus].q_limit.has_q_limits) {
                continue;
            }

            // === Calculate total specified Q from regulated load/generators ===
            // For voltage-regulated generators, the "specified Q" is typically ~0 because the regulator
            // controls voltage and provides whatever Q is needed. For const_i/const_y types, we scale
            // by voltage to get the voltage-dependent specified value.
            RealValue<sym> specified_regulating_q{};
            for (Idx const load_gen : load_gens) {
                if (input.load_gen_status[load_gen] == status_off) {
                    continue;
                }
                for (Idx const regulator : regulators_per_load_gen.get_element_range(load_gen)) {
                    if (input.voltage_regulator[regulator].status != status_off) {
                        specified_regulating_q += specified_q(load_gen, bus, input);
                    }
                }
            }

            // === Calculate q_required: the ACTUAL reactive power from regulated devices ===
            //
            // At this point in the algorithm, del_x_pq_[bus].q() contains the reactive power mismatch:
            //     del_x_pq_[bus].q() = Q_specified_all - Q_calc_network
            //
            // Where:
            //   - Q_specified_all = sum of specified Q from ALL load/gens at this bus (regulating + non-regulating)
            //                       This was accumulated in add_loads() by summing input.s_injection[load_gen]
            //   - Q_calc_network  = actual reactive power calculated from network equations (Y*U voltages)
            //                       This includes: branch flows, shunt elements, and ALL device injections
            //
            // Rearranging the mismatch equation:
            //     Q_calc_network = Q_specified_all - del_x_pq_[bus].q()
            //
            // The ACTUAL reactive power provided by regulated devices is:
            //     q_required = Q_calc_network - Q_from_nonregulated_devices
            //                = (Q_specified_all - del_x_pq_[bus].q()) - Q_specified_nonregulating
            //
            // Since Q_specified_all = Q_specified_regulating + Q_specified_nonregulating:
            //                = (Q_specified_regulating + Q_specified_nonregulating - del_x_pq_[bus].q())
            //                  - Q_specified_nonregulating
            //                = Q_specified_regulating - del_x_pq_[bus].q()
            //
            // Why we need to include specified_regulating_q:
            //   The specified Q values from regulating generators were already added to del_x_pq_[bus].q()
            //   in add_loads(). To extract the ACTUAL Q that regulators are providing (including their
            //   contribution to voltage control), we must "subtract back out" this mismatch term.
            //   This works because: actual = specified - mismatch.
            //
            // The result q_required represents the TOTAL reactive power that the regulated generators
            // are actually providing to the bus, which is what we must compare against the combined
            // bus Q-limits (bus_q_min, bus_q_max).
            //

            RealValue<sym> const q_required = specified_regulating_q - del_x_pq_[bus].q();
            // RealValue<sym> const q_required = -del_x_pq_[bus].q();

            auto const limit =
                violated_limit(q_required, bus_control_[bus].q_limit.bus_q_min, bus_control_[bus].q_limit.bus_q_max);
            if (limit != LimitViolation::none) {
                for (Idx const load_gen : load_gens) {
                    for (Idx const regulator : regulators_per_load_gen.get_element_range(load_gen)) {
                        if (input.load_gen_status[load_gen] != status_off &&
                            input.voltage_regulator[regulator].status != status_off) {
                            double const q_limit_scalar = (limit == LimitViolation::upper)
                                                              ? input.voltage_regulator[regulator].q_max
                                                              : input.voltage_regulator[regulator].q_min;

                            if constexpr (is_symmetric_v<sym>) {
                                clamped_regulators_per_load_gen_[load_gen] = q_limit_scalar;
                            } else {
                                // preserve interphase scaling from specified Q
                                RealValue<asymmetric_t> const base_q = imag(input.s_injection[load_gen]);
                                double const base_q_total = base_q(0) + base_q(1) + base_q(2);

                                if (std::abs(base_q_total) > numerical_tolerance) {
                                    double const scale = q_limit_scalar / base_q_total;
                                    clamped_regulators_per_load_gen_[load_gen] = RealValue<asymmetric_t>{
                                        base_q(0) * scale, base_q(1) * scale, base_q(2) * scale};
                                } else {
                                    // divide equally if input values are (close to) 0
                                    clamped_regulators_per_load_gen_[load_gen] = RealValue<asymmetric_t>{
                                        q_limit_scalar / 3.0, q_limit_scalar / 3.0, q_limit_scalar / 3.0};
                                }
                            }
                        }
                    }
                }
                // TODO(frie-soptim): don't forget to reset regulators if the bus is switched back to PV in the future
                // clamped_regulators_per_load_gen_[load_gen] = RealValue<sym>{nan};

                bus_control_[bus].q_limit.limit_violated = limit;
                bus_control_[bus].q_limit.recalc_after_limit_violation = true;
                bus_control_[bus].type = BusType::pq;
                switched = true;
            }
        }
        return switched;
    }

    static bool is_value_nan(RealValue<sym> const& value) {
        if constexpr (is_symmetric_v<sym>) {
            return is_nan(value);
        } else {
            // all three phases are set together, need to check only one phase for NaN
            return is_nan(value(0));
        }
    }

    void add_linear_initial_guess_loads(IdxRange const& load_gens, PFJacBlock<sym>& block,
                                        PowerFlowInput<sym> const& input) {
        for (Idx const load_number : load_gens) {
            auto const& s_input = input.s_injection[load_number];

            bool is_regulated = false;
            for (Idx const regulator : voltage_regulators_per_load_gen_.get().get_element_range(load_number)) {
                if (input.voltage_regulator[regulator].status != status_off &&
                    input.load_gen_status[load_number] != status_off) {
                    is_regulated = true;
                    break;
                }
            }

            // Ignore specified Q for regulated load_gens, otherwise it might lead to the calculation of
            // an incorrect initial voltage/angle. The voltage at the regulated bus will be set to the
            // reference voltage after the linear solver, but the "wrong" angle remains. And, importantly,
            // the "wrong" voltages at PQ buses remain as well. All this might then lead to a situation
            // that triggers a q-limit violation and a (potentially unnecessary) bus type switch.
            // TODO(frie-soptim): this might "fix itself" when the switch back to PV is implemented, but
            // the solver would then still need more iterations to converge.
            ComplexValue<sym> const& s =
                is_regulated ? ComplexValue<sym>{RealValue<sym>{real(s_input)}, RealValue<sym>{0.0}} : s_input;

            // Y_load = P - jQ -> G=P, B=-Q
            // System: [[G, -B], [B, G]] [Ur, Ui]^T = [Ir, Ii]^T
            // Diagonal mapping: coeff of Ur in Real Eq = G = real(Y_load) = P
            //                   coeff of Ui in Real Eq = -B = imag(Y_load) = Q
            //                   coeff of Ur in Imag Eq = B = -imag(Y_load) = -Q
            //                   coeff of Ui in Imag Eq = G = real(Y_load) = P
            ComplexValue<sym> const y_load = -conj(s);
            add_diag(block.real_imag(), -imag(y_load));
            add_diag(block.real_real(), real(y_load));
            add_diag(block.imag_imag(), real(y_load));
            add_diag(block.imag_real(), imag(y_load));
        }
    }

    void add_linear_initial_guess_sources(IdxRange const& sources, PFJacBlock<sym>& block, PolarPhasor<sym>& rhs,
                                          YBus<sym> const& y_bus, PowerFlowInput<sym> const& input) {
        for (Idx const source_number : sources) {
            ComplexTensor<sym> const y_source =
                y_bus.math_model_param().source_param[source_number].template y_ref<sym>();
            ComplexValue<sym> const u_source{input.source[source_number]};

            // coeff of Ui in Real Eq = -B
            block.real_imag() -= imag(y_source);
            // coeff of Ur in Real Eq = G
            block.real_real() += real(y_source);
            // coeff of Ui in Imag Eq = G
            block.imag_imag() += real(y_source);
            // coeff of Ur in Imag Eq = B
            block.imag_real() += imag(y_source);

            add_linear_rhs(rhs, y_source, u_source);
        }
    }

    void add_loads(IdxRange const& load_gens, Idx bus_number, Idx diagonal_position, PowerFlowInput<sym> const& input,
                   std::vector<LoadGenType> const& load_gen_type) {
        using enum LoadGenType;
        for (Idx const load_number : load_gens) {
            RealValue<sym> const& clamped_q = clamped_regulators_per_load_gen_[load_number];
            if (!is_value_nan(clamped_q)) {
                // load_gen hit a Q-limit, use the clamped value
                del_x_pq_[bus_number].p() += real(input.s_injection[load_number]);
                del_x_pq_[bus_number].q() += clamped_q;
                // TODO(frie-soptim): prevent regulation of const_i/const_y load_gens in validation
                continue;
            }

            LoadGenType const type = load_gen_type[load_number];
            // modify jacobian and del_pq based on type
            switch (type) {
            case const_pq:
                add_const_power_load(bus_number, load_number, input);
                break;
            case const_y:
                add_const_impedance_load(bus_number, load_number, diagonal_position, input);
                break;
            case const_i:
                add_const_current_load(bus_number, load_number, diagonal_position, input);
                break;
            default:
                throw MissingCaseForEnumError("Jacobian and deviation calculation", type);
            }
        }
    }

    void add_const_power_load(Idx bus_number, Idx load_number, PowerFlowInput<sym> const& input) {
        // PQ_sp = PQ_base
        del_x_pq_[bus_number].p() += real(input.s_injection[load_number]);
        del_x_pq_[bus_number].q() += imag(input.s_injection[load_number]);
        // -dPQ_sp/dV * V = 0
    }

    void add_const_impedance_load(Idx bus_number, Idx load_number, Idx diagonal_position,
                                  PowerFlowInput<sym> const& input) {
        // PQ_sp = PQ_base * V^2
        del_x_pq_[bus_number].p() += real(input.s_injection[load_number]) * x_[bus_number].v() * x_[bus_number].v();
        del_x_pq_[bus_number].q() += imag(input.s_injection[load_number]) * x_[bus_number].v() * x_[bus_number].v();
        // -dPQ_sp/dV * V = -PQ_base * 2 * V^2
        add_diag(data_jac_[diagonal_position].n(),
                 -real(input.s_injection[load_number]) * 2.0 * x_[bus_number].v() * x_[bus_number].v());
        add_diag(data_jac_[diagonal_position].l(),
                 -imag(input.s_injection[load_number]) * 2.0 * x_[bus_number].v() * x_[bus_number].v());
    }

    void add_const_current_load(Idx bus_number, Idx load_number, Idx diagonal_position,
                                PowerFlowInput<sym> const& input) {
        // PQ_sp = PQ_base * V
        del_x_pq_[bus_number].p() += real(input.s_injection[load_number]) * x_[bus_number].v();
        del_x_pq_[bus_number].q() += imag(input.s_injection[load_number]) * x_[bus_number].v();
        // -dPQ_sp/dV * V = -PQ_base * V
        add_diag(data_jac_[diagonal_position].n(), -real(input.s_injection[load_number]) * x_[bus_number].v());
        add_diag(data_jac_[diagonal_position].l(), -imag(input.s_injection[load_number]) * x_[bus_number].v());
    }

    void add_sources(IdxRange const& sources, Idx bus_number, Idx diagonal_position, YBus<sym> const& y_bus,
                     PowerFlowInput<sym> const& input, ComplexValueVector<sym> const& u) {
        for (Idx const source_number : sources) {
            ComplexTensor<sym> const y_ref = y_bus.math_model_param().source_param[source_number].template y_ref<sym>();
            ComplexValue<sym> const u_ref{input.source[source_number]};
            // calculate block, um = ui, us = uref
            PFJacBlock<sym> block_mm = calculate_hnml(y_ref, u[bus_number], u[bus_number]);
            PFJacBlock<sym> block_ms = calculate_hnml(-y_ref, u[bus_number], u_ref);
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
            del_x_pq_[bus_number].p() -= p_cal;
            del_x_pq_[bus_number].q() -= q_cal;
            // append to jacobian block
            // hnml -= -dPQ_cal/(dtheta,dV)
            // hnml += dPQ_cal/(dtheta,dV)
            data_jac_[diagonal_position].h() += block_mm.h();
            data_jac_[diagonal_position].n() += block_mm.n();
            data_jac_[diagonal_position].m() += block_mm.m();
            data_jac_[diagonal_position].l() += block_mm.l();
        }
    }

    // Helper to map complex admittance to real-domain block matrix using PFJacBlock getters.
    // This mapping ensures G aligns with the N/M sub-blocks as in the NR Jacobian.
    static void set_linear_block(PFJacBlock<sym>& block, ComplexTensor<sym> const& y) {
        RealTensor<sym> const g = real(y);
        RealTensor<sym> const b = imag(y);
        block.real_imag() = -b;
        block.real_real() = g;
        block.imag_imag() = g;
        block.imag_real() = b;
    }

    // Helper to map complex injection I = Y * U to real-domain RHS [Ir, Ii]^T using PolarPhasor getters.
    static void add_linear_rhs(PolarPhasor<sym>& rhs, ComplexTensor<sym> const& y, ComplexValue<sym> const& u) {
        ComplexValue<sym> const i_rhs = dot(y, u);
        rhs.i_real() += real(i_rhs);
        rhs.i_imag() += imag(i_rhs);
    }
};

} // namespace newton_raphson_pf

using newton_raphson_pf::NewtonRaphsonPFSolver;

} // namespace power_grid_model::math_solver
