# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

from pathlib import Path
from typing import Dict
from unittest.mock import MagicMock, mock_open, patch

import numpy as np
import pytest

from power_grid_model import LoadGenType, initialize_array
from power_grid_model.core.power_grid_meta import power_grid_meta_data
from power_grid_model.data_types import Dataset
from power_grid_model.utils import (
    export_json_data,
    get_component_batch_size,
    get_dataset_batch_size,
    get_dataset_scenario,
    json_deserialize_from_file,
    json_serialize_to_file,
    msgpack_deserialize_from_file,
    msgpack_serialize_to_file,
)


def test_get_dataset_scenario():
    data = {
        "foo": np.array([["bar", "baz"], ["foobar", "foobaz"]]),
        "hi": {
            "data": np.array(["hello", "hey"]),
            "indptr": np.array([0, 0, 2]),
        },
    }
    result = get_dataset_scenario(data, 0)
    assert result.keys() == data.keys()
    np.testing.assert_array_equal(result["foo"], data["foo"][0])
    np.testing.assert_array_equal(result["hi"], data["hi"]["data"][0:0])

    result = get_dataset_scenario(data, 1)
    assert result.keys() == data.keys()
    np.testing.assert_array_equal(result["foo"], data["foo"][1])
    np.testing.assert_array_equal(result["hi"], data["hi"]["data"][0:2])

    with pytest.raises(IndexError):
        get_dataset_scenario(data, 2)


def test_get_data_set_batch_size():
    line = initialize_array("update", "line", (3, 2))
    line["id"] = [[5, 6], [6, 7], [7, 5]]
    line["from_status"] = [[1, 1], [1, 1], [1, 1]]

    asym_load = initialize_array("update", "asym_load", (3, 2))
    asym_load["id"] = [[9, 10], [9, 10], [9, 10]]

    batch_data = {"line": line, "asym_load": asym_load}

    assert get_dataset_batch_size(batch_data) == 3


def test_get_dataset_batch_size_sparse():
    data = {
        "node": {
            "data": np.zeros(shape=3, dtype=power_grid_meta_data["input"]["node"]),
            "indptr": np.array([0, 2, 3, 3]),
        },
        "sym_load": {
            "data": np.zeros(shape=2, dtype=power_grid_meta_data["input"]["sym_load"]),
            "indptr": np.array([0, 0, 1, 2]),
        },
        "asym_load": {
            "data": np.zeros(shape=4, dtype=power_grid_meta_data["input"]["asym_load"]),
            "indptr": np.array([0, 2, 3, 4]),
        },
    }

    assert get_dataset_batch_size(data) == 3


def test_get_dataset_batch_size_mixed():
    line = initialize_array("update", "line", (3, 2))
    line["id"] = [[5, 6], [6, 7], [7, 5]]
    line["from_status"] = [[1, 1], [1, 1], [1, 1]]

    asym_load = initialize_array("update", "asym_load", (2, 2))
    asym_load["id"] = [[9, 10], [9, 10]]

    data_dense = {"line": line, "asym_load": asym_load}
    data_sparse = {
        "node": {
            "data": np.zeros(shape=3, dtype=power_grid_meta_data["input"]["node"]),
            "indptr": np.array([0, 2, 3, 3, 5]),
        },
        "sym_load": {
            "data": np.zeros(shape=2, dtype=power_grid_meta_data["input"]["sym_load"]),
            "indptr": np.array([0, 0, 1, 2]),
        },
        "asym_load": {
            "data": np.zeros(shape=4, dtype=power_grid_meta_data["input"]["asym_load"]),
            "indptr": np.array([0, 2, 3]),
        },
    }
    with pytest.raises(ValueError):
        get_dataset_batch_size(data_dense)
    with pytest.raises(ValueError):
        get_dataset_batch_size(data_sparse)


def test_get_component_batch_size():
    asym_load = initialize_array("update", "asym_load", (3, 2))
    asym_load["id"] = [[9, 10], [9, 10], [9, 10]]

    sym_load = {
        "data": np.zeros(shape=2, dtype=power_grid_meta_data["input"]["sym_load"]),
        "indptr": np.array([0, 0, 1, 2]),
    }
    assert get_component_batch_size(asym_load) == 3
    assert get_component_batch_size(sym_load) == 3


@patch("builtins.open", new_callable=mock_open)
@patch("power_grid_model.utils.json_deserialize")
def test_json_deserialize_from_file(deserialize_mock: MagicMock, open_mock: MagicMock):
    handle = open_mock()
    handle.read.return_value = '{"version": "1.0", "data": {"foo": [{"val": 123}]}, "bar": {"baz": 456}}'
    deserialize_mock.return_value = {"foo": [{"val": 123}]}  # type: ignore
    assert json_deserialize_from_file(file_path=Path("output.json")) == deserialize_mock.return_value
    handle.read.assert_called_once()
    deserialize_mock.assert_called_once_with(handle.read.return_value)


@patch("builtins.open", new_callable=mock_open)
@patch("power_grid_model.utils.json_serialize")
def test_json_serialize(serialize_mock: MagicMock, open_mock: MagicMock):
    serialize_mock.return_value = '{"version": "1.0", "data": {"foo": [{"val": 123}]}, "bar": {"baz": 456}}'
    data: Dataset = {}  # type: ignore
    json_serialize_to_file(file_path=Path("output.json"), data=data, use_compact_list=False, indent=2)
    serialize_mock.assert_called_once_with(data=data, dataset_type=None, use_compact_list=False, indent=2)
    handle = open_mock()
    handle.write.assert_called_once_with(serialize_mock.return_value)


@patch("builtins.open", new_callable=mock_open)
@patch("power_grid_model.utils.msgpack_deserialize")
def test_msgpack_deserialize_from_file(deserialize_mock: MagicMock, open_mock: MagicMock):
    handle = open_mock()
    handle.read.return_value = b'{"version": "1.0", "data": {"foo": [{"val": 123}]}, "bar": {"baz": 456}}'
    deserialize_mock.return_value = {"foo": [{"val": 123}]}  # type: ignore
    assert msgpack_deserialize_from_file(file_path=Path("output.msgpack")) == deserialize_mock.return_value
    handle.read.assert_called_once()
    deserialize_mock.assert_called_once_with(handle.read.return_value)


@patch("builtins.open", new_callable=mock_open)
@patch("power_grid_model.utils.msgpack_serialize")
def test_msgpack_serialize(serialize_mock: MagicMock, open_mock: MagicMock):
    serialize_mock.return_value = b'{"version": "1.0", "data": {"foo": [{"val": 123}]}, "bar": {"baz": 456}}'
    data: Dataset = {}  # type: ignore
    msgpack_serialize_to_file(file_path=Path("output.msgpack"), data=data, use_compact_list=False)
    serialize_mock.assert_called_once_with(data=data, dataset_type=None, use_compact_list=False)
    handle = open_mock()
    handle.write.assert_called_once_with(serialize_mock.return_value)
