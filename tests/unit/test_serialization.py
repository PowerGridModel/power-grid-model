# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

import json

import msgpack
import pytest

from power_grid_model import JsonDeserializer, MsgpackDeserializer
from power_grid_model.core.error_handling import assert_no_error
from power_grid_model.errors import PowerGridSerializationError


def basic_dict(data_type: str = "input"):
    return {"version": "0.0", "type": data_type, "is_batch": False, "data": {}, "attributes": {}}


def to_json(data):
    return json.dumps(data)


def to_msgpack(data):
    return msgpack.packb(data)


@pytest.mark.parametrize("raw_buffer", (True, False))
def test_json_deserializer_create_destroy(raw_buffer: bool):
    data = to_json(basic_dict())
    if raw_buffer:
        data = bytes(data, "utf-8")

    deserializer = JsonDeserializer(data)
    assert_no_error()
    assert deserializer
    del deserializer


def test_msgpack_deserializer_create_destroy():
    data = to_msgpack(basic_dict())
    deserializer = MsgpackDeserializer(data)
    assert_no_error()
    assert deserializer
    del deserializer
