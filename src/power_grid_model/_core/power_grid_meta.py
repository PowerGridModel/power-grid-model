# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Load meta data from C core and define numpy structured array
"""

from dataclasses import dataclass
from enum import IntEnum
from typing import Any

import numpy as np

from power_grid_model._core.dataset_definitions import (
    ComponentTypeLike,
    ComponentTypeVar,
    DatasetType,
    DatasetTypeLike,
    _str_to_component_type,
    _str_to_datatype,
)
from power_grid_model._core.power_grid_core import AttributePtr, ComponentPtr, DatasetPtr, power_grid_core as pgc
from power_grid_model.data_types import DenseBatchArray, SingleArray


# constant enum for ctype
# pylint: disable=invalid-name
class PGMCType(IntEnum):
    """enumeration for ctype"""

    int32 = 0
    int8 = 1
    double = 2
    double3 = 3


_CTYPE_NUMPY_MAP = {PGMCType.double: "f8", PGMCType.int32: "i4", PGMCType.int8: "i1", PGMCType.double3: "(3,)f8"}
_ENDIANNESS = "<" if pgc.is_little_endian() == 1 else ">"
_NAN_VALUE_MAP = {
    f"{_ENDIANNESS}f8": np.nan,
    f"{_ENDIANNESS}(3,)f8": np.nan,
    f"{_ENDIANNESS}i4": np.iinfo(f"{_ENDIANNESS}i4").min,
    f"{_ENDIANNESS}i1": np.iinfo(f"{_ENDIANNESS}i1").min,
}


@dataclass
class ComponentMetaData:
    """
    Data class for component metadata
    """

    dtype: np.dtype
    dtype_dict: dict[str, Any]
    nans: dict[str, float | int]
    nan_scalar: np.ndarray

    def __getitem__(self, item):
        """
        Get item of dataclass

        Args:
            item: item name

        Returns:

        """
        return getattr(self, item)


DatasetMetaData = dict[ComponentTypeVar, ComponentMetaData]
PowerGridMetaData = dict[DatasetType, DatasetMetaData]
"""
The data types for all dataset types and components used by the Power Grid Model.
"""


def _generate_meta_data() -> PowerGridMetaData:
    """

    Returns: meta data for all dataset

    """
    py_meta_data = {}
    n_datasets = pgc.meta_n_datasets()
    for i in range(n_datasets):
        dataset = pgc.meta_get_dataset_by_idx(i)
        py_meta_data[_str_to_datatype(pgc.meta_dataset_name(dataset))] = _generate_meta_dataset(dataset)
    return py_meta_data


def _generate_meta_dataset(dataset: DatasetPtr) -> DatasetMetaData:
    """

    Args:
        dataset: dataset

    Returns: meta data for one dataset

    """
    py_meta_dataset = {}
    n_components = pgc.meta_n_components(dataset)
    for i in range(n_components):
        component = pgc.meta_get_component_by_idx(dataset, i)
        py_meta_dataset[_str_to_component_type(pgc.meta_component_name(component))] = _generate_meta_component(
            component
        )
    return py_meta_dataset


def _generate_meta_component(component: ComponentPtr) -> ComponentMetaData:
    """

    Args:
        component: component

    Returns: meta data for one component

    """

    dtype_dict = _generate_meta_attributes(component)
    dtype = np.dtype({k: v for k, v in dtype_dict.items() if k != "nans"})  # type: ignore
    nans = dict(zip(dtype_dict["names"], dtype_dict["nans"]))
    if dtype.alignment != pgc.meta_component_alignment(component):
        raise TypeError(f'Aligment mismatch for component type: "{pgc.meta_component_name(component)}" !')
    # get single nan scalar
    nan_scalar = np.empty(1, dtype=dtype)
    for key, value in nans.items():
        nan_scalar[key] = value
    return ComponentMetaData(dtype=dtype, dtype_dict=dtype_dict, nans=nans, nan_scalar=nan_scalar)


def _generate_meta_attributes(component: ComponentPtr) -> dict:
    """

    Args:
        component: component

    Returns: meta data for all attributes

    """
    names = []
    formats = []
    offsets = []
    nans = []
    n_attrs = pgc.meta_n_attributes(component)
    for i in range(n_attrs):
        attribute: AttributePtr = pgc.meta_get_attribute_by_idx(component, i)
        attr_name: str = pgc.meta_attribute_name(attribute)
        attr_ctype: int = pgc.meta_attribute_ctype(attribute)
        attr_offset: int = pgc.meta_attribute_offset(attribute)
        attr_np_type = f"{_ENDIANNESS}{_CTYPE_NUMPY_MAP[PGMCType(attr_ctype)]}"
        attr_nan = _NAN_VALUE_MAP[attr_np_type]
        names.append(attr_name)
        formats.append(attr_np_type)
        offsets.append(attr_offset)
        nans.append(attr_nan)
    return {
        "names": names,
        "formats": formats,
        "offsets": offsets,
        "itemsize": pgc.meta_component_size(component),
        "aligned": True,
        "nans": nans,
    }


# store meta data
power_grid_meta_data = _generate_meta_data()
"""
The data types for all dataset types and components used by the Power Grid Model.
"""


def initialize_array(
    data_type: DatasetTypeLike,
    component_type: ComponentTypeLike,
    shape: tuple | int,
    empty: bool = False,
) -> SingleArray | DenseBatchArray:
    """
    Initializes an array for use in Power Grid Model calculations

    Args:
        data_type: input, update, sym_output, or asym_output
        component_type: one component type, e.g. node
        shape: shape of initialization
            integer, it is a 1-dimensional array
            tuple, it is an N-dimensional (tuple.shape) array
        empty: if True, leave the memory block un-initialized

    Returns:
        np structured array with all entries as null value
    """
    data_type = _str_to_datatype(data_type)
    component_type = _str_to_component_type(component_type)
    if not isinstance(shape, tuple):
        shape = (shape,)
    if empty:
        return np.empty(shape=shape, dtype=power_grid_meta_data[data_type][component_type].dtype, order="C")
    return np.full(
        shape=shape,
        fill_value=power_grid_meta_data[data_type][component_type].nan_scalar,
        dtype=power_grid_meta_data[data_type][component_type].dtype,
        order="C",
    )
