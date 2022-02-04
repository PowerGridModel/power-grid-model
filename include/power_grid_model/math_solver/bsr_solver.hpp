// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MATH_SOLVER_BSR_SOLVER_HPP
#define POWER_GRID_MODEL_MATH_SOLVER_BSR_SOLVER_HPP
// BSR solver adaptor based on which solver is selected

// if use mkl at runtime, also set use mkl
#ifdef POWER_GRID_MODEL_USE_MKL_AT_RUNTIME
#ifndef POWER_GRID_MODEL_USE_MKL
#define POWER_GRID_MODEL_USE_MKL
#endif
#endif

#include "../exception.hpp"
#include "../power_grid_model.hpp"
// include both solver
#include <variant>

#include "eigen_superlu_solver.hpp"
#include "mkl_pardiso_solver.hpp"

namespace power_grid_model {

#ifdef POWER_GRID_MODEL_USE_MKL_AT_RUNTIME

// wrapper solver
template <class T>
class BSRSolver {
    using SolverType = std::variant<PARDISOSolver<T>, EigenSuperLUSolver<T>>;

   public:
    template <class... Args,
              // disable overload for solver itself
              // allow proper overload for copy/move construction
              class = std::enable_if_t<(!std::is_same_v<std::decay_t<Args>, BSRSolver<T>> && ...)>>
    explicit BSRSolver(Args&&... args)
        :  // template<class... Args> Args&&... args perfect forwarding
          solver_{get_pardiso_handle().has_pardiso ?
                                                   // initialize with pardiso
                      SolverType{std::in_place_type<PARDISOSolver<T>>, std::forward<Args>(args)...}
                                                   :
                                                   // initialize with eigen
                      SolverType{std::in_place_type<EigenSuperLUSolver<T>>, std::forward<Args>(args)...}} {
    }

    template <class... Args>
    void solve(Args&&... args) {
        // template<class... Args> Args&&... args perfect forwarding
        // call solver based on use mkl
        if (get_pardiso_handle().has_pardiso) {
            std::get<PARDISOSolver<T>>(solver_).solve(std::forward<Args>(args)...);
        }
        else {
            std::get<EigenSuperLUSolver<T>>(solver_).solve(std::forward<Args>(args)...);
        }
    }

    template <class... Args>
    void prefactorize(Args&&... args) {
        // template<class... Args> Args&&... args perfect forwarding
        if (get_pardiso_handle().has_pardiso) {
            std::get<PARDISOSolver<T>>(solver_).prefactorize(std::forward<Args>(args)...);
        }
        else {
            std::get<EigenSuperLUSolver<T>>(solver_).prefactorize(std::forward<Args>(args)...);
        }
    }

    void invalidate_prefactorization() {
        if (get_pardiso_handle().has_pardiso) {
            std::get<PARDISOSolver<T>>(solver_).invalidate_prefactorization();
        }
        else {
            std::get<EigenSuperLUSolver<T>>(solver_).invalidate_prefactorization();
        }
    }

   private:
    SolverType solver_;
};

#else

// select solver statically at compile time
#ifdef POWER_GRID_MODEL_USE_MKL
// use mkl solver
template <class T>
using BSRSolver = PARDISOSolver<T>;
#else   // !POWER_GRID_MODEL_USE_MKL
// use eigen solver
template <class T>
using BSRSolver = EigenSuperLUSolver<T>;
#endif  // POWER_GRID_MODEL_USE_MKL

#endif  // POWER_GRID_MODEL_USE_MKL_AT_RUNTIME

}  // namespace power_grid_model

#endif
