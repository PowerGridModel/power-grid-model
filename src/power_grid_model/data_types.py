# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0
"""
Many data types are used throughout the power grid model project. In an attempt to clarify type hints, some types
have been defined and explained in this file
"""

from typing import Any, Dict, List, Tuple, Union

import numpy as np

# When we're dropping python 3.8, we should introduce
#  SingleArray = np.ndarray (ndim=1)
#  DenseBatchArray = np.ndarray (ndim=2)

SparseBatchArray = Dict[str, np.ndarray]
"""
A sparse batch array is a dictionary containing the keys "indptr" and "data".
    indptr: a one-dimensional numpy int32 array
    data: a one-dimensional structured numpy array. The exact dtype depends on the type component.

Example: {"indptr": <1d-array>, "data": <1d-array>}
"""

BatchArray = Union[np.ndarray, SparseBatchArray]
"""
A batch is a either a dense or a sparse batch array

Examples:
    dense:  <2d-array>
    sparse: {"indptr": <1d-array>, "data": <1d-array>}
"""

SingleDataset = Dict[str, np.ndarray]
"""
A single dataset is a dictionary where the keys are the component types and the values are one-dimensional
structured numpy arrays.

Example: {"node": <1d-array>, "line": <1d-array>}
"""

BatchDataset = Dict[str, BatchArray]
"""
A batch dataset is a dictionary where the keys are the component types and the values are either two-dimensional
structured numpy arrays (dense batch array) or dictionaries with an indptr and a one-dimensional structured numpy
array (sparse batch array).

Example: {"node": <2d-array>, "line": {"indptr": <1d-array>, "data": <1d-array>}}
"""

Dataset = Union[SingleDataset, BatchDataset]
"""
A general data set can be a single or a batch dataset.

Examples:
    single: {"node": <1d-array>, "line": <1d-array>}
    batch:  {"node": <2d-array>, "line": {"indptr": <1d-array>, "data": <1d-array>}}

"""

BatchList = List[SingleDataset]
"""
A batch list is an alternative representation of a batch. It is a list of single datasets, where each single dataset
is actually a batch. The batch list is intended as an intermediate data type, during conversions.

Example: [{"node": <1d-array>, "line": <1d-array>}, {"node": <1d-array>, "line": <1d-array>}]
"""

Nominal = int
"""
Nominal values can be IDs, booleans, enums, tap pos

Example: 123
"""

RealValue = float
"""
Symmetrical values can be anything like cable properties, symmetric loads, etc.

Example: 10500.0
"""

AsymValue = Tuple[RealValue, RealValue, RealValue]
"""
Asymmetrical values are three-phase values like p or u_measured.

Example: (10400.0, 10500.0, 10600.0)
"""

AttributeValue = Union[Nominal, RealValue, AsymValue]
"""
When representing a grid as a native python structure, each attribute (u_rated etc) is either a nominal value,
a real value, or a tuple of three real values.

Examples:
    nominal: 123
    real:    10500.0
    asym:    (10400.0, 10500.0, 10600.0)

"""

Component = Dict[str, AttributeValue]
"""
A component, when represented in native python format, is a dictionary, where the keys are the attributes and the values
are the corresponding values.

Example: {"id": 1, "u_rated": 10500.0}
"""

ComponentList = List[Component]
"""
A component list is a list containing components. In essence it stores the same information as a np.ndarray,
but in a native python format, without using numpy.

Example: [{"id": 1, "u_rated": 10500.0}, {"id": 2, "u_rated": 10500.0}]
"""

SinglePythonDataset = Dict[str, ComponentList]
"""
A single dataset in native python representation is a dictionary, where the keys are the component names and the
values are a list of all the instances of such a component. In essence it stores the same information as a
SingleDataset, but in a native python format, without using numpy.

Example:
    {
        "node": [{"id": 1, "u_rated": 10500.0}, {"id": 2, "u_rated": 10500.0}],
        "line": [{"id": 3, "from_node": 1, "to_node": 2, ...}],
    }
"""

BatchPythonDataset = List[SinglePythonDataset]
"""
A batch dataset in native python representation is a list of dictionaries, where the keys are the component names and
the values are a list of all the instances of such a component. In essence it stores the same information as a
BatchDataset, but in a native python format, without using numpy. Actually it looks more like the BatchList.

Example:
    [
        {
            "line": [{"id": 3, "from_status": 0, "to_status": 0, ...}],
        },
        {
            "line": [{"id": 3, "from_status": 1, "to_status": 1, ...}],
        }
    ]
"""

PythonDataset = Union[SinglePythonDataset, BatchPythonDataset]
"""
A general python data set can be a single or a batch python dataset.

Examples:
    single:
        {
            "node": [{"id": 1, "u_rated": 10500.0}, {"id": 2, "u_rated": 10500.0}],
            "line": [{"id": 3, "from_node": 1, "to_node": 2, ...}],
        }
    batch:
        [
            {
                "line": [{"id": 3, "from_status": 0, "to_status": 0, ...}],
            },
            {
                "line": [{"id": 3, "from_status": 1, "to_status": 1, ...}],
            }
        ]
"""

ExtraInfo = Dict[int, Any]
"""
Extra info is a dictionary that contains information about the objects. It is indexed on the object IDs and the
actual information can be anything.

Example:
    {
        1: "First Node",
        2: "Second Node",
        3: {"name": "Cable", "material": "Aluminum"}
    }
"""
