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

The biggest challenge in the design of C-API is the handling of input/output/update data buffers.
We define the following concepts in the data hierarchy:

* Dataset: a collection of data buffers for a given purpose. 
At this moment, we have four dataset types: `input`, `update`, `sym_output`, `asym_output`.
* Component: a homogeneous data buffer for a component in our [data model](../user_manual/components.md), e.g., `node`.
* Attribute: a property of given component. For example, `u_rated` attribute of `node` is the rated voltage of the node.

### Create and Destroy Buffer

Data buffers are almost always allocated and freed in the heap. We provide two ways of doing so.

* You can use the function `PGM_create_buffer` and `PGM_destroy_buffer` to create and destroy buffer.
In this way, the library is handling the memory (de-)allocation.
* You can call some memory (de-)allocation function in your own code according to your platform, 
e.g., `aligned_alloc` and `free`.
You need to first call `PGM_meta_*` functions to retrieve the size and alignment of a component.

NOTE: Do not mix these two methods in creation and destruction.
You cannot use `PGM_destroy_buffer` to release a buffer created in your own code, or vice versa.

### Set and Get Attribute

Once you have the data buffer, you need to set or get attributes. We provide two ways of doing so.

* You can use the function `PGM_buffer_set_value` and `PGM_buffer_get_value` to get and set values.
* You can do pointer cast directly on the buffer pointer, by shifting the pointer to proper offset
and cast it to a certain value type. 
You need to first call `PGM_meta_*` functions to retrieve the correct offset.

Pointer cast is generally more efficient and flexible because you are not calling into the 
dynamic library everytime. But it requires the user to retrieve the offset information first.
Using the buffer helper function is more convenient but with some overhead.

### Set NaN Function

In the C-API we have a function `PGM_buffer_set_nan` which sets all the attributes in a buffer to `NaN`.
In the calculation core, if an optional attribute is `NaN`, it will use the default value.
