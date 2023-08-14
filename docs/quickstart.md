<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->
# Quick Start

In this quick start a simple 10kV network as below is calculated.
A line connects two nodes. One node has a source. One node has a symmetric load.
The code in the quick start is in {{ "[quick_example.py]({}/scripts/quick_example.py)".format(gh_link_head_blob) }}.

```
node_1 ---line_3--- node_2
 |                    |
source_5            sym_load_4
```

The library uses a graph data model to represent the physical components and their attributes,
see [Graph Data Model](user_manual/data-model).

Firstly, import the main model class
as well as some helper functions for enumerations and meta data.

```python
from power_grid_model import LoadGenType
from power_grid_model import PowerGridModel
from power_grid_model import initialize_array
```

## Input Data

The library uses dictionary of
[numpy structured arrays](https://numpy.org/doc/stable/user/basics.rec.html)
as the main (input and output) data exchange format between Python and C++ core.
The documentation [Native Data Interface](advanced_documentation/native-data-interface)
explains the detailed design of this interface.

The helper function {py:class}`power_grid_model.initialize_array` can be used to
easily generate an array of the correct format.

```python
# node
node = initialize_array('input', 'node', 2)
node['id'] = [1, 2]
node['u_rated'] = [10.5e3, 10.5e3]
```

The code above generates a node input array with two nodes,
and assigns the attributes of the nodes to the array.
Similarly, we can create input arrays for line, load, and generation.
A dictionary of such arrays is used for `input_data` and `update_data`. 

```python
# line
line = initialize_array('input', 'line', 1)
line['id'] = [3]
line['from_node'] = [1]
line['to_node'] = [2]
line['from_status'] = [1]
line['to_status'] = [1]
line['r1'] = [0.25]
line['x1'] = [0.2]
line['c1'] = [10e-6]
line['tan1'] = [0.0]
line['i_n'] = [1000]
# load
sym_load = initialize_array('input', 'sym_load', 1)
sym_load['id'] = [4]
sym_load['node'] = [2]
sym_load['status'] = [1]
sym_load['type'] = [LoadGenType.const_power]
sym_load['p_specified'] = [2e6]
sym_load['q_specified'] = [0.5e6]
# source
source = initialize_array('input', 'source', 1)
source['id'] = [5]
source['node'] = [1]
source['status'] = [1]
source['u_ref'] = [1.0]
# all
input_data = {
    'node': node,
    'line': line,
    'sym_load': sym_load,
    'source': source
}
```

Another example of how to create components can also be found 
in [Input data](ex_input_data).

```{note}
The keys of the dictonary of arrays are unique and should match with the respective `type name` of the component. 
(Eg. See that type name of {hoverxref}`user_manual/components:node` is `node`)
```

## Instantiate Model

We can instantiate the model by calling the constructor of {py:class}`power_grid_model.PowerGridModel`

```python
model = PowerGridModel(input_data, system_frequency=50.0)
```

## Power Flow Calculation

To perform calculations, use the object methods {py:class}`power_grid_model.PowerGridModel.calculate_power_flow` 
or {py:class}`power_grid_model.PowerGridModel.calculate_state_estimation` functions. 
Refer [Calculations](user_manual/calculations) for more details on the many optional arguments.

```python
result = model.calculate_power_flow()
```

Both input and output data are dictionaries of structured `numpy` arrays.
We can use `pandas` to convert them to data frames and print them.

```python
print('Node Input')
print(pd.DataFrame(input_data['node']))
print('Node Result')
print(pd.DataFrame(result['node']))
```

You can print the data in tables.

```
Node Input
   id  u_rated
0   1  10500.0
1   2  10500.0
Node Result
   id  energized      u_pu             u   u_angle
0   1          1  0.999964  10499.619561 -0.000198
1   2          1  0.994801  10445.415523 -0.003096
```

## Validation

To validate the `input_data` and `update_data` for valid values, use {py:class}`power_grid_model.validation.validate_input_data` and {py:class}`power_grid_model.validation.validate_batch_data`. Refer [Data Validator](user_manual/data-validator) for more details

## Batch Data

Optionally, we can add batch scenarios using `update_data` argument. 
The code below initializes a symmetric load update array with a shape of `(5, 4)`. This is in the form of $$\text{number of batches} \times \text{number of components}$$)
The `update_data` is an optional argument to the {py:class}`power_grid_model.PowerGridModel` object or the calculation functions.

```python
from power_grid_model import initialize_array

sym_load_update = initialize_array('update', 'sym_load', (5, 4))
# --- Add other sym_load attributes --- 
update_data = {
    "sym_load": sym_load_update
}

model = PowerGridModel(input_data=input_data, update_data=update_data)
```