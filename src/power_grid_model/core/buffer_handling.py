# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Power grid model buffer handler
"""


from dataclasses import dataclass
from typing import Dict, Mapping, Optional, Tuple, Union

import numpy as np

from power_grid_model.core.error_handling import VALIDATOR_MSG
from power_grid_model.core.index_integer import IdxC, IdxNp
from power_grid_model.core.power_grid_core import IdxPtr, VoidPtr
from power_grid_model.core.power_grid_meta import ComponentMetaData


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


# prepared data for c api
@dataclass
class CBuffer:
    """
    Buffer for a single component.
    """

    data: VoidPtr
    indptr: Optional[IdxPtr]  # type: ignore
    n_elements_per_scenario: int
    batch_size: int
    total_elements: int


def _get_raw_data_view(data: np.ndarray, schema: ComponentMetaData) -> VoidPtr:
    """
    Get a raw view on the data.

    Args:
        data: the data.
        schema: the schema the raw buffer should obey.

    Returns:
        a raw view on the data set.
    """
    return np.ascontiguousarray(data, dtype=schema.dtype).ctypes.data_as(VoidPtr)


def _get_indptr_view(indptr: np.ndarray) -> IdxPtr:  # type: ignore[valid-type]
    """
    Get a raw view on the index pointer.

    Args:
        indptr: the index pointer.

    Returns:
        a raw view on the index pointer.
    """
    return np.ascontiguousarray(indptr, dtype=IdxNp).ctypes.data_as(IdxPtr)


def _get_uniform_buffer_properties(data: np.ndarray) -> BufferProperties:
    """
    Extract the properties of the uniform batch dataset component.

    Args:
        data (np.ndarray): the dataset component.

    Raises:
        KeyError: if the dataset component is not sparse.
        ValueError: if the dataset component contains conflicting or bad data.

    Returns:
        the properties of the dataset component.
    """
    is_sparse = False

    if data.ndim not in (1, 2):
        raise ValueError(f"Array can only be 1D or 2D. {VALIDATOR_MSG}")

    is_batch = data.ndim == 2
    batch_size = data.shape[0] if is_batch else 1
    n_elements_per_scenario = data.shape[-1]
    n_total_elements = batch_size * n_elements_per_scenario

    return BufferProperties(
        is_sparse=is_sparse,
        is_batch=is_batch,
        batch_size=batch_size,
        n_elements_per_scenario=n_elements_per_scenario,
        n_total_elements=n_total_elements,
    )


def _get_sparse_buffer_properties(data: Mapping[str, np.ndarray]) -> BufferProperties:
    """
    Extract the properties of the sparse batch dataset component.

    Args:
        data (Mapping[str, np.ndarray]): the sparse dataset component.

    Raises:
        KeyError: if the dataset component is not sparse.
        ValueError: if the dataset component contains conflicting or bad data.

    Returns:
        the properties of the dataset component.
    """
    is_sparse = True

    contents = data["data"]
    indptr = data["indptr"]

    if contents.ndim != 1:
        raise ValueError(f"Data array can only be 1D. {VALIDATOR_MSG}")
    if indptr.ndim != 1:
        raise ValueError(f"indptr can only be 1D. {VALIDATOR_MSG}")
    if indptr[0] != 0 or indptr[-1] != contents.size:
        raise ValueError(f"indptr should start from zero and end at size of data array. {VALIDATOR_MSG}")
    if np.any(np.diff(indptr) < 0):
        raise ValueError(f"indptr should be increasing. {VALIDATOR_MSG}")

    is_batch = True
    batch_size = indptr.size - 1
    n_elements_per_scenario = -1
    n_total_elements = contents.size

    return BufferProperties(
        is_sparse=is_sparse,
        is_batch=is_batch,
        batch_size=batch_size,
        n_elements_per_scenario=n_elements_per_scenario,
        n_total_elements=n_total_elements,
    )


def get_buffer_properties(data: Union[np.ndarray, Mapping[str, np.ndarray]]) -> BufferProperties:
    """
    Extract the properties of the dataset component

    Args:
        data (Union[np.ndarray, Mapping[str, np.ndarray]]): the dataset component.

    Raises:
        ValueError: if the dataset component contains conflicting or bad data.

    Returns:
        the properties of the dataset component.
    """
    if isinstance(data, np.ndarray):
        return _get_uniform_buffer_properties(data)

    return _get_sparse_buffer_properties(data)


def _get_uniform_buffer_view(data: np.ndarray, schema: ComponentMetaData) -> CBuffer:
    """
    Get a C API compatible view on a uniform buffer.

    Args:
        data: the data.
        schema: the schema that the data should obey.

    Returns:
        the C API buffer view.
    """
    properties = _get_uniform_buffer_properties(data)

    return CBuffer(
        data=_get_raw_data_view(data, schema),
        indptr=IdxPtr(),
        n_elements_per_scenario=properties.n_elements_per_scenario,
        batch_size=properties.batch_size,
        total_elements=properties.n_total_elements,
    )


def _get_sparse_buffer_view(data: Mapping[str, np.ndarray], schema: ComponentMetaData) -> CBuffer:
    """
    Get a C API compatible view on a sparse buffer.

    Args:
        data: the data.
        schema: the schema that the data should obey.

    Returns:
        the C API buffer view.
    """
    contents = data["data"]
    indptr = data["indptr"]

    properties = _get_sparse_buffer_properties(data)

    return CBuffer(
        data=_get_raw_data_view(contents, schema),
        indptr=_get_indptr_view(indptr),
        n_elements_per_scenario=properties.n_elements_per_scenario,
        batch_size=properties.batch_size,
        total_elements=properties.n_total_elements,
    )


def get_buffer_view(data: Union[np.ndarray, Mapping[str, np.ndarray]], schema: ComponentMetaData) -> CBuffer:
    """
    Get a C API compatible view on a buffer.

    Args:
        data: the data.
        schema: the schema that the data should obey.

    Returns:
        the C API buffer view.
    """
    if isinstance(data, np.ndarray):
        return _get_uniform_buffer_view(data, schema)

    return _get_sparse_buffer_view(data, schema)


def create_buffer(properties: BufferProperties, schema: ComponentMetaData) -> Union[np.ndarray, Dict[str, np.ndarray]]:
    """
    Create a buffer with the provided properties and type.

    Args:
        properties: the desired buffer properties.
        schema: the data type of the buffer.

    Raises:
        ValueError: if the buffer properties are not consistent.

    Returns:
        Union[np.ndarray, Dict[str, np.ndarray]]: a buffer with the correct properties.
    """
    if properties.is_sparse:
        return _create_sparse_buffer(properties=properties, schema=schema)

    return _create_uniform_buffer(properties=properties, schema=schema)


def _create_uniform_buffer(properties: BufferProperties, schema: ComponentMetaData) -> np.ndarray:
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

    shape: Union[int, Tuple[int, int]] = (
        (properties.batch_size, properties.n_elements_per_scenario)
        if properties.is_batch
        else properties.n_elements_per_scenario
    )
    return np.empty(shape=shape, dtype=schema.dtype)


def _create_sparse_buffer(properties: BufferProperties, schema: ComponentMetaData) -> Dict[str, np.ndarray]:
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
    data = np.empty(properties.n_total_elements, dtype=schema.dtype)
    indptr = np.array([0] * properties.batch_size + [properties.n_total_elements], dtype=IdxC)
    return {"data": data, "indptr": indptr}
