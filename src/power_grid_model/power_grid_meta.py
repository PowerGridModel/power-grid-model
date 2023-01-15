# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0


from power_grid_model.power_grid_core import power_grid_core as pgc
import numpy as np


_ctype_numpy_map = {
    "double": "f8",
    "int32_t": "i4",
    "int8_t": "i1",
    "double[3]": "(3,)f8"
}
_endianness = '<' if pgc.is_little_endian() else '>'
_nan_value_map = {
    f'{_endianness}f8': np.nan,
    f'{_endianness}(3,)f8': np.nan,
    f'{_endianness}i4': np.iinfo(f'{_endianness}i4').min,
    f'{_endianness}i1': np.iinfo(f'{_endianness}i1').min,
}


def _generate_meta_data() -> dict:
    """

    Returns:

    """
    py_meta_data = {}
    n_dataset = pgc.meta_n_datasets()
    for i in range(n_dataset):
        dataset = pgc.meta_dataset_name(i).decode()
        py_meta_data[dataset] = _generate_meta_dataset(dataset)
    return py_meta_data


def _generate_meta_dataset(dataset: str) -> dict:
    """

    Args:
        dataset:

    Returns:

    """
    py_meta_dataset = {}
    n_classes = pgc.meta_n_classes(dataset.encode())
    for i in range(n_classes):
        class_name = pgc.meta_class_name(dataset, i).decode()
        py_meta_dataset[class_name] = _generate_meta_class(dataset, class_name)
    return py_meta_dataset


def _generate_meta_class(dataset: str, class_name: str) -> dict:
    """

    Args:
        dataset:
        class_name:

    Returns:

    """
    py_meta_class = {}
    return py_meta_class


power_grid_meta_data = _generate_meta_data()
