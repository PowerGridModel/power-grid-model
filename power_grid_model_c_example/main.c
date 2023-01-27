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

#include <stdio.h>

#include "power_grid_model_c.h"

int main(int argc, char** argv) {
    (void)(argc);
    (void)(argv);
    printf("%s", "This is an example to call the C-API of Power grid Model.\n");

    // create handle
    PGM_Handle* handle = PGM_create_handle();

    // release all the resources
    PGM_destroy_handle(handle);

    return 0;
}
