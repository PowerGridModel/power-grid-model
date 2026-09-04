// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "load_dataset.hpp"

#include <power_grid_model_cpp/buffer.hpp>
#include <power_grid_model_cpp/dataset.hpp>
#include <power_grid_model_cpp/handle.hpp>
#include <power_grid_model_cpp/logger.hpp>
#include <power_grid_model_cpp/model.hpp>
#include <power_grid_model_cpp/options.hpp>

#include <power_grid_model_c/basics.h>
#include <power_grid_model_c/dataset_definitions.h>
#include <power_grid_model_c/handle.h>
#include <power_grid_model_c/logger.h>
#include <power_grid_model_c/model.h>

#include <doctest/doctest.h>

#include <algorithm>
#include <string>
#include <utility>

namespace {
using namespace std::string_literals;
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
    ~HandleGuard() { destroy(); }

    void destroy() {
        PGM_destroy_handle(h);
        h = nullptr;
    }
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
// Helper: call PGM_logger_get_output and collect the result into a std::string.
auto get_output(PGM_Handle* h, PGM_Logger* l) {
    std::string result;
    PGM_logger_get_output(
        h, l,
        [](char const* data, PGM_Idx size, void* ctx) noexcept {
            auto& output = *static_cast<std::string*>(ctx);
            if (size == 0) {
                output.clear();
                return;
            }
            output.assign(data, static_cast<std::size_t>(size));
        },
        &result);
    return result;
}

// Count newline-terminated lines, used to compare logger output volume without relying on
// exact text equality (individual lines carry independent millisecond timestamps).
std::ptrdiff_t count_lines(std::string const& text) { return std::ranges::count(text, '\n'); }

// Run a minimal single-scenario power flow using the C++ Model API.
// Loggers registered on `model` (via Model::add_logger) will receive output from this call.
void run_calculate(power_grid_model_cpp::Model& model) {
    Buffer node_output{PGM_def_sym_output_node, 2};
    node_output.set_nan();
    DatasetMutable output_ds{"sym_output", false, 1};
    output_ds.add_buffer("node", 2, 2, nullptr, node_output);

    power_grid_model_cpp::Options opt{};
    opt.set_calculation_type(PGM_power_flow);
    opt.set_symmetric(1);

    model.calculate(opt, output_ds);
}

power_grid_model_cpp::Model make_cpp_model() {
    auto const owning_input = load_dataset(input_json);
    DatasetConst const const_input{owning_input.dataset};
    return power_grid_model_cpp::Model{50.0, const_input};
}
} // namespace

TEST_CASE("Logger - invalid type returns error") {
    HandleGuard g;
    PGM_Logger* bad = PGM_create_logger(g.h, 999);
    CHECK(bad == nullptr);
    CHECK(PGM_error_code(g.h) == PGM_regular_error);
}

TEST_CASE("Logger - do-nothing logger produces no output and clear is a no-op") {
    HandleGuard g;
    LoggerGuard lg{g.h, PGM_do_nothing_logger};

    int callback_calls = 0;
    PGM_logger_get_output(
        g.h, lg.l,
        [](char const* /*data*/, PGM_Idx size, void* ctx) noexcept {
            ++(*static_cast<int*>(ctx));
            CHECK(size == 0);
        },
        &callback_calls);
    CHECK(PGM_error_code(g.h) == PGM_no_error);
    // The callback must be invoked exactly once, even though the output is empty.
    CHECK(callback_calls == 1);

    PGM_logger_clear(g.h, lg.l);
    CHECK(PGM_error_code(g.h) == PGM_no_error);
    CHECK(get_output(g.h, lg.l).empty());
}

TEST_CASE("Logger - get_output with null callback returns a regular error") {
    HandleGuard g;
    LoggerGuard lg{g.h, PGM_text_logger};

    PGM_logger_get_output(g.h, lg.l, nullptr, nullptr);
    CHECK(PGM_error_code(g.h) == PGM_regular_error);
}

TEST_CASE("Logger - unregister stops subsequent output") {
    HandleGuard g;
    LoggerGuard lg{g.h, PGM_text_logger};

    PGM_register_logger(g.h, lg.l);
    PGM_PowerGridModel* first_model = run_calculate(g.h);
    CHECK(PGM_error_code(g.h) == PGM_no_error);
    CHECK(!get_output(g.h, lg.l).empty());

    PGM_logger_clear(g.h, lg.l);
    PGM_unregister_logger(g.h, lg.l);
    CHECK(PGM_error_code(g.h) == PGM_no_error);
    PGM_destroy_model(first_model);

    PGM_PowerGridModel* second_model = run_calculate(g.h);
    CHECK(PGM_error_code(g.h) == PGM_no_error);
    CHECK(get_output(g.h, lg.l).empty());
    PGM_destroy_model(second_model);
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

    std::string out = get_output(g.h, lg.l);
    CHECK(PGM_error_code(g.h) == PGM_no_error);
    // Text logger should have written something; cannot assert exact content but must be non-empty.
    CHECK(!out.empty());

    PGM_unregister_logger(g.h, lg.l);
    PGM_destroy_model(model);
}

TEST_CASE("Logger - benchmark logger captures output after calculate") {
    HandleGuard g;
    LoggerGuard lg{g.h, PGM_benchmark_logger};

    PGM_register_logger(g.h, lg.l);

    PGM_PowerGridModel* model = run_calculate(g.h);
    CHECK(PGM_error_code(g.h) == PGM_no_error);

    std::string out = get_output(g.h, lg.l);
    CHECK(PGM_error_code(g.h) == PGM_no_error);
    // Benchmark output must be non-empty and contain TAB-separated fields.
    CHECK(!out.empty());
    CHECK(out.find('\t') != std::string::npos);

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
    std::string out = get_output(g.h, lg.l);
    CHECK(out.empty());

    PGM_unregister_logger(g.h, lg.l);
    PGM_destroy_model(model);
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

    std::string out = get_output(g.h, lg.l);
    CHECK(!out.empty());

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

    std::string text_out = get_output(g.h, text_lg.l);
    std::string bench_out = get_output(g.h, bench_lg.l);
    CHECK(!text_out.empty());
    CHECK(!bench_out.empty());

    PGM_unregister_logger(g.h, text_lg.l);
    PGM_unregister_logger(g.h, bench_lg.l);
    PGM_destroy_model(model);
}

TEST_CASE("Logger - registering the same logger twice is idempotent") {
    HandleGuard g;
    LoggerGuard lg{g.h, PGM_text_logger};

    PGM_register_logger(g.h, lg.l);
    PGM_register_logger(g.h, lg.l); // second registration — must be a silent no-op
    CHECK(PGM_error_code(g.h) == PGM_no_error);

    PGM_PowerGridModel* model = run_calculate(g.h);
    CHECK(PGM_error_code(g.h) == PGM_no_error);

    // Output must not be doubled — compare line count with a single registration
    // (exact text equality is not usable: each line carries an independent millisecond timestamp).
    std::string out_double = get_output(g.h, lg.l);
    PGM_unregister_logger(g.h, lg.l);
    PGM_destroy_model(model);

    // Fresh run with a single registration for reference
    HandleGuard g2;
    LoggerGuard lg2{g2.h, PGM_text_logger};
    PGM_register_logger(g2.h, lg2.l);
    PGM_PowerGridModel* model2 = run_calculate(g2.h);
    std::string out_single = get_output(g2.h, lg2.l);
    PGM_unregister_logger(g2.h, lg2.l);
    PGM_destroy_model(model2);

    CHECK(count_lines(out_double) == count_lines(out_single));
}

TEST_CASE("Logger - PGM_unregister_all_loggers removes all loggers") {
    HandleGuard g;
    LoggerGuard text_lg{g.h, PGM_text_logger};
    LoggerGuard bench_lg{g.h, PGM_benchmark_logger};

    PGM_register_logger(g.h, text_lg.l);
    PGM_register_logger(g.h, bench_lg.l);

    PGM_unregister_all_loggers(g.h);
    CHECK(PGM_error_code(g.h) == PGM_no_error);

    // After unregistering all, a calculation should produce no output in either logger
    PGM_PowerGridModel* model = run_calculate(g.h);
    CHECK(PGM_error_code(g.h) == PGM_no_error);

    std::string text_out = get_output(g.h, text_lg.l);
    std::string bench_out = get_output(g.h, bench_lg.l);
    CHECK(text_out.empty());
    CHECK(bench_out.empty());

    PGM_destroy_model(model);
    // loggers are already unregistered; safe to destroy them via LoggerGuard
}

TEST_CASE("Logger - destroying a registered logger does not crash a subsequent calculation") {
    HandleGuard g;
    PGM_Logger* logger = PGM_create_logger(g.h, PGM_text_logger);
    PGM_register_logger(g.h, logger);

    // Destroy the wrapper while still registered: the underlying implementation must stay
    // alive (shared with the handle's composite) so the calculation below does not crash.
    PGM_destroy_logger(logger);
    CHECK(PGM_error_code(g.h) == PGM_no_error);

    PGM_PowerGridModel* model = run_calculate(g.h);
    CHECK(PGM_error_code(g.h) == PGM_no_error);

    // There is no PGM_Logger* left to read output from individually; clean up via the handle.
    PGM_unregister_all_loggers(g.h);
    CHECK(PGM_error_code(g.h) == PGM_no_error);

    PGM_destroy_model(model);
}

TEST_CASE("Logger - destroying the handle while a logger is registered does not crash") {
    PGM_Handle* h = PGM_create_handle();
    LoggerGuard lg{h, PGM_text_logger};

    PGM_register_logger(h, lg.l);
    PGM_PowerGridModel* model = run_calculate(h);
    CHECK(PGM_error_code(h) == PGM_no_error);
    PGM_destroy_model(model);

    // Destroy the handle (and its composite_logger registrations) while the logger wrapper
    // is still alive. Must not crash; the logger wrapper itself remains usable afterwards.
    PGM_destroy_handle(h);

    HandleGuard g2;
    std::string out = get_output(g2.h, lg.l);
    CHECK(!out.empty());
}

TEST_CASE("Logger - model logs through the handle passed to PGM_calculate, not the creation handle") {
    HandleGuard creation_handle;
    HandleGuard calc_handle;
    LoggerGuard creation_lg{creation_handle.h, PGM_text_logger};
    LoggerGuard calc_lg{calc_handle.h, PGM_text_logger};

    // The model is bound to creation_handle's composite logger at PGM_create_model time.
    PGM_register_logger(creation_handle.h, creation_lg.l);
    auto const owning_input = load_dataset(input_json);
    DatasetConst const const_input{owning_input.dataset};
    PGM_PowerGridModel* model = PGM_create_model(creation_handle.h, 50.0, const_input.get());
    REQUIRE(model != nullptr);

    // Destroy the creation handle before calculating. PGM_calculate must reseat the model's
    // logger without dereferencing the now-stale reference to creation_handle's composite.
    creation_handle.destroy();

    // Register a different logger on a different handle and calculate using that handle.
    PGM_register_logger(calc_handle.h, calc_lg.l);

    Buffer node_output{PGM_def_sym_output_node, 2};
    node_output.set_nan();
    DatasetMutable output_ds{"sym_output", false, 1};
    output_ds.add_buffer("node", 2, 2, nullptr, node_output);

    power_grid_model_cpp::Options opt{};
    PGM_set_calculation_type(calc_handle.h, opt.get(), PGM_power_flow);
    PGM_set_symmetric(calc_handle.h, opt.get(), 1);

    PGM_calculate(calc_handle.h, model, opt.get(), output_ds.get(), nullptr);
    CHECK(PGM_error_code(calc_handle.h) == PGM_no_error);

    // PGM_calculate reseats the model's logger to the handle passed to that call, so output
    // is captured by calc_handle's logger, not the destroyed creation handle's logger.
    CHECK(!get_output(calc_handle.h, calc_lg.l).empty());
    CHECK(get_output(calc_handle.h, creation_lg.l).empty());

    PGM_unregister_logger(calc_handle.h, calc_lg.l);
    PGM_destroy_model(model);
}

// --- C++ API (power_grid_model_cpp::Logger / Model) ---

TEST_CASE("CPP Logger - value construction / empty output before calculation") {
    power_grid_model_cpp::Logger logger{PGM_text_logger};
    CHECK(logger.get_output().empty());
}

TEST_CASE("CPP Logger - add_logger / calculate / get_output round trip") {
    auto model = make_cpp_model();
    power_grid_model_cpp::Logger logger{PGM_text_logger};

    model.add_logger(logger);
    run_calculate(model);

    CHECK(!logger.get_output().empty());
}

TEST_CASE("CPP Logger - clear() empties output and keeps registration") {
    auto model = make_cpp_model();
    power_grid_model_cpp::Logger logger{PGM_text_logger};

    model.add_logger(logger);
    run_calculate(model);
    CHECK(!logger.get_output().empty());

    logger.clear();
    CHECK(logger.get_output().empty());

    // registration must still be active
    run_calculate(model);
    CHECK(!logger.get_output().empty());
}

TEST_CASE("CPP Logger - remove_logger stops output from that logger only") {
    auto model = make_cpp_model();
    power_grid_model_cpp::Logger logger_a{PGM_text_logger};
    power_grid_model_cpp::Logger logger_b{PGM_text_logger};

    model.add_logger(logger_a);
    model.add_logger(logger_b);
    model.remove_logger(logger_a);

    run_calculate(model);

    CHECK(logger_a.get_output().empty());
    CHECK(!logger_b.get_output().empty());
}

TEST_CASE("CPP Logger - remove_all_loggers detaches everything") {
    auto model = make_cpp_model();
    power_grid_model_cpp::Logger text_logger{PGM_text_logger};
    power_grid_model_cpp::Logger bench_logger{PGM_benchmark_logger};

    model.add_logger(text_logger);
    model.add_logger(bench_logger);
    model.remove_all_loggers();

    run_calculate(model);

    CHECK(text_logger.get_output().empty());
    CHECK(bench_logger.get_output().empty());
}

TEST_CASE("CPP Logger - logger wrapper survives model destruction and retains readable output") {
    power_grid_model_cpp::Logger logger{PGM_text_logger};
    {
        auto model = make_cpp_model();
        model.add_logger(logger);
        run_calculate(model);
    } // model destroyed here; logger wrapper must remain valid and readable
    CHECK(!logger.get_output().empty());
}

TEST_CASE("CPP Logger - destroying the Logger wrapper while registered does not crash") {
    auto model = make_cpp_model();
    {
        power_grid_model_cpp::Logger logger{PGM_text_logger};
        model.add_logger(logger);
    } // logger wrapper destroyed here while still registered on `model`

    // Must not crash; there is no wrapper left to read output from individually.
    run_calculate(model);
    model.remove_all_loggers();
}

TEST_CASE("CPP Logger - same logger can be attached to multiple models") {
    auto model_a = make_cpp_model();
    auto model_b = make_cpp_model();
    power_grid_model_cpp::Logger logger{PGM_text_logger};

    model_a.add_logger(logger);
    model_b.add_logger(logger);

    run_calculate(model_a);
    auto const after_a = count_lines(logger.get_output());
    CHECK(after_a > 0);

    run_calculate(model_b);
    auto const after_b = count_lines(logger.get_output());
    CHECK(after_b > after_a); // combined output from both models
}

TEST_CASE("CPP Logger - move construction preserves registration and output access") {
    auto model = make_cpp_model();
    power_grid_model_cpp::Logger logger{PGM_text_logger};
    model.add_logger(logger);

    power_grid_model_cpp::Logger moved_logger{std::move(logger)};

    run_calculate(model);
    CHECK(!moved_logger.get_output().empty());

    model.remove_logger(moved_logger);
}

TEST_CASE("CPP Logger - move assignment preserves registration and output access") {
    auto model = make_cpp_model();
    power_grid_model_cpp::Logger logger{PGM_text_logger};
    power_grid_model_cpp::Logger moved_logger{PGM_benchmark_logger};
    model.add_logger(logger);

    moved_logger = std::move(logger);

    run_calculate(model);
    CHECK(!moved_logger.get_output().empty());

    model.remove_logger(moved_logger);
}

TEST_CASE("CPP Logger - model copy construction starts without registrations") {
    auto model = make_cpp_model();
    power_grid_model_cpp::Logger logger{PGM_text_logger};
    model.add_logger(logger);

    power_grid_model_cpp::Model model_copy{model}; // copy construction: fresh handle, no registrations
    run_calculate(model_copy);                     // must not reach `logger`

    CHECK(logger.get_output().empty());

    run_calculate(model); // the original model's registration is unaffected
    CHECK(!logger.get_output().empty());
}

TEST_CASE("CPP Logger - model copy assignment retains destination registrations") {
    auto model = make_cpp_model();
    auto source = make_cpp_model();
    power_grid_model_cpp::Logger logger{PGM_text_logger};
    model.add_logger(logger);

    model = source; // copy assignment: destination handle (and its registrations) is kept
    run_calculate(model);

    CHECK(!logger.get_output().empty());
}

TEST_CASE("CPP Logger - model move transfers registrations") {
    auto model = make_cpp_model();
    power_grid_model_cpp::Logger logger{PGM_text_logger};
    model.add_logger(logger);

    power_grid_model_cpp::Model moved{std::move(model)};
    run_calculate(moved);

    CHECK(!logger.get_output().empty());
}

TEST_CASE("CPP Logger - model move assignment transfers registrations and releases destination's") {
    auto source = make_cpp_model();
    power_grid_model_cpp::Logger source_logger{PGM_text_logger};
    source.add_logger(source_logger);

    auto destination = make_cpp_model();
    power_grid_model_cpp::Logger destination_logger{PGM_text_logger};
    destination.add_logger(destination_logger);

    destination = std::move(source); // destination's own handle (and its registrations) is replaced
    run_calculate(destination);

    CHECK(!source_logger.get_output().empty());     // now reachable via the moved-in handle
    CHECK(destination_logger.get_output().empty()); // its original handle was replaced, not merged
}
