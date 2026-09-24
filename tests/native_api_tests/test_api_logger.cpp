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
#include <power_grid_model_c/options.h>

#include <doctest/doctest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {
using power_grid_model_cpp::Buffer;
using power_grid_model_cpp::DatasetConst;
using power_grid_model_cpp::DatasetMutable;
using power_grid_model_cpp_test::load_dataset;

// Minimal 2-node network JSON.
constexpr auto const input_json = R"json({
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
})json";

// Minimal RAII wrappers so tests don't leak on CHECK failures.

struct HandleGuard : public std::unique_ptr<PGM_Handle, void (*)(PGM_Handle*)> {
    HandleGuard() : std::unique_ptr<PGM_Handle, void (*)(PGM_Handle*)>(PGM_create_handle(), &PGM_destroy_handle) {}
};

struct LoggerGuard : public std::unique_ptr<PGM_Logger, void (*)(PGM_Logger*)> {
    LoggerGuard(PGM_Handle* handle, PGM_Idx type)
        : std::unique_ptr<PGM_Logger, void (*)(PGM_Logger*)>(PGM_create_logger(handle, type), &PGM_destroy_logger) {}
};

// Run a minimal single-scenario power flow using the provided handle.
// All C API calls use that handle so loggers registered to it will receive output.
void run_calculate(PGM_Handle* handle) {
    auto const owning_input = load_dataset(input_json);

    // Convert DatasetMutable (input) -> PGM_ConstDataset for PGM_create_model.
    DatasetConst const const_input{owning_input.dataset};

    PGM_PowerGridModel* model = PGM_create_model(handle, 50.0, const_input.get());
    REQUIRE(PGM_error_code(handle) == PGM_no_error);
    REQUIRE(model != nullptr);

    // Minimal sym_output: 2 nodes.
    Buffer node_output{PGM_def_sym_output_node, 2};
    node_output.set_nan();
    DatasetMutable output_ds{"sym_output", false, 1};
    output_ds.add_buffer("node", 2, 2, nullptr, node_output);

    power_grid_model_cpp::Options opt{};
    PGM_set_calculation_type(handle, opt.get(), PGM_power_flow);
    PGM_set_symmetric(handle, opt.get(), 1);

    PGM_calculate(handle, model, opt.get(), output_ds.get(), nullptr);
    PGM_destroy_model(model);
}

void run_batch_calculate(PGM_Handle* handle) {
    auto const owning_input = load_dataset(input_json);
    DatasetConst const const_input{owning_input.dataset};

    PGM_PowerGridModel* model = PGM_create_model(handle, 50.0, const_input.get());
    REQUIRE(PGM_error_code(handle) == PGM_no_error);
    REQUIRE(model != nullptr);

    std::vector<std::int8_t> const source_status{1, 0};
    DatasetConst update_dataset{"update", true, 2};
    update_dataset.add_buffer("source", 1, 2, nullptr, nullptr);
    update_dataset.add_attribute_buffer("source", "status", source_status.data());

    Buffer node_output{PGM_def_sym_output_node, 4};
    node_output.set_nan();
    DatasetMutable output_dataset{"sym_output", true, 2};
    output_dataset.add_buffer("node", 2, 4, nullptr, node_output);

    power_grid_model_cpp::Options opt{};
    PGM_set_calculation_type(handle, opt.get(), PGM_power_flow);
    PGM_set_symmetric(handle, opt.get(), 1);

    PGM_calculate(handle, model, opt.get(), output_dataset.get(), update_dataset.get());
    PGM_destroy_model(model);
}
// Helper: call PGM_logger_get_output and collect the result into a std::string.
auto get_output(PGM_Handle* h, PGM_Logger* l) {
    std::string result;
    PGM_logger_get_output(
        h, l,
        [](char const* data, PGM_Idx size, auto ctx) {
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
std::ptrdiff_t count_lines(std::string_view text) { return std::ranges::count(text, '\n'); }

void check_tag_presence(std::string_view output, std::initializer_list<int> tags, bool should_be_present) {
    for (auto const tag : tags) {
        auto marker = std::string{"Z] Tag:"};
        marker += std::to_string(tag);
        marker += ':';
        CHECK_MESSAGE((output.find(marker) != std::string::npos) == should_be_present, marker);
    }
}

void check_text_output(std::string const& output, bool is_batch) {
    CHECK(output.starts_with('['));
    CHECK(output.ends_with('\n'));

    check_tag_presence(output, {2100, 2210, 2200, 2220, 2221, 2242, 2225, 2226, 2227, 2246, 3000}, true);
    check_tag_presence(output, {-1, 0, 1000, 2222, 1300, 1400, 2231, 2223, 2224, 2244, 2232, 2235, 2248}, false);

    if (is_batch) {
        check_tag_presence(output, {64, 1200, 128, 1100, 1201}, true);
    } else {
        check_tag_presence(output, {64, 1200, 128, 1100, 1201}, false);
    }
}

// Run a minimal single-scenario power flow using the C++ Model API.
// Loggers registered on `model` (via Model::attach_logger) will receive output from this call.
void run_calculate_cpp(power_grid_model_cpp::Model& model) {
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
    HandleGuard const g;
    PGM_Logger const* bad = PGM_create_logger(g.get(), 999);
    CHECK(bad == nullptr);
    CHECK(PGM_error_code(g.get()) == PGM_regular_error);
}

TEST_CASE("Logger - get_output with null callback returns a regular error") {
    HandleGuard const g;
    LoggerGuard const lg{g.get(), PGM_logger_type_info};

    PGM_logger_get_output(g.get(), lg.get(), nullptr, nullptr);
    CHECK(PGM_error_code(g.get()) == PGM_regular_error);
}

TEST_CASE("Logger - unregister stops subsequent output") {
    HandleGuard const g;
    LoggerGuard const lg{g.get(), PGM_logger_type_info};

    PGM_register_logger(g.get(), lg.get());
    run_calculate(g.get());
    CHECK(PGM_error_code(g.get()) == PGM_no_error);
    CHECK(!get_output(g.get(), lg.get()).empty());

    PGM_logger_clear(g.get(), lg.get());
    PGM_unregister_logger(g.get(), lg.get());
    CHECK(PGM_error_code(g.get()) == PGM_no_error);

    run_calculate(g.get());
    CHECK(PGM_error_code(g.get()) == PGM_no_error);
    CHECK(get_output(g.get(), lg.get()).empty());
}

TEST_CASE("Logger - unregistering and registering again restores output without duplication") {
    HandleGuard const g;
    LoggerGuard const lg{g.get(), PGM_logger_type_info};

    PGM_register_logger(g.get(), lg.get());
    run_calculate(g.get());
    CHECK(PGM_error_code(g.get()) == PGM_no_error);
    std::string const first_output = get_output(g.get(), lg.get());
    CHECK(!first_output.empty());

    PGM_logger_clear(g.get(), lg.get());
    CHECK(PGM_error_code(g.get()) == PGM_no_error);
    PGM_unregister_logger(g.get(), lg.get());
    CHECK(PGM_error_code(g.get()) == PGM_no_error);

    PGM_register_logger(g.get(), lg.get());
    CHECK(PGM_error_code(g.get()) == PGM_no_error);
    run_calculate(g.get());
    CHECK(PGM_error_code(g.get()) == PGM_no_error);

    auto const second_output = get_output(g.get(), lg.get());
    CHECK(!second_output.empty());
    CHECK(count_lines(second_output) == count_lines(first_output));

    PGM_unregister_logger(g.get(), lg.get());
}

TEST_CASE("Logger - unregister non-registered logger is no-op") {
    HandleGuard const g;
    LoggerGuard const lg{g.get(), PGM_logger_type_info};

    PGM_unregister_logger(g.get(), lg.get()); // never registered
    CHECK(PGM_error_code(g.get()) == PGM_no_error);
}

TEST_CASE("Logger - text logger captures output after calculate") {
    HandleGuard const g;
    LoggerGuard const lg{g.get(), PGM_logger_type_info};

    PGM_register_logger(g.get(), lg.get());

    run_calculate(g.get());
    CHECK(PGM_error_code(g.get()) == PGM_no_error);

    std::string const out = get_output(g.get(), lg.get());
    CHECK(PGM_error_code(g.get()) == PGM_no_error);
    // Text logger should have written something; not asserting exact content but must be non-empty.
    CHECK(!out.empty());

    PGM_unregister_logger(g.get(), lg.get());
}

TEST_CASE("Logger - model calculations produce text output") {
    HandleGuard const g;
    LoggerGuard const lg{g.get(), PGM_logger_type_info};

    PGM_register_logger(g.get(), lg.get());

    SUBCASE("single calculation") {
        run_calculate(g.get());
        CHECK(PGM_error_code(g.get()) == PGM_no_error);
        check_text_output(get_output(g.get(), lg.get()), false);
    }

    SUBCASE("batch calculation") {
        run_batch_calculate(g.get());
        CHECK(PGM_error_code(g.get()) == PGM_no_error);
        check_text_output(get_output(g.get(), lg.get()), true);
    }

    PGM_unregister_logger(g.get(), lg.get());
}

// TODO(mgovers): re-enable once benchmark logger becomes available
// TEST_CASE("Logger - benchmark logger captures output after calculate") {
//     HandleGuard const g;
//     LoggerGuard const lg{g.get(), PGM_benchmark_logger};

//     PGM_register_logger(g.get(), lg.get());

//     run_calculate(g.get());
//     CHECK(PGM_error_code(g.get()) == PGM_no_error);

//     std::string const out = get_output(g.get(), lg.get());
//     CHECK(PGM_error_code(g.get()) == PGM_no_error);
//     // Benchmark output must be non-empty and contain TAB-separated fields.
//     CHECK(!out.empty());
//     CHECK(out.find('\t') != std::string::npos);

//     PGM_unregister_logger(g.get(), lg.get());
// }

TEST_CASE("Logger - text logger clear wipes output") {
    HandleGuard const g;
    LoggerGuard const lg{g.get(), PGM_logger_type_info};

    PGM_register_logger(g.get(), lg.get());
    run_calculate(g.get());
    CHECK(PGM_error_code(g.get()) == PGM_no_error);

    // Clear and verify empty
    PGM_logger_clear(g.get(), lg.get());
    CHECK(PGM_error_code(g.get()) == PGM_no_error);
    std::string const out = get_output(g.get(), lg.get());
    CHECK(out.empty());

    PGM_unregister_logger(g.get(), lg.get());
}

TEST_CASE("Logger - loggers persist across clear_error on handle") {
    HandleGuard const g;
    LoggerGuard const lg{g.get(), PGM_logger_type_info};

    PGM_register_logger(g.get(), lg.get());

    // Simulate an error clearing (happens at start of each call_with_catch)
    PGM_clear_error(g.get());

    // Logger must still be registered: run a calculation and check output is captured
    run_calculate(g.get());
    CHECK(PGM_error_code(g.get()) == PGM_no_error);

    std::string const out = get_output(g.get(), lg.get());
    CHECK(!out.empty());

    PGM_unregister_logger(g.get(), lg.get());
}

// TODO(mgovers): re-enable once benchmark logger becomes available
// TEST_CASE("Logger - text and benchmark loggers registered simultaneously") {
//     HandleGuard const g;
//     LoggerGuard const text_lg{g.get(), PGM_logger_type_info};
//     LoggerGuard const bench_lg{g.get(), PGM_benchmark_logger};

//     PGM_register_logger(g.get(), text_lg.get());
//     PGM_register_logger(g.get(), bench_lg.get());

//     run_calculate(g.get());
//     CHECK(PGM_error_code(g.get()) == PGM_no_error);

//     std::string const text_out = get_output(g.get(), text_lg.get());
//     std::string const bench_out = get_output(g.get(), bench_lg.get());
//     CHECK(!text_out.empty());
//     CHECK(!bench_out.empty());

//     PGM_unregister_logger(g.get(), text_lg.get());
//     PGM_unregister_logger(g.get(), bench_lg.get());
// }

TEST_CASE("Logger - registering the same logger twice is idempotent") {
    HandleGuard const g;
    LoggerGuard const lg{g.get(), PGM_logger_type_info};

    PGM_register_logger(g.get(), lg.get());
    PGM_register_logger(g.get(), lg.get()); // second registration — must be a silent no-op
    CHECK(PGM_error_code(g.get()) == PGM_no_error);

    run_calculate(g.get());
    CHECK(PGM_error_code(g.get()) == PGM_no_error);

    // Output must not be doubled — compare line count with a single registration
    // (exact text equality is not usable: each line carries an independent millisecond timestamp).
    std::string const out_double = get_output(g.get(), lg.get());
    PGM_unregister_logger(g.get(), lg.get());

    // Fresh run with a single registration for reference
    HandleGuard const g2;
    LoggerGuard const lg2{g2.get(), PGM_logger_type_info};
    PGM_register_logger(g2.get(), lg2.get());
    run_calculate(g2.get());
    std::string const out_single = get_output(g2.get(), lg2.get());
    PGM_unregister_logger(g2.get(), lg2.get());

    CHECK(count_lines(out_double) == count_lines(out_single));
}

// TODO(mgovers): re-enable once benchmark logger becomes available
// TEST_CASE("Logger - PGM_unregister_all_loggers removes all loggers") {
//     HandleGuard const g;
//     LoggerGuard const text_lg{g.get(), PGM_logger_type_info};
//     LoggerGuard const bench_lg{g.get(), PGM_benchmark_logger};

//     PGM_register_logger(g.get(), text_lg.get());
//     PGM_register_logger(g.get(), bench_lg.get());

//     PGM_unregister_all_loggers(g.get());
//     CHECK(PGM_error_code(g.get()) == PGM_no_error);

//     // After unregistering all, a calculation should produce no output in either logger
//     run_calculate(g.get());
//     CHECK(PGM_error_code(g.get()) == PGM_no_error);

//     std::string const text_out = get_output(g.get(), text_lg.get());
//     std::string const bench_out = get_output(g.get(), bench_lg.get());
//     CHECK(text_out.empty());
//     CHECK(bench_out.empty());
//     // loggers are already unregistered; safe to destroy them via LoggerGuard
// }

TEST_CASE("Logger - destroying a registered logger does not crash a subsequent calculation") {
    HandleGuard const g;
    PGM_Logger* logger = PGM_create_logger(g.get(), PGM_logger_type_info);
    PGM_register_logger(g.get(), logger);

    // Destroy the wrapper while still registered: the underlying implementation must stay
    // alive (shared with the handle's composite) so the calculation below does not crash.
    PGM_destroy_logger(logger);
    CHECK(PGM_error_code(g.get()) == PGM_no_error);

    run_calculate(g.get());
    CHECK(PGM_error_code(g.get()) == PGM_no_error);

    // There is no PGM_Logger* left to read output from individually; clean up via the handle.
    PGM_unregister_all_loggers(g.get());
    CHECK(PGM_error_code(g.get()) == PGM_no_error);
}

TEST_CASE("Logger - destroying the handle while a logger is registered does not crash") {
    PGM_Handle* h = PGM_create_handle();
    LoggerGuard const lg{h, PGM_logger_type_info};

    PGM_register_logger(h, lg.get());
    run_calculate(h);
    CHECK(PGM_error_code(h) == PGM_no_error);

    // Destroy the handle (and its logger registrations) while the logger wrapper
    // is still alive. Must not crash; the logger wrapper itself remains usable afterwards.
    PGM_destroy_handle(h);

    HandleGuard const g2;
    std::string const out = get_output(g2.get(), lg.get());
    CHECK(!out.empty());
}

TEST_CASE("Logger - model logs through the handle passed to PGM_calculate, not the model creation handle") {
    HandleGuard creation_handle;
    HandleGuard const calc_handle;
    LoggerGuard const creation_lg{creation_handle.get(), PGM_logger_type_info};
    LoggerGuard const calc_lg{calc_handle.get(), PGM_logger_type_info};

    // The model is bound to creation_handle's composite logger at PGM_create_model time.
    PGM_register_logger(creation_handle.get(), creation_lg.get());
    auto const owning_input = load_dataset(input_json);
    DatasetConst const const_input{owning_input.dataset};
    PGM_PowerGridModel* model = PGM_create_model(creation_handle.get(), 50.0, const_input.get());
    REQUIRE(model != nullptr);

    // Destroy the creation handle before calculating. PGM_calculate must reseat the model's
    // logger without dereferencing the now-stale reference to creation_handle's composite.
    creation_handle.reset(nullptr);

    // Register a different logger on a different handle and calculate using that handle.
    PGM_register_logger(calc_handle.get(), calc_lg.get());

    Buffer node_output{PGM_def_sym_output_node, 2};
    node_output.set_nan();
    DatasetMutable output_ds{"sym_output", false, 1};
    output_ds.add_buffer("node", 2, 2, nullptr, node_output);

    power_grid_model_cpp::Options opt{};
    PGM_set_calculation_type(calc_handle.get(), opt.get(), PGM_power_flow);
    PGM_set_symmetric(calc_handle.get(), opt.get(), 1);

    PGM_calculate(calc_handle.get(), model, opt.get(), output_ds.get(), nullptr);
    CHECK(PGM_error_code(calc_handle.get()) == PGM_no_error);

    // PGM_calculate reseats the model's logger to the handle passed to that call, so output
    // is captured by calc_handle's logger, not the destroyed creation handle's logger.
    CHECK(!get_output(calc_handle.get(), calc_lg.get()).empty());
    CHECK(get_output(calc_handle.get(), creation_lg.get()).empty());

    PGM_unregister_logger(calc_handle.get(), calc_lg.get());
    PGM_destroy_model(model);
}

// --- C++ API (power_grid_model_cpp::Logger / Model) ---

TEST_CASE("CPP Logger - value construction / empty output before calculation") {
    power_grid_model_cpp::Logger logger{PGM_logger_type_info};
    CHECK(logger.get_output().empty());
}

TEST_CASE("CPP Logger - clear() empties output and keeps registration") {
    auto model = make_cpp_model();
    power_grid_model_cpp::Logger logger{PGM_logger_type_info};

    model.attach_logger(logger);
    run_calculate_cpp(model);
    CHECK(!logger.get_output().empty());

    logger.clear();
    CHECK(logger.get_output().empty());

    logger.clear();
    CHECK(logger.get_output().empty());

    // registration must still be active
    run_calculate_cpp(model);
    CHECK(!logger.get_output().empty());
}

TEST_CASE("CPP Logger - detach_logger stops output from that logger only") {
    auto model = make_cpp_model();
    power_grid_model_cpp::Logger logger_a{PGM_logger_type_info};
    power_grid_model_cpp::Logger logger_b{PGM_logger_type_info};

    model.attach_logger(logger_a);
    model.attach_logger(logger_b);
    model.detach_logger(logger_a);

    run_calculate_cpp(model);

    CHECK(logger_a.get_output().empty());
    CHECK(!logger_b.get_output().empty());
}

// TODO(mgovers): re-enable once benchmark logger becomes available
// TEST_CASE("CPP Logger - detach_all_loggers detaches everything") {
//     auto model = make_cpp_model();
//     power_grid_model_cpp::Logger text_logger{PGM_logger_type_info};
//     power_grid_model_cpp::Logger bench_logger{PGM_benchmark_logger};

//     model.attach_logger(text_logger);
//     model.attach_logger(bench_logger);
//     model.detach_all_loggers();

//     run_calculate_cpp(model);

//     CHECK(text_logger.get_output().empty());
//     CHECK(bench_logger.get_output().empty());
// }

TEST_CASE("CPP Logger - logger wrapper survives model destruction and retains readable output") {
    power_grid_model_cpp::Logger logger{PGM_logger_type_info};
    {
        auto model = make_cpp_model();
        model.attach_logger(logger);
        run_calculate_cpp(model);
    } // model destroyed here; logger wrapper must remain valid and readable
    CHECK(!logger.get_output().empty());
}

TEST_CASE("CPP Logger - destroying the Logger wrapper while registered does not crash") {
    auto model = make_cpp_model();
    {
        power_grid_model_cpp::Logger logger{PGM_logger_type_info};
        model.attach_logger(logger);
    } // logger wrapper destroyed here while still registered on `model`

    // Must not crash; there is no wrapper left to read output from individually.
    run_calculate_cpp(model);
    model.detach_all_loggers();
}

TEST_CASE("CPP Logger - same logger can be attached to multiple models") {
    auto model_a = make_cpp_model();
    auto model_b = make_cpp_model();
    power_grid_model_cpp::Logger logger{PGM_logger_type_info};

    model_a.attach_logger(logger);
    model_b.attach_logger(logger);

    run_calculate_cpp(model_a);
    auto const after_a = count_lines(logger.get_output());
    CHECK(after_a > 0);

    run_calculate_cpp(model_b);
    auto const after_b = count_lines(logger.get_output());
    CHECK(after_b > after_a); // combined output from both models
}

TEST_CASE("CPP Logger - move construction preserves registration and output access") {
    auto model = make_cpp_model();
    power_grid_model_cpp::Logger logger{PGM_logger_type_info};
    model.attach_logger(logger);

    power_grid_model_cpp::Logger moved_logger{std::move(logger)};

    run_calculate_cpp(model);
    CHECK(!moved_logger.get_output().empty());

    model.detach_logger(moved_logger);
}

// TODO(mgovers): re-enable once benchmark logger becomes available
// TEST_CASE("CPP Logger - move assignment preserves registration and output access") {
//     auto model = make_cpp_model();
//     power_grid_model_cpp::Logger logger{PGM_logger_type_info};
//     power_grid_model_cpp::Logger moved_logger{PGM_benchmark_logger};
//     model.attach_logger(logger);

//     moved_logger = std::move(logger);

//     run_calculate_cpp(model);
//     CHECK(!moved_logger.get_output().empty());

//     model.detach_logger(moved_logger);
// }

TEST_CASE("CPP Logger - model copy construction starts without registrations") {
    auto model = make_cpp_model();
    power_grid_model_cpp::Logger logger{PGM_logger_type_info};
    model.attach_logger(logger);

    power_grid_model_cpp::Model model_copy{model}; // copy construction: fresh handle, no registrations
    run_calculate_cpp(model_copy);                 // must not reach `logger`

    CHECK(logger.get_output().empty());

    run_calculate_cpp(model); // the original model's registration is unaffected
    CHECK(!logger.get_output().empty());
}

TEST_CASE("CPP Logger - model copy assignment retains destination registrations") {
    auto model = make_cpp_model();
    auto source = make_cpp_model();
    power_grid_model_cpp::Logger logger{PGM_logger_type_info};
    model.attach_logger(logger);

    model = source; // copy assignment: destination handle (and its registrations) is kept
    run_calculate_cpp(model);

    CHECK(!logger.get_output().empty());
}

TEST_CASE("CPP Logger - model move transfers registrations") {
    auto model = make_cpp_model();
    power_grid_model_cpp::Logger logger{PGM_logger_type_info};
    model.attach_logger(logger);

    power_grid_model_cpp::Model moved{std::move(model)};
    run_calculate_cpp(moved);

    CHECK(!logger.get_output().empty());
}

TEST_CASE("CPP Logger - model move assignment transfers registrations and releases destination's") {
    auto source = make_cpp_model();
    power_grid_model_cpp::Logger source_logger{PGM_logger_type_info};
    source.attach_logger(source_logger);

    auto destination = make_cpp_model();
    power_grid_model_cpp::Logger destination_logger{PGM_logger_type_info};
    destination.attach_logger(destination_logger);

    destination = std::move(source); // destination's own handle (and its registrations) is replaced
    run_calculate_cpp(destination);

    CHECK(!source_logger.get_output().empty());     // now reachable via the moved-in handle
    CHECK(destination_logger.get_output().empty()); // its original handle was replaced, not merged
}
