// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

// example file for power grid model C API

/*
This example will calculate the following network,
consisting 1 source, 1 node, 2 sym_load


source_0 --node_1---- sym_load_2
                   |
                   |---- sym_load_3
*/

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "power_grid_model_c.h"

int main(int argc, char** argv) {
    (void)(argc);
    (void)(argv);
    printf("%s", "\nThis is an example to call the C-API of Power Grid Model.\n");

    // create handle
    PGM_Handle* handle = PGM_create_handle();

    /**** create input buffer ****/
    // we create input buffer data using two ways of creating buffer
    // use PGM function to create node and sym_load buffer
    void* node_input = PGM_create_buffer(handle, "input", "node", 1);
    void* sym_load_input = PGM_create_buffer(handle, "input", "sym_load", 2);
    // allocate source buffer in the caller
    size_t source_size = PGM_meta_component_size(handle, "input", "source");
    size_t source_alignment = PGM_meta_component_alignment(handle, "input", "source");
#ifdef _WIN32
    void* source_input = _aligned_malloc(source_size * 1, source_alignment);
#else
    void* source_input = aligned_alloc(source_alignment, source_size * 1);
#endif

    /***** Assign attribute to the input buffer *****/
    // We use two ways to assign, via pointer cast and via helper function
    // For all attributes of all components, see
    // https://power-grid-model.readthedocs.io/en/stable/user_manual/components.html

    // node attribute, we use pointer cast
    size_t node_id_offset = PGM_meta_attribute_offset(handle, "input", "node", "id");
    size_t node_u_rated_offset = PGM_meta_attribute_offset(handle, "input", "node", "u_rated");
    // pointer cast of offset
    *(PGM_ID*)((char*)node_input + node_id_offset) = 1;
    *(double*)((char*)node_input + node_u_rated_offset) = 10e3;  // 10 kV node

    // source attribute, we use helper function
    // set to NaN for all values
    PGM_buffer_set_nan(handle, "input", "source", source_input, 1);
    PGM_ID source_id = 0;
    PGM_ID node = 1;    // also used for load
    int8_t status = 1;  // also used for load
    double u_ref = 1.0;
    double sk = 1e6;  // 1 MVA short circuit capacity
    PGM_buffer_set_value(handle, "input", "source", "id", source_input, &source_id, 1, -1);
    PGM_buffer_set_value(handle, "input", "source", "node", source_input, &node, 1, -1);
    PGM_buffer_set_value(handle, "input", "source", "status", source_input, &status, 1, -1);
    PGM_buffer_set_value(handle, "input", "source", "u_ref", source_input, &u_ref, 1, -1);
    PGM_buffer_set_value(handle, "input", "source", "sk", source_input, &sk, 1, -1);

    // sym_load attribute, we use helper function
    PGM_ID sym_load_id[] = {2, 3};
    int8_t load_type = 0;                               // const power
    double pq_specified[] = {50e3, 10e3, 100e3, 20e3};  // p2, q2, p3, p3
    PGM_buffer_set_value(handle, "input", "sym_load", "id", sym_load_input, sym_load_id, 2, -1);
    // node, status, type are the same for two sym_load, there for the scr_stride is zero
    PGM_buffer_set_value(handle, "input", "sym_load", "node", sym_load_input, &node, 2, 0);
    PGM_buffer_set_value(handle, "input", "sym_load", "status", sym_load_input, &status, 2, 0);
    PGM_buffer_set_value(handle, "input", "sym_load", "type", sym_load_input, &load_type, 2, 0);
    // the stride of p and q input is 2 double value, i.e. 16 bytes
    PGM_buffer_set_value(handle, "input", "sym_load", "p_specified", sym_load_input, pq_specified, 2, 16);
    PGM_buffer_set_value(handle, "input", "sym_load", "q_specified", sym_load_input, pq_specified + 1, 2, 16);

    /**** initialize model ****/
    // component names and sizes
    char const* components[] = {"source", "sym_load", "node"};  // we use this array for mutiple places
    PGM_Idx component_sizes[] = {1, 2, 1};
    void const* input_data[] = {source_input, sym_load_input, node_input};
    // create model
    PGM_PowerGridModel* model = PGM_create_model(handle, 50.0, 3, components, component_sizes, input_data);
    assert(PGM_err_code(handle) == PGM_no_error);

    /**** create output buffer ****/
    // we only create output buffer for node
    // we create of buffer size of 3
    // for one-time calculation, we only need one
    // for batch calculation, we need buffer size of 3 because we are going to run 3 scenarios
    void* node_output = PGM_create_buffer(handle, "sym_output", "node", 3);
    void** output_data = &node_output;
    // value arrays to retrieve, for three scenarios
    double u_pu[3];
    double u_angle[3];

    /**** one time calculation ****/
    // create options with default value
    PGM_Options* opt = PGM_create_options(handle);
    PGM_calculate(
        // one time calculation parameter
        handle, model, opt, 1, components + 2 /* node at position 2*/, output_data,
        // batch parameter
        0, 0, NULL, NULL, NULL, NULL);
    assert(PGM_err_code(handle) == PGM_no_error);
    // get value and print
    PGM_buffer_get_value(handle, "sym_output", "node", "u_pu", node_output, u_pu, 1, -1);
    PGM_buffer_get_value(handle, "sym_output", "node", "u_angle", node_output, u_angle, 1, -1);
    printf("\nOne-time Calculation\n");
    printf("Node result u_pu: %f, u_angle: %f\n", u_pu[0], u_angle[0]);

    /**** one time calculation error ****/
    // we set max iteration to very low so that it will diverge.
    PGM_set_max_iter(handle, opt, 1);
    PGM_calculate(
        // one time calculation parameter
        handle, model, opt, 1, components + 2 /* node at position 2*/, output_data,
        // batch parameter
        0, 0, NULL, NULL, NULL, NULL);
    // print error code and message
    printf("\nOne-time Calculation Error\n");
    printf("Error code: %ld, error message: %s\n", PGM_err_code(handle), PGM_err_msg(handle));
    // set back to normal iteration
    PGM_set_max_iter(handle, opt, 20);

    /**** release all the resources ****/
    PGM_destroy_options(opt);
    PGM_destroy_buffer(node_output);
    PGM_destroy_model(model);
#ifdef _WIN32
    _aligned_free(source_input);
#else
    free(source_input);
#endif
    PGM_destroy_buffer(sym_load_input);
    PGM_destroy_buffer(node_input);
    PGM_destroy_handle(handle);
    return 0;
}
