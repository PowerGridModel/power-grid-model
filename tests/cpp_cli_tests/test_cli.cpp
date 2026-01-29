// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_ENABLE_EXPERIMENTAL

#include <cstdlib>
#include <doctest/doctest.h>
#include <filesystem>
#include <fstream>
#include <power_grid_model_cpp.hpp>
#include <string>
#include <string_view>

namespace power_grid_model_cpp {

namespace fs = std::filesystem;

constexpr std::string_view cli_executable = POWER_GRID_MODEL_CLI_EXECUTABLE;

fs::path get_cli_tmp_path() {
    // Get the system temp directory
    fs::path const tmpdir = fs::temp_directory_path();
    fs::path const cli_test_dir = tmpdir / "pgm_cli_test";

    // Remove the dir if it exists (including contents)
    if (fs::exists(cli_test_dir)) {
        fs::remove_all(cli_test_dir);
    }

    // Create the empty directory
    if (!fs::create_directory(cli_test_dir)) {
        throw std::runtime_error("Failed to create cli_test temp directory");
    }

    // Return the path
    return cli_test_dir;
}

TEST_CASE("Test CLI version") {
    std::string const command = std::string{cli_executable} + " --version" + " > version.txt";
    int ret = std::system(command.c_str());
    REQUIRE(ret == 0);
    fs::path const version_file = get_cli_tmp_path() / "version.txt";
    std::ifstream version_ifs("version.txt");
    REQUIRE(version_ifs);
    std::string version_line;
    std::getline(version_ifs, version_line);
    CHECK(version_line == PGM_version());
}

} // namespace power_grid_model_cpp
