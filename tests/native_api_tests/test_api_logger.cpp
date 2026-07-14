// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "load_dataset.hpp"

#include <power_grid_model_cpp/buffer.hpp>
#include <power_grid_model_cpp/dataset.hpp>
#include <power_grid_model_cpp/handle.hpp>
#include <power_grid_model_cpp/options.hpp>

#include <power_grid_model_c/basics.h>
#include <power_grid_model_c/dataset_definitions.h>
#include <power_grid_model_c/handle.h>
#include <power_grid_model_c/logger.h>
#include <power_grid_model_c/model.h>

#include <doctest/doctest.h>

#include <string>
#include <string_view>

namespace {
using namespace std::string_literals;
using namespace std::string_view_literals;
using power_grid_model_cpp::Buffer;
using power_grid_model_cpp::DatasetConst;
using power_grid_model_cpp::DatasetMutable;
using power_grid_model_cpp_test::load_dataset;

// Minimal 2-node network JSON.
auto const input_json = R"json({
  "version": "1.0",
  "type": "input",
  "is_batch": false,
  "attributes": {},
  "data": {
    "node": [
      {"id": 0, "u_rated": 100},
      {"id": 4, "u_rated": 100}
    ],
    "source": [
      {"id": 1, "node": 0, "status": 1, "u_ref": 1, "sk": 1000, "rx_ratio": 0}
    ],
    "sym_load": [
      {"id": 2, "node": 0, "status": 1, "type": 2, "p_specified": 0, "q_specified": 500}
    ],
    "line": [
      {"id": 5, "from_node": 0, "to_node": 4, "from_status": 0, "to_status": 1},
      {"id": 6, "from_node": 4, "to_node": 0, "from_status": 0, "to_status": 0}
    ]
  }
})json"s;

// Minimal RAII wrappers so tests don't leak on CHECK failures.

struct HandleGuard {
    PGM_Handle* h = PGM_create_handle();
    ~HandleGuard() { PGM_destroy_handle(h); }
};

struct LoggerGuard {
    PGM_Logger* l;
    explicit LoggerGuard(PGM_Handle* handle, PGM_Idx type) : l{PGM_create_logger(handle, type)} {}
    ~LoggerGuard() {
        if (l) {
            PGM_destroy_logger(l);
        }
    }
};

// Run a minimal single-scenario power flow using the provided handle.
// All C API calls use that handle so loggers registered to it will receive output.
// Returns the model; caller owns it and must call PGM_destroy_model.
PGM_PowerGridModel* run_calculate(PGM_Handle* handle) {
    auto const owning_input = load_dataset(input_json);

    // Convert DatasetMutable (input) -> PGM_ConstDataset for PGM_create_model.
    DatasetConst const const_input{owning_input.dataset};

    PGM_PowerGridModel* model = PGM_create_model(handle, 50.0, const_input.get());
    if (PGM_error_code(handle) != PGM_no_error || model == nullptr) {
        return nullptr;
    }

    // Minimal sym_output: 2 nodes.
    Buffer node_output{PGM_def_sym_output_node, 2};
    node_output.set_nan();
    DatasetMutable output_ds{"sym_output", false, 1};
    output_ds.add_buffer("node", 2, 2, nullptr, node_output);

    power_grid_model_cpp::Options opt{};
    PGM_set_calculation_type(handle, opt.get(), PGM_power_flow);
    PGM_set_symmetric(handle, opt.get(), 1);

    PGM_calculate(handle, model, opt.get(), output_ds.get(), nullptr);
    return model;
}
} // namespace

TEST_CASE("Logger - create and destroy do-nothing logger") {
    HandleGuard g;
    LoggerGuard lg{g.h, PGM_do_nothing_logger};
    CHECK(lg.l != nullptr);
    CHECK(PGM_error_code(g.h) == PGM_no_error);
}

TEST_CASE("Logger - create and destroy text logger") {
    HandleGuard g;
    LoggerGuard lg{g.h, PGM_text_logger};
    CHECK(lg.l != nullptr);
    CHECK(PGM_error_code(g.h) == PGM_no_error);
}

TEST_CASE("Logger - create and destroy benchmark logger") {
    HandleGuard g;
    LoggerGuard lg{g.h, PGM_benchmark_logger};
    CHECK(lg.l != nullptr);
    CHECK(PGM_error_code(g.h) == PGM_no_error);
}

TEST_CASE("Logger - invalid type returns error") {
    HandleGuard g;
    PGM_Logger* bad = PGM_create_logger(g.h, 999);
    CHECK(bad == nullptr);
    CHECK(PGM_error_code(g.h) == PGM_regular_error);
}

TEST_CASE("Logger - get_output on do-nothing logger is empty") {
    HandleGuard g;
    LoggerGuard lg{g.h, PGM_do_nothing_logger};
    char const* out = PGM_logger_get_output(g.h, lg.l);
    CHECK(PGM_error_code(g.h) == PGM_no_error);
    CHECK(out != nullptr);
    CHECK(std::string_view{out}.empty());
}

TEST_CASE("Logger - register / unregister preserves handle error state") {
    HandleGuard g;
    LoggerGuard lg{g.h, PGM_text_logger};

    PGM_register_logger(g.h, lg.l);
    CHECK(PGM_error_code(g.h) == PGM_no_error);

    PGM_unregister_logger(g.h, lg.l);
    CHECK(PGM_error_code(g.h) == PGM_no_error);
}

TEST_CASE("Logger - unregister non-registered logger is no-op") {
    HandleGuard g;
    LoggerGuard lg{g.h, PGM_text_logger};

    PGM_unregister_logger(g.h, lg.l); // never registered
    CHECK(PGM_error_code(g.h) == PGM_no_error);
}

TEST_CASE("Logger - text logger captures output after calculate") {
    HandleGuard g;
    LoggerGuard lg{g.h, PGM_text_logger};

    PGM_register_logger(g.h, lg.l);

    PGM_PowerGridModel* model = run_calculate(g.h);
    CHECK(PGM_error_code(g.h) == PGM_no_error);

    char const* out = PGM_logger_get_output(g.h, lg.l);
    CHECK(PGM_error_code(g.h) == PGM_no_error);
    // Text logger should have written something; cannot assert exact content but must be non-empty.
    CHECK(!std::string_view{out}.empty());

    PGM_unregister_logger(g.h, lg.l);
    PGM_destroy_model(model);
}

TEST_CASE("Logger - benchmark logger captures output after calculate") {
    HandleGuard g;
    LoggerGuard lg{g.h, PGM_benchmark_logger};

    PGM_register_logger(g.h, lg.l);

    PGM_PowerGridModel* model = run_calculate(g.h);
    CHECK(PGM_error_code(g.h) == PGM_no_error);

    char const* out = PGM_logger_get_output(g.h, lg.l);
    CHECK(PGM_error_code(g.h) == PGM_no_error);
    // Benchmark output must be non-empty and contain TAB-separated fields.
    std::string_view sv{out};
    CHECK(!sv.empty());
    CHECK(sv.find('\t') != std::string_view::npos);

    PGM_unregister_logger(g.h, lg.l);
    PGM_destroy_model(model);
}

TEST_CASE("Logger - text logger clear wipes output") {
    HandleGuard g;
    LoggerGuard lg{g.h, PGM_text_logger};

    PGM_register_logger(g.h, lg.l);
    PGM_PowerGridModel* model = run_calculate(g.h);
    CHECK(PGM_error_code(g.h) == PGM_no_error);

    // Clear and verify empty
    PGM_logger_clear(g.h, lg.l);
    CHECK(PGM_error_code(g.h) == PGM_no_error);
    char const* out = PGM_logger_get_output(g.h, lg.l);
    CHECK(std::string_view{out}.empty());

    PGM_unregister_logger(g.h, lg.l);
    PGM_destroy_model(model);
}

TEST_CASE("Logger - do-nothing logger clear is no-op") {
    HandleGuard g;
    LoggerGuard lg{g.h, PGM_do_nothing_logger};

    PGM_logger_clear(g.h, lg.l);
    CHECK(PGM_error_code(g.h) == PGM_no_error);
    char const* out = PGM_logger_get_output(g.h, lg.l);
    CHECK(std::string_view{out}.empty());
}

TEST_CASE("Logger - loggers persist across clear_error on handle") {
    HandleGuard g;
    LoggerGuard lg{g.h, PGM_text_logger};

    PGM_register_logger(g.h, lg.l);

    // Simulate an error clearing (happens at start of each call_with_catch)
    PGM_clear_error(g.h);

    // Logger must still be registered: run a calculation and check output is captured
    PGM_PowerGridModel* model = run_calculate(g.h);
    CHECK(PGM_error_code(g.h) == PGM_no_error);

    char const* out = PGM_logger_get_output(g.h, lg.l);
    CHECK(!std::string_view{out}.empty());

    PGM_unregister_logger(g.h, lg.l);
    PGM_destroy_model(model);
}

TEST_CASE("Logger - text and benchmark loggers registered simultaneously") {
    HandleGuard g;
    LoggerGuard text_lg{g.h, PGM_text_logger};
    LoggerGuard bench_lg{g.h, PGM_benchmark_logger};

    PGM_register_logger(g.h, text_lg.l);
    PGM_register_logger(g.h, bench_lg.l);

    PGM_PowerGridModel* model = run_calculate(g.h);
    CHECK(PGM_error_code(g.h) == PGM_no_error);

    char const* text_out = PGM_logger_get_output(g.h, text_lg.l);
    char const* bench_out = PGM_logger_get_output(g.h, bench_lg.l);
    CHECK(!std::string_view{text_out}.empty());
    CHECK(!std::string_view{bench_out}.empty());

    PGM_unregister_logger(g.h, text_lg.l);
    PGM_unregister_logger(g.h, bench_lg.l);
    PGM_destroy_model(model);
}
