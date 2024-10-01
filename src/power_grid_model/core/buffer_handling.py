# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Power grid model buffer handler
"""

from dataclasses import dataclass
from typing import cast

import numpy as np

from power_grid_model._utils import is_sparse
from power_grid_model.core.error_handling import VALIDATOR_MSG
from power_grid_model.core.index_integer import IdxC, IdxNp
from power_grid_model.core.power_grid_core import IdxPtr, VoidPtr
from power_grid_model.core.power_grid_meta import ComponentMetaData
from power_grid_model.data_types import (
    AttributeType,
    ComponentData,
    DenseBatchData,
    IndexPointer,
    SingleComponentData,
    SparseBatchArray,
    SparseBatchData,
)


@dataclass
class BufferProperties:
    """
    Helper class to collect info on the dataset.
    """

    is_sparse: bool
    is_batch: bool
    batch_size: int
    n_elements_per_scenario: int
    n_total_elements: int
    columns: list[AttributeType] | None


# prepared attribute data for c api
@dataclass
class CAttributeBuffer:
    """
    Buffer for a single attribute.
    """

    data: VoidPtr  # type: ignore


# prepared component data for c api
@dataclass
class CBuffer:
    """
    Buffer for a single component.
    """

    data: VoidPtr | None
    indptr: IdxPtr | None  # type: ignore
    n_elements_per_scenario: int
    batch_size: int
    total_elements: int
    attribute_data: dict[AttributeType, CAttributeBuffer]


def _get_raw_data_view(data: np.ndarray, dtype: np.dtype) -> VoidPtr:
    """
    Get a raw view on the data.

    Args:
        data: the data.
        dtype: the dtype the raw buffer should obey.

    Returns:
        a raw view on the data set.
    """
    return np.ascontiguousarray(data, dtype=dtype).ctypes.data_as(VoidPtr)


def _get_raw_component_data_view(
    data: np.ndarray | dict[AttributeType, np.ndarray], schema: ComponentMetaData
) -> VoidPtr | None:
    """
    Get a raw view on the data.

    Args:
        data: the data.
        schema: the schema the raw buffer should obey.

    Returns:
        a raw view on the data set.
    """
    if isinstance(data, np.ndarray):
        return _get_raw_data_view(data, dtype=schema.dtype)
    return None


def _get_raw_attribute_data_view(data: np.ndarray, schema: ComponentMetaData, attribute: AttributeType) -> VoidPtr:
    """
    Get a raw view on the data.

    Args:
        data: the data.
        schema: the schema the raw buffer should obey.

    Returns:
        a raw view on the data set.
    """
    return _get_raw_data_view(data, dtype=schema.dtype[attribute].base)


def _get_indptr_view(indptr: np.ndarray) -> IdxPtr:  # type: ignore[valid-type]
    """
    Get a raw view on the index pointer.

    Args:
        indptr: the index pointer.

    Returns:
        a raw view on the index pointer.
    """
    return np.ascontiguousarray(indptr, dtype=IdxNp).ctypes.data_as(IdxPtr)


def _get_uniform_buffer_properties(
    data: SingleComponentData | DenseBatchData,
    schema: ComponentMetaData,
    is_batch: bool | None,
    batch_size: int | None,
) -> BufferProperties:
    """
    Extract the properties of the uniform batch dataset component.

    Args:
        data (SingleComponentData | DenseBatchData): the dataset component.
        schema (ComponentMetaData): the dataset type.
        is_batch (bool | None): whether the data is a batch dataset.
        batch_size (int | None): the batch size.

    Raises:
        KeyError: if the dataset component is not sparse.
        ValueError: if the dataset component contains conflicting or bad data.

    Returns:
        the properties of the dataset component.
    """
    if is_batch is not None and batch_size is not None and batch_size != 1 and not is_batch:
        raise ValueError(f"Inconsistent 'is batch' and 'batch size'. {VALIDATOR_MSG}")

    is_sparse_property = False

    if isinstance(data, np.ndarray):
        ndim = data.ndim
        shape = data.shape
        columns = None
    elif data:
        attribute, attribute_data = next(iter(data.items()))
        ndim = attribute_data.ndim - schema.dtype[attribute].ndim
        shape = attribute_data.shape[:ndim]
        columns = list(data)
    else:
        raise ValueError("Empty columnar buffer is ambiguous.{VALIDATOR_MSG}")

    if isinstance(data, dict) and schema is not None:
        for attribute, attribute_data in data.items():
            if attribute_data.ndim != ndim + schema.dtype[attribute].ndim or attribute_data.shape[:ndim] != shape:
                raise ValueError(f"Data buffers must be consistent. {VALIDATOR_MSG}")

    if ndim not in (1, 2):
        raise ValueError(f"Array can only be 1D or 2D. {VALIDATOR_MSG}")

    actual_is_batch = ndim == 2
    actual_batch_size = shape[0] if actual_is_batch else 1
    n_elements_per_scenario = shape[-1]
    n_total_elements = actual_batch_size * n_elements_per_scenario

    if is_batch is not None and is_batch != actual_is_batch:
        raise ValueError(f"Provided 'is batch' must be equal to actual 'is batch'. {VALIDATOR_MSG}")
    if batch_size is not None and batch_size != actual_batch_size:
        raise ValueError(f"Provided 'batch size' must be equal to actual batch size. {VALIDATOR_MSG}")

    return BufferProperties(
        is_sparse=is_sparse_property,
        is_batch=actual_is_batch,
        batch_size=actual_batch_size,
        n_elements_per_scenario=n_elements_per_scenario,
        n_total_elements=n_total_elements,
        columns=columns,
    )


def _get_sparse_buffer_properties(
    data: SparseBatchData,
    schema: ComponentMetaData,
    batch_size: int | None,
) -> BufferProperties:
    """
    Extract the properties of the sparse batch dataset component.

    Args:
        data (SparseBatchData): the sparse dataset component.
        schema (ComponentMetaData | None): the dataset type.
        batch_size (int | None): the batch size.

    Raises:
        KeyError: if the dataset component is not sparse.
        ValueError: if the dataset component contains conflicting or bad data.

    Returns:
        the properties of the dataset component.
    """
    is_sparse_property = True

    contents = data["data"]
    indptr = data["indptr"]

    if not isinstance(indptr, np.ndarray):
        raise TypeError(f"indptr must be of type IndexPointer [np.ndarray]. {VALIDATOR_MSG}")

    ndim = 1
    columns: list[AttributeType] | None = None
    if isinstance(contents, np.ndarray):
        shape: tuple[int, ...] = contents.shape
    elif not contents:
        raise ValueError(f"Empty columnar buffer is ambiguous. {VALIDATOR_MSG}")
    elif isinstance(contents, dict):
        attribute, attribute_data = next(iter(contents.items()))
        shape = attribute_data.shape[:ndim]
        columns = list(contents)

    if isinstance(contents, dict):
        for attribute, attribute_data in contents.items():
            if attribute_data.ndim != ndim + schema.dtype[attribute].ndim or attribute_data.shape[:ndim] != shape:
                raise ValueError(f"Data buffers must be consistent. {VALIDATOR_MSG}")

    contents_size = sum(shape)
    if ndim != 1:
        raise ValueError(f"Data array in sparse data can only be 1D. {VALIDATOR_MSG}")
    if indptr.ndim != 1:
        raise ValueError(f"indptr can only be 1D. {VALIDATOR_MSG}")
    if indptr[0] != 0 or indptr[-1] != contents_size:
        raise ValueError(f"indptr should start from zero and end at size of data array. {VALIDATOR_MSG}")
    if np.any(np.diff(indptr) < 0):
        raise ValueError(f"indptr should be increasing. {VALIDATOR_MSG}")

    if batch_size is not None and batch_size != indptr.size - 1:
        raise ValueError(f"Provided batch size must be equal to actual batch size. {VALIDATOR_MSG}")

    is_batch = True
    n_elements_per_scenario = -1
    n_total_elements = contents_size

    return BufferProperties(
        is_sparse=is_sparse_property,
        is_batch=is_batch,
        batch_size=indptr.size - 1,
        n_elements_per_scenario=n_elements_per_scenario,
        n_total_elements=n_total_elements,
        columns=columns,
    )


def get_buffer_properties(
    data: ComponentData,
    schema: ComponentMetaData,
    is_batch: bool | None = None,
    batch_size: int | None = None,
) -> BufferProperties:
    """
    Extract the properties of the dataset component

    Args:
        data (ComponentData): the dataset component.
        schema (ComponentMetaData | None): the dataset type [optional if data is not columnar]
        is_batch (bool | None): whether the data is a batch dataset. [optional]
        batch_size (int | None): the batch size. [optional]

    Raises:
        ValueError: if the dataset component contains conflicting or bad data.

    Returns:
        the properties of the dataset component.
    """
    if isinstance(data, np.ndarray) or "indptr" not in data:
        return _get_uniform_buffer_properties(data=data, schema=schema, is_batch=is_batch, batch_size=batch_size)

    if is_batch is not None and not is_batch:
        raise ValueError("Sparse data must be batch data")

    return _get_sparse_buffer_properties(data=cast(SparseBatchArray, data), schema=schema, batch_size=batch_size)


def _get_attribute_buffer_views(
    data: np.ndarray | dict[AttributeType, np.ndarray], schema: ComponentMetaData
) -> dict[AttributeType, CAttributeBuffer]:
    """
    Get C API compatible views on attribute buffers.

    Args:
        data (dict[AttributeType, np.ndarray]): the data.
        schema (ComponentMetaData): the schema that the data should obey.

    Returns:
        dict[AttributeType, CAttributeBuffer]: the C API attribute buffer view per attribute.
    """
    if isinstance(data, np.ndarray):
        return {}

    return {
        attribute: CAttributeBuffer(
            data=_get_raw_attribute_data_view(data=attribute_data, schema=schema, attribute=attribute)
        )
        for attribute, attribute_data in data.items()
    }


def _get_uniform_buffer_view(
    data: DenseBatchData,
    schema: ComponentMetaData,
    is_batch: bool | None,
    batch_size: int | None,
) -> CBuffer:
    """
    Get a C API compatible view on a uniform buffer.

    Args:
        data: the data.
        schema: the schema that the data should obey.
        is_batch (bool | None): whether the data is a batch dataset.
        batch_size (int | None): the batch size.

    Returns:
        the C API buffer view.
    """
    properties = _get_uniform_buffer_properties(data, schema=schema, is_batch=is_batch, batch_size=batch_size)

    return CBuffer(
        data=_get_raw_component_data_view(data=data, schema=schema),
        indptr=IdxPtr(),
        n_elements_per_scenario=properties.n_elements_per_scenario,
        batch_size=properties.batch_size,
        total_elements=properties.n_total_elements,
        attribute_data=_get_attribute_buffer_views(data=data, schema=schema),
    )


def _get_sparse_buffer_view(
    data: SparseBatchArray,
    schema: ComponentMetaData,
    batch_size: int | None,
) -> CBuffer:
    """
    Get a C API compatible view on a sparse buffer.

    Args:
        data: the data.
        schema: the schema that the data should obey.
        batch_size (int | None): the batch size.

    Returns:
        the C API buffer view.
    """
    contents = data["data"]
    indptr = data["indptr"]

    properties = _get_sparse_buffer_properties(data, schema=schema, batch_size=batch_size)

    return CBuffer(
        data=_get_raw_component_data_view(data=contents, schema=schema),
        indptr=_get_indptr_view(indptr),
        n_elements_per_scenario=properties.n_elements_per_scenario,
        batch_size=properties.batch_size,
        total_elements=properties.n_total_elements,
        attribute_data=_get_attribute_buffer_views(data=contents, schema=schema),
    )


def get_buffer_view(
    data: ComponentData,
    schema: ComponentMetaData,
    is_batch: bool | None = None,
    batch_size: int | None = None,
) -> CBuffer:
    """
    Get a C API compatible view on a buffer.

    Args:
        data: the data.
        schema: the schema that the data should obey.
        is_batch (bool | None): whether the data is a batch dataset. [optional]
        batch_size (int | None): the batch size. [optional]

    Returns:
        the C API buffer view.
    """
    if not is_sparse(data):
        return _get_uniform_buffer_view(cast(DenseBatchData, data), schema, is_batch, batch_size)

    if is_batch is not None and not is_batch:
        raise ValueError("Sparse data must be batch data")

    return _get_sparse_buffer_view(cast(SparseBatchArray, data), schema, batch_size)


def create_buffer(properties: BufferProperties, schema: ComponentMetaData) -> ComponentData:
    """
    Create a buffer with the provided properties and type.

    Args:
        properties: the desired buffer properties.
        schema: the data type of the buffer.

    Raises:
        ValueError: if the buffer properties are not consistent.

    Returns:
        np.ndarray | dict[[str, np.ndarray]: a buffer with the correct properties.
    """
    if properties.is_sparse:
        return _create_sparse_buffer(properties=properties, schema=schema)

    return _create_uniform_buffer(properties=properties, schema=schema)


def _create_uniform_buffer(properties: BufferProperties, schema: ComponentMetaData) -> DenseBatchData:
    """
    Create a uniform buffer with the provided properties and type.

    Args:
        properties: the desired buffer properties.
        schema: the data type of the buffer.

    Raises:
        ValueError: if the buffer properties are not uniform.

    Returns:
        A uniform buffer with the correct properties.
    """
    if properties.is_sparse:
        raise ValueError(f"A uniform buffer cannot be sparse. {VALIDATOR_MSG}")

    shape: int | tuple[int, int] = (
        (properties.batch_size, properties.n_elements_per_scenario)
        if properties.is_batch
        else properties.n_elements_per_scenario
    )
    return _create_contents_buffer(shape=shape, dtype=schema.dtype, columns=properties.columns)


def _create_sparse_buffer(properties: BufferProperties, schema: ComponentMetaData) -> SparseBatchData:
    """
    Create a sparse buffer with the provided properties and type.

    Args:
        properties: the desired buffer properties.
        schema: the data type of the buffer.

    Raises:
        ValueError: if the buffer properties are not sparse.

    Returns:
        A sparse buffer with the correct properties.
    """
    data: SingleComponentData = _create_contents_buffer(
        shape=properties.n_total_elements,
        dtype=schema.dtype,
        columns=properties.columns,
    )
    indptr: IndexPointer = np.array([0] * properties.batch_size + [properties.n_total_elements], dtype=IdxC)
    return cast(SparseBatchData, {"data": data, "indptr": indptr})


def _create_contents_buffer(shape, dtype, columns: list[AttributeType] | None) -> SingleComponentData | DenseBatchData:
    if columns is None:
        return np.empty(shape=shape, dtype=dtype)

    return {attribute: np.empty(shape=shape, dtype=dtype[attribute]) for attribute in columns}
