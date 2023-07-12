// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include "power_grid_model_c.h"

#include <iostream>
#include <memory>

namespace {
// custom deleter
template <auto func>
struct DeleterFunctor {
    template <typename T>
    void operator()(T* arg) const {
        func(arg);
    }
};

using HandlePtr = std::unique_ptr<PGM_Handle, DeleterFunctor<&PGM_destroy_handle>>;
}  // namespace

auto main() -> int {
    // get handle
    HandlePtr const unique_handle{PGM_create_handle()};
    std::cout << (unique_handle ? "Handle created: API is available.\n" : "No handle could be created.\n");
    return unique_handle ? 0 : 1;
}
