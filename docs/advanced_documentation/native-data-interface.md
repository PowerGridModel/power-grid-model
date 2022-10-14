<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

# Native Data Interface

The calculation core of `power-grid-model` is written in C++.
To interface with Python side, a format is needed to exchange the input/output data
between Python and native C++ compiled code.
This library chooses dictionary of
[numpy structured arrays](https://numpy.org/doc/stable/user/basics.rec.html) as the data format.
Each entry in the dictionary represents one type of components:
the key is the component type, the value is a `numpy` structured array.
Each element in the array represents one single physical component.

## Structured Array

To use the component type `node` as an example, the input data of a `node` is defined in C++ as follows
(not exactly the definition in real code, only a demonstration example).
A node input data contains two attributes: `id` and `u_rated`.

```C++
struct NodeInput {
    int32_t id;
    double u_rated;
};
```

One can create a `std::vector<NodeInput>` to hold input dataset for multiple nodes.
In the example below a node input dataset is created with two nodes.
The `id` is 1 and 2, and the attribute `u_rated` is 150 kV and 10 kV.

```c++
std::vector<NodeInput> node_input{ { 1, 150e3 }, { 2, 10e3 } };
```

In the Python side, we create a `numpy` structured array with exactly the same memory layout.
We specify the same attributes with the same data types and memory offset.

```python
import numpy as np
node_dtype = np.dtype(
    {
        'names': ['id', 'u_rated'],
        'formats': ['<i4', '<f8'],  # little endian in x86-64
        'offsets': [0, 8],
        'itemsize': 16,
        'aligned': True
    }
)
```

To recreate the same node input dataset, we just create a `numpy` array using this special defined `dtype`.
The `numpy` array has exactly the same data layout as the `std::vector<NodeInput>` above.


```python
node = np.empty(shape=2, dtype=node_dtype)
node['id'] = [1, 2]
node['u_rated'] = [150e3, 10e3]
```

We further save this array into a dictionary.
With other types of components, the dictionary is a valid input dataset for the constructor of `PowerGridModel`,
see [Python API Reference](../api_reference/python-api-reference.md).

```python
input_data = {'node': node}
```

In the `cython` wrapper the pointers to all the array data will be retrieved and passed to the C++ code.
This is also true for result dataset.
The memory block of result dataset is allocated using `numpy`.
The pointers are passed into C++ code so that the C++ program can write results into those memory blocks.

## Basic Data Types

The basic data types used in the interface between C++ and Python are shown in the table below.

| C++ type     | `numpy.dtype` | null value                                          |  usage   |
| ---          | ---           | ---                                                 | ---      |
| `int32_t`    | `'i4'`        | - 2^31                                              | ids of physical components |
| `int8_t`     | `'i1'`        | - 2^7                                               | enumeration types, boolean types, and small integers (e.g. tap position of transformer) |
| `double`     | `'f8'`        | NaN ([IEEE 754](https://en.wikipedia.org/wiki/NaN)) | physical quantities (e.g. voltage) |
| `double[3]`  | `'(3, )f8'`   | NaN ([IEEE 754](https://en.wikipedia.org/wiki/NaN)) | three-phase asymmetric physical quantities (e.g. voltage) |

*The [endianness](https://en.wikipedia.org/wiki/Endianness)
of C++ and Python side is also matched. For `x86-64` platform the little endian is used, so that
the `numpy.dtype` is `'<i4'` for `int32_t` in C++.*

In the input and update dataset there are some optional attributes.
Therefore, for each data type, an entry has to be reserved for null value.
When the C++ code sees the null value in an attribute,
it will treat this attribute (of the relevant component) as not available.
For example, the following update dataset will set `from_status` of the line #5 to `0`,
and keep the `to_status` as unchanged (change not available).

```python
from power_grid_model import power_grid_meta_data
import numpy as np

line_update = np.empty(shape=1, dtype=power_grid_meta_data['update']['line']['dtype'])
line_update['id'] = [5]
line_update['from_status'] = [0]
line_update['to_status'] = [-128]
```

### Enumerations

All the enumeration types are defined as 8-bit signed integer as underlying type:
`int8_t` in C++ and `'i1'` in Python.
The enumerations are defined in the Python module `power_grid_model.enum`
In C++ the enumeration is defined with the same integer values.
Please refer the [Enum](../api_reference/python-api-reference.md#enum) for list of enumerations.


## Meta-data Helper Module

Carefully mapping between the C++ `struct` and `numpy.dtype`
is needed to have exactly the same memory layout.
Fortunately, the user do not have to worry about this.
The module `power_grid_model.power_grid_meta_data`
retrieves the exact memory layout of all the input/update/output dataset from C++
and predefines all the corresponding `numpy.dtype`.
The detailed explanation of all attributes of each component is given in
[Graph Data model](../user_manual/data-model.md#attributes-of-components).


One can import the `power_grid_meta_data` to get all the predefined `numpy.dtype` and create relevant arrays.
The code below creates an array which is compatible with transformer input dataset.

```python
from power_grid_model import power_grid_meta_data

transformer = np.empty(shape=5, dtype=power_grid_meta_data['input']['transformer']['dtype'])
```

Furthermore, there is an even more convenient function `initialize_array`
to directly create a `numpy` array with specified data type,
and initialize all the values to null value as above.
So you only have to assign meaningful values into the array.
See [Python API Reference](../api_reference/python-api-reference.md) for detailed documentation.

In the code below, a line update dataset is created. It sets the `from_status` of all the lines to `1`,
but leave the `to_status` unchanged (it will have the null value `-128`).

```python
from power_grid_model import initialize_array

line_update = initialize_array('update', 'line', 5)
line_update['id'] = [1, 2, 3, 4, 5]
line_update['from_status'] = 1
```
