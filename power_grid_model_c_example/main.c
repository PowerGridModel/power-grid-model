// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
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
#2: source: u_ref = 1.10, sym_load_3: -200 kW
*/

#include "power_grid_model_c.h"
#include "power_grid_model_c/dataset_definitions.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    (void)(argc);
    (void)(argv);
    printf("%s", "\nThis is an example to call the C API of Power Grid Model.\n");

    // create handle
    PGM_Handle* handle = PGM_create_handle();

    /**** create input buffer ****/
    // we create input buffer data using two ways of creating buffer
    // use PGM function to create node and sym_load buffer
    void* node_input = PGM_create_buffer(handle, PGM_def_input_node, 1);
    assert(PGM_error_code(handle) == PGM_no_error);
    void* sym_load_input = PGM_create_buffer(handle, PGM_def_input_sym_load, 2);
    assert(PGM_error_code(handle) == PGM_no_error);
    // allocate source buffer in the caller
    size_t source_size = PGM_meta_component_size(handle, PGM_def_input_source);
    size_t source_alignment = PGM_meta_component_alignment(handle, PGM_def_input_source);
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
    size_t node_id_offset = PGM_meta_attribute_offset(handle, PGM_def_input_node_id);
    size_t node_u_rated_offset = PGM_meta_attribute_offset(handle, PGM_def_input_node_u_rated);
    // pointer cast of offset
    *(PGM_ID*)((char*)node_input + node_id_offset) = 1;
    *(double*)((char*)node_input + node_u_rated_offset) = 10e3; // 10 kV node

    // source attribute, we use helper function
    // set to NaN for all values, it is recommended for input and update buffers
    PGM_buffer_set_nan(handle, PGM_def_input_source, source_input, 0, 1);
    PGM_ID source_id = 0;
    PGM_ID node = 1;   // also used for load
    int8_t status = 1; // also used for load
    double u_ref = 1.0;
    double sk = 1e6; // 1 MVA short circuit capacity
    PGM_buffer_set_value(handle, PGM_def_input_source_id, source_input, &source_id, 0, 1, -1);
    PGM_buffer_set_value(handle, PGM_def_input_source_node, source_input, &node, 0, 1, -1);
    PGM_buffer_set_value(handle, PGM_def_input_source_status, source_input, &status, 0, 1, -1);
    PGM_buffer_set_value(handle, PGM_def_input_source_u_ref, source_input, &u_ref, 0, 1, -1);
    PGM_buffer_set_value(handle, PGM_def_input_source_sk, source_input, &sk, 0, 1, -1);
    assert(PGM_error_code(handle) == PGM_no_error);

    // sym_load attribute, we use helper function
    PGM_ID sym_load_id[] = {2, 3};
    int8_t load_type = 0;                              // const power
    double pq_specified[] = {50e3, 10e3, 100e3, 20e3}; // p2, q2, p3, p3
    PGM_buffer_set_value(handle, PGM_def_input_sym_load_id, sym_load_input, sym_load_id, 0, 2, -1);
    // node, status, type are the same for two sym_load, therefore the scr_stride is zero
    PGM_buffer_set_value(handle, PGM_def_input_sym_load_node, sym_load_input, &node, 0, 2, 0);
    PGM_buffer_set_value(handle, PGM_def_input_sym_load_status, sym_load_input, &status, 0, 2, 0);
    PGM_buffer_set_value(handle, PGM_def_input_sym_load_type, sym_load_input, &load_type, 0, 2, 0);
    // the stride of p and q input is 2 double value, i.e. 16 bytes
    PGM_buffer_set_value(handle, PGM_def_input_sym_load_p_specified, sym_load_input, pq_specified, 0, 2, 16);
    PGM_buffer_set_value(handle, PGM_def_input_sym_load_q_specified, sym_load_input, pq_specified + 1, 0, 2, 16);
    assert(PGM_error_code(handle) == PGM_no_error);

    /**** initialize model ****/
    // input dataset
    PGM_ConstDataset* input_dataset = PGM_create_dataset_const(handle, "input", 0, 1);
    PGM_dataset_const_add_buffer(handle, input_dataset, "node", 1, 1, NULL, node_input);
    PGM_dataset_const_add_buffer(handle, input_dataset, "source", 1, 1, NULL, source_input);
    PGM_dataset_const_add_buffer(handle, input_dataset, "sym_load", 2, 2, NULL, sym_load_input);
    assert(PGM_error_code(handle) == PGM_no_error);
    // create model
    PGM_PowerGridModel* model = PGM_create_model(handle, 50.0, input_dataset);
    assert(PGM_error_code(handle) == PGM_no_error);

    /**** create output buffer ****/
    // we only create output buffer for node
    // we create of buffer size of 3
    // for one-time calculation, we only need one
    // for batch calculation, we need buffer size of 3 because we are going to run 3 scenarios
    void* node_output = PGM_create_buffer(handle, PGM_def_sym_output_node, 3);
    assert(PGM_error_code(handle) == PGM_no_error);
    // value arrays to retrieve, for three scenarios
    double u_pu[3];
    double u_angle[3];
    // dataset single
    PGM_MutableDataset* single_output_dataset = PGM_create_dataset_mutable(handle, "sym_output", 0, 1);
    PGM_dataset_mutable_add_buffer(handle, single_output_dataset, "node", 1, 1, NULL, node_output);
    assert(PGM_error_code(handle) == PGM_no_error);
    // dataset batch
    PGM_MutableDataset* batch_output_dataset = PGM_create_dataset_mutable(handle, "sym_output", 1, 3);
    PGM_dataset_mutable_add_buffer(handle, batch_output_dataset, "node", 1, 3, NULL, node_output);
    assert(PGM_error_code(handle) == PGM_no_error);

    /**** one time calculation ****/
    // create options with default value
    PGM_Options* opt = PGM_create_options(handle);
    PGM_calculate(handle, model, opt, single_output_dataset, NULL);
    assert(PGM_error_code(handle) == PGM_no_error);
    // get value and print
    PGM_buffer_get_value(handle, PGM_def_sym_output_node_u_pu, node_output, u_pu, 0, 1, -1);
    PGM_buffer_get_value(handle, PGM_def_sym_output_node_u_angle, node_output, u_angle, 0, 1, -1);
    printf("\nOne-time Calculation\n");
    printf("Node result u_pu: %f, u_angle: %f\n", u_pu[0], u_angle[0]);

    /**** One time calculation error ****/
    // we set max iteration to very low so that it will not converge.
    PGM_set_max_iter(handle, opt, 1);
    PGM_calculate(handle, model, opt, single_output_dataset, NULL);
    assert(PGM_error_code(handle) != PGM_no_error);
    // print error code and message
    printf("\nOne-time Calculation Error\n");
    printf("Error code: %d, error message: %s", (int)PGM_error_code(handle), PGM_error_message(handle));
    // set back to normal iteration
    PGM_set_max_iter(handle, opt, 20);
    // clear error
    PGM_clear_error(handle);

    /**** prepare batch update dataset ****/

    // 1 source update per scenario
    void* source_update = PGM_create_buffer(handle, PGM_def_update_source, 3);
    assert(PGM_error_code(handle) == PGM_no_error);
    // set to NaN for all values, it is recommended for input and update buffers
    PGM_buffer_set_nan(handle, PGM_def_update_source, source_update, 0, 3);
    double u_ref_update[] = {0.95, 1.05, 1.1};
    // set all source id to the same id, stride is zero
    PGM_buffer_set_value(handle, PGM_def_update_source_id, source_update, &source_id, 0, 3, 0);
    PGM_buffer_set_value(handle, PGM_def_update_source_u_ref, source_update, u_ref_update, 0, 3, -1);

    // 2 load update in scenario 0, 1 load update in scenario 1, 1 load update in scenario 2
    void* load_update = PGM_create_buffer(handle, PGM_def_update_sym_load, 4);
    PGM_buffer_set_nan(handle, PGM_def_update_sym_load, load_update, 0, 4);
    PGM_ID load_update_id[] = {2, 3, 2, 3}; // 2, 3 for #0, 2 for #1, 3 for #2
    double p_update[] = {100e3, 200e3, 0.0, -200e3};
    PGM_buffer_set_value(handle, PGM_def_update_sym_load_id, load_update, load_update_id, 0, 4, -1);
    PGM_buffer_set_value(handle, PGM_def_update_sym_load_p_specified, load_update, p_update, 0, 4, -1);
    PGM_Idx indptr_load[] = {0, 2, 3, 4}; // 2 updates for #0, 1 update for #1, 2 update for #2

    // update batch dataset
    PGM_ConstDataset* batch_update_dataset = PGM_create_dataset_const(handle, "update", 1, 3);
    PGM_dataset_const_add_buffer(handle, batch_update_dataset, "source", 1, 3, NULL, source_update);
    PGM_dataset_const_add_buffer(handle, batch_update_dataset, "sym_load", -1, 4, indptr_load, load_update);
    assert(PGM_error_code(handle) == PGM_no_error);

    /**** Batch calculation ****/
    PGM_calculate(handle, model, opt, batch_output_dataset, batch_update_dataset);
    assert(PGM_error_code(handle) == PGM_no_error);
    // get node result and print
    PGM_buffer_get_value(handle, PGM_def_sym_output_node_u_pu, node_output, u_pu, 0, 3, -1);
    PGM_buffer_get_value(handle, PGM_def_sym_output_node_u_angle, node_output, u_angle, 0, 3, -1);
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
    p_update[2] = 100e12;    // very high load for scenario 1
    load_update_id[3] = 100; // unknown id for scenario 2
    PGM_buffer_set_value(handle, PGM_def_update_sym_load_id, load_update, load_update_id, 0, 4, -1);
    PGM_buffer_set_value(handle, PGM_def_update_sym_load_p_specified, load_update, p_update, 0, 4, -1);
    // calculate
    PGM_calculate(handle, model, opt, batch_output_dataset, batch_update_dataset);
    assert(PGM_error_code(handle) != PGM_no_error);
    // print error
    printf("\nBatch Calculation Error\n");
    printf("Error code: %d\n", (int)PGM_error_code(handle));
    // print error in detail
    PGM_Idx n_failed_scenarios = PGM_n_failed_scenarios(handle);
    PGM_Idx const* failed_scenarios = PGM_failed_scenarios(handle);
    char const** batch_errs = PGM_batch_errors(handle);
    for (i = 0; i != n_failed_scenarios; ++i) {
        printf("Failed scenario %d, error message: %s", (int)failed_scenarios[i], batch_errs[i]);
    }
    // print normal results
    printf("Normal result:\n");
    printf("Scenario 0, u_pu: %f, u_angle: %f\n", u_pu[0], u_angle[0]);
    // clear error
    PGM_clear_error(handle);

    /**** release all the resources ****/
    // Here we need to release all the resources allocated
    // If you are using C++, you can wrap the resource returned by PGM
    //     in a smart pointer with the destroy function as custom deleter
    PGM_destroy_dataset_const(batch_update_dataset);
    PGM_destroy_buffer(load_update);
    PGM_destroy_buffer(source_update);
    PGM_destroy_options(opt);
    PGM_destroy_buffer(node_output);
    PGM_destroy_dataset_mutable(batch_output_dataset);
    PGM_destroy_dataset_mutable(single_output_dataset);
    PGM_destroy_model(model);
#ifdef _WIN32
    _aligned_free(source_input);
#else
    free(source_input);
#endif
    PGM_destroy_dataset_const(input_dataset);
    PGM_destroy_buffer(sym_load_input);
    PGM_destroy_buffer(node_input);
    PGM_destroy_handle(handle);
    return 0;
}
