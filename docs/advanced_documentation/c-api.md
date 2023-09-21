<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

# Use Power Grid Model through C API

While many users use Python API for Power Grid Model.
This library also provides a C API. 
The main use case of C API is to integrate Power Grid Model into a non-Python application/library, namely, C, C++, JAVA, C#, etc. 

The C API consists of some 
{{ "[header files]({}/power_grid_model_c/power_grid_model_c/include)".format(gh_link_head_blob) }}
and a dynamic library (`.so` or `.dll`) built by a 
{{ "[cmake project]({}/power_grid_model_c/power_grid_model_c/CMakeLists.txt)".format(gh_link_head_blob) }}.
Please refer to the [Build Guide](./build-guide.md) about how to build the library.

You can refer to the [C API Reference](../api_reference/power-grid-model-c-api-reference.rst)
for a detailed documentation of the API. 
Please also have a look at an 
{{ "[example]({}/power_grid_model_c_example/main.c)".format(gh_link_head_blob) }}
of C program to use this C API.

In this documentation, the main design choices and concepts of the C API are presented.

## Finding and linking the package

The package can be loaded using the Config mode of the `find_package` CMake command. An
{{ "[example project]({}/tests/package_tests/CMakeLists.txt)".format(gh_link_head_blob) }} is provided by the
{{ "[Git project]({})".format(gh_link_head_tree) }}, which is also used for testing the package.

```{note}
Since the C API is a dynamically linked library, the user is responsible for placing the library in the right location,
and making it available to their binaries, e.g. by adding its location to `PATH` or `RPATH`.
```

## Opaque Struct/Pointer

As a common C API practice, we use [opaque struct/pointer](https://en.wikipedia.org/wiki/Opaque_pointer) in the API. 
The user creates the object by `PGM_create_*` function and release the object by `PGM_destroy_*` function.
In other function calls, the user provide the relevant opaque pointer as the argument.
The real memory layout of the object is unknown to the user.
In this way, we can provide backwards API/ABI compatibility.

## Handle

During the construction and calculation of Power Grid Model, there could be errors.
Moreover, we might want to also retrieve some meta information from the calculation process.
The C API uses a handle opaque object `PGM_Handle` to store all these kinds of error messages and information.
You need to pass a handle pointer to most of the functions in the C API.

For example, after calling `PGM_create_model`, you can use `PGM_error_code` and `PGM_error_message` 
to check if there is error during the creation and the error message.

If you are calling the C API in multiple threads, each thread should have its own handle object created by `PGM_create_handle`.

## Calculation Options

To execute a power grid calculation you need to specify many options, 
e.g., maximum number of iterations, error tolerance, etc.
We could have declared all the calculation options as individual arguments in the `PGM_calculate` function.
However, due to the lack of default argument in C, 
this would mean that the C API has a breaking change everytime we add a new option,
which happends very often.

To solve this issue, we use another opaque object `PGM_Options`. The user creates an object with default options by `PGM_create_options`. You can then specify individual options by `PGM_set_*`. 
In the `PGM_calculate` function you need to pass a pointer to `PGM_Options`.
In this way, we can ensure the API backwards compatibility.
If we add a new option, it will get a default value in the `PGM_create_options` function.

## Buffer and Attributes

The biggest challenge in the design of C API is the handling of input/output/update data buffers.
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

In the C API we have a function `PGM_buffer_set_nan` which sets all the attributes in a buffer to `NaN`.
In the calculation core, if an optional attribute is `NaN`, it will use the default value.

If you just want to set some attributes and keep everything else as `NaN`, 
calling `PGM_buffer_set_nan` before you set attribute is convenient.
This is useful especially in `update` dataset because you do not always update all the mutable attributes.

### Backwards Compatibility

If you do want to set all the attributes in a component, you can skip the function call to `PGM_buffer_set_nan`.
This will provide better performance as there is no use of setting `NaN`.

However, this is at the cost of backwards compatibility.
If we add a new optional attribute for this component in the future,
the new version of the library will not be backwards compatible to your current code.
Because you do not set everything to `NaN` and you only set values to the previously known attributes.
The newly added attribute will have rubbish value in the memory.

Therefore, it is your choice of trade-off: maximum performance or backwards compatibility.

NOTE: you do not need to call `PGM_buffer_set_nan` on output buffers, 
because the buffer will be overwritten in the calculation core with the real output data.

## Dataset views

```{warning}
This feature is still under construction. Not all parts of the C API implement this feature.
```

For larger datasets that cannot or should not be treated independently,
`PGM_dataset_*` interfaces are provided.
Currently implemented are `PGM_dataset_const` and `PGM_dataset_writable`.
These two dataset types expose a dataset to the power-grid-model with the following permissions on buffers:

| Dataset interface        | power-grid-model permissions | User permissions    | Treat as        |
| ------------------------ | ---------------------------- | ------------------- | --------------- |
| `PGM_dataset_const_*`    | Read                         | Create, read, write | `const * const` |
| `PGM_dataset_writable_*` | Read, write                  | Read                | `* const`       |

A constant dataset is completely user-owned.
The user is responsible for creating and destroying both the dataset and its buffers.
This dataset is perfect for input and update datasets.

A writable dataset, instead, cannot be created by the user, but will be provided by the deserializer.
The user can then provide buffers to which the deserializer can write its data (and `indptr`).
This allows the buffers to have lifetimes beyond the lifetime of the deserializer.
This dataset type is only meant to be used for providing user buffers to the deserializer.
