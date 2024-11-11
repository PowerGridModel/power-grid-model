# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Data types involving PGM datasets.

Many data types are used throughout the power grid model project. In an attempt to clarify type hints, some types
have been defined and explained in this file.
"""

from typing import TypeAlias, TypedDict, TypeVar

import numpy as np

from power_grid_model._core.dataset_definitions import ComponentTypeVar

AttributeType: TypeAlias = str
"""
An attribute type is a string reprenting the attribute type of a specific component.

- Examples:

    - "id"
    - "u_rated"
"""

SparseDataComponentType: TypeAlias = str
"""
A string representing the component type of sparse data structures.

Must be either "data" or "indptr".
"""

SingleArray: TypeAlias = np.ndarray
"""
A single array is a one-dimensional structured numpy array containing a list of components of the same type.

- Examples:

    - structure: <1d-array>
    - concrete: array([(0, 10500.0), (0, 10500.0)], dtype=power_grid_meta_data["input"]["node"].dtype)
"""

SingleColumn: TypeAlias = np.ndarray
"""
A single column is a one-dimensional structured numpy array containing a list of the same attribute of
multiple components of the same type.

- Examples:

    - structure: <1d-array>
    - concrete:

        - array([0, 1], dtype=power_grid_meta_data["input"]["node"].dtype.fields["id"][0])
        - array([10500.0, 10500.0], dtype=power_grid_meta_data["input"]["node"].dtype.fields["u_rated"][0])
"""

SingleColumnarData = dict[AttributeType, SingleColumn]
"""
Single columnar data is a dictionary where the keys are the attribute types of the same component
and the values are :class:`SingleColumn`.

- Example: {"id": :class:`AttributeType`, "u_rated": :class:`SingleColumn`}
"""

DenseBatchArray: TypeAlias = np.ndarray
"""
A dense batch array is a two-dimensional structured numpy array containing a list of components of 
the same type for each scenario. Otherwise similar to :class:`SingleArray`.
"""

BatchColumn: TypeAlias = np.ndarray
"""
A batch column is a two-dimensional structured numpy array containing a list of the same attribute of
multiple components of the same type. Otherwise, similar to :class:`SingleColumn`.
"""

DenseBatchColumnarData = dict[AttributeType, BatchColumn]
"""
Batch columnar data is a dictionary where the keys are the attribute types of the same component
and the values are :class:`BatchColumn`.

- Example: {"id": :class:`AttributeType`, "from_status": :class:`BatchColumn`}
"""

IndexPointer: TypeAlias = np.ndarray
"""
An index pointer is a one-dimensional numpy int64 array containing n+1 elements where n is the amount
of scenarios, representing the start and end indices for each batch scenario as follows:

    - The elements are the indices in the data that point to the first element of that scenario.
    - The last element is one after the data index of the last element of the last scenario.
    - The first element and last element will therefore be 0 and the size of the data, respectively.
"""


class SparseBatchArray(TypedDict):
    """
    A sparse batch array is a dictionary containing the keys `indptr` and `data`.

    - data: a :class:`SingleArray`. The exact dtype depends on the type of component.
    - indptr: an :class:`IndexPointer` representing the start and end indices for each batch scenario.

    - Examples:

        - structure: {"indptr": :class:`IndexPointer`, "data": :class:`SingleArray`}
        - concrete example: {"indptr": [0, 2, 2, 3], "data": [(0, 1, 1), (1, 1, 1), (0, 0, 0)]}

            - the scenario 0 sets the statuses of components with ids 0 and 1 to 1
              (and keeps defaults for other components)
            - scenario 1 keeps the default values for all components
            - scenario 2 sets the statuses of component with id 0 to 0 (and keeps defaults for other components)
    """

    indptr: IndexPointer
    data: SingleArray


class SparseBatchColumnarData(TypedDict):
    """
    Sparse batch columnar data is a dictionary containing the keys `indptr` and `data`.

    - data: a :class:`SingleColumnarData`. The exact supported attribute columns depend on the component type.
    - indptr: an :class:`IndexPointer` representing the start and end indices for each batch scenario.

    - Examples:

        - structure: {"indptr": :class:`IndexPointer`, "data": :class:`SingleColumnarData`}
        - concrete example: {"indptr": [0, 2, 2, 3], "data": {"id": [0, 1, 0], "status": [1, 1, 0]}}

            - the scenario 0 sets the status of components with ids 0 and 1 to 1
              (and keeps defaults for other components)
            - scenario 1 keeps the default values for all components
            - scenario 2 sets the status of component with id 0 to 0 (and keeps defaults for other components)
    """

    indptr: IndexPointer
    data: SingleColumnarData


DenseBatchData = DenseBatchArray | DenseBatchColumnarData
"""
Dense batch data can be a :class:`DenseBatchArray` or a :class:`DenseBatchColumnarData`.
"""

SparseBatchData = SparseBatchArray | SparseBatchColumnarData
"""
Sparse batch data can be a :class:`SparseBatchArray` or a :class:`SparseBatchColumnarData`.
"""

BatchArray = DenseBatchArray | SparseBatchArray
"""
A batch array is a either a :class:`DenseBatchArray` or a :class:`SparseBatchArray`.
"""

BatchColumnarData = DenseBatchColumnarData | SparseBatchColumnarData
"""
Batch columnar data is either a :class:`DenseBatchColumnarData` or a :class:`SparseBatchColumnarData`.
"""

DataArray = SingleArray | BatchArray
"""
A data array can be a :class:`SingleArray` or a :class:`BatchArray`.
"""

ColumnarData = SingleColumnarData | BatchColumnarData
"""
Columnar data can be :class:`SingleColumnarData` or :class:`BatchColumnarData`.
"""

_SingleComponentData = TypeVar("_SingleComponentData", SingleArray, SingleColumnarData)  # deduction helper
SingleComponentData = SingleArray | SingleColumnarData
"""
Single component data can be :class:`SingleArray` or :class:`SingleColumnarData`.
"""

_BatchComponentData = TypeVar("_BatchComponentData", BatchArray, BatchColumnarData)  # deduction helper
BatchComponentData = BatchArray | BatchColumnarData
"""
Batch component data can be :class:`BatchArray` or :class:`BatchColumnarData`.
"""

_ComponentData = TypeVar("_ComponentData", SingleComponentData, BatchComponentData)  # deduction helper
ComponentData = DataArray | ColumnarData
"""
Component data can be :class:`DataArray` or :class:`ColumnarData`.
"""

SingleDataset = dict[ComponentTypeVar, _SingleComponentData]
"""
A single dataset is a dictionary where the keys are the component types and the values are
:class:`ComponentData`

- Example: {"node": :class:`SingleArray`, "line": :class:`SingleColumnarData`}
"""

BatchDataset = dict[ComponentTypeVar, _BatchComponentData]
"""
A batch dataset is a dictionary where the keys are the component types and the values are :class:`BatchComponentData`

- Example: {"node": :class:`DenseBatchArray`, "line": :class:`SparseBatchArray`,
            "link": :class:`DenseBatchColumnarData`, "transformer": :class:`SparseBatchColumnarData`}
"""

Dataset = dict[ComponentTypeVar, _ComponentData]
"""
A general data set can be a :class:`SingleDataset` or a :class:`BatchDataset`.

- Examples:

    - single: {"node": :class:`SingleArray`, "line": :class:`SingleColumnarData`}

    - batch: {"node": :class:`DenseBatchArray`, "line": :class:`SparseBatchArray`,
             "link": :class:`DenseBatchColumnarData`, "transformer": :class:`SparseBatchColumnarData`}

"""

BatchList = list[SingleDataset]
"""
A batch list is an alternative representation of a batch. It is a list of single datasets, where each single dataset
is actually a batch. The batch list is intended as an intermediate data type, during conversions.

- Example: [:class:`SingleDataset`, {"node": :class:`SingleDataset`}]
"""

NominalValue = int
"""
Nominal values can be IDs, booleans, enums, tap pos.

- Example: 123
"""

RealValue = float
"""
Symmetrical values can be anything like cable properties, symmetric loads, etc.

- Example: 10500.0
"""

AsymValue = tuple[RealValue, RealValue, RealValue]
"""
Asymmetrical values are three-phase values like p or u_measured.

- Example: (10400.0, 10500.0, 10600.0)
"""

AttributeValue = RealValue | NominalValue | AsymValue
"""
When representing a grid as a native python structure, each attribute (u_rated etc) is either a nominal value,
a real value, or a tuple of three real values.

- Examples:

    - real: 10500.0
    - nominal: 123
    - asym: (10400.0, 10500.0, 10600.0)
"""

Component = dict[AttributeType, AttributeValue | str]
"""
A component, when represented in native python format, is a dictionary, where the keys are the attributes and the values
are the corresponding values. It is allowed to add extra fields, containing either an AttributeValue or a string.

- Example: {"id": 1, "u_rated": 10500.0, "original_id": "Busbar #1"}
"""

ComponentList = list[Component]
"""
A component list is a list containing components. In essence it stores the same information as a np.ndarray,
but in a native python format, without using numpy.

- Example: [{"id": 1, "u_rated": 10500.0}, {"id": 2, "u_rated": 10500.0}]
"""

SinglePythonDataset = dict[ComponentTypeVar, ComponentList]
"""
A single dataset in native python representation is a dictionary, where the keys are the component names and the
values are a list of all the instances of such a component. In essence it stores the same information as a
SingleDataset, but in a native python format, without using numpy.

- Example:

  {
    "node": [{"id": 1, "u_rated": 10500.0}, {"id": 2, "u_rated": 10500.0}], 
    "line": [{"id": 3, "from_node": 1, "to_node": 2, ...}],
  }
"""

BatchPythonDataset = list[SinglePythonDataset]
"""
A batch dataset in native python representation is a list of dictionaries, where the keys are the component names and
the values are a list of all the instances of such a component. In essence it stores the same information as a
BatchDataset, but in a native python format, without using numpy. Actually it looks more like the BatchList.

- Example:

  [{"line": [{"id": 3, "from_status": 0, "to_status": 0, ...}],},
   {"line": [{"id": 3, "from_status": 1, "to_status": 1, ...}],}]
"""

PythonDataset = SinglePythonDataset | BatchPythonDataset
"""
A general python data set can be a single or a batch python dataset.

- Examples:

  - single:

    {
      "node": [{"id": 1, "u_rated": 10500.0}, {"id": 2, "u_rated": 10500.0}],
      "line": [{"id": 3, "from_node": 1, "to_node": 2, ...}],
    }

  - batch:

    [{"line": [{"id": 3, "from_status": 0, "to_status": 0, ...}],},
     {"line": [{"id": 3, "from_status": 1, "to_status": 1, ...}],}]
"""
