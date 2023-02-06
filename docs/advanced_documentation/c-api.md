<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

# Use Power Grid Model through C-API

While many users use Python API for Power Grid Model.
This library also provides a C-API. 
The main use case of C-API is to integrate Power Grid Model into a non-Python application/library, namely, C, C++, JAVA, C#, etc. 

The C-API consists of a single 
{{ "[header file]({}/include/power_grid_model_c.h)".format(gh_link_head_blob) }}
and a dynamic library (`.so` or `.dll`) built by a 
{{ "[cmake project]({}/power_grid_model_c/CMakeLists.txt)".format(gh_link_head_blob) }}.
Please refer to the [Build Guide](./build-guide.md) about how to build the library.

You can refer to the [C-API Reference](../api_reference/power-grid-model-c-api-reference.rst)
for a detailed documentation of the API. 
Please also have a look at an 
{{ "[example]({}/power_grid_model_c_example/main.c)".format(gh_link_head_blob) }}
of C program to use this C-API.

In this documentation, the main design choices and concepts of the C-API are presented.

## Opaque Struct/Pointer

As a common C-API practice, we use [opaque struct/pointer](https://en.wikipedia.org/wiki/Opaque_pointer) in the API. 
The user creates the object by `PGM_create_*` function and release the object by `PGM_destroy_*` function.
In other function calls, the user provide the relevant opaque pointer as the argument.
The real memory layout of the object is unknown to the user.
In this way, we can provide backwards API/ABI compatibility.

## Handle

During the construction and calculation of Power Grid Model, there could be errors.
Moreover, we might want to also retrieve some meta information from the calculation process.
The C-API uses a handle opaque object `PGM_Handle` to store all these kinds of error messages and information.
You need to pass a handle pointer to most of the functions in the C-API.

For example, after calling `PGM_create_model`, you can use `PGM_err_code` and `PGM_err_msg` 
to check if there is error during the creation and the error message.

If you are calling the C-API in multiple threads, each thread should have its own handle object created by `PGM_create_handle`.

## Calculation Options

To execute a power grid calculation you need to specify many options, 
e.g., maximum number of iterations, error tolerance, etc.
We could have declared all the calculation options as individual arguments in the `PGM_calculate` function.
However, due to the lack of default argument in C, 
this would mean that the C-API has a breaking change everytime we add a new option,
which happends very often.

To solve this issue, we use another opaque object `PGM_Options`. The user creates an object with default options by `PGM_create_options`. You can then specify individual options by `PGM_set_*`. 
In the `PGM_calculate` function you need to pass a pointer to `PGM_Options`.
In this way, we can ensure the API backwards compatibility.
If we add a new option, it will get a default value in the `PGM_create_options` function.

## Buffer and Attributes
