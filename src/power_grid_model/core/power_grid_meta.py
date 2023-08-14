# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

"""
Load meta data from C core and define numpy structured array
"""

from ctypes import Array, c_char_p, c_void_p
from dataclasses import dataclass
from enum import IntEnum
from typing import Any, Dict, Mapping, Optional, Union

import numpy as np

from power_grid_model.core.error_handling import VALIDATOR_MSG
from power_grid_model.core.index_integer import IdxC, IdxNp
from power_grid_model.core.power_grid_core import AttributePtr, ComponentPtr, DatasetPtr, IdxPtr
from power_grid_model.core.power_grid_core import power_grid_core as pgc


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
    dtype_dict: Dict[str, Any]
    nans: Dict[str, Union[float, int]]
    nan_scalar: np.ndarray

    def __getitem__(self, item):
        """
        Get item of dataclass

        Args:
            item: item name

        Returns:

        """
        return getattr(self, item)


DatasetMetaData = Dict[str, ComponentMetaData]
PowerGridMetaData = Dict[str, DatasetMetaData]


def _generate_meta_data() -> PowerGridMetaData:
    """

    Returns: meta data for all dataset

    """
    py_meta_data = {}
    n_datasets = pgc.meta_n_datasets()
    for i in range(n_datasets):
        dataset = pgc.meta_get_dataset_by_idx(i)
        py_meta_data[pgc.meta_dataset_name(dataset)] = _generate_meta_dataset(dataset)
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
        py_meta_dataset[pgc.meta_component_name(component)] = _generate_meta_component(component)
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
    # return component
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


def initialize_array(data_type: str, component_type: str, shape: Union[tuple, int], empty: bool = False):
    """
    Initializes an array for use in Power Grid Model calculations

    Args:
        data_type: input, update, sym_output, or asym_output
        component_type: one component type, e.g. node
        shape: shape of initialization
            integer, it is a 1-dimensional array
            tuple, it is an N-dimensional (tuple.shape) array
        empty: if leave the memory block un-initialized

    Returns:
        np structured array with all entries as null value
    """
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


# prepared data for c api
@dataclass
class CBuffer:
    """
    Buffer for a single component
    """

    data: c_void_p
    indptr: Optional[IdxPtr]  # type: ignore
    n_elements_per_scenario: int
    batch_size: int


@dataclass
class CDataset:
    """
    Dataset definition
    """

    dataset: Dict[str, CBuffer]
    batch_size: int
    n_components: int
    components: Array
    n_component_elements_per_scenario: Array
    indptrs_per_component: Array
    data_ptrs_per_component: Array


# pylint: disable=R0912
def prepare_cpp_array(
    data_type: str, array_dict: Mapping[str, Union[np.ndarray, Mapping[str, np.ndarray]]]
) -> CDataset:
    """
    prepare array for cpp pointers
    Args:
        data_type: input, update, or symmetric/asymmetric output
        array_dict:
            key: component type
            value:
                data array: can be 1D or 2D (in batches)
                or
                dict with
                    key:
                        data -> data array in flat for batches
                        indptr -> index pointer for variable length input
    Returns:
        instance of CDataset ready to be fed into C API
    """
    # process
    schema = power_grid_meta_data[data_type]
    dataset_dict = {}
    for component_name, entry in array_dict.items():
        if component_name not in schema:
            continue
        # homogeneous
        if isinstance(entry, np.ndarray):
            data = entry
            ndim = entry.ndim
            indptr_c = IdxPtr()
            if ndim == 1:
                batch_size = 1
                n_elements_per_scenario = entry.shape[0]
            elif ndim == 2:  # (n_batch, n_component)
                batch_size = entry.shape[0]
                n_elements_per_scenario = entry.shape[1]
            else:
                raise ValueError(f"Array can only be 1D or 2D. {VALIDATOR_MSG}")
        # with indptr
        else:
            data = entry["data"]
            indptr: np.ndarray = entry["indptr"]
            batch_size = indptr.size - 1
            n_elements_per_scenario = -1
            if data.ndim != 1:
                raise ValueError(f"Data array can only be 1D. {VALIDATOR_MSG}")
            if indptr.ndim != 1:
                raise ValueError(f"indptr can only be 1D. {VALIDATOR_MSG}")
            if indptr[0] != 0 or indptr[-1] != data.size:
                raise ValueError(f"indptr should start from zero and end at size of data array. {VALIDATOR_MSG}")
            if np.any(np.diff(indptr) < 0):
                raise ValueError(f"indptr should be increasing. {VALIDATOR_MSG}")
            indptr_c = np.ascontiguousarray(indptr, dtype=IdxNp).ctypes.data_as(IdxPtr)
        # convert data array
        data_c = np.ascontiguousarray(data, dtype=schema[component_name].dtype).ctypes.data_as(c_void_p)
        dataset_dict[component_name] = CBuffer(
            data=data_c, indptr=indptr_c, n_elements_per_scenario=n_elements_per_scenario, batch_size=batch_size
        )
    # total set
    n_components = len(dataset_dict)
    if n_components == 0:
        batch_size = 1
    else:
        batch_sizes = np.array([x.batch_size for x in dataset_dict.values()])
        if np.unique(batch_sizes).size > 1:
            raise ValueError(f"Batch sizes across all the types should be the same! {VALIDATOR_MSG}")
        batch_size = batch_sizes[0]
    return CDataset(
        dataset=dataset_dict,
        batch_size=batch_size,
        n_components=n_components,
        components=(c_char_p * n_components)(*(x.encode() for x in dataset_dict)),
        n_component_elements_per_scenario=(IdxC * n_components)(
            *(x.n_elements_per_scenario for x in dataset_dict.values())
        ),
        indptrs_per_component=(IdxPtr * n_components)(*(x.indptr for x in dataset_dict.values())),  # type: ignore
        data_ptrs_per_component=(c_void_p * n_components)(*(x.data for x in dataset_dict.values())),
    )
