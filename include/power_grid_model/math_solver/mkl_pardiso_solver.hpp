// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MATH_SOLVER_MKL_PARDISO_SOLVER_HPP
#define POWER_GRID_MODEL_MATH_SOLVER_MKL_PARDISO_SOLVER_HPP
// BSR solver using MKL PARDISO
// https://software.intel.com/en-us/mkl-developer-reference-c-pardiso

#ifdef POWER_GRID_MODEL_USE_MKL

#include "../exception.hpp"
#include "../power_grid_model.hpp"

#ifdef POWER_GRID_MODEL_USE_MKL_AT_RUNTIME

// add iostream header to report loading mkl status
#include <iostream>
#include <string>
// add header for envget
#include <stdlib.h>
// add std forward
#include <memory>

// add platform dependent header for loading dll or so
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#define POWER_GRID_MODEL_CALLING_CONVENTION __cdecl
namespace power_grid_model {
using LibHandle = HINSTANCE;
constexpr std::array mkl_rt_files{"mkl_rt.dll", "mkl_rt.1.dll", "mkl_rt.2.dll"};
auto constexpr load_mkl_single = [](char const* f) {
    return LoadLibrary(f);
};

auto constexpr load_func = GetProcAddress;
auto constexpr close_lib = FreeLibrary;
auto constexpr load_solver_env = []() {
    std::array<char, 256> solver_env{};
    size_t len_env{};
    getenv_s(&len_env, solver_env.data(), 255, "POWER_GRID_MODEL_SPARSE_SOLVER");
    return std::string{solver_env.data()};
};
}  // namespace power_grid_model
#endif  // _WIN32

#if defined(__linux__) || defined(__APPLE__)
#include <dlfcn.h>
#define POWER_GRID_MODEL_CALLING_CONVENTION
namespace power_grid_model {
using LibHandle = void*;
#ifdef __linux__
constexpr std::array mkl_rt_files{"libmkl_rt.so", "libmkl_rt.so.1", "libmkl_rt.so.2"};
#else   // __APPLE__
constexpr std::array mkl_rt_files{"libmkl_rt.dylib", "libmkl_rt.1.dylib", "libmkl_rt.2.dylib"};
#endif  // __linux__ or __APPLE__
auto constexpr load_mkl_single = [](char const* f) {
    return dlopen(f, RTLD_LAZY);
};
auto constexpr load_func = dlsym;
auto constexpr close_lib = dlclose;
auto constexpr load_solver_env = []() {
    char const* str = getenv("POWER_GRID_MODEL_SPARSE_SOLVER");
    if (str) {
        return std::string{str};
    }
    else {
        return std::string{};
    }
};
}  // namespace power_grid_model
#endif  // __linux__

// define handle
namespace power_grid_model {

// load mkl rt
auto constexpr load_mkl = []() -> LibHandle {
    for (char const* const mkl_rt : mkl_rt_files) {
        LibHandle const lib = load_mkl_single(mkl_rt);
        if (lib) {
            return lib;
        }
    }
    return nullptr;
};

// mkl integer pointer
using PardisoIntPtr = Idx*;
using PardisoIntConstPtr = Idx const*;

// define function pointer type for pardiso
using PardisoInitFuncPtr = void(POWER_GRID_MODEL_CALLING_CONVENTION*)(void*, PardisoIntConstPtr, PardisoIntPtr);
using PardisoFuncPtr = void(POWER_GRID_MODEL_CALLING_CONVENTION*)(void*, PardisoIntConstPtr, PardisoIntConstPtr,
                                                                  PardisoIntConstPtr, PardisoIntConstPtr,
                                                                  PardisoIntConstPtr, const void*, PardisoIntConstPtr,
                                                                  PardisoIntConstPtr, PardisoIntPtr, PardisoIntConstPtr,
                                                                  PardisoIntPtr, PardisoIntConstPtr, void*, void*,
                                                                  PardisoIntPtr);

// struct of pardiso handle for function pointers
struct PardisoHandle {
    // no copy
    PardisoHandle(PardisoHandle const&) = delete;
    PardisoHandle& operator=(PardisoHandle const&) = delete;
    // constructor to load pardiso either from link time or runtime
    PardisoHandle() {
#ifdef __aarch64__
        std::cout << "\nMKL is not available in Mac Arm64. Eigen solver is used.\n";
#else
        // load pardiso from runtime
        // check environment variable to see if
        // user prefer to used MKL
        std::string const user_solver = load_solver_env();
        // user specifies solver if the environment variable is defined
        bool const user_set_solver = !user_solver.empty();
        // user prefer to use mkl if the environment variable is defined
        // AND the variable is defined as "MKL"
        bool const user_prefer_use_mkl = user_set_solver && (user_solver == "MKL");
        // return immediately if user specifies not to use MKL
        if (user_set_solver && !user_prefer_use_mkl) {
            std::cout << "\nEigen solver is used as specified by the user.\n";
            return;
        }

        // load dll or so based on platform
        lib_handle_ = load_mkl();
        if (lib_handle_) {
            pardisoinit = (PardisoInitFuncPtr)load_func(lib_handle_, "pardisoinit");
            pardiso = (PardisoFuncPtr)load_func(lib_handle_, "pardiso");
        }

        // display message of loading
        has_pardiso = pardisoinit && pardiso;
        if (has_pardiso && user_set_solver) {
            std::cout << "\nMKL solver is used as specified by the user.\n";
        }
        else if (has_pardiso && !user_set_solver) {
            std::cout << "\nMKL solver is used as default.\n";
        }
        else if (!has_pardiso && user_set_solver) {
            std::cout << "\nWARNING: MKL runtime is not found. "
                         "Cannot use MKL solver as specified by the user. "
                         "Use Eigen solver instead!\n";
        }
        else {
            std::cout << "\nEigen solver is used because MKL runtime is not found.\n";
        }
#endif
    }

    // release library if use mkl at runtime
    ~PardisoHandle() {
        if (lib_handle_) {
            close_lib(lib_handle_);
        }
    }

    bool has_pardiso{false};
    PardisoInitFuncPtr pardisoinit{};
    PardisoFuncPtr pardiso{};

   private:
    LibHandle lib_handle_{};
};

// get pardiso handle from function, to avoid global initialization issue
inline PardisoHandle const& get_pardiso_handle() {
    static PardisoHandle const handle{};
    return handle;
}
// forward pardiso function
template <class... Args>
void pardiso(Args&&... args) {
    get_pardiso_handle().pardiso(std::forward<Args>(args)...);
}
template <class... Args>
void pardisoinit(Args&&... args) {
    get_pardiso_handle().pardisoinit(std::forward<Args>(args)...);
}

}  // namespace power_grid_model

#else  // !POWER_GRID_MODEL_USE_MKL_AT_RUNTIME
// include mkl header if link mkl at link time (not at runtime)
#include "mkl.h"
#endif  // POWER_GRID_MODEL_USE_MKL_AT_RUNTIME

// actual definition of solver
namespace power_grid_model {

template <class T>
class PARDISOSolver final {
   private:
    static_assert(std::is_same_v<T, double> || std::is_same_v<T, DoubleComplex>);
    struct BSRHandle {
        std::array<void*, 64> pt;   // pointer handle for pardiso
        std::array<Idx, 64> iparm;  // parameter for pardiso

        Idx matrix_size_in_block;             // size of matrix in block form
        Idx block_size;                       // size of block
        IdxVector perm;                       // permutation
        std::shared_ptr<IdxVector const> ia;  // indptr
        std::shared_ptr<IdxVector const> ja;  // column indices
    };

   public:
    PARDISOSolver(Idx matrix_size_in_block,                    // size of matrix in block form
                  Idx block_size,                              // size of block
                  std::shared_ptr<IdxVector const> const& ia,  // indptr
                  std::shared_ptr<IdxVector const> const& ja   // column indices
                  )
        : bsr_handle_{} {
        // assign values
        bsr_handle_.matrix_size_in_block = matrix_size_in_block;
        bsr_handle_.block_size = block_size;
        bsr_handle_.ia = ia;
        bsr_handle_.ja = ja;
        // initialize pardiso param
        pardisoinit(bsr_handle_.pt.data(), &mtype_, bsr_handle_.iparm.data());
        // bsr format if block size > 1, otherwise use 0 for csr format
        bsr_handle_.iparm[36] = bsr_handle_.block_size > 1 ? bsr_handle_.block_size : 0;
        bsr_handle_.iparm[34] = 1;  // zero based
        bsr_handle_.iparm[5] = 0;   // 0 for solution in x
        bsr_handle_.iparm[27] = 0;  // double precision
        bsr_handle_.iparm[4] = 1;   // use input permutation
        // permutation as 0, 1, 2, ..., N-1
        bsr_handle_.perm.resize(bsr_handle_.matrix_size_in_block);
        std::iota(bsr_handle_.perm.begin(), bsr_handle_.perm.end(), 0);
        // initialize solver
        Idx const error = initialize_pardiso();
        if (error != 0) {
            release_pardiso();
            throw SparseMatrixError{error};
        }
    }

    // copy construction
    PARDISOSolver(PARDISOSolver const& other) : bsr_handle_{other.bsr_handle_}, prefactorized_{false} {
        // new handle pt
        bsr_handle_.pt = {};
        // re initialize
        Idx const error = initialize_pardiso();
        if (error != 0) {
            release_pardiso();
            throw SparseMatrixError{error};
        }
    }
    // copy assignment
    PARDISOSolver& operator=(PARDISOSolver const& other) {
        // release old solver
        release_pardiso();
        // copy new handle
        bsr_handle_ = other.bsr_handle_;
        // new handle pt
        bsr_handle_.pt = {};
        // re initialize
        Idx const error = initialize_pardiso();
        if (error != 0) {
            release_pardiso();
            throw SparseMatrixError{error};
        }
        prefactorized_ = false;
        return *this;
    }

    // move construction
    PARDISOSolver(PARDISOSolver&& other) noexcept
        : bsr_handle_{std::move(other.bsr_handle_)}, prefactorized_{other.prefactorized_} {
        // set handle at other to zero
        other.bsr_handle_.pt = {};
    }
    // move assignment
    PARDISOSolver& operator=(PARDISOSolver&& other) noexcept {
        assert(this != &other);
        // release old solver
        release_pardiso();
        // move new handle
        bsr_handle_ = std::move(other.bsr_handle_);
        // set handle at other to zero
        other.bsr_handle_.pt = {};
        prefactorized_ = other.prefactorized_;
        return *this;
    }

    void invalidate_prefactorization() {
        prefactorized_ = false;
    }

    void solve(void const* data, void* b, void* x, bool use_prefactorization = false) {
        Idx error;

        if (use_prefactorization) {
            if (!prefactorized_) {
                prefactorize(data);
            }

            // solve prefactorized
            constexpr Idx phase_solve = 33;
            pardiso(bsr_handle_.pt.data(), &maxfct_, &mnum_, &mtype_, &phase_solve, &bsr_handle_.matrix_size_in_block,
                    data, bsr_handle_.ia->data(), bsr_handle_.ja->data(), bsr_handle_.perm.data(), &nrhs_,
                    bsr_handle_.iparm.data(), &msglvl_, b, x, &error);
        }
        else {
            // no prefactorization, factorize + solve
            constexpr Idx phase_factorization_solve = 23;
            // solve + refine
            pardiso(bsr_handle_.pt.data(), &maxfct_, &mnum_, &mtype_, &phase_factorization_solve,
                    &bsr_handle_.matrix_size_in_block, data, bsr_handle_.ia->data(), bsr_handle_.ja->data(),
                    bsr_handle_.perm.data(), &nrhs_, bsr_handle_.iparm.data(), &msglvl_, b, x, &error);
        }

        if (error != 0) {
            throw SparseMatrixError{error};
        }

        if (bsr_handle_.iparm[13] != 0) {
            throw SparseMatrixError{};
        }
    }

    void prefactorize(void const* data) {
        Idx error;
        constexpr Idx phase_factorization = 22;

        pardiso(bsr_handle_.pt.data(), &maxfct_, &mnum_, &mtype_, &phase_factorization,
                &bsr_handle_.matrix_size_in_block, data, bsr_handle_.ia->data(), bsr_handle_.ja->data(),
                bsr_handle_.perm.data(), &nrhs_, bsr_handle_.iparm.data(), &msglvl_, nullptr, nullptr, &error);

        if (error != 0) {
            throw SparseMatrixError{error};
        }

        prefactorized_ = true;
    }

    ~PARDISOSolver() noexcept {
        release_pardiso();
    }

   private:
    static constexpr Idx maxfct_ = 1;  // one factorization in total
    static constexpr Idx mnum_ = 1;    // the first factorization (only one)
    static constexpr Idx nrhs_ = 1;    // only one right hand side
    static constexpr Idx msglvl_ = 0;  // do not print message
    // type of matrix
    // 1 for real structurally  symmetric
    // 3 for complex structurally  symmetric
    static constexpr Idx mtype_ = std::is_same_v<T, double> ? 1 : 3;
    BSRHandle bsr_handle_;
    bool prefactorized_{false};

    Idx initialize_pardiso() noexcept {
        constexpr Idx phase_analysis = 11;
        Idx error;
        pardiso(bsr_handle_.pt.data(), &maxfct_, &mnum_, &mtype_, &phase_analysis, &bsr_handle_.matrix_size_in_block,
                nullptr, bsr_handle_.ia->data(), bsr_handle_.ja->data(), bsr_handle_.perm.data(), &nrhs_,
                bsr_handle_.iparm.data(), &msglvl_, nullptr, nullptr, &error);
        return error;
    }

    void release_pardiso() noexcept {
        // if pt contains all zero, nothing will happen
        constexpr Idx phase_release_memory = -1;
        Idx error;
        pardiso(bsr_handle_.pt.data(), &maxfct_, &mnum_, &mtype_, &phase_release_memory,
                &bsr_handle_.matrix_size_in_block, nullptr, nullptr, nullptr, nullptr, &nrhs_, bsr_handle_.iparm.data(),
                &msglvl_, nullptr, nullptr, &error);
    }
};

}  // namespace power_grid_model

#endif  // POWER_GRID_MODEL_USE_MKL

#endif
