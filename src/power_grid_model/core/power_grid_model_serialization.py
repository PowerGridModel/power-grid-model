# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0


"""
Power grid model (de)serialization
"""

from enum import IntEnum
from typing import Union

from power_grid_model.core.error_handling import assert_no_error
from power_grid_model.core.power_grid_core import power_grid_core as pgc


class SerializationType(IntEnum):
    """Serialization Format Types"""

    JSON = 0
    MSGPACK = 1


class Deserializer:
    """
    Deserializer for the Power grid model
    """

    def __init__(self, data: Union[str, bytes], serialization_type: SerializationType):
        raw_data = data if isinstance(data, bytes) else data.encode()
        self._deserializer = pgc.create_deserializer_from_binary_buffer(
            raw_data, len(raw_data), serialization_type.value
        )
        assert_no_error()

    def __del__(self):
        pgc.destroy_deserializer(self._deserializer)
        assert_no_error()


class JsonDeserializer(Deserializer):  # pylint: disable=too-few-public-methods
    """
    JSON deserializer for the Power grid model
    """

    def __init__(self, data: Union[str, bytes]):
        super().__init__(data, SerializationType.JSON)


class MsgpackDeserializer(Deserializer):  # pylint: disable=too-few-public-methods
    """
    msgpack deserializer for the Power grid model
    """

    def __init__(self, data: bytes):
        super().__init__(data, SerializationType.MSGPACK)
