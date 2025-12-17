// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model_cpp.hpp>

#include <iostream>

int main() {
    power_grid_model_cpp::Handle handle;
    if (handle.get() != nullptr) {
        std::cout << "Successfully created a Power Grid Model handle." << std::endl;
        return 0;
    } else {
        std::cout << "Failed to create a Power Grid Model handle." << std::endl;
        return 1;
    }
}
