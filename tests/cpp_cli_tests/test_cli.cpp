// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_ENABLE_EXPERIMENTAL

#include <power_grid_model_c/dataset_definitions.h>
#include <power_grid_model_cpp.hpp>

#include <doctest/doctest.h>
#include <nlohmann/json.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numbers>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>

namespace power_grid_model_cpp {

namespace {
namespace fs = std::filesystem;
using std::numbers::sqrt3;

// namespace for hardcode json
namespace {

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

} // namespace

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
fs::path output_path(PGM_SerializationFormat format) {
    return format == PGM_json ? tmp_path() / "output.json" : tmp_path() / "output.pgmb";
}
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

std::vector<double> get_i_source_ref(bool is_batch) {
    if (!is_batch) {
        return {0.0};
    }
    // 3-D batch update
    double const u_rated = 10e3;
    std::vector<double> const u_ref{0.9, 1.0, 1.1};
    std::vector<double> const p_specified{1e6, 2e6, 3e6, 4e6};
    std::vector<double> const q_specified{0.1e6, 0.2e6, 0.3e6, 0.4e6, 0.5e6};
    Idx const size_u_ref = std::ssize(u_ref);
    Idx const size_p_specified = std::ssize(p_specified);
    Idx const size_q_specified = std::ssize(q_specified);
    Idx const total_batch_size = size_u_ref * size_p_specified * size_q_specified;

    // calculate source current manually
    std::vector<double> i_source_ref(total_batch_size);
    for (Idx i = 0; i < size_u_ref; ++i) {
        for (Idx j = 0; j < size_p_specified; ++j) {
            for (Idx k = 0; k < size_q_specified; ++k) {
                Idx const index = i * size_p_specified * size_q_specified + j * size_q_specified + k;
                double const s = std::abs(std::complex<double>{p_specified[j], q_specified[k]});
                i_source_ref[index] = s / (sqrt3 * u_rated * u_ref[i]);
            }
        }
    }
    return i_source_ref;
}

struct BufferRef {
    PGM_SymmetryType symmetric{PGM_symmetric};
    bool use_attribute_buffer{false};
    Buffer const* row_buffer;
    AttributeBuffer const* attribute_buffer;

    void check_i_source(std::vector<double> const& i_source_ref) const {
        Idx const batch_size = i_source_ref.size();
        for (Idx idx = 0; idx < batch_size; ++idx) {
            double const i_calculated = [this, idx]() {
                if (use_attribute_buffer) {
                    if (symmetric == PGM_symmetric) {
                        auto const& data_vector = attribute_buffer->get_data_vector<double>();
                        return data_vector.at(idx);
                    } else {
                        auto const& data_vector = attribute_buffer->get_data_vector<std::array<double, 3>>();
                        auto const& val_array = data_vector.at(idx);
                        CHECK(val_array[0] == doctest::Approx(val_array[1]));
                        CHECK(val_array[0] == doctest::Approx(val_array[2]));
                        return val_array[0];
                    }
                } else {
                    // use row buffer
                    if (symmetric == PGM_symmetric) {
                        double value{};
                        row_buffer->get_value(PGM_def_sym_output_source_i, &value, idx, 0);
                        return value;
                    } else {
                        std::array<double, 3> val_array{};
                        row_buffer->get_value(PGM_def_asym_output_source_i, val_array.data(), idx, 0);
                        CHECK(val_array[0] == doctest::Approx(val_array[1]));
                        CHECK(val_array[0] == doctest::Approx(val_array[2]));
                        return val_array[0];
                    }
                }
            }();
            CHECK(i_calculated == doctest::Approx(i_source_ref.at(idx)));
        }
    }
};

struct CLITestCase {
    bool is_batch{false};
    bool batch_p_msgpack{false};
    bool has_frequency{false};
    bool has_calculation_type{false};
    bool has_calculation_method{false};
    std::optional<PGM_SymmetryType> symmetry{};
    bool has_error_tolerance{false};
    bool has_max_iterations{false};
    bool has_threading{false};
    std::optional<PGM_SerializationFormat> output_serialization{};
    std::optional<Idx> output_json_indent{};
    std::optional<bool> output_compact_serialization{};
    bool component_filter{false};
    bool attribute_filter{false};

    PGM_SerializationFormat get_output_format() const {
        if (output_serialization.has_value()) {
            return output_serialization.value();
        } else if (is_batch && batch_p_msgpack) {
            return PGM_msgpack;
        } else {
            return PGM_json;
        }
    }
    bool has_output_filter() const { return component_filter || attribute_filter; }
    PGM_SymmetryType get_symmetry() const {
        if (symmetry.has_value()) {
            return symmetry.value();
        } else {
            return PGM_symmetric;
        }
    }
    bool output_columnar() const {
        if (output_compact_serialization.has_value()) {
            return output_compact_serialization.value();
        }
        return get_output_format() == PGM_msgpack;
    }

    std::string build_command() const {
        std::stringstream command;
        command << cli_executable;
        command << " -i " << input_path();
        if (is_batch) {
            command << " -b " << batch_u_ref_path();
            command << " -b " << (batch_p_msgpack ? batch_p_path_msgpack() : batch_p_path());
            command << " -b " << batch_q_path();
        }
        command << " -o " << output_path(get_output_format());
        if (has_frequency) {
            command << " --system-frequency 50.0";
        }
        if (has_calculation_type) {
            command << " --calculation-type " << "power_flow";
        }
        if (has_calculation_method) {
            command << " --calculation-method " << "newton_raphson";
        }
        if (symmetry.has_value()) {
            command << (symmetry.value() == PGM_symmetric ? " -s" : " -a");
        }
        if (has_error_tolerance) {
            command << " --error-tolerance 1e-8";
        }
        if (has_max_iterations) {
            command << " --max-iterations 20";
        }
        if (has_threading) {
            command << " --threading -1";
        }
        if (output_serialization.has_value()) {
            if (output_serialization.value() == PGM_msgpack) {
                command << " --msgpack";
            } else {
                command << " --json";
            }
        }
        if (output_json_indent.has_value()) {
            command << " --indent " << output_json_indent.value();
        }
        if (output_compact_serialization.has_value()) {
            if (output_compact_serialization.value()) {
                command << " --compact";
            } else {
                command << " --no-compact";
            }
        }
        if (component_filter) {
            command << " --oc source";
        }
        if (attribute_filter) {
            command << " --oa source.i";
        }
        command << " > " << stdout_path();
        return command.str();
    }

    BufferRef get_source_buffer(OwningDataset const& dataset) const {
        auto const& owning_memory = dataset.storage;
        auto const& info = dataset.dataset.get_info();
        Idx const source_idx = info.component_idx("source");
        auto const* const row_buffer = [this, &owning_memory, &info, source_idx]() {
            if (has_output_filter()) {
                REQUIRE(info.n_components() == 1);
                REQUIRE(source_idx == 0);
            }
            return &owning_memory.buffers[source_idx];
        }();
        auto const* const attribute_buffer = [this, &owning_memory, row_buffer,
                                              source_idx]() -> AttributeBuffer const* {
            if (output_columnar()) {
                REQUIRE(row_buffer->get() == nullptr);
                if (attribute_filter) {
                    REQUIRE(owning_memory.attribute_buffers[source_idx].size() == 1);
                    return &owning_memory.attribute_buffers[source_idx][0];
                } else {
                    for (auto const& attr_buf : owning_memory.attribute_buffers[source_idx]) {
                        if (MetaData::attribute_name(attr_buf.get_attribute()) == "i") {
                            return &attr_buf;
                        }
                    }
                    DOCTEST_FAIL("Attribute 'i' buffer not found");
                }
            }
            // when no filter, buffer should not be nullptr, and return nullptr for attribute buffer
            REQUIRE(row_buffer->get() != nullptr);
            return nullptr;
        }();
        return BufferRef{.symmetric = get_symmetry(),
                         .use_attribute_buffer = output_columnar(),
                         .row_buffer = row_buffer,
                         .attribute_buffer = attribute_buffer};
    }

    void check_results() const {
        fs::path const out_path = output_path(get_output_format());
        OwningDataset const output_owning_dataset = load_dataset(out_path, get_output_format(), true);
        auto const i_source_ref = get_i_source_ref(is_batch);
        Idx const batch_size = output_owning_dataset.dataset.get_info().batch_size();
        REQUIRE(batch_size == std::ssize(i_source_ref));
        REQUIRE(is_batch == output_owning_dataset.dataset.get_info().is_batch());
        auto const buffer_ref = get_source_buffer(output_owning_dataset);
        buffer_ref.check_i_source(i_source_ref);
    }

    void run_command_and_check() const {
        prepare_data();
        std::string const command = build_command();
        INFO("CLI command: ", command);
        int ret = std::system(command.c_str());
        std::string const stdout_content = read_stdout_content();
        INFO("CLI stdout content: ", stdout_content);
        REQUIRE(ret == 0);
        check_results();
    }
};

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

TEST_CASE("Test run CLI") {
    std::vector<CLITestCase> const test_cases = {
        // basic non-batch, symmetric, json
        CLITestCase{},
        // basic batch, symmetric, json
        CLITestCase{.is_batch = true},
        // batch, asymmetric, msgpack
        CLITestCase{.is_batch = true, .symmetry = PGM_asymmetric, .output_serialization = PGM_msgpack},
        // batch, symmetric, json, with all options set
        CLITestCase{.is_batch = true,
                    .batch_p_msgpack = true,
                    .has_frequency = true,
                    .has_calculation_type = true,
                    .has_calculation_method = true,
                    .symmetry = PGM_symmetric,
                    .has_error_tolerance = true,
                    .has_max_iterations = true,
                    .has_threading = true,
                    .output_serialization = PGM_json,
                    .output_json_indent = 4,
                    .output_compact_serialization = true,
                    .component_filter = true,
                    .attribute_filter = true},
        // batch, asymmetric, msgpack, with component and attribute filter
        CLITestCase{.is_batch = true,
                    .symmetry = PGM_asymmetric,
                    .output_serialization = PGM_msgpack,
                    .component_filter = true,
                    .attribute_filter = true},
    };
    for (auto const& test_case : test_cases) {
        SUBCASE(test_case.build_command().c_str()) { test_case.run_command_and_check(); }
    }
}

} // namespace power_grid_model_cpp
