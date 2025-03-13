// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// runtime dispatch for math solver
// this can separate math solver into a different translation unit

#pragma once

#include "y_bus.hpp"

#include "../calculation_parameters.hpp"
#include "../common/common.hpp"
#include "../common/exception.hpp"
#include "../common/three_phase_tensor.hpp"
#include "../common/timer.hpp"

#include <memory>

namespace power_grid_model {

namespace math_solver {

template <template <symmetry_tag> class MathSolverType> struct math_solver_tag {};

struct MathSolverDispatcher {
    template <symmetry_tag sym> struct MathSolverDispatcherConfig {
        template <template <symmetry_tag> class MathSolverType>
        constexpr MathSolverDispatcherConfig(math_solver_tag<MathSolverType>)
            : create{[](std::shared_ptr<MathModelTopology const> const& topo_ptr) {
                  return new MathSolverType<sym>{topo_ptr};
              }},
              destroy{[](void const* solver) { delete reinterpret_cast<MathSolverType<sym> const*>(solver); }},
              copy{[](void const* solver) {
                  return new MathSolverType<sym>{*reinterpret_cast<MathSolverType<sym> const*>(solver)};
              }},
              run_power_flow{[](void* solver, PowerFlowInput<sym> const& input, double err_tol, Idx max_iter,
                                CalculationInfo& calculation_info, CalculationMethod calculation_method,
                                YBus<sym> const& y_bus) {
                  return reinterpret_cast<MathSolverType<sym>*>(solver)->run_power_flow(
                      input, err_tol, max_iter, calculation_info, calculation_method, y_bus);
              }},
              run_state_estimation{[](void* solver, StateEstimationInput<sym> const& input, double err_tol,
                                      Idx max_iter, CalculationInfo& calculation_info,
                                      CalculationMethod calculation_method, YBus<sym> const& y_bus) {
                  return reinterpret_cast<MathSolverType<sym>*>(solver)->run_state_estimation(
                      input, err_tol, max_iter, calculation_info, calculation_method, y_bus);
              }},
              run_short_circuit{[](void* solver, ShortCircuitInput const& input, CalculationInfo& calculation_info,
                                   CalculationMethod calculation_method, YBus<sym> const& y_bus) {
                  return reinterpret_cast<MathSolverType<sym>*>(solver)->run_short_circuit(input, calculation_info,
                                                                                           calculation_method, y_bus);
              }},
              clear_solver{[](void* solver) { reinterpret_cast<MathSolverType<sym>*>(solver)->clear_solver(); }},
              parameters_changed{[](void* solver, bool changed) {
                  reinterpret_cast<MathSolverType<sym>*>(solver)->parameters_changed(changed);
              }} {}

        std::add_pointer_t<void*(std::shared_ptr<MathModelTopology const> const&)> create;
        std::add_pointer_t<void(void const*)> destroy;
        std::add_pointer_t<void*(void const*)> copy;
        std::add_pointer_t<SolverOutput<sym>(void*, PowerFlowInput<sym> const&, double, Idx, CalculationInfo&,
                                             CalculationMethod, YBus<sym> const&)>
            run_power_flow;
        std::add_pointer_t<SolverOutput<sym>(void*, StateEstimationInput<sym> const&, double, Idx, CalculationInfo&,
                                             CalculationMethod, YBus<sym> const&)>
            run_state_estimation;
        std::add_pointer_t<ShortCircuitSolverOutput<sym>(void*, ShortCircuitInput const&, CalculationInfo&,
                                                         CalculationMethod, YBus<sym> const&)>
            run_short_circuit;
        std::add_pointer_t<void(void*)> clear_solver;
        std::add_pointer_t<void(void*, bool)> parameters_changed;
    };

    template <template <symmetry_tag> class MathSolverType>
    constexpr MathSolverDispatcher(math_solver_tag<MathSolverType>)
        : sym_config{math_solver_tag<MathSolverType>{symmetric_t{}}},
          asym_config{math_solver_tag<MathSolverType>{asymmetric_t{}}} {}

    template <symmetry_tag sym> get_dispather_config() const {
        if constexpr (is_symmetric_v<sym>) {
            return sym_config;
        } else {
            return asym_config;
        }
    }
    MathSolverDispatcherConfig<symmetric_t> sym_config;
    MathSolverDispatcherConfig<asymmetric_t> asym_config;
};

template <symmetry_tag sym> class MathSolveProxy {
  public:
    explicit MathSolveProxy(MathSolverDispatcher const* dispatcher,
                            std::shared_ptr<MathModelTopology const> const& topo_ptr)
        : dispatcher_{dispatcher},
          solver_{dispatcher_->get_dispather_config<sym>().create(topo_ptr),
                  dispatcher_->get_dispather_config<sym>().destroy} {}
    MathSolveProxy(MathSolveProxy const& other) { *this = other; }
    MathSolveProxy& operator=(MathSolveProxy const& other) {
        if (this != &other) {
            solver_.reset();
            dispatcher_ = other.dispatcher_;
            solver_ = std::unique_ptr<void*, std::add_pointer_t<void(void const*)>>{
                dispatcher_->get_dispather_config<sym>().copy(other.get_ptr()),
                dispatcher_->get_dispather_config<sym>().destroy};
        }
        return *this;
    }
    MathSolveProxy(MathSolveProxy&& other) noexcept = default;
    MathSolveProxy& operator=(MathSolveProxy&& other) noexcept = default;
    ~MathSolveProxy() = default;

    SolverOutput<sym> run_power_flow(PowerFlowInput<sym> const& input, double err_tol, Idx max_iter,
                                     CalculationInfo& calculation_info, CalculationMethod calculation_method,
                                     YBus<sym> const& y_bus) {
        return dispatcher_->get_dispather_config<sym>().run_power_flow(get_ptr(), input, err_tol, max_iter,
                                                                       calculation_info, calculation_method, y_bus);
    }

    SolverOutput<sym> run_state_estimation(StateEstimationInput<sym> const& input, double err_tol, Idx max_iter,
                                           CalculationInfo& calculation_info, CalculationMethod calculation_method,
                                           YBus<sym> const& y_bus) {
        return dispatcher_->get_dispather_config<sym>().run_state_estimation(
            get_ptr(), input, err_tol, max_iter, calculation_info, calculation_method, y_bus);
    }

    ShortCircuitSolverOutput<sym> run_short_circuit(ShortCircuitInput const& input, CalculationInfo& calculation_info,
                                                    CalculationMethod calculation_method, YBus<sym> const& y_bus) {
        return dispatcher_->get_dispather_config<sym>().run_short_circuit(get_ptr(), input, calculation_info,
                                                                          calculation_method, y_bus);
    }

    void clear_solver() { dispatcher_->get_dispather_config<sym>().clear_solver(get_ptr()); }

    void parameters_changed(bool changed) {
        dispatcher_->get_dispather_config<sym>().parameters_changed(get_ptr(), changed);
    }

  private:
    MathSolverDispatcher const* dispatcher_{};
    std::unique_ptr<void*, std::add_pointer_t<void(void const*)>> solver_;

    void* get_ptr() { return solver_.get(); }
    void const* get_ptr() const { return solver_.get(); }
};

} // namespace math_solver

template <symmetry_tag sym> using MathSolveProxy = math_solver::MathSolveProxy<sym>;

template <template <symmetry_tag> class MathSolverType> using MathSolverDispatcher = math_solver::MathSolverDispatcher;

} // namespace power_grid_model
