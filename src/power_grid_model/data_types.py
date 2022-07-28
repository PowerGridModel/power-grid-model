# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0
"""
Many data types are used throughout the power grid model project. In an attempt to clarify type hints, some types
have been defined and explained in this file
"""

from typing import Any, Dict, List, Tuple, TypeAlias, Union

import numpy as np

SingleArray: TypeAlias = np.ndarray[Any, Any]
"""
A single array is a one-dimensional structured numpy array. The exact dtype depends on the type of component.
"""

DenseBatchArray: TypeAlias = np.ndarray[Any, Any]
"""
A dense batch array is a a two-dimensional structured numpy array. The exact dtype depends on the type of component.
"""

SparseBatchArray: TypeAlias = dict[str, np.ndarray]
"""
A sparse batch array is a dictionary containing the keys "indptr" and "data".
    indptr: a one-dimensional numpy int32 array
    data: a one-dimensional structured numpy array. The exact dtype depends on the type component.
"""

BatchArray: TypeAlias = Union[DenseBatchArray, SparseBatchArray]
"""
A batch is a either a dense or a sparse batch array
"""

SingleDataset: TypeAlias = Dict[str, SingleArray]
"""
A single dataset is a dictionary where the keys are the component types and the values are one-dimensional
structured numpy arrays, e.g.: {"node": <1d-array>, "line": <1d-array>}
"""

BatchDataset: TypeAlias = Dict[str, BatchArray]
"""
A batch dataset is a dictionary where the keys are the component types and the values are either two-dimensional
structured numpy arrays (dense batch array) or dictionaries with an indptr and a one-dimensional structured numpy
array (sparse batch array) e.g.: {"node": <2d-array>, "line": {"indptr": <1d-array>, "data": <1d-array>}}
"""

BatchList: TypeAlias = List[SingleDataset]
"""
A batch list is an alternative representation of a batch. It is a list of single datasets, where each single dataset
is actually a batch. The batch list is intended as an intermediate data type, during conversions.
"""

Dataset: TypeAlias = Union[SingleDataset, BatchDataset]
"""
A general data set can be a single or a batch dataset.
"""

Nominal: TypeAlias = int
"""
Nominal values can be IDs, booleans, enums, tap pos
"""

RealValue: TypeAlias = float
"""
Symmetrical values can be anything like cable properties, symmetric loads, etc.
"""

AsymValue: TypeAlias = Tuple[RealValue, RealValue, RealValue]
"""
Asymmetrical values are three-phase values like p or u_measured.
"""

AttributeValue: TypeAlias = Union[Nominal, RealValue, AsymValue]
"""
When representing a grid as a native python structure, each attribute (u_rated etc) is either a nominal value,
a real value, or a tuple of three real values.
"""

Component: TypeAlias = Dict[str, AttributeValue]
"""
A component, when represented in native python format, is a dictionary, where the keys are the attributes and the values
are the corresponding values.
"""

ComponentList: TypeAlias = List[Component]
"""
A component list is a list containing components. In essence it stores the same information as a SingleArray,
but in a native python format, without using numpy.
"""

SinglePythonDataset: TypeAlias = Dict[str, ComponentList]
"""
A single dataset in native python representation is a dictionary, where the keys are the component names and the
values are a list of all the instances of such a component. In essence it stores the same information as a
SingleDataset, but in a native python format, without using numpy.
"""

BatchPythonDataset: TypeAlias = List[SinglePythonDataset]
"""
A batch dataset in native python representation is a list of dictionaries, where the keys are the component names and
the values are a list of all the instances of such a component. In essence it stores the same information as a
BatchDataset, but in a native python format, without using numpy. Actually it looks more like the BatchList.
"""

PythonDataset: TypeAlias = Union[SinglePythonDataset, BatchPythonDataset]
"""
A general python data set can be a single or a batch python dataset.
"""

ExtraInfo: TypeAlias = Dict[int, Any]
"""
Extra info is a dictionary that contains information about the objects. It is indexed on the object IDs and the
actual information can be anything.
"""
