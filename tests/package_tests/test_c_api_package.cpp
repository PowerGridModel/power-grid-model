// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "power_grid_model_c.h"
#include "power_grid_model_cpp.hpp"

#include <iostream>
#include <memory>

namespace {
// custom deleter
template <auto func> struct DeleterFunctor {
    template <typename T> void operator()(T* arg) const { func(arg); }
};

using HandlePtr = std::unique_ptr<PGM_Handle, DeleterFunctor<&PGM_destroy_handle>>;
} // namespace

auto main() -> int {
    // get handle from C-API
    HandlePtr const c_handle{PGM_create_handle()};
    std::cout << (c_handle ? "Handle created: C-API is available.\n" : "No handle could be created.\n");
    // get handle from C++ API
    power_grid_model_cpp::Handle const cpp_handle{};
    std::cout << (cpp_handle.get() ? "Handle created: C++ API is available.\n" : "No handle could be created.\n");
    int return_code = (c_handle != nullptr) && (cpp_handle.get() != nullptr) ? 0 : 1;
    return return_code;
}
