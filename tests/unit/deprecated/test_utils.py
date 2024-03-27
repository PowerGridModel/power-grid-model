# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

from pathlib import Path
from unittest.mock import MagicMock, mock_open, patch

import pytest

from power_grid_model.data_types import Dataset
from power_grid_model.utils import export_json_data


@patch("builtins.open", new_callable=mock_open)
@patch("power_grid_model.utils.json_serialize")
def test_export_json_data_format_version_1_0(convert_mock: MagicMock, open_mock: MagicMock):
    convert_mock.return_value = '{"version": "1.0", "data": {"foo": [{"val": 123}]}, "bar": {"baz": 456}}'
    data: Dataset = {}  # type: ignore

    with pytest.deprecated_call():
        export_json_data(json_file=Path("output.json"), data=data, indent=2, use_deprecated_format=False)

    convert_mock.assert_called_once_with(data={}, dataset_type=None, use_compact_list=False, indent=2)
    handle = open_mock()
    handle.write.assert_called_once_with(convert_mock.return_value)


@patch("builtins.open", new_callable=mock_open)
@patch("power_grid_model.utils.json_serialize")
def test_export_json_data__deprecated_format(convert_mock: MagicMock, open_mock: MagicMock):
    convert_mock.return_value = '{"version": "1.0", "data": {"foo": [{"val": 123}]}, "bar": {"baz": 456}}'
    data: Dataset = {}  # type: ignore

    with pytest.deprecated_call():
        export_json_data(json_file=Path("output.json"), data=data, indent=2)

    convert_mock.assert_called_once_with(data={}, use_compact_list=False, indent=2)
    handle = open_mock()
    handle.write.assert_called_once_with('{"foo": [{"val": 123}]}')
