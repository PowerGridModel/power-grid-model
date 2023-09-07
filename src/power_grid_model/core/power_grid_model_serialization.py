# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0


"""
Power grid model (de)serialization
"""

from ctypes import byref
from enum import IntEnum
from typing import Callable, Dict, Mapping, Tuple, Union

import numpy as np

from power_grid_model.core.error_handling import assert_no_error
from power_grid_model.core.index_integer import IdxC
from power_grid_model.core.power_grid_core import CharPtr
from power_grid_model.core.power_grid_core import power_grid_core as pgc
from power_grid_model.core.power_grid_dataset import CConstDataset, CWritableDataset
from power_grid_model.errors import PowerGridSerializationError


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

        self._dataset_ptr = pgc.deserializer_get_dataset(self._deserializer)
        assert_no_error()

        self._dataset = CWritableDataset(self._dataset_ptr)
        assert_no_error()

    def __del__(self):
        pgc.destroy_deserializer(self._deserializer)

    def load(self) -> Tuple[str, Dict[str, np.ndarray]]:
        """
        Load the deserialized data to a new dataset.

        Returns:
            A tuple containing:
                the type of the data set
                the deserialized data set in Power grid model input format
        """
        pgc.deserializer_parse_to_buffer(self._deserializer)
        return self._dataset.get_info().dataset_type(), self._dataset.get_data()


class Serializer:
    """
    Serializer for the Power grid model
    """

    def __init__(
        self,
        dataset_type,
        data: Dict[str, Union[np.ndarray, Mapping[str, np.ndarray]]],
        serialization_type: SerializationType,
    ):
        self._data = data
        self._dataset_type = dataset_type

        if self._data:
            first = next(iter(self._data.values()))
            if not isinstance(first, np.ndarray):
                raise PowerGridSerializationError("Sparse batch data sets are not supported.")

        self._dataset = CConstDataset(self._dataset_type, self._data)
        assert_no_error()

        self._serializer = pgc.create_serializer(self._dataset.get_dataset_ptr(), serialization_type.value)
        assert_no_error()

    def __del__(self):
        pgc.destroy_serializer(self._serializer)

    def dump_str(self, *, use_compact_list: bool = False, indent: int = 2) -> str:
        """
        Dump the data to a decoded str; if supported.

        Args:
            use_compact_list: whether or not to use compact lists (sparse data). Defaults to False.
            indent:
                use specified indentation to make data more readable
                Use 0 or negative value for no indentation. Defaults to 2

        Returns:
            a serialized string containing dataset
        """
        data = pgc.serializer_get_to_zero_terminated_string(self._serializer, int(use_compact_list), indent)
        assert_no_error()
        return data

    def dump_bytes(self, *, use_compact_list: bool = False) -> bytes:
        """
        Dump the data to a bytes object; if supported.

        Args:
            use_compact_list (bool, optional): whether or not to use compact lists (sparse data). Defaults to False.

        Returns:
            the raw bytes of the serialization of the datast
        """
        raw_data = CharPtr()  # pylint: disable(not-callable)
        size = IdxC()  # pylint: disable(not-callable)
        pgc.serializer_get_to_binary_buffer(self._serializer, int(use_compact_list), byref(raw_data), byref(size))
        assert_no_error()

        result = raw_data[: size.value]
        if not isinstance(result, bytes):
            raise PowerGridSerializationError("Invalid output data type")

        return result

    dump: Callable = dump_bytes
    """
    Dump the data in the recommended data format (e.g. str or bytes, depending on the serialization_type).

    Args:
        *: any serialization_type dependent args
    """


class _BytesSerializer(Serializer):
    """
    Base type for serialization to bytes
    """

    dump = Serializer.dump_bytes


class _StringSerializer(Serializer):
    """
    Base type for serialization to str
    """

    dump = Serializer.dump_str


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


class JsonSerializer(_StringSerializer):  # pylint: disable=too-few-public-methods
    """
    JSON deserializer for the Power grid model
    """

    def __init__(self, dataset_type: str, data: Dict[str, Union[np.ndarray, Mapping[str, np.ndarray]]]):
        super().__init__(dataset_type, data, SerializationType.JSON)


class MsgpackSerializer(_BytesSerializer):  # pylint: disable=too-few-public-methods
    """
    msgpack deserializer for the Power grid model
    """

    def __init__(self, dataset_type: str, data: Dict[str, Union[np.ndarray, Mapping[str, np.ndarray]]]):
        super().__init__(dataset_type, data, SerializationType.MSGPACK)
