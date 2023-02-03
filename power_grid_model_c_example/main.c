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

We do one-time calculation on the following value
node: 10kV
source: 1.0 p.u. u_ref, 1 MVA sk
sym_load_2: 50 kW, 10 kvar
sym_load_3: 100 kW, 20 kvar

We do batch calculation with 3 scenarios, with the following mutation
#0: source: u_ref = 0.95, sym_load_2: 100 kW, sym_load_3: 200 kW
#1: source: u_ref = 1.05, sym_load_2: 0 kW
#3: source: u_ref = 1.10, sym_load_3: -200 kW
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
    // Pointer cast is generally more efficient and flexible because you are not calling into the shared
    //     object everytime. But it requires the user to retrieve the offset information first.
    // Using the buffer helper function is more convenient but with some overhead.

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
    // node, status, type are the same for two sym_load, therefore the scr_stride is zero
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

    /**** One time calculation error ****/
    // we set max iteration to very low so that it will diverge.
    PGM_set_max_iter(handle, opt, 1);
    PGM_calculate(
        // one time calculation parameter
        handle, model, opt, 1, components + 2 /* node at position 2*/, output_data,
        // batch parameter
        0, 0, NULL, NULL, NULL, NULL);
    // print error code and message
    printf("\nOne-time Calculation Error\n");
    printf("Error code: %d, error message: %s", (int)PGM_err_code(handle), PGM_err_msg(handle));
    // set back to normal iteration
    PGM_set_max_iter(handle, opt, 20);

    /**** prepare batch update dataset ****/

    // 1 source update per scenario
    void* source_update = PGM_create_buffer(handle, "update", "source", 3);
    PGM_buffer_set_nan(handle, "update", "source", source_update, 3);
    double u_ref_update[] = {0.95, 1.05, 1.1};
    // set all source id to the same id, stride is zero
    PGM_buffer_set_value(handle, "update", "source", "id", source_update, &source_id, 3, 0);
    PGM_buffer_set_value(handle, "update", "source", "u_ref", source_update, u_ref_update, 3, -1);

    // 2 load update in scenario 0, 1 load update in scenario 1, 1 load update in scenario 2
    void* load_update = PGM_create_buffer(handle, "update", "sym_load", 4);
    PGM_buffer_set_nan(handle, "update", "sym_load", load_update, 4);
    PGM_ID load_update_id[] = {2, 3, 2, 3};  // 2, 3 for #0, 2 for #1, 3 for #2
    double p_update[] = {100e3, 200e3, 0.0, -200e3};
    PGM_buffer_set_value(handle, "update", "sym_load", "id", load_update, load_update_id, 4, -1);
    PGM_buffer_set_value(handle, "update", "sym_load", "p_specified", load_update, p_update, 4, -1);
    PGM_Idx indptr_load[] = {0, 2, 3, 4};  // 2 updates for #0, 1 update for #1, 2 update for #2

    // update meta data
    PGM_Idx n_component_elements_per_scenario[] = {1, -1};         // 1 per scenario for source, variable for load
    PGM_Idx const* indptrs_per_component[] = {NULL, indptr_load};  // variable for load
    void const* update_data[] = {source_update, load_update};

    /**** Batch calculation ****/
    PGM_calculate(
        // one time calculation parameter
        handle, model, opt, 1, components + 2 /* node at position 2*/, output_data,
        // batch parameter
        3, 2, components, n_component_elements_per_scenario, indptrs_per_component, update_data);
    assert(PGM_err_code(handle) == PGM_no_error);
    // get node result and print
    PGM_buffer_get_value(handle, "sym_output", "node", "u_pu", node_output, u_pu, 3, -1);
    PGM_buffer_get_value(handle, "sym_output", "node", "u_angle", node_output, u_angle, 3, -1);
    printf("\nBatch Calculation\n");
    int i;
    for (i = 0; i != 3; ++i) {
        printf("Scenario %d, u_pu: %f, u_angle: %f\n", i, u_pu[i], u_angle[i]);
    }

    /**** Batch calculation error ****/
    // we set some errors in batch data
    // scenario 0 is normal
    // scenario 1 has a very high load so the calculation will diverge
    // scenario 2 has a unknown id
    p_update[2] = 100e12;     // very high load for scenario 1
    load_update_id[3] = 100;  // unknown id for scenario 2
    PGM_buffer_set_value(handle, "update", "sym_load", "id", load_update, load_update_id, 4, -1);
    PGM_buffer_set_value(handle, "update", "sym_load", "p_specified", load_update, p_update, 4, -1);
    // calculate
    PGM_calculate(
        // one time calculation parameter
        handle, model, opt, 1, components + 2 /* node at position 2*/, output_data,
        // batch parameter
        3, 2, components, n_component_elements_per_scenario, indptrs_per_component, update_data);
    // print error
    printf("\nBatch Calculation Error\n");
    printf("Error code: %d\n", (int)PGM_err_code(handle));
    // print error in detail
    PGM_Idx n_failed_scenarios = PGM_n_failed_scenarios(handle);
    PGM_Idx const* failed_scenarios = PGM_failed_scenarios(handle);
    char const** batch_errs = PGM_batch_errs(handle);
    for (i = 0; i != n_failed_scenarios; ++i) {
        printf("Failed scenario %d, error message: %s", (int)failed_scenarios[i], batch_errs[i]);
    }
    // print normal results
    printf("Normal result:\n");
    printf("Scenario %d, u_pu: %f, u_angle: %f\n", i, u_pu[0], u_angle[0]);

    /**** release all the resources ****/
    // Here we need to release all the resources allocated
    // If you are using C++, you can wrap the resource returned by PGM
    //     in a smart pointer with the destroy function as custom deleter
    PGM_destroy_buffer(load_update);
    PGM_destroy_buffer(source_update);
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
