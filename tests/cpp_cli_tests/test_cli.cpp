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

namespace {
using namespace std::string_literals;
namespace fs = std::filesystem;

// input
auto const input_json = R"json({
  "version": "1.0",
  "type": "input",
  "is_batch": false,
  "attributes": {},
  "data": {
    "sym_load": [
      {"id": 2, "node": 0, "status": 1, "type": 0, "p_specified": 0, "q_specified": 0}
    ],
    "source": [
      {"id": 1, "node": 0, "status": 1, "u_ref": 1, "sk": 1e20}
    ],
    "node": [
      {"id": 0, "u_rated": 10e3}
    ]
  }
})json"s;

constexpr std::string_view cli_executable = POWER_GRID_MODEL_CLI_EXECUTABLE;

fs::path tmp_path() {
    // Get the system temp directory
    fs::path const tmpdir = fs::temp_directory_path();
    // Return the path
    return tmpdir / "pgm_cli_test";
}

fs::path input_path() { return tmp_path() / "input.json"; }

void clear_and_create_tmp_path() {
    fs::path const cli_test_dir = tmp_path();

    // Remove the dir if it exists (including contents)
    if (fs::exists(cli_test_dir)) {
        fs::remove_all(cli_test_dir);
    }

    // Create the empty directory
    if (!fs::create_directory(cli_test_dir)) {
        throw std::runtime_error("Failed to create cli_test temp directory");
    }
}

void save_input_data() {
    fs::path const input_file = input_path();
    std::ofstream ofs(input_file);
    ofs << input_json;
    ofs.close();
}

} // namespace

TEST_CASE("Test CLI version") {
    clear_and_create_tmp_path();
    fs::path const version_file = tmp_path() / "version.txt";
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
