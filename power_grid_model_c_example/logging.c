// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// Example file for logging calculations with the Power Grid Model C API.

/*
This is a demonstration example only and it is NOT intended to be used in production. The example uses a dummy network
that is not representative of a real power grid, and errors are NOT properly handled. It is the user's responsibility to
handle errors and validate the input data in a real application.
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

static void print_log_output(char const* data, PGM_Idx size, void* user_data) {
    (void)user_data;
    if (size > 0) {
        (void)fwrite(data, 1U, (size_t)size, stdout);
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
        "\"node\":[{\"id\":0,\"u_rated\":10000}],"
        "\"source\":[{\"id\":1,\"node\":0,\"status\":1,\"u_ref\":1,\"sk\":1e12}],"
        "\"sym_load\":[{\"id\":2,\"node\":0,\"status\":1,\"type\":0,\"p_specified\":100000,\"q_specified\":0}]"
        "}"
        "}";

    PGM_Handle* handle = PGM_create_handle();

    // Create and register the logger before the calculation whose diagnostics it should collect.
    PGM_Logger* logger = PGM_create_logger(handle, PGM_logger_type_info);
    CHECK_NO_ERROR(handle);
    PGM_register_logger(handle, logger);
    CHECK_NO_ERROR(handle);

    PGM_Deserializer* deserializer = PGM_create_deserializer_from_null_terminated_string(handle, json_data, PGM_json);
    CHECK_NO_ERROR(handle);
    PGM_WritableDataset* writable_input = PGM_deserializer_get_dataset(handle, deserializer);
    CHECK_NO_ERROR(handle);

    void* node_input = PGM_create_buffer(handle, PGM_def_input_node, 1);
    void* source_input = PGM_create_buffer(handle, PGM_def_input_source, 1);
    void* load_input = PGM_create_buffer(handle, PGM_def_input_sym_load, 1);
    PGM_dataset_writable_set_buffer(handle, writable_input, "node", NULL, node_input);
    PGM_dataset_writable_set_buffer(handle, writable_input, "source", NULL, source_input);
    PGM_dataset_writable_set_buffer(handle, writable_input, "sym_load", NULL, load_input);
    CHECK_NO_ERROR(handle);
    PGM_deserializer_parse_to_buffer(handle, deserializer);
    CHECK_NO_ERROR(handle);

    PGM_ConstDataset* input = PGM_create_dataset_const_from_writable(handle, writable_input);
    PGM_PowerGridModel* model = PGM_create_model(handle, 50.0, input);
    CHECK_NO_ERROR(handle);

    void* node_output = PGM_create_buffer(handle, PGM_def_sym_output_node, 1);
    PGM_MutableDataset* output = PGM_create_dataset_mutable(handle, "sym_output", 0, 1);
    PGM_dataset_mutable_add_buffer(handle, output, "node", 1, 1, NULL, node_output);
    PGM_Options* options = PGM_create_options(handle);
    CHECK_NO_ERROR(handle);

    PGM_calculate(handle, model, options, output, NULL);
    CHECK_NO_ERROR(handle);

    printf("Calculation log:\n");
    // The logger output is not null-terminated, so the callback uses its size argument.
    PGM_logger_get_output(handle, logger, print_log_output, NULL);
    CHECK_NO_ERROR(handle);

    // Unregister before destroying the caller-owned wrapper to release this handle's registration.
    PGM_unregister_logger(handle, logger);
    PGM_destroy_logger(logger);

    PGM_destroy_options(options);
    PGM_destroy_dataset_mutable(output);
    PGM_destroy_buffer(node_output);
    PGM_destroy_model(model);
    PGM_destroy_dataset_const(input);
    PGM_destroy_buffer(load_input);
    PGM_destroy_buffer(source_input);
    PGM_destroy_buffer(node_input);
    PGM_destroy_deserializer(deserializer);
    PGM_destroy_handle(handle);
    return 0;
}
// NOLINTEND(misc-include-cleaner)
// IWYU pragma: end_keep
