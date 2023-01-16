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
        class_name = pgc.meta_class_name(dataset.encode(), i).decode()
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
    dtype = np.dtype({k: v for k, v in numpy_dtype_dict.items() if k != 'nans'})
    if dtype.alignment != pgc.meta_class_alignment(dataset.encode(), class_name.encode()):
        raise TypeError(f'Aligment mismatch for component type: "{class_name}" !')
    py_meta_class = {
        'dtype': dtype,
        'dtype_dict': numpy_dtype_dict,
        'nans': {x: y for x, y in zip(numpy_dtype_dict['names'], numpy_dtype_dict['nans'])}
    }
    # get single nan scalar
    nan_scalar = np.zeros(1, dtype=py_meta_class['dtype'])
    for k, v in py_meta_class['nans'].items():
        nan_scalar[k] = v
    py_meta_class['nan_scalar'] = nan_scalar
    return py_meta_class


def _generate_meta_attributes(dataset: str, class_name: str) -> dict:
    """

    Args:
        dataset:
        class_name:

    Returns:

    """
    numpy_dtype_dict = {
        'names': [],
        'formats': [],
        'offsets': [],
        'itemsize': pgc.meta_class_size(dataset.encode(), class_name.encode()),
        'aligned': True,
        'nans': []
    }
    n_attrs = pgc.meta_n_attributes(dataset.encode(), class_name.encode())
    for i in range(n_attrs):
        field_name: str = pgc.meta_attribute_name(dataset.encode(), class_name.encode(), i).decode()
        field_ctype: str = pgc.meta_attribute_ctype(dataset.encode(), class_name.encode(), field_name.encode()).decode()
        field_offset: int = pgc.meta_attribute_offset(dataset.encode(), class_name.encode(), field_name.encode())
        field_np_type = f"{_endianness}{_ctype_numpy_map[field_ctype]}"
        field_nan = _nan_value_map[field_np_type]
        numpy_dtype_dict['names'].append(field_name)
        numpy_dtype_dict['formats'].append(field_np_type)
        numpy_dtype_dict['offsets'].append(field_offset)
        numpy_dtype_dict['nans'].append(field_nan)
    return numpy_dtype_dict


power_grid_meta_data = _generate_meta_data()
