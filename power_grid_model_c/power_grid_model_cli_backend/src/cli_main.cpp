// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "power_grid_model_cli_backend/cli_main.h"

#include "cli_functions.hpp"

#include <power_grid_model_cpp/errors.hpp>

#include <exception>
#include <format>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

using namespace power_grid_model_cpp;

namespace {

void write_output(PGM_CLIMessageCallback callback, // NOSONAR(S5205)
                  void* user_data,                 // NOSONAR(S5008)
                  std::ostream& stream, std::string const& message) {
    if (callback != nullptr) {
        callback(message.c_str(), user_data);
    } else {
        stream << message;
    }
}

} // namespace

int PGM_cli_main(int argc, char** argv,                                                      // CLI argument
                 PGM_CLIMessageCallback cout_callback, PGM_CLIMessageCallback cerr_callback, // NOSONAR(S5205)
                 void* user_data) noexcept {                                                 // NOSONAR(S5008)
    // we add mutex here to prevent the function called by multiple threads simultaneously
    // This is not relevant if the function is called by the CLI executable
    // but if the user link to the CLI backend shared library and calls this function as
    // part of their own multi-threaded application, this mutex will prevent concurrent execution.
    static std::mutex cli_mutex;
    std::scoped_lock<std::mutex> const lock{cli_mutex};

    CLIOptions cli_options;
    if (auto const parse_result = parse_cli_options(argc, argv, cli_options); parse_result) {
        write_output(cout_callback, user_data, std::cout, parse_result.stdout_message);
        write_output(cerr_callback, user_data, std::cerr, parse_result.stderr_message);
        return parse_result.exit_code;
    }

    if (cli_options.verbose) {
        std::ostringstream options_stream;
        options_stream << cli_options;
        write_output(cout_callback, user_data, std::cout, options_stream.str());
    }

    try {
        pgm_calculation(cli_options);
    } catch (PowerGridError const& e) {
        write_output(cerr_callback, user_data, std::cerr, std::format("PowerGridError: {}\n", e.what()));
        return static_cast<int>(e.error_code());
    } catch (std::exception const& e) {
        write_output(cerr_callback, user_data, std::cerr, std::format("Exception: {}\n", e.what()));
        return 1;
    } catch (...) {
        write_output(cerr_callback, user_data, std::cerr, "Unknown exception caught.\n");
        return 1;
    }

    return 0;
}
