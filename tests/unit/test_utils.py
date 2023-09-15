# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

from pathlib import Path
from unittest.mock import MagicMock, mock_open, patch

import numpy as np
import pytest

from power_grid_model.data_types import BatchDataset, BatchList, Dataset
from power_grid_model.utils import (
    export_json_data,
    json_deserialize_from_file,
    json_serialize_to_file,
    msgpack_deserialize,
    msgpack_deserialize_from_file,
    msgpack_serialize,
    msgpack_serialize_to_file,
)


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


@patch("builtins.open", new_callable=mock_open)
@patch("power_grid_model.utils.json_serialize")
def test_export_json_data_format_version_1_0(convert_mock: MagicMock, open_mock: MagicMock):
    convert_mock.return_value = '{"version": "1.0", "data": {"foo": [{"val": 123}]}, "bar": {"baz": 456}}'
    data: Dataset = {}  # type: ignore
    export_json_data(json_file=Path("output.json"), data=data, indent=2, use_deprecated_format=False)
    convert_mock.assert_called_once_with(data={}, dataset_type=None, use_compact_list=False, indent=2)
    handle = open_mock()
    handle.write.assert_called_once_with(convert_mock.return_value)


@patch("builtins.open", new_callable=mock_open)
@patch("power_grid_model.utils.json_serialize")
def test_export_json_data__deprecated_format(convert_mock: MagicMock, open_mock: MagicMock):
    convert_mock.return_value = '{"version": "1.0", "data": {"foo": [{"val": 123}]}, "bar": {"baz": 456}}'
    data: Dataset = {}  # type: ignore
    export_json_data(json_file=Path("output.json"), data=data, indent=2)
    convert_mock.assert_called_once_with(data={}, use_compact_list=False, indent=2)
    handle = open_mock()
    handle.write.assert_called_once_with('{"foo": [{"val": 123}]}')
