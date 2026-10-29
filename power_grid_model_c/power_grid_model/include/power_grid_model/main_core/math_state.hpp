// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../calculation_parameters.hpp"
#include "../common/common.hpp"
#include "../common/counting_iterator.hpp"
#include "../math_solver/math_solver_dispatch.hpp"
#include "../math_solver/y_bus.hpp"

#include <cassert>
#include <concepts>
#include <functional>
#include <type_traits>
#include <vector>

namespace power_grid_model::main_core {

class MathState {
  public:
    MathState() = default;
    MathState(MathState const& other)
        : y_bus_vec_sym_{other.y_bus_vec_sym_},
          y_bus_vec_asym_{other.y_bus_vec_asym_},
          math_solvers_sym_{other.math_solvers_sym_},
          math_solvers_asym_{other.math_solvers_asym_} {
        // Copy-constructing a Y-bus drops its (instance-local) parameter-change callbacks; re-link each copied Y-bus to
        // its own copied solver(s) so that the copy contains exactly and only callbacks to the new solvers.
        link_solvers(y_bus_vec_sym_, math_solvers_sym_);
        link_solvers(y_bus_vec_asym_, math_solvers_asym_);
    }
    MathState(MathState&& other) noexcept
        : // Move-construct each vector; the moved-from vectors are left in a valid but unspecified state
          y_bus_vec_sym_{std::move(other.y_bus_vec_sym_)},
          y_bus_vec_asym_{std::move(other.y_bus_vec_asym_)},
          math_solvers_sym_{std::move(other.math_solvers_sym_)},
          math_solvers_asym_{std::move(other.math_solvers_asym_)} {}
    MathState& operator=(MathState const& other) {
        if (this != &other) {
            // copy-and-move: the copy constructor performs the re-linking; the move preserves element addresses
            *this = MathState{other};
        }
        return *this;
    }
    MathState& operator=(MathState&& other) noexcept {
        if (this != &other) {
            y_bus_vec_sym_ = std::move(other.y_bus_vec_sym_);
            y_bus_vec_asym_ = std::move(other.y_bus_vec_asym_);
            math_solvers_sym_ = std::move(other.math_solvers_sym_);
            math_solvers_asym_ = std::move(other.math_solvers_asym_);
        }
        return *this;
    }
    ~MathState() { clear(); }

    // register a parameter-change callback from each Y-bus to its corresponding solver
    template <symmetry_tag sym>
    static void link_solvers(std::vector<YBus<sym>>& y_bus_vec, std::vector<MathSolverProxy<sym>>& solvers) {
        assert(y_bus_vec.size() == solvers.size());
        for (Idx idx = 0; idx != std::ssize(y_bus_vec); ++idx) {
            y_bus_vec[idx].add_parameters_changed_callback([solver = std::ref(solvers[idx])](bool changed) {
                auto& solver_proxy = solver.get();
                auto& underlying_solver = solver_proxy.get();
                underlying_solver.parameters_changed(changed);
            });
        }
    }

    void clear() {
        math_solvers_sym_.clear();
        math_solvers_asym_.clear();
        y_bus_vec_sym_.clear();
        y_bus_vec_asym_.clear();
    }

    template <symmetry_tag sym> auto& get_solvers(this auto& math_state) {
        if constexpr (is_symmetric_v<sym>) {
            return math_state.math_solvers_sym_;
        } else {
            return math_state.math_solvers_asym_;
        }
    }

    template <symmetry_tag sym> auto& get_y_bus(this auto& math_state) {
        if constexpr (is_symmetric_v<sym>) {
            return math_state.y_bus_vec_sym_;
        } else {
            return math_state.y_bus_vec_asym_;
        }
    }

  private:
    std::vector<YBus<symmetric_t>> y_bus_vec_sym_;
    std::vector<YBus<asymmetric_t>> y_bus_vec_asym_;
    std::vector<MathSolverProxy<symmetric_t>> math_solvers_sym_;
    std::vector<MathSolverProxy<asymmetric_t>> math_solvers_asym_;
};

inline void clear(MathState& math_state) { math_state.clear(); }

template <symmetry_tag sym, typename State>
    requires std::same_as<std::remove_const_t<State>, MathState>
inline auto& get_solvers(State& math_state) {
    return math_state.template get_solvers<sym>();
}

template <symmetry_tag sym, typename State>
    requires std::same_as<std::remove_const_t<State>, MathState>
inline auto& get_y_bus(State& math_state) {
    return math_state.template get_y_bus<sym>();
}

template <symmetry_tag sym>
inline void update_y_bus(MathState& math_state, std::vector<MathModelParam<sym>> math_model_params) {
    auto& y_bus_vec = get_y_bus<sym>(math_state);

    assert(y_bus_vec.size() == math_model_params.size());

    for (Idx const i : IdxRange{std::ssize(y_bus_vec)}) {
        y_bus_vec[i].update_admittance(std::move(math_model_params[i]));
    }
}

template <symmetry_tag sym>
inline void update_y_bus(MathState& math_state,
                         std::vector<MathModelParamIncrement<sym>> const& math_model_param_increments) {
    auto& y_bus_vec = get_y_bus<sym>(math_state);

    assert(y_bus_vec.size() == math_model_param_increments.size());

    for (Idx const i : IdxRange{std::ssize(y_bus_vec)}) {
        y_bus_vec[i].update_admittance_increment(math_model_param_increments[i]);
    }
}

} // namespace power_grid_model::main_core
