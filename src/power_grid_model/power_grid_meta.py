# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0
from typing import Union

import numpy as np

from power_grid_model.power_grid_core import power_grid_core as pgc

_ctype_numpy_map = {"double": "f8", "int32_t": "i4", "int8_t": "i1", "double[3]": "(3,)f8"}
_endianness = "<" if pgc.is_little_endian() == 1 else ">"
_nan_value_map = {
    f"{_endianness}f8": np.nan,
    f"{_endianness}(3,)f8": np.nan,
    f"{_endianness}i4": np.iinfo(f"{_endianness}i4").min,
    f"{_endianness}i1": np.iinfo(f"{_endianness}i1").min,
}


def _generate_meta_data() -> dict:
    """

    Returns:

    """
    py_meta_data = {}
    n_dataset = pgc.meta_n_datasets()
    for i in range(n_dataset):
        dataset = pgc.meta_dataset_name(i)
        py_meta_data[dataset] = _generate_meta_dataset(dataset)
    return py_meta_data


def _generate_meta_dataset(dataset: str) -> dict:
    """

    Args:
        dataset:

    Returns:

    """
    py_meta_dataset = {}
    n_classes = pgc.meta_n_classes(dataset)
    for i in range(n_classes):
        class_name = pgc.meta_class_name(dataset, i)
        py_meta_dataset[class_name] = _generate_meta_class(dataset, class_name)
    return py_meta_dataset


def _generate_meta_class(dataset: str, class_name: str) -> dict:
    """

    Args:
        dataset:
        class_name:

    Returns:

    """

    numpy_dtype_dict = _generate_meta_attributes(dataset, class_name)
    dtype = np.dtype({k: v for k, v in numpy_dtype_dict.items() if k != "nans"})
    if dtype.alignment != pgc.meta_class_alignment(dataset, class_name):
        raise TypeError(f'Aligment mismatch for component type: "{class_name}" !')
    py_meta_class = {
        "dtype": dtype,
        "dtype_dict": numpy_dtype_dict,
        "nans": {x: y for x, y in zip(numpy_dtype_dict["names"], numpy_dtype_dict["nans"])},
    }
    # get single nan scalar
    nan_scalar = np.zeros(1, dtype=py_meta_class["dtype"])
    for k, v in py_meta_class["nans"].items():
        nan_scalar[k] = v
    py_meta_class["nan_scalar"] = nan_scalar
    return py_meta_class


def _generate_meta_attributes(dataset: str, class_name: str) -> dict:
    """

    Args:
        dataset:
        class_name:

    Returns:

    """
    numpy_dtype_dict = {
        "names": [],
        "formats": [],
        "offsets": [],
        "itemsize": pgc.meta_class_size(dataset, class_name),
        "aligned": True,
        "nans": [],
    }
    n_attrs = pgc.meta_n_attributes(dataset, class_name)
    for i in range(n_attrs):
        field_name: str = pgc.meta_attribute_name(dataset, class_name, i)
        field_ctype: str = pgc.meta_attribute_ctype(dataset, class_name, field_name)
        field_offset: int = pgc.meta_attribute_offset(dataset, class_name, field_name)
        field_np_type = f"{_endianness}{_ctype_numpy_map[field_ctype]}"
        field_nan = _nan_value_map[field_np_type]
        numpy_dtype_dict["names"].append(field_name)
        numpy_dtype_dict["formats"].append(field_np_type)
        numpy_dtype_dict["offsets"].append(field_offset)
        numpy_dtype_dict["nans"].append(field_nan)
    return numpy_dtype_dict


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
        return np.empty(shape=shape, dtype=power_grid_meta_data[data_type][component_type]["dtype"], order="C")
    else:
        return np.full(
            shape=shape,
            fill_value=power_grid_meta_data[data_type][component_type]["nan_scalar"],
            dtype=power_grid_meta_data[data_type][component_type]["dtype"],
            order="C",
        )
