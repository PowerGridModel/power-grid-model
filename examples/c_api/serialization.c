// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// example file for the power grid model (de)serialization with the C API

/*
This example will demonstrate how to use the C API of the Power Grid Model to deserialize input data from a JSON string,
perform a power flow calculation, and serialize the output dataset back to a JSON string.

This is a demonstration example only and it is NOT intended to be used in production. The example uses a dummy network
that is not representative of a real power grid, and errors are NOT properly handled. It is the user's responsibility to
handle errors and validate the input data in a real application.

The dummy network consists of 1 source, 1 node, and 1 load:
source_1 - node_0 - load_2
*/

// IWYU pragma: begin_keep
// NOLINTBEGIN(misc-include-cleaner)
#include "power_grid_model_c.h"
#include "power_grid_model_c/dataset_definitions.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define CHECK_NO_ERROR(handle)                                                                                         \
    if (PGM_error_code(handle) != PGM_no_error) {                                                                      \
        (void)printf("PGM error: %s\n", PGM_error_message(handle));                                                    \
        abort();                                                                                                       \
    }

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    // generate artificial input data in JSON format
    static const char* json_data =
        "{\n"
        "  \"version\": \"1.0\",\n"
        "  \"type\": \"input\",\n"
        "  \"is_batch\": false,\n"
        "  \"attributes\": {},\n"
        "  \"data\": {\n"
        "    \"sym_load\": [\n"
        "      {\"id\": 2, \"node\": 0, \"status\": 1, \"type\": 0, \"p_specified\": 0, \"q_specified\": 0}\n"
        "    ],\n"
        "    \"source\": [\n"
        "      {\"id\": 1, \"node\": 0, \"status\": 1, \"u_ref\": 1, \"sk\": 1e20}\n"
        "    ],\n"
        "    \"node\": [\n"
        "      {\"id\": 0, \"u_rated\": 10e3}\n"
        "    ]\n"
        "  }\n"
        "}";

    // create handle
    PGM_Handle* handle = PGM_create_handle();

    // create deserializer from JSON string
    PGM_Deserializer* deserializer = PGM_create_deserializer_from_null_terminated_string(handle, json_data, PGM_json);
    CHECK_NO_ERROR(handle);

    // get the writable dataset from deserializer
    PGM_WritableDataset* writable_dataset = PGM_deserializer_get_dataset(handle, deserializer);
    CHECK_NO_ERROR(handle);

    // get the dataset info
    PGM_DatasetInfo const* dataset_info = PGM_dataset_writable_get_info(handle, writable_dataset);
    CHECK_NO_ERROR(handle);

    // query dataset information
    PGM_Idx n_components = PGM_dataset_info_n_components(handle, dataset_info);
    PGM_Idx is_batch = PGM_dataset_info_is_batch(handle, dataset_info);
    PGM_Idx batch_size = PGM_dataset_info_batch_size(handle, dataset_info);
    char const* input_dataset_name = PGM_dataset_info_name(handle, dataset_info);
    char const* output_dataset_name = PGM_meta_dataset_name(handle, PGM_def_sym_output);
    CHECK_NO_ERROR(handle);

    // allocate memory for input and output buffers for each component with NULL initialization
    void** input_buffers = (void**)calloc((size_t)n_components, sizeof(void*));
    void** output_buffers = (void**)calloc((size_t)n_components, sizeof(void*));

    // create output dataset
    PGM_MutableDataset* output_dataset = PGM_create_dataset_mutable(handle, output_dataset_name, is_batch, batch_size);
    CHECK_NO_ERROR(handle);

    // fill input and output buffers for each component
    for (PGM_Idx component_idx = 0; component_idx < n_components; ++component_idx) {
        char const* component_name = PGM_dataset_info_component_name(handle, dataset_info, component_idx);
        PGM_Idx total_elements = PGM_dataset_info_total_elements(handle, dataset_info, component_idx);
        PGM_MetaComponent const* input_meta_component =
            PGM_meta_get_component_by_name(handle, input_dataset_name, component_name);
        PGM_MetaComponent const* output_meta_component =
            PGM_meta_get_component_by_name(handle, output_dataset_name, component_name);
        CHECK_NO_ERROR(handle);

        if (total_elements > 0) {
            // input buffers
            input_buffers[component_idx] = PGM_create_buffer(handle, input_meta_component, total_elements);
            PGM_dataset_writable_set_buffer(handle, writable_dataset, component_name, NULL,
                                            input_buffers[component_idx]);
            CHECK_NO_ERROR(handle);

            // output buffers
            output_buffers[component_idx] = PGM_create_buffer(handle, output_meta_component, total_elements);
            PGM_dataset_mutable_add_buffer(handle, output_dataset, component_name, total_elements, total_elements, NULL,
                                           output_buffers[component_idx]);
            CHECK_NO_ERROR(handle);
        }
    }

    // parse the JSON data into the input buffers
    PGM_deserializer_parse_to_buffer(handle, deserializer);
    CHECK_NO_ERROR(handle);

    // create const (input) dataset from writable dataset
    PGM_ConstDataset* deserialized_input = PGM_create_dataset_const_from_writable(handle, writable_dataset);
    CHECK_NO_ERROR(handle);

    // create model from deserialized data
    PGM_PowerGridModel* model = PGM_create_model(handle, 50.0, deserialized_input);
    CHECK_NO_ERROR(handle);

    // create calculation options for asymmetric calculation
    PGM_Options* options = PGM_create_options(handle);
    PGM_set_symmetric(handle, options, PGM_symmetric);
    CHECK_NO_ERROR(handle);

    // run power flow calculation
    PGM_calculate(handle, model, options, output_dataset, NULL);
    CHECK_NO_ERROR(handle);

    // serialize the output to JSON
    PGM_ConstDataset* const_output_dataset = PGM_create_dataset_const_from_mutable(handle, output_dataset);
    PGM_Serializer* serializer = PGM_create_serializer(handle, const_output_dataset, PGM_json);
    CHECK_NO_ERROR(handle);

    // print the serialized JSON output
    char const* output_json = PGM_serializer_get_to_zero_terminated_string(handle, serializer, 0, 2);
    CHECK_NO_ERROR(handle);
    printf("\nSerialized output dataset to JSON:\n%s\n", output_json);

    // clean up
    PGM_destroy_serializer(serializer);
    PGM_destroy_dataset_const(const_output_dataset);
    PGM_destroy_dataset_const(deserialized_input);
    PGM_destroy_dataset_mutable(output_dataset);
    PGM_destroy_options(options);
    PGM_destroy_model(model);
    for (PGM_Idx component_idx = 0; component_idx < n_components; ++component_idx) {
        if (input_buffers[component_idx] != NULL) {
            PGM_destroy_buffer(input_buffers[component_idx]);
        }
        if (output_buffers[component_idx] != NULL) {
            PGM_destroy_buffer(output_buffers[component_idx]);
        }
    }
    free((void*)input_buffers);
    free((void*)output_buffers);
    PGM_destroy_deserializer(deserializer);
    CHECK_NO_ERROR(handle);
    PGM_destroy_handle(handle);

    return 0;
}
// NOLINTEND(misc-include-cleaner)
// IWYU pragma: end_keep
