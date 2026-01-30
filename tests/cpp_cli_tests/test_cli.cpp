// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_ENABLE_EXPERIMENTAL

#include <cstdlib>
#include <doctest/doctest.h>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <power_grid_model_cpp.hpp>
#include <string>
#include <string_view>

namespace power_grid_model_cpp {

namespace {
namespace fs = std::filesystem;

constexpr std::string_view input_json = R"json({
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
})json";

constexpr std::string_view batch_u_ref_json = R"json({
  "version": "1.0",
  "type": "update",
  "is_batch": true,
  "attributes": {},
  "data": [
    {
      "source": [
        {"u_ref": 0.9}
      ]
    },
    {
      "source": [
        {"u_ref": 1.0}
      ]
    },
    {
      "source": [
        {"u_ref": 1.1}
      ]
    }
  ]
})json";

constexpr std::string_view batch_p_json = R"json({
  "version": "1.0",
  "type": "update",
  "is_batch": true,
  "attributes": { "sym_load": ["p_specified"] },
  "data": [
    {
      "sym_load": [
        [1e6]
      ]
    },
    {
      "sym_load": [
        [2e6]
      ]
    },
    {
      "sym_load": [
        [3e6]
      ]
    },
    {
      "sym_load": [
        [4e6]
      ]
    }
  ]
})json";

constexpr std::string_view batch_q_json = R"json({
  "version": "1.0",
  "type": "update",
  "is_batch": true,
  "attributes": {},
  "data": [
    {
      "sym_load": [
        {"q_specified": 0.1e6}
      ]
    },
    {
      "sym_load": [
        {"q_specified": 0.2e6}
      ]
    },
    {
      "sym_load": [
        {"q_specified": 0.3e6}
      ]
    },
    {
      "sym_load": [
        {"q_specified": 0.4e6}
      ]
    },
    {
      "sym_load": [
        {"q_specified": 0.5e6}
      ]
    }
  ]
})json";

constexpr std::string_view cli_executable = POWER_GRID_MODEL_CLI_EXECUTABLE;

fs::path tmp_path() {
    // Get the system temp directory
    fs::path const tmpdir = fs::temp_directory_path();
    // Return the path
    return tmpdir / "pgm_cli_test";
}

fs::path input_path() { return tmp_path() / "input.json"; }
fs::path batch_u_ref_path() { return tmp_path() / "batch_u_ref.json"; }
fs::path batch_p_path() { return tmp_path() / "batch_p.json"; }
fs::path batch_q_path() { return tmp_path() / "batch_q.json"; }
fs::path batch_p_path_msgpack() { return tmp_path() / "batch_p.pgmb"; }

fs::path stdout_path() { return tmp_path() / "stdout.txt"; }

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

void save_data(std::string_view json_data, fs::path const& path, PGM_SerializationFormat format) {
    if (std::ofstream ofs(path, std::ios::binary); ofs) {
        if (format == PGM_json) {
            ofs << json_data;
        } else {
            nlohmann::json const j = nlohmann::json::parse(json_data);
            std::string msgpack_buffer;
            nlohmann::json::to_msgpack(j, msgpack_buffer);
            ofs << msgpack_buffer;
        }
    } else {
        throw std::runtime_error("Failed to open file for writing: " + path.string());
    }
    // try to read the file, discard results
    load_dataset(path, format, true);
}

void prepare_data() {
    clear_and_create_tmp_path();
    save_data(input_json, input_path(), PGM_json);
    save_data(batch_u_ref_json, batch_u_ref_path(), PGM_json);
    save_data(batch_p_json, batch_p_path(), PGM_json);
    save_data(batch_p_json, batch_p_path_msgpack(), PGM_msgpack);
    save_data(batch_q_json, batch_q_path(), PGM_json);
}

std::string read_stdout_content() {
    fs::path const file_name = stdout_path();
    std::ifstream version_ifs(file_name);
    REQUIRE(version_ifs);

    // Get file size
    version_ifs.seekg(0, std::ios::end);
    std::streamsize size = version_ifs.tellg();
    version_ifs.seekg(0, std::ios::beg);

    // Read the entire file
    std::string file_content(size, '\0');
    version_ifs.read(file_content.data(), size);
    return file_content;
}

} // namespace

TEST_CASE("Test CLI version") {
    prepare_data();
    std::string const command = std::string{cli_executable} + " --version" + " > " + stdout_path().string();
    int ret = std::system(command.c_str());
    std::string const file_content = read_stdout_content();
    INFO("CLI stdout content: ", file_content);
    REQUIRE(ret == 0);
    // Extract the first line
    std::string const first_line = file_content.substr(0, file_content.find('\n'));
    CHECK(first_line == PGM_version());
}

} // namespace power_grid_model_cpp
