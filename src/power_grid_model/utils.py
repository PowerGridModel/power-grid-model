# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

"""
This file contains all the helper functions for testing purpose
"""

import json
import typing
import warnings
from pathlib import Path
from typing import Optional
from typing import cast as cast_type

import numpy as np

from power_grid_model.core.power_grid_dataset import get_dataset_type
from power_grid_model.core.serialization import (  # pylint: disable=unused-import
    json_deserialize,
    json_serialize,
    msgpack_deserialize,
    msgpack_serialize,
)
from power_grid_model.data_types import BatchDataset, Dataset, SingleDataset
from power_grid_model.errors import PowerGridError, PowerGridSerializationError

_DEPRECATED_METHOD_MSG = "This method is deprecated."
_DEPRECATED_METHOD_IMPORT_JSON_DATA_MSG = f"{_DEPRECATED_METHOD_MSG} Please use import_json_data instead."


def import_json_data(json_file: Path, data_type: str, *args, **kwargs) -> Dataset:
    """
    Import json data.

    This method is deprecated. Please use import_json_deserialize instead.

    Args:
        json_file: path to the json file
        data_type: type of data: input, update, sym_output, or asym_output
        args: All extra positional arguments are ignored for compatibility.
        kwargs: All extra keyword arguments are ignored for compatibility.

    Returns:
        A single or batch dataset for power-grid-model
    """
    if args:
        warnings.warn(
            "Provided positional arguments at index 2 and following may be deprecated and are ignored.",
            DeprecationWarning,
            stacklevel=2,
        )
    if kwargs:
        warnings.warn(
            f"Provided keyword arguments {list(kwargs.keys())} may be deprecated and are ignored.",
            DeprecationWarning,
            stacklevel=2,
        )

    with open(json_file, mode="r", encoding="utf-8") as file_pointer:
        try:
            return json_deserialize(file_pointer.read())

        except Exception as exception:
            try:
                file_pointer.seek(0)
                data = _compatibility_deprecated_import_json_data_v0(file_pointer=file_pointer, data_type=data_type)
                warnings.warn(
                    "The file you provided contains a deprecated format for which support may be removed in the future."
                    " Consider upgrading it using 'export_json_data(json_file, import_json_data(json_file))'.",
                    DeprecationWarning,
                    stacklevel=2,
                )
                return data
            except (PowerGridError, ValueError):
                pass

            raise exception


def export_json_data(json_file: Path, data: Dataset, indent: Optional[int] = 2, compact: bool = False):
    """
    Export json data in most recent format.

    Args:
        json_file: path to json file
        data: a single or batch dataset for power-grid-model
        indent: indent of the file, default 2
        compact: write components on a single line

    Returns:
        Save to file
    """
    with open(json_file, mode="w", encoding="utf-8") as file_pointer:
        file_pointer.write(json_serialize(data=data, indent=0 if indent is None else indent, use_compact_list=compact))


def import_input_data(json_file: Path) -> SingleDataset:
    """
    [deprecated] Import input json data.

    This method is deprecated. Please use import_json_deserialize instead.

    Args:
        json_file: path to the json file

    Returns:
        A single dataset for power-grid-model
    """
    warnings.warn(_DEPRECATED_METHOD_IMPORT_JSON_DATA_MSG, DeprecationWarning, stacklevel=2)

    data = import_json_data(json_file=json_file, data_type="input")
    assert isinstance(data, dict)
    assert all(isinstance(component, np.ndarray) and component.ndim == 1 for component in data.values())
    return cast_type(SingleDataset, data)


def import_update_data(json_file: Path) -> BatchDataset:
    """
    [deprecated] Import update json data.

    This method is deprecated. Please use import_json_data instead.

    Args:
        json_file: path to the json file

    Returns:
        A batch dataset for power-grid-model
    """
    warnings.warn(_DEPRECATED_METHOD_IMPORT_JSON_DATA_MSG, DeprecationWarning, stacklevel=2)

    return cast_type(BatchDataset, import_json_data(json_file=json_file, data_type="update"))


def _compatibility_deprecated_import_json_data_v0(file_pointer: typing.IO, data_type: str):
    data = json.load(file_pointer)
    if "version" not in data:  # convert old format to version 1.0
        data = {
            "attributes": {},
            "data": data,
            "is_batch": isinstance(data, list),
            "type": data_type,
            "version": "1.0",
        }

    result = json_deserialize(json.dumps(data))
    if get_dataset_type(result) != data_type:
        raise PowerGridSerializationError("An internal error occured during deserialization")

    return result
