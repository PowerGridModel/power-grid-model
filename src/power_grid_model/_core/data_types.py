# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Data types involving PGM datasets.

Data types for library-internal use. In an attempt to clarify type hints, some types
have been defined and explained in this file.
"""

from typing import TypeAlias, TypedDict, TypeVar

import numpy as np

from power_grid_model._core.dataset_definitions import ComponentTypeVar

SingleArray: TypeAlias = np.ndarray

AttributeType: TypeAlias = str

SingleColumn: TypeAlias = np.ndarray

DenseBatchArray: TypeAlias = np.ndarray

SingleColumnarData = dict[AttributeType, SingleColumn]

_SingleComponentData = TypeVar("_SingleComponentData", SingleArray, SingleColumnarData)  # deduction helper
SingleComponentData = SingleArray | SingleColumnarData


SingleDataset = dict[ComponentTypeVar, _SingleComponentData]

BatchList = list[SingleDataset]

BatchColumn: TypeAlias = np.ndarray

DenseBatchColumnarData = dict[AttributeType, BatchColumn]

IndexPointer: TypeAlias = np.ndarray


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


SparseBatchData = SparseBatchArray | SparseBatchColumnarData

SparseDataComponentType: TypeAlias = str

BatchColumnarData = DenseBatchColumnarData | SparseBatchColumnarData

ColumnarData = SingleColumnarData | BatchColumnarData
BatchArray = DenseBatchArray | SparseBatchArray


BatchComponentData = BatchArray | BatchColumnarData

_BatchComponentData = TypeVar("_BatchComponentData", BatchArray, BatchColumnarData)  # deduction helper


BatchDataset = dict[ComponentTypeVar, _BatchComponentData]


DataArray = SingleArray | BatchArray


_ComponentData = TypeVar("_ComponentData", SingleComponentData, BatchComponentData)  # deduction helper
ComponentData = DataArray | ColumnarData

Dataset = dict[ComponentTypeVar, _ComponentData]


DenseBatchData = DenseBatchArray | DenseBatchColumnarData

NominalValue = int

RealValue = float

AsymValue = tuple[RealValue, RealValue, RealValue]

AttributeValue = RealValue | NominalValue | AsymValue

Component = dict[AttributeType, AttributeValue | str]

ComponentList = list[Component]

SinglePythonDataset = dict[ComponentTypeVar, ComponentList]

BatchPythonDataset = list[SinglePythonDataset]

PythonDataset = SinglePythonDataset | BatchPythonDataset
