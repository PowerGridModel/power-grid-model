# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0


import pytest

from power_grid_model.errors import PowerGridSerializationError
from power_grid_model.utils import json_deserialize

JSON_DATA = """{
  "version": "1.0",
  "type": "input",
  "is_batch": false,
  "attributes": {},
  "data": {}
}"""
CORRUPTED_JSON_DATA = JSON_DATA[:-1]  # Remove the last character to corrupt the JSON


def json_deserialize_valid_json():
    """Test json_deserialize with valid JSON data."""
    for _ in range(1000):
        result = json_deserialize(JSON_DATA)
        assert result == {}


def json_deserialize_invalid_json():
    """Test json_deserialize with invalid JSON data."""
    for _ in range(1000):
        with pytest.raises(PowerGridSerializationError):
            json_deserialize(CORRUPTED_JSON_DATA)


def test_python_threading_json_deserialization():
    json_deserialize_valid_json()
    json_deserialize_invalid_json()
