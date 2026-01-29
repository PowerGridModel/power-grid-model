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
    // Return the path
    return tmpdir / "pgm_cli_test";
}

void clear_and_create_cli_tmp_path() {
    fs::path const cli_test_dir = get_cli_tmp_path();

    // Remove the dir if it exists (including contents)
    if (fs::exists(cli_test_dir)) {
        fs::remove_all(cli_test_dir);
    }

    // Create the empty directory
    if (!fs::create_directory(cli_test_dir)) {
        throw std::runtime_error("Failed to create cli_test temp directory");
    }
}

TEST_CASE("Test CLI version") {
    clear_and_create_cli_tmp_path();
    fs::path const version_file = get_cli_tmp_path() / "version.txt";
    std::string const command = std::string{cli_executable} + " --version" + " > " + version_file.string();
    int ret = std::system(command.c_str());
    REQUIRE(ret == 0);
    std::ifstream version_ifs(version_file);
    REQUIRE(version_ifs);
    std::string version_line;
    std::getline(version_ifs, version_line);
    CHECK(version_line == PGM_version());
}

} // namespace power_grid_model_cpp
