// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "cli_options.hpp"

#include <power_grid_model_cpp.hpp>

#include <iostream>

using namespace power_grid_model_cpp;

int main(int argc, char** argv) {
    ClIOptions cli_options;
    if (auto parse_result = parse_cli_options(argc, argv, cli_options); parse_result) {
        return parse_result.exit_code;
    }

    std::cout << cli_options << std::endl;

    return 0;
}
