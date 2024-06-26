# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Many data types are used throughout the power grid model project. In an attempt to clarify type hints, some types
have been defined and explained in this file.
"""

from typing import Dict, List, Tuple, Union

import numpy as np

from power_grid_model.core.dataset_definitions import ComponentType

# When we're dropping python 3.8, we should introduce proper NumPy type hinting

SingleArray = Union[np.ndarray]
"""
A single array is a one-dimensional structured containing a list of components of the same type.

- Examples:

    - structure: <1d-array>
    - concrete: array([(0, 10500.0), (0, 10500.0)], dtype=power_grid_meta_data["input"]["node"].dtype)
"""

DenseBatchArray = Union[np.ndarray]
"""
A dense batch array is a two-dimensional structured numpy array containing a list of components of 
the same type for each scenario.
"""

SparseBatchArray = Dict[str, Union[np.ndarray, SingleArray]]
"""
A sparse batch array is a dictionary containing the keys `indptr` and `data`.

- data: a :class:`SingleArray`. The exact dtype depends on the type of component.
- indptr: a one-dimensional numpy int64 array containing n+1 elements where n is the amount of scenarios.

    - The elements are the indices in the data that point to the first element of that scenario.
    - The last element is one after the data index of the last element of the last scenario.
    - Usually, the last element will therefore be the size of the data.

- Examples:

    - structure: {"indptr": <1d-array>, "data": :class:`SingleArray`}
    - concrete example: {"indptr": [0, 2, 2, 3], "data": [(0, 1, 1), (1, 1, 1), (0, 0, 0)]}

        - the scenario 0 sets the statuses of components ids 0 and 1 to 1 (and keeps defaults for other components)
        - scenario 1 keeps the default values for all components
        - scenario 2 sets the statuses of component with id 0 to 0 (and keeps defaults for other components)
"""

BatchArray = Union[DenseBatchArray, SparseBatchArray]
"""
A batch array is a either a :class:`DenseBatchArray` or a :class:`SparseBatchArray`.
"""

DataArray = Union[SingleArray, BatchArray]
"""
A data array can be a :class:`SingleArray` or a :class:`BatchArray`.
"""

SingleDataset = Dict[ComponentType, SingleArray]
"""
A single dataset is a dictionary where the keys are the component types and the values are
:class:`SingleArray`

- Example: {"node": :class:`SingleArray`, "line": :class:`SingleArray`}
"""

BatchDataset = Dict[ComponentType, BatchArray]
"""
A batch dataset is a dictionary where the keys are the component types and the values are :class:`BatchArray`

- Example: {"node": :class:`DenseBatchArray`, "line": :class:`SparseBatchArray`}
"""

Dataset = Union[SingleDataset, BatchDataset]
"""
A general data set can be a :class:`SingleDataset` or a :class:`BatchDataset`.

- Examples:

    - single: {"node": :class:`SingleArray`, "line": :class:`SingleArray`}
    - batch: {"node": :class:`DenseBatchArray`, "line": :class:`SparseBatchArray`}
"""

BatchList = List[SingleDataset]
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

AsymValue = Tuple[RealValue, RealValue, RealValue]
"""
Asymmetrical values are three-phase values like p or u_measured.

- Example: (10400.0, 10500.0, 10600.0)
"""

AttributeValue = Union[RealValue, NominalValue, AsymValue]
"""
When representing a grid as a native python structure, each attribute (u_rated etc) is either a nominal value,
a real value, or a tuple of three real values.

- Examples:

    - real: 10500.0
    - nominal: 123
    - asym: (10400.0, 10500.0, 10600.0)
"""

Component = Dict[str, Union[AttributeValue, str]]
"""
A component, when represented in native python format, is a dictionary, where the keys are the attributes and the values
are the corresponding values. It is allowed to add extra fields, containing either an AttributeValue or a string.

- Example: {"id": 1, "u_rated": 10500.0, "original_id": "Busbar #1"}
"""

ComponentList = List[Component]
"""
A component list is a list containing components. In essence it stores the same information as a np.ndarray,
but in a native python format, without using numpy.

- Example: [{"id": 1, "u_rated": 10500.0}, {"id": 2, "u_rated": 10500.0}]
"""

SinglePythonDataset = Dict[ComponentType, ComponentList]
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

BatchPythonDataset = List[SinglePythonDataset]
"""
A batch dataset in native python representation is a list of dictionaries, where the keys are the component names and
the values are a list of all the instances of such a component. In essence it stores the same information as a
BatchDataset, but in a native python format, without using numpy. Actually it looks more like the BatchList.

- Example: 

  [{"line": [{"id": 3, "from_status": 0, "to_status": 0, ...}],},
   {"line": [{"id": 3, "from_status": 1, "to_status": 1, ...}],}]
"""

PythonDataset = Union[SinglePythonDataset, BatchPythonDataset]
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
