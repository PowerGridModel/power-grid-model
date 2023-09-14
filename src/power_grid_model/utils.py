# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

"""
This file contains all the helper functions for testing purpose
"""

import json
import warnings
from pathlib import Path
from typing import IO, Any, Optional, cast

import numpy as np

from power_grid_model.core.serialization import (  # pylint: disable=unused-import
    json_deserialize,
    json_serialize,
    msgpack_deserialize,
    msgpack_serialize,
)
from power_grid_model.data_types import BatchDataset, Dataset, SingleDataset
from power_grid_model.errors import PowerGridSerializationError

_DEPRECATED_JSON_DESERIALIZATION_MSG = "This method is deprecated. Please use json_deserialize."
_DEPRECATED_JSON_SERIALIZATION_MSG = "This method is deprecated. Please use json_serialize."


def _compatibility_deprecated_import_json_data(json_file: Path, data_type: str):
    with open(json_file, mode="r", encoding="utf-8") as file_pointer:
        data = json.load(file_pointer)

    if "version" not in data:  # convert old format to version 1.0
        data = {
            "attributes": {},
            "data": data,
            "is_batch": isinstance(data, list),
            "type": data_type,
            "version": "1.0",
        }

    result, result_dataset_type = json_deserialize(json.dumps(data))
    if result_dataset_type != data_type:
        raise PowerGridSerializationError("An internal error occured during deserialization")

    return result


def import_json_data(json_file: Path, data_type: str, *args, **kwargs) -> Dataset:
    """
    [deprecated] import json data.

    This method is deprecated. Please use json_deserialize instead (requires loading the file yourself).

    Args:
        json_file: path to the json file
        data_type: type of data: input, update, sym_output, or asym_output
        args [deprecated]: All extra positional arguments are ignored
        kwargs [deprecated]: All extra keyword arguments are ignored

    Returns:
        A single or batch dataset for power-grid-model
    """
    warnings.warn(_DEPRECATED_JSON_DESERIALIZATION_MSG, DeprecationWarning, stacklevel=2)
    if args:
        warnings.warn(
            "Provided positional arguments at index 2 and following are deprecated.", DeprecationWarning, stacklevel=2
        )
    if kwargs:
        warnings.warn(
            f"Provided keyword arguments {list(kwargs.keys())} are deprecated.", DeprecationWarning, stacklevel=2
        )

    return _compatibility_deprecated_import_json_data(json_file=json_file, data_type=data_type)


def import_input_data(json_file: Path) -> SingleDataset:
    """
    [deprecated] import input json data

    This method is deprecated. Please use json_deserialize instead (requires loading the file yourself).

    Args:
        json_file: path to the json file

    Returns:
        A single dataset for power-grid-model
    """
    warnings.warn(_DEPRECATED_JSON_DESERIALIZATION_MSG, DeprecationWarning, stacklevel=2)

    data = _compatibility_deprecated_import_json_data(json_file=json_file, data_type="input")
    assert isinstance(data, dict)
    assert all(isinstance(component, np.ndarray) and component.ndim == 1 for component in data.values())
    return cast(SingleDataset, data)


def import_update_data(json_file: Path) -> BatchDataset:
    """
    [deprecated] import update json data

    This method is deprecated. Please use json_deserialize instead (requires loading the file yourself).

    Args:
        json_file: path to the json file

    Returns:
        A batch dataset for power-grid-model
    """
    warnings.warn(_DEPRECATED_JSON_DESERIALIZATION_MSG, DeprecationWarning, stacklevel=2)

    return cast(BatchDataset, _compatibility_deprecated_import_json_data(json_file=json_file, data_type="update"))


def export_json_data(json_file: Path, data: Dataset, indent: Optional[int] = 2, compact: bool = False):
    """
    [deprecated] export json data

    This method is deprecated. Please use json_serialize instead (requires opening the file yourself).

    Args:
        json_file: path to json file
        data: a single or batch dataset for power-grid-model
        indent: indent of the file, default 2
        compact: write components on a single line

    Returns:
        Save to file
    """
    warnings.warn(_DEPRECATED_JSON_SERIALIZATION_MSG, DeprecationWarning, stacklevel=2)

    return _compatibility_deprecated_export_json_data(json_file=json_file, data=data, indent=indent, compact=compact)


def _compatibility_deprecated_export_json_data(
    json_file: Path, data: Dataset, indent: Optional[int] = 2, compact: bool = False
):
    with open(json_file, mode="w", encoding="utf-8") as file_pointer:
        file_pointer.write(json_serialize(data=data, indent=0 if indent is None else indent, use_compact_list=compact))


def compact_json_dump(data: Any, io_stream: IO[str], indent: int, max_level: int, level: int = 0):
    """Custom compact JSON writer that is intended to put data belonging to a single object on a single line.

    For example::
    {
        "node": [
            {"id": 0, "u_rated": 10500.0},
            {"id": 1, "u_rated": 10500.0},
        ],
        "line": [
            {"id": 2, "node_from": 0, "node_to": 1, ...}
        ]
    }

    The function is being called recursively, starting at level 0 and recursing until max_level is reached. It is
    basically a full json writer, but for efficiency reasons, on the last levels the native json.dump method is used.
    """
    warnings.warn(_DEPRECATED_JSON_SERIALIZATION_MSG, DeprecationWarning, stacklevel=2)
    return _compatibility_deprecated_compact_json_dump(
        data=data, io_stream=io_stream, indent=indent, max_level=max_level, level=level
    )


def _compatibility_deprecated_compact_json_dump(
    data: Any, io_stream: IO[str], indent: int, max_level: int, level: int = 0
):
    # Let's define a 'tab' indent, depending on the level
    tab = " " * level * indent

    # If we are at the max_level, or the data simply doesn't contain any more levels, write the indent and serialize
    # the data on a single line.
    if level >= max_level or not isinstance(data, (list, dict)):
        io_stream.write(tab)
        json.dump(data, io_stream, indent=None)
        return

    # We'll need the number of objects later on
    n_obj = len(data)

    # If the data is a list:
    # 1. start with an opening bracket
    # 2. dump each element in the list
    # 3. add a comma and a new line after each element, except for the last element, there we don't need a comma.
    # 4. finish with a closing bracket
    if isinstance(data, list):
        io_stream.write(tab + "[\n")
        for i, obj in enumerate(data, start=1):
            compact_json_dump(obj, io_stream, indent, max_level, level + 1)
            io_stream.write(",\n" if i < n_obj else "\n")
        io_stream.write(tab + "]")
        return

    # If the data is a dictionary:
    # 1. start with an opening curly bracket
    # 2. for each element: write it's key, plus a colon ':'
    # 3. if the next level would be the max_level, add a space and dump the element on a single,
    #    else add a new line before dumping the element recursively.
    # 4. add a comma and a new line after each element, except for the last element, there we don't need a comma.
    # 5. finish with a closing curly bracket
    io_stream.write(tab + "{\n")
    for i, (key, obj) in enumerate(data.items(), start=1):
        io_stream.write(tab + " " * indent + f'"{key}":')
        if level == max_level - 1 or not isinstance(obj, (list, dict)):
            io_stream.write(" ")
            json.dump(obj, io_stream, indent=None)
        else:
            io_stream.write("\n")
            compact_json_dump(obj, io_stream, indent, max_level, level + 2)
        io_stream.write(",\n" if i < n_obj else "\n")
    io_stream.write(tab + "}\n")
