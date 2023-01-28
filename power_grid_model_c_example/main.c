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

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "power_grid_model_c.h"

int main(int argc, char** argv) {
    (void)(argc);
    (void)(argv);
    printf("%s", "This is an example to call the C-API of Power Grid Model.\n");

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
    PGM_ID node = 1;
    int8_t status = 1;
    double u_ref = 1.0;
    double sk = 1e6;  // 1 MVA short circuit capacity
    PGM_buffer_set_value(handle, "input", "source", "id", source_input, &source_id, 1, -1);
    PGM_buffer_set_value(handle, "input", "source", "node", source_input, &node, 1, -1);
    PGM_buffer_set_value(handle, "input", "source", "status", source_input, &status, 1, -1);
    PGM_buffer_set_value(handle, "input", "source", "u_ref", source_input, &u_ref, 1, -1);
    PGM_buffer_set_value(handle, "input", "source", "sk", source_input, &sk, 1, -1);

    // sym_load attribute, we use helper function
    PGM_ID sym_load_id[] = {2, 3};
    int8_t load_type = 0;  // const power
    PGM_buffer_set_value(handle, "input", "source", "id", sym_load_input, sym_load_id, 2, -1);
    // 
    PGM_buffer_set_value(handle, "input", "source", "id", sym_load_input, sym_load_id, 2, -1);


    // release all the resources
    PGM_destroy_handle(handle);
    PGM_destroy_buffer(node_input);
    PGM_destroy_buffer(sym_load_input);
#ifdef _WIN32
    _aligned_free(source_input);
#else
    free(source_input);
#endif

    return 0;
}
