// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "power_grid_model_cli_backend/cli_main.h"

#include "cli_functions.hpp"

#include <power_grid_model_cpp/handle.hpp>

#include <exception>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

using namespace power_grid_model_cpp;

namespace {

void write_output(PGM_CLIMessageCallback callback, void* user_data, std::ostream& stream,
                  std::string const& message) { // NOSONAR(S5008,S5205)
    if (callback != nullptr) {
        callback(message.c_str(), user_data);
    } else {
        stream << message;
    }
}

} // namespace

int PGM_cli_main(int argc, char** argv, PGM_CLIMessageCallback cout_callback, PGM_CLIMessageCallback cerr_callback,
                 void* user_data) noexcept { // NOSONAR(S5008,S5205)
    static std::mutex cli_mutex;
    std::scoped_lock<std::mutex> const lock{cli_mutex};

    ClIOptions cli_options;
    if (auto const parse_result = parse_cli_options(argc, argv, cli_options); parse_result) {
        write_output(cout_callback, user_data, std::cout, parse_result.stdout_message);
        write_output(cerr_callback, user_data, std::cerr, parse_result.stderr_message);
        return parse_result.exit_code;
    }

    if (cli_options.verbose) {
        std::ostringstream output_stream;
        output_stream << cli_options << '\n';
        write_output(cout_callback, user_data, std::cout, output_stream.str());
    }

    try {
        pgm_calculation(cli_options);
    } catch (PowerGridError const& e) {
        write_output(cerr_callback, user_data, std::cerr, std::string{"PowerGridError: "} + e.what() + '\n');
        return static_cast<int>(e.error_code());
    } catch (std::exception const& e) {
        write_output(cerr_callback, user_data, std::cerr, std::string{"Exception: "} + e.what() + '\n');
        return 1;
    } catch (...) {
        write_output(cerr_callback, user_data, std::cerr, "Unknown exception caught.\n");
        return 1;
    }

    return 0;
}
