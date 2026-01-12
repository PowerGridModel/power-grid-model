# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0


import threading

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
    result = json_deserialize(JSON_DATA)
    assert result == {}


def json_deserialize_invalid_json():
    """Test json_deserialize with invalid JSON data."""
    with pytest.raises(PowerGridSerializationError):
        json_deserialize(CORRUPTED_JSON_DATA)


def test_python_threading_json_deserialization():
    for _ in range(1000):
        thread_1 = threading.Thread(target=json_deserialize_valid_json)
        thread_2 = threading.Thread(target=json_deserialize_invalid_json)
        thread_1.start()
        thread_2.start()
        thread_1.join()
        thread_2.join()
