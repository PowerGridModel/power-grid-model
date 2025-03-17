// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// runtime dispatch for math solver
// this can separate math solver into a different translation unit

#pragma once

#include "../calculation_parameters.hpp"
#include "../common/common.hpp"
#include "../common/exception.hpp"
#include "../common/three_phase_tensor.hpp"
#include "../common/timer.hpp"

#include <memory>

namespace power_grid_model {

namespace math_solver {

// forward declare YBus
template <symmetry_tag sym> class YBus;

// abstract base class
template <symmetry_tag sym> class MathSolverBase {
  public:
    virtual ~MathSolverBase() = default;

    virtual MathSolverBase<sym>* clone() const = 0;

    virtual SolverOutput<sym> run_power_flow(PowerFlowInput<sym> const& input, double err_tol, Idx max_iter,
                                             CalculationInfo& calculation_info, CalculationMethod calculation_method,
                                             YBus<sym> const& y_bus) = 0;
    virtual SolverOutput<sym> run_state_estimation(StateEstimationInput<sym> const& input, double err_tol, Idx max_iter,
                                                   CalculationInfo& calculation_info,
                                                   CalculationMethod calculation_method, YBus<sym> const& y_bus) = 0;
    virtual ShortCircuitSolverOutput<sym> run_short_circuit(ShortCircuitInput const& input,
                                                            CalculationInfo& calculation_info,
                                                            CalculationMethod calculation_method,
                                                            YBus<sym> const& y_bus) = 0;
    virtual void clear_solver() = 0;
    virtual void parameters_changed(bool changed) = 0;

  protected:
    MathSolverBase() = default;
    MathSolverBase(MathSolverBase const&) = default;
    MathSolverBase& operator=(MathSolverBase const&) = default;
    MathSolverBase(MathSolverBase&&) noexcept = default;
    MathSolverBase& operator=(MathSolverBase&&) noexcept = default;
};

// tag of math solver concrete types
template <template <symmetry_tag> class MathSolverType> struct math_solver_tag {};

class MathSolverDispatcher {
  public:
    template <symmetry_tag sym> struct Config {
        template <template <class> class MathSolverType>
        constexpr Config(math_solver_tag<MathSolverType> /* unused */)
            : create{[](std::shared_ptr<MathModelTopology const> const& topo_ptr) -> MathSolverBase<sym>* {
                  return new MathSolverType<sym>{topo_ptr};
              }} {}

        std::add_pointer_t<MathSolverBase<sym>*(std::shared_ptr<MathModelTopology const> const&)> create;
    };

    template <template <class> class MathSolverType>
    constexpr MathSolverDispatcher(math_solver_tag<MathSolverType> tag) : sym_config_{tag}, asym_config_{tag} {}

    template <symmetry_tag sym> Config<sym> const& get_config() const {
        if constexpr (is_symmetric_v<sym>) {
            return sym_config_;
        } else {
            return asym_config_;
        }
    }

  private:
    Config<symmetric_t> sym_config_;
    Config<asymmetric_t> asym_config_;
};

template <symmetry_tag sym> class MathSolverProxy {
  public:
    explicit MathSolverProxy(MathSolverDispatcher const* dispatcher,
                             std::shared_ptr<MathModelTopology const> const& topo_ptr)
        : dispatcher_{dispatcher}, solver_{dispatcher_->get_config<sym>().create(topo_ptr)} {}
    MathSolverProxy(MathSolverProxy const& other) : dispatcher_{other.dispatcher_}, solver_{other.get().clone()} {}
    MathSolverProxy& operator=(MathSolverProxy const& other) {
        if (this != &other) {
            solver_.reset();
            dispatcher_ = other.dispatcher_;
            solver_.reset(other.get().clone());
        }
        return *this;
    }
    MathSolverProxy(MathSolverProxy&& other) noexcept = default;
    MathSolverProxy& operator=(MathSolverProxy&& other) noexcept = default;
    ~MathSolverProxy() = default;

    MathSolverBase<sym>& get() { return *solver_; }
    MathSolverBase<sym> const& get() const { return *solver_; }

  private:
    MathSolverDispatcher const* dispatcher_{};
    std::unique_ptr<MathSolverBase<sym>> solver_;
};

} // namespace math_solver

template <symmetry_tag sym> using MathSolverProxy = math_solver::MathSolverProxy<sym>;

using MathSolverDispatcher = math_solver::MathSolverDispatcher;

} // namespace power_grid_model
