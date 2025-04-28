# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Data types involving PGM datasets.

Many data types are used throughout the power grid model project. In an attempt to clarify type hints, some types
are defined/exposed and explained.
"""

# Import non class symbols as private to reassign and provide a docstring.
# Not necessary for class, as sphinx autodoc finds their docstrings
from power_grid_model._core.data_types import (  # pylint: disable=unused-import
    AsymValue as _AsymValue,
    AttributeType as _AttributeType,
    AttributeValue as _AttributeValue,
    BatchArray as _BatchArray,
    BatchColumn as _BatchColumn,
    BatchColumnarData as _BatchColumnarData,
    BatchComponentData as _BatchComponentData,
    BatchDataset as _BatchDataset,
    BatchList as _BatchList,
    BatchPythonDataset as _BatchPythonDataset,
    ColumnarData as _ColumnarData,
    Component as _Component,
    ComponentData as _ComponentData,
    ComponentList as _ComponentList,
    DataArray as _DataArray,
    Dataset as _Dataset,
    DenseBatchArray as _DenseBatchArray,
    DenseBatchColumnarData as _DenseBatchColumnarData,
    DenseBatchData as _DenseBatchData,
    IndexPointer as _IndexPointer,
    NominalValue as _NominalValue,
    PythonDataset as _PythonDataset,
    RealValue as _RealValue,
    SingleArray as _SingleArray,
    SingleColumn as _SingleColumn,
    SingleColumnarData as _SingleColumnarData,
    SingleComponentData as _SingleComponentData,
    SingleDataset as _SingleDataset,
    SinglePythonDataset as _SinglePythonDataset,
    SparseBatchArray,
    SparseBatchColumnarData,
    SparseBatchData as _SparseBatchData,
    SparseDataComponentType as _SparseDataComponentType,
)

AsymValue = _AsymValue
"""
Asymmetrical values are three-phase values like p or u_measured.

- Example: (10400.0, 10500.0, 10600.0)
"""

AttributeType = _AttributeType
"""
An attribute type is a string reprenting the attribute type of a specific component.

- Examples:

    - "id"
    - "u_rated"
"""

AttributeValue = _AttributeValue
"""
When representing a grid as a native python structure, each attribute (u_rated etc) is either a nominal value,
a real value, or a tuple of three real values.

- Examples:

    - real: 10500.0
    - nominal: 123
    - asym: (10400.0, 10500.0, 10600.0)
"""

BatchArray = _BatchArray
"""
A batch array is a either a :class:`DenseBatchArray` or a :class:`SparseBatchArray`.
"""

BatchColumn = _BatchColumn
"""
A batch column is a two-dimensional structured numpy array containing a list of the same attribute of
multiple components of the same type. Otherwise, similar to :class:`SingleColumn`.
"""

BatchColumnarData = _BatchColumnarData
"""
Batch columnar data is either a :class:`DenseBatchColumnarData` or a :class:`SparseBatchColumnarData`.
"""

BatchComponentData = _BatchComponentData
"""
Batch component data can be :class:`BatchArray` or :class:`BatchColumnarData`.
"""

BatchDataset = _BatchDataset
"""
A batch dataset is a dictionary where the keys are the component types and the values are :class:`BatchComponentData`

- Example: {"node": :class:`DenseBatchArray`, "line": :class:`SparseBatchArray`,
            "link": :class:`DenseBatchColumnarData`, "transformer": :class:`SparseBatchColumnarData`}
"""

BatchList = _BatchList
"""
A batch list is an alternative representation of a batch. It is a list of single datasets, where each single dataset
is actually a batch. The batch list is intended as an intermediate data type, during conversions.

- Example: [:class:`SingleDataset`, {"node": :class:`SingleDataset`}]
"""

BatchPythonDataset = _BatchPythonDataset
"""
A batch dataset in native python representation is a list of dictionaries, where the keys are the component names and
the values are a list of all the instances of such a component. In essence it stores the same information as a
BatchDataset, but in a native python format, without using numpy. Actually it looks more like the BatchList.

- Example:

  [{"line": [{"id": 3, "from_status": 0, "to_status": 0, ...}],},
   {"line": [{"id": 3, "from_status": 1, "to_status": 1, ...}],}]
"""

ColumnarData = _ColumnarData
"""
Columnar data can be :class:`SingleColumnarData` or :class:`BatchColumnarData`.
"""

Component = _Component
"""
A component, when represented in native python format, is a dictionary, where the keys are the attributes and the values
are the corresponding values. It is allowed to add extra fields, containing either an AttributeValue or a string.

- Example: {"id": 1, "u_rated": 10500.0, "original_id": "Busbar #1"}
"""

ComponentData = _ComponentData
"""
Component data can be :class:`DataArray` or :class:`ColumnarData`.
"""

ComponentList = _ComponentList
"""
A component list is a list containing components. In essence it stores the same information as a np.ndarray,
but in a native python format, without using numpy.

- Example: [{"id": 1, "u_rated": 10500.0}, {"id": 2, "u_rated": 10500.0}]
"""

DataArray = _DataArray
"""
A data array can be a :class:`SingleArray` or a :class:`BatchArray`.
"""

Dataset = _Dataset
"""
A general data set can be a :class:`SingleDataset` or a :class:`BatchDataset`.

- Examples:

    - single: {"node": :class:`SingleArray`, "line": :class:`SingleColumnarData`}

    - batch: {"node": :class:`DenseBatchArray`, "line": :class:`SparseBatchArray`,
             "link": :class:`DenseBatchColumnarData`, "transformer": :class:`SparseBatchColumnarData`}

"""

DenseBatchArray = _DenseBatchArray
"""
A dense batch array is a two-dimensional structured numpy array containing a list of components of 
the same type for each scenario. Otherwise similar to :class:`SingleArray`.
"""


DenseBatchColumnarData = _DenseBatchColumnarData
"""
Batch columnar data is a dictionary where the keys are the attribute types of the same component
and the values are :class:`BatchColumn`.

- Example: {"id": :class:`AttributeType`, "from_status": :class:`BatchColumn`}
"""

DenseBatchData = _DenseBatchData
"""
Dense batch data can be a :class:`DenseBatchArray` or a :class:`DenseBatchColumnarData`.
"""

IndexPointer = _IndexPointer
"""
An index pointer is a one-dimensional numpy int64 array containing n+1 elements where n is the amount
of scenarios, representing the start and end indices for each batch scenario as follows:

    - The elements are the indices in the data that point to the first element of that scenario.
    - The last element is one after the data index of the last element of the last scenario.
    - The first element and last element will therefore be 0 and the size of the data, respectively.
"""


NominalValue = _NominalValue
"""
Nominal values can be IDs, booleans, enums, tap pos.

- Example: 123
"""

PythonDataset = _PythonDataset
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

RealValue = _RealValue
"""
Symmetrical values can be anything like cable properties, symmetric loads, etc.

- Example: 10500.0
"""

SingleArray = _SingleArray
"""
A single array is a one-dimensional structured numpy array containing a list of components of the same type.

- Examples:

    - structure: <1d-array>
    - concrete: array([(0, 10500.0), (0, 10500.0)], dtype=power_grid_meta_data["input"]["node"].dtype)
"""

SingleColumn = _SingleColumn
"""
A single column is a one-dimensional structured numpy array containing a list of the same attribute of
multiple components of the same type.

- Examples:

    - structure: <1d-array>
    - concrete:

        - array([0, 1], dtype=power_grid_meta_data["input"]["node"].dtype.fields["id"][0])
        - array([10500.0, 10500.0], dtype=power_grid_meta_data["input"]["node"].dtype.fields["u_rated"][0])
"""


SingleColumnarData = _SingleColumnarData
"""
Single columnar data is a dictionary where the keys are the attribute types of the same component
and the values are :class:`SingleColumn`.

- Example: {"id": :class:`AttributeType`, "u_rated": :class:`SingleColumn`}
"""


SingleComponentData = _SingleComponentData
"""
Single component data can be :class:`SingleArray` or :class:`SingleColumnarData`.
"""

SingleDataset = _SingleDataset
"""
A single dataset is a dictionary where the keys are the component types and the values are
:class:`ComponentData`

- Example: {"node": :class:`SingleArray`, "line": :class:`SingleColumnarData`}
"""

SinglePythonDataset = _SinglePythonDataset
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

SparseBatchData = _SparseBatchData
"""
Sparse batch data can be a :class:`SparseBatchArray` or a :class:`SparseBatchColumnarData`.
"""

SparseDataComponentType = _SparseDataComponentType
"""
A string representing the component type of sparse data structures.

Must be either "data" or "indptr".
"""
