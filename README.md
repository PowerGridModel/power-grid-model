<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->
[![PyPI version](https://badge.fury.io/py/power-grid-model.svg)](https://badge.fury.io/py/power-grid-model)
[![License: MIT](https://img.shields.io/badge/License-MPL2.0-informational.svg)](https://github.com/alliander-opensource/power-grid-model/blob/main/LICENSE)
[![Build and Test C++ and Python](https://github.com/alliander-opensource/power-grid-model/actions/workflows/main.yml/badge.svg)](https://github.com/alliander-opensource/power-grid-model/actions/workflows/main.yml)
[![Check Code Quality](https://github.com/alliander-opensource/power-grid-model/actions/workflows/check-code-quality.yml/badge.svg)](https://github.com/alliander-opensource/power-grid-model/actions/workflows/check-code-quality.yml)
[![REUSE Compliance Check](https://github.com/alliander-opensource/power-grid-model/actions/workflows/reuse-compliance.yml/badge.svg)](https://github.com/alliander-opensource/power-grid-model/actions/workflows/reuse-compliance.yml)

[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=alliander-opensource_power-grid-model&metric=alert_status)](https://sonarcloud.io/summary/new_code?id=alliander-opensource_power-grid-model)
[![Coverage](https://sonarcloud.io/api/project_badges/measure?project=alliander-opensource_power-grid-model&metric=coverage)](https://sonarcloud.io/summary/new_code?id=alliander-opensource_power-grid-model)
[![Maintainability Rating](https://sonarcloud.io/api/project_badges/measure?project=alliander-opensource_power-grid-model&metric=sqale_rating)](https://sonarcloud.io/summary/new_code?id=alliander-opensource_power-grid-model)
[![Reliability Rating](https://sonarcloud.io/api/project_badges/measure?project=alliander-opensource_power-grid-model&metric=reliability_rating)](https://sonarcloud.io/summary/new_code?id=alliander-opensource_power-grid-model)
[![Security Rating](https://sonarcloud.io/api/project_badges/measure?project=alliander-opensource_power-grid-model&metric=security_rating)](https://sonarcloud.io/summary/new_code?id=alliander-opensource_power-grid-model)
[![Vulnerabilities](https://sonarcloud.io/api/project_badges/measure?project=alliander-opensource_power-grid-model&metric=vulnerabilities)](https://sonarcloud.io/summary/new_code?id=alliander-opensource_power-grid-model)



# Power Grid Model

`power-grid-model` is a Python library for steady-state distribution power system analysis.
The core of the library is written in C++.
Currently, it supports the following calculations:

* Symmetric and asymmetric power flow calculation with Newton-Raphson method and linear method
* Symmetric and asymmetric state estimation with iterative linear method

# Installation

## Install from PyPI

You can directly install the package from PyPI.

```
pip install power-grid-model
```

## Build and install from Source

To install the library from source, refer to the [Build Guide](docs/build-guide.md).

# Quick Start

In this quick start a simple 10kV network as below is calculated.
A line connects two nodes. One node has a source. One node has a symmetric load.
The code in the quick start is in [quick_example.py](scripts/quick_example.py).

```
node_1 ---line_3--- node_2
 |                    |
source_5            sym_load_4
```

The library uses a graph data model to represent the physical components and their attributes,
see [Components](docs/components.md).

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
The documentation [Native Data Interface](docs/native-data-interface.md)
explains the detailed design of this interface.

The helper function `initialize_array` can be used to
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

## Instantiate Model

We can instantiate the model by calling the constructor of `PowerGridModel`

```python
model = PowerGridModel(input_data, system_frequency=50.0)
```

## Power Flow Calculation

To calculate power flow, call the method `calculate_power_flow`.
This method has many optional arguments, see [Python API Reference](docs/python-api-reference.md)
for a detailed explanation.

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

# Folder Structure

The repository folder structure is as follows. The `examples`, `docs` and `scripts` are self-explanatory.

- The C++ calculation core is inside [include/power-grid-model](include/power-grid-model).
- The python interface code is in [src/power_grid_model](src/power_grid_model)
- The code for validation of input data is in [validation](src/power_grid_model/validation) folder.
- The [tests](tests) folder is divided in the following way:
  - `cpp_unit_tests` contains the tests for the C++ calculation core.
  - `benchmark_cpp` contains a benchmark test case generator in C++.
  - `unit` folder contains tests for the python code.
  - `data` contains validation test cases designed for every component and algorithm. Some sample network types are also included. 
  The validation is either against popular power system analysis software or hand calculation.

# Examples

Please refer to [Examples](examples) for more detailed examples for power flow and state estimation. 
Notebooks for validating the input data and exporting input/output data are also included.

# License
This project is licensed under the Mozilla Public License, version 2.0 - see [LICENSE](LICENSE) for details.

# Licenses third-party libraries
This project includes third-party libraries, 
which are licensed under their own respective Open-Source licenses.
SPDX-License-Identifier headers are used to show which license is applicable. 
The concerning license files can be found in the LICENSES directory.

# Contributing
Please read [CODE_OF_CONDUCT](CODE_OF_CONDUCT.md) and [CONTRIBUTING](CONTRIBUTING.md) for details on the process 
for submitting pull requests to us.

# Contact
Please read [SUPPORT](SUPPORT.md) for how to connect and get into contact with the Power Gird Model project.
