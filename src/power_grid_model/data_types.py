# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Data types involving PGM datasets.

Many data types are used throughout the power grid model project. In an attempt to clarify type hints, some types
have been defined and explained.
"""

from typing import TypeAlias

from power_grid_model._core.data_types import (  # pylint: disable=unused-import
    AsymValue,
    AttributeType,
    AttributeValue,
    BatchArray,
    BatchColumn,
    BatchColumnarData,
    BatchComponentData,
    BatchDataset,
    BatchList,
    BatchPythonDataset,
    ColumnarData,
    Component,
    ComponentData,
    ComponentList,
    DataArray,
    Dataset,
    DenseBatchArray,
    DenseBatchColumnarData,
    DenseBatchData,
    IndexPointer,
    NominalValue,
    PythonDataset,
    RealValue,
    SingleArray,
    SingleColumn,
    SingleColumnarData,
    SingleComponentData,
    SingleDataset,
    SinglePythonDataset,
    SparseBatchArray,
    SparseBatchColumnarData,
    SparseBatchData,
)

SparseDataComponentType: TypeAlias = str
"""
A string representing the component type of sparse data structures.

Must be either "data" or "indptr".
"""
