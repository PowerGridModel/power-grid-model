// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// NOTE: the cpp headers are experimental
// you should define POWER_GRID_MODEL_CPP_HEADERS_EXPERIMENTAL to enable them

#pragma once

#ifndef POWER_GRID_MODEL_CPP_BASICS_HPP
#define POWER_GRID_MODEL_CPP_BASICS_HPP

#ifndef POWER_GRID_MODEL_CPP_HEADERS_EXPERIMENTAL
// #error "POWER_GRID_MODEL_CPP_HEADERS_EXPERIMENTAL must be defined to include this header"
#endif

#ifdef PGM_DLL_EXPORTS
#error "You should not define PGM_DLL_EXPORTS when using the C++ headers"
#endif

#include "../power_grid_model_c/basics.h"
#include <memory>

namespace power_grid_model {

using Idx = PGM_Idx;
using ID = PGM_ID;

// custom deleter
template <auto func> struct DeleterFunctor {
    template <typename T> void operator()(T* arg) const { func(arg); }
};

// unique pointer definition
template <typename T, auto func> using UniquePtr = std::unique_ptr<T, DeleterFunctor<func>>;

} // namespace power_grid_model

#endif // POWER_GRID_MODEL_CPP_BASICS_HPP