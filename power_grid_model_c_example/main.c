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

    // create input buffer
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
