// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// Example file for logging calculations with the Power Grid Model C API.
// It is the C counterpart of the Python Logging Example notebook in docs/example_notebooks.

/*
This is a demonstration example only and it is NOT intended to be used in production. The example uses a dummy network
that is not representative of a real power grid, and errors are NOT properly handled. It is the user's responsibility to
handle errors and validate the input data in a real application.

There are two types of loggers:
- PGM_logger_type_info: a timestamped line for every logged event, to follow what a calculation does.
- PGM_benchmark_logger: one line per event code with totals per calculation phase, for performance analysis.
The events are timings in seconds and iteration counts. LogEvent in power_grid_model/common/logging.hpp defines their
codes, for example 100 for the total time and 3253 for the number of power flow iterations.

Logger output gives debugging hints for advanced users. It is not conclusive and may change between releases.
Logging has a performance cost, which is why it is opt-in: a logger only collects output while it is registered to the
handle used for the calculation.

This example uses the 3-node network from the Power Flow Example:

 -----------------------line_8---------------
 |                                          |
node_1 ---line_3--- node_2 ----line_5---- node_6
 |                    |                     |
source_10          sym_load_4           sym_load_7
*/

// IWYU pragma: begin_keep
// NOLINTBEGIN(misc-include-cleaner)
#include "power_grid_model_c.h"
#include "power_grid_model_c/dataset_definitions.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define CHECK_NO_ERROR(handle)                                                                                         \
    if (PGM_error_code(handle) != PGM_no_error) {                                                                      \
        (void)printf("PGM error: %s\n", PGM_error_message(handle));                                                    \
        abort();                                                                                                       \
    }

// The logger output is not null-terminated, so the callbacks below use the size argument.
// Every line of the output ends with a newline.

struct PrintLogOutputUserData {
    FILE* stream;
};

// Writes the logger output to the FILE stream provided in the user_data.
// This function will be provided as a function pointer to the logger.
static void print_log_output_impl(char const* data, PGM_Idx size, void* user_data) {
    if (size > 0) {
        struct PrintLogOutputUserData* print_log_output_user_data = (struct PrintLogOutputUserData*)user_data;
        (void)fwrite(data, 1U, (size_t)size, print_log_output_user_data->stream);
    }
}

static void print_logger(PGM_Handle* handle, PGM_Logger* logger) {
    // use stdout in the user_data (the stream to write to)
    struct PrintLogOutputUserData print_log_output_user_data = {.stream = stdout};

    // provide the implementation as a function pointer to PGM_logger_get_output,
    PGM_logger_get_output(handle, logger, print_log_output_impl, &print_log_output_user_data);
    CHECK_NO_ERROR(handle);
}

struct CountLogLinesUserData {
    PGM_Idx n_lines;
};

// Assigns the number of logged lines to the provided user_data pointer.
// This function will be provided as a function pointer to the logger.
static void count_log_lines_impl(char const* data, PGM_Idx size, void* user_data) {
    struct CountLogLinesUserData* count_log_lines_user_data = (struct CountLogLinesUserData*)user_data;
    PGM_Idx n_lines = 0;
    for (PGM_Idx i = 0; i != size; ++i) {
        if (data[i] == '\n' || i == size - 1) { // on newline or end of data (if the data does not end with a newline)
            n_lines++;
        }
    }
    count_log_lines_user_data->n_lines = n_lines;
}

static PGM_Idx count_logger_lines(PGM_Handle* handle, PGM_Logger* logger) {
    // use n_lines as the user_data (the result to be returned)
    struct CountLogLinesUserData count_log_lines_user_data = {.n_lines = 0};

    // provide the implementation as a function pointer to PGM_logger_get_output
    PGM_logger_get_output(handle, logger, count_log_lines_impl, &count_log_lines_user_data);
    CHECK_NO_ERROR(handle);

    return count_log_lines_user_data.n_lines;
}

struct ForwardLogLinesUserData {
    char const* log_name; // the null-terminated log prefix, e.g.: "power_grid_model"
};

// Forward each line to the application log, which this example prints to stdout.
// This function will be provided as a function pointer to the logger.
static void forward_log_lines_impl(char const* data, PGM_Idx size, void* user_data) {
    struct ForwardLogLinesUserData* forward_log_lines_user_data = (struct ForwardLogLinesUserData*)user_data;
    char const* log_name = forward_log_lines_user_data->log_name;
    PGM_Idx begin = 0;
    for (PGM_Idx end = 0; end != size; ++end) {
        if (data[end] == '\n') {
            printf("INFO %s: %.*s\n", log_name, (int)(end - begin), data + begin);
            begin = end + 1;
        }
    }
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    static char const* const json_data =
        "{"
        "\"version\":\"1.0\","
        "\"type\":\"input\","
        "\"is_batch\":false,"
        "\"attributes\":{},"
        "\"data\":{"
        "\"node\":[{\"id\":1,\"u_rated\":10500},{\"id\":2,\"u_rated\":10500},{\"id\":6,\"u_rated\":10500}],"
        "\"line\":["
        "{\"id\":3,\"from_node\":1,\"to_node\":2,\"from_status\":1,\"to_status\":1,"
        "\"r1\":0.25,\"x1\":0.2,\"c1\":10e-6,\"tan1\":0,\"i_n\":1000},"
        "{\"id\":5,\"from_node\":2,\"to_node\":6,\"from_status\":1,\"to_status\":1,"
        "\"r1\":0.25,\"x1\":0.2,\"c1\":10e-6,\"tan1\":0,\"i_n\":1000},"
        "{\"id\":8,\"from_node\":1,\"to_node\":6,\"from_status\":1,\"to_status\":1,"
        "\"r1\":0.25,\"x1\":0.2,\"c1\":10e-6,\"tan1\":0,\"i_n\":1000}"
        "],"
        "\"sym_load\":["
        "{\"id\":4,\"node\":2,\"status\":1,\"type\":0,\"p_specified\":20e6,\"q_specified\":5e6},"
        "{\"id\":7,\"node\":6,\"status\":1,\"type\":0,\"p_specified\":10e6,\"q_specified\":2e6}"
        "],"
        "\"source\":[{\"id\":10,\"node\":1,\"status\":1,\"u_ref\":1}]"
        "}"
        "}";

    PGM_Handle* handle = PGM_create_handle();

    /**** create the model ****/
    PGM_Deserializer* deserializer = PGM_create_deserializer_from_null_terminated_string(handle, json_data, PGM_json);
    CHECK_NO_ERROR(handle);
    PGM_WritableDataset* writable_input = PGM_deserializer_get_dataset(handle, deserializer);
    CHECK_NO_ERROR(handle);

    void* node_input = PGM_create_buffer(handle, PGM_def_input_node, 3);
    void* source_input = PGM_create_buffer(handle, PGM_def_input_source, 1);
    void* load_input = PGM_create_buffer(handle, PGM_def_input_sym_load, 2);
    void* line_input = PGM_create_buffer(handle, PGM_def_input_line, 3);
    PGM_dataset_writable_set_buffer(handle, writable_input, "node", NULL, node_input);
    PGM_dataset_writable_set_buffer(handle, writable_input, "source", NULL, source_input);
    PGM_dataset_writable_set_buffer(handle, writable_input, "sym_load", NULL, load_input);
    PGM_dataset_writable_set_buffer(handle, writable_input, "line", NULL, line_input);
    CHECK_NO_ERROR(handle);
    PGM_deserializer_parse_to_buffer(handle, deserializer);
    CHECK_NO_ERROR(handle);

    PGM_ConstDataset* input = PGM_create_dataset_const_from_writable(handle, writable_input);
    PGM_PowerGridModel* model = PGM_create_model(handle, 50.0, input);
    CHECK_NO_ERROR(handle);

    // The node output buffer has room for 3 nodes in 3 batch scenarios. Single calculations use the first 3 rows.
    void* node_output = PGM_create_buffer(handle, PGM_def_sym_output_node, 9);
    PGM_MutableDataset* output = PGM_create_dataset_mutable(handle, "sym_output", 0, 1);
    PGM_dataset_mutable_add_buffer(handle, output, "node", 3, 3, NULL, node_output);
    PGM_Options* options = PGM_create_options(handle);
    CHECK_NO_ERROR(handle);

    /**** info logger ****/
    // Create and register the logger before the calculation whose diagnostics it should collect.
    PGM_Logger* info_logger = PGM_create_logger(handle, PGM_logger_type_info);
    CHECK_NO_ERROR(handle);
    PGM_register_logger(handle, info_logger);
    CHECK_NO_ERROR(handle);
    PGM_calculate(handle, model, options, output, NULL);
    CHECK_NO_ERROR(handle);
    // The output is kept after unregistering.
    PGM_unregister_logger(handle, info_logger);
    printf("Info logger:\n");
    print_logger(handle, info_logger);

    /**** benchmark logger ****/
    // Each line has the form EVENT_CODE<TAB>VALUE.
    PGM_Logger* benchmark_logger = PGM_create_logger(handle, PGM_benchmark_logger);
    CHECK_NO_ERROR(handle);
    PGM_register_logger(handle, benchmark_logger);
    CHECK_NO_ERROR(handle);
    PGM_calculate(handle, model, options, output, NULL);
    CHECK_NO_ERROR(handle);
    PGM_unregister_logger(handle, benchmark_logger);
    printf("\nBenchmark logger:\n");
    print_logger(handle, benchmark_logger);

    /**** multiple simultaneous loggers ****/
    // Loggers of different types can be registered to the same handle at the same time.
    // Each logger captures its own output.
    // A logger keeps its output when it is registered again, so first clear the output of the sections above.
    PGM_logger_clear_content(handle, info_logger);
    CHECK_NO_ERROR(handle);
    PGM_logger_clear_content(handle, benchmark_logger);
    CHECK_NO_ERROR(handle);
    PGM_register_logger(handle, info_logger);
    CHECK_NO_ERROR(handle);
    PGM_register_logger(handle, benchmark_logger);
    CHECK_NO_ERROR(handle);
    PGM_calculate(handle, model, options, output, NULL);
    CHECK_NO_ERROR(handle);
    PGM_unregister_all_loggers(handle);
    printf("\nMultiple loggers, info logger:\n");
    print_logger(handle, info_logger);
    printf("\nMultiple loggers, benchmark logger:\n");
    print_logger(handle, benchmark_logger);

    /**** accumulation and clearing content ****/
    // Output accumulates across calculations while the logger is registered. A benchmark logger adds up the timings and
    // keeps the highest iteration count, so its number of lines stays the same.
    PGM_logger_clear_content(handle, benchmark_logger);
    CHECK_NO_ERROR(handle);
    PGM_register_logger(handle, benchmark_logger);
    CHECK_NO_ERROR(handle);
    for (int i = 0; i != 4; ++i) {
        PGM_calculate(handle, model, options, output, NULL);
        CHECK_NO_ERROR(handle);
    }
    printf("\nCombined output of 4 calculations (%d lines):\n", (int)count_logger_lines(handle, benchmark_logger));
    print_logger(handle, benchmark_logger);

    // Clearing the content resets the output without unregistering the logger.
    PGM_logger_clear_content(handle, benchmark_logger);
    CHECK_NO_ERROR(handle);
    printf("\nAfter clearing the content: %d lines\n", (int)count_logger_lines(handle, benchmark_logger));
    PGM_calculate(handle, model, options, output, NULL);
    CHECK_NO_ERROR(handle);
    PGM_unregister_logger(handle, benchmark_logger);
    printf("After one more calculation: %d lines\n", (int)count_logger_lines(handle, benchmark_logger));

    /**** application logging integration ****/
    // The callback receives the whole output in one call, so it splits the output into lines for the application log.
    // Clear the content after forwarding it, so that no line is forwarded twice.
    char log_name[] = "power_grid_model";
    PGM_logger_clear_content(handle, info_logger);
    CHECK_NO_ERROR(handle);
    PGM_register_logger(handle, info_logger);
    CHECK_NO_ERROR(handle);
    printf("\nINFO %s: Starting power flow calculation\n", log_name);
    PGM_calculate(handle, model, options, output, NULL);
    CHECK_NO_ERROR(handle);
    PGM_unregister_logger(handle, info_logger);
    struct ForwardLogLinesUserData forward_log_lines_user_data = {.log_name = log_name};
    PGM_logger_get_output(handle, info_logger, forward_log_lines_impl, &forward_log_lines_user_data);
    CHECK_NO_ERROR(handle);
    PGM_logger_clear_content(handle, info_logger);
    CHECK_NO_ERROR(handle);

    /**** batch calculation ****/
    // Loggers also collect the events from all threads of a batch calculation.
    // If the calculation fails, the logger output is not guaranteed to be correct.
    // Vary the active power of sym_load_4 across 3 scenarios.
    PGM_ID const load_id = 4;
    double const p_update[] = {10e6, 20e6, 30e6};
    void* load_update = PGM_create_buffer(handle, PGM_def_update_sym_load, 3);
    CHECK_NO_ERROR(handle);
    PGM_buffer_set_nan(handle, PGM_def_update_sym_load, load_update, 0, 3);
    PGM_buffer_set_value(handle, PGM_def_update_sym_load_id, load_update, &load_id, 0, 3, 0);
    PGM_buffer_set_value(handle, PGM_def_update_sym_load_p_specified, load_update, p_update, 0, 3, -1);
    PGM_ConstDataset* batch_update = PGM_create_dataset_const(handle, "update", 1, 3);
    PGM_dataset_const_add_buffer(handle, batch_update, "sym_load", 1, 3, NULL, load_update);
    CHECK_NO_ERROR(handle);
    PGM_MutableDataset* batch_output = PGM_create_dataset_mutable(handle, "sym_output", 1, 3);
    PGM_dataset_mutable_add_buffer(handle, batch_output, "node", 3, 9, NULL, node_output);
    CHECK_NO_ERROR(handle);
    PGM_set_threading(handle, options, 0); // use all available threads
    CHECK_NO_ERROR(handle);

    PGM_logger_clear_content(handle, benchmark_logger);
    CHECK_NO_ERROR(handle);
    PGM_register_logger(handle, benchmark_logger);
    CHECK_NO_ERROR(handle);
    PGM_calculate(handle, model, options, batch_output, batch_update);
    CHECK_NO_ERROR(handle);
    PGM_unregister_logger(handle, benchmark_logger);
    printf("\nBatch calculation, benchmark logger:\n");
    print_logger(handle, benchmark_logger);

    /**** release all the resources ****/
    // Unregister a logger before destroying it. Destroying a registered logger is safe, but it keeps collecting output
    // until PGM_unregister_all_loggers() is called or the handle is destroyed.
    // Do not use the same logger from multiple user threads at the same time without external synchronization.
    PGM_destroy_logger(benchmark_logger);
    PGM_destroy_logger(info_logger);

    PGM_destroy_dataset_mutable(batch_output);
    PGM_destroy_dataset_const(batch_update);
    PGM_destroy_buffer(load_update);
    PGM_destroy_options(options);
    PGM_destroy_dataset_mutable(output);
    PGM_destroy_buffer(node_output);
    PGM_destroy_model(model);
    PGM_destroy_dataset_const(input);
    PGM_destroy_buffer(line_input);
    PGM_destroy_buffer(load_input);
    PGM_destroy_buffer(source_input);
    PGM_destroy_buffer(node_input);
    PGM_destroy_deserializer(deserializer);
    PGM_destroy_handle(handle);
    return 0;
}
// NOLINTEND(misc-include-cleaner)
// IWYU pragma: end_keep
