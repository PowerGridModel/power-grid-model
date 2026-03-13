// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "cli_functions.hpp"

#include <power_grid_model_cpp/handle.hpp>

#include <exception>
#include <iostream>

using namespace power_grid_model_cpp;

int main(int argc, char** argv) {
    ClIOptions cli_options;
    if (auto const parse_result = parse_cli_options(argc, argv, cli_options); parse_result) {
        return parse_result.exit_code;
    }

    if (cli_options.verbose) {
        std::cout << cli_options << '\n';
    }

    try {
        pgm_calculation(cli_options);
    } catch (PowerGridError const& e) {
        std::cerr << "PowerGridError: " << e.what() << '\n';
        return static_cast<int>(e.error_code());
    } catch (std::exception const& e) {
        std::cerr << "Exception: " << e.what() << '\n';
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception caught." << '\n';
        return 1;
    }

    return 0;
}
