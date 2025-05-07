# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Power grid model (de)serialization
"""

from abc import ABC, abstractmethod
from ctypes import byref
from enum import IntEnum

from power_grid_model._core.data_types import Dataset
from power_grid_model._core.dataset_definitions import DatasetType, _map_to_component_types, _str_to_datatype
from power_grid_model._core.error_handling import assert_no_error
from power_grid_model._core.errors import PowerGridSerializationError
from power_grid_model._core.index_integer import IdxC
from power_grid_model._core.power_grid_core import (
    CharPtr,
    DeserializerPtr,
    SerializerPtr,
    WritableDatasetPtr,
    power_grid_core as pgc,
)
from power_grid_model._core.power_grid_dataset import CConstDataset, CWritableDataset
from power_grid_model._core.typing import ComponentAttributeMapping


class SerializationType(IntEnum):
    """Serialization Format Types"""

    JSON = 0
    MSGPACK = 1


class Deserializer:
    """
    Deserializer for the Power grid model
    """

    _deserializer: DeserializerPtr
    _dataset_ptr: WritableDatasetPtr
    _dataset: CWritableDataset
    _data_filter: ComponentAttributeMapping

    def __new__(
        cls,
        data: str | bytes,
        serialization_type: SerializationType,
        data_filter: ComponentAttributeMapping,
    ):
        instance = super().__new__(cls)

        raw_data = data if isinstance(data, bytes) else data.encode()
        instance._deserializer = pgc.create_deserializer_from_binary_buffer(
            raw_data, len(raw_data), serialization_type.value
        )
        assert_no_error()

        instance._dataset_ptr = pgc.deserializer_get_dataset(instance._deserializer)
        assert_no_error()

        instance._data_filter = data_filter
        instance._dataset = CWritableDataset(instance._dataset_ptr, data_filter=data_filter)
        assert_no_error()

        return instance

    def __del__(self):
        if hasattr(self, "_deserializer"):
            pgc.destroy_deserializer(self._deserializer)

    def load(self) -> Dataset:
        """
        Load the deserialized data to a new dataset.

        Raises:
            ValueError: if the data is inconsistent with the rest of the dataset or a component is unknown.
            PowerGridError: if there was an internal error.

        Returns:
            A tuple containing the deserialized dataset in Power grid model input format and the type of the dataset.
        """
        pgc.deserializer_parse_to_buffer(self._deserializer)
        return self._dataset.get_data()


class Serializer(ABC):
    """
    Serializer for the Power grid model
    """

    _data: Dataset
    _dataset: CConstDataset
    _serializer: SerializerPtr

    def __new__(cls, data: Dataset, serialization_type: SerializationType, dataset_type: DatasetType | None = None):
        instance = super().__new__(cls)

        instance._data = data
        instance._dataset = CConstDataset(instance._data, dataset_type=dataset_type)
        assert_no_error()

        instance._serializer = pgc.create_serializer(instance._dataset.get_dataset_ptr(), serialization_type.value)
        assert_no_error()

        return instance

    def __del__(self):
        if hasattr(self, "_serializer"):
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
            A serialized string containing the dataset.
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
            The raw bytes of the serialization of the datast.
        """
        raw_data = CharPtr()  # pylint: disable(not-callable)
        size = IdxC()  # pylint: disable(not-callable)
        pgc.serializer_get_to_binary_buffer(self._serializer, int(use_compact_list), byref(raw_data), byref(size))
        assert_no_error()

        result = raw_data[: size.value]
        if not isinstance(result, bytes):
            raise PowerGridSerializationError("Invalid output data type")

        return result

    @abstractmethod
    def dump(self, *args, **kwargs):
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

    def __new__(cls, data: str | bytes, data_filter: ComponentAttributeMapping):
        return super().__new__(cls, data, SerializationType.JSON, data_filter=data_filter)


class MsgpackDeserializer(Deserializer):  # pylint: disable=too-few-public-methods
    """
    msgpack deserializer for the Power grid model
    """

    def __new__(cls, data: bytes, data_filter: ComponentAttributeMapping):
        return super().__new__(cls, data, SerializationType.MSGPACK, data_filter=data_filter)


class JsonSerializer(_StringSerializer):  # pylint: disable=too-few-public-methods
    """
    JSON deserializer for the Power grid model
    """

    def __new__(cls, data: Dataset, dataset_type: DatasetType | None = None):
        return super().__new__(cls, data, SerializationType.JSON, dataset_type=dataset_type)


class MsgpackSerializer(_BytesSerializer):  # pylint: disable=too-few-public-methods
    """
    msgpack deserializer for the Power grid model
    """

    def __new__(cls, data: Dataset, dataset_type: DatasetType | None = None):
        return super().__new__(cls, data, SerializationType.MSGPACK, dataset_type=dataset_type)


def json_deserialize(
    data: str | bytes,
    data_filter: ComponentAttributeMapping = None,
) -> Dataset:
    """
    Load serialized JSON data to a new dataset.

    Args:
        data: the data to deserialize.
        data_filter: the data filter to apply to the dataset.

    Raises:
        ValueError: if the data is inconsistent with the rest of the dataset or a component is unknown.
        PowerGridError: if there was an internal error.

    Returns:
        A tuple containing the deserialized dataset in Power grid model input format and the type of the dataset.
    """
    result = JsonDeserializer(data, data_filter=data_filter).load()
    assert_no_error()
    return result


def json_serialize(
    data: Dataset,
    dataset_type: DatasetType | None = None,
    use_compact_list: bool = False,
    indent: int = 2,
) -> str:
    """
    Dump data to a JSON str.

    If the dataset_type is not specified or None, it will be deduced from the dataset if possible.
    Deduction is not possible in case data is empty.

    Args:
        data: the dataset
        dataset_type: the type of the dataset. Defaults to None. Required str-type if data is empty.
        use_compact_list: whether or not to use compact lists (sparse data). Defaults to False.
        indent:
            use specified indentation to make data more readable
            Use 0 or negative value for no indentation. Defaults to 2

    Raises:
        PowerGridError: if there was an internal error.

    Returns:
        A serialized string containing the dataset.
    """
    data = _map_to_component_types(data)
    if dataset_type is not None:
        dataset_type = _str_to_datatype(dataset_type)
    result = JsonSerializer(data=data, dataset_type=dataset_type).dump(use_compact_list=use_compact_list, indent=indent)
    assert_no_error()
    return result


def msgpack_deserialize(data: bytes, data_filter: ComponentAttributeMapping = None) -> Dataset:
    """
    Load serialized msgpack data to a new dataset.

    Args:
        data: the data to deserialize.

    Raises:
        ValueError: if the data is inconsistent with the rest of the dataset or a component is unknown.
        PowerGridError: if there was an internal error.

    Returns:
        A tuple containing the deserialized dataset in Power grid model input format and the type of the dataset.
    """
    result = MsgpackDeserializer(data, data_filter=data_filter).load()
    assert_no_error()
    return result


def msgpack_serialize(
    data: Dataset,
    dataset_type: DatasetType | None = None,
    use_compact_list: bool = False,
) -> bytes:
    """
    Dump the data to raw msgpack bytes.

    If the dataset_type is not specified or None, it will be deduced from the dataset if possible.
    Deduction is not possible in case data is empty.

    Args:
        data: the dataset
        dataset_type: the type of the dataset. Defaults to None. Required str-type if data is empty.
        use_compact_list: whether or not to use compact lists (sparse data). Defaults to False.

    Raises:
        KeyError: if the dataset_type was not provided and could not be deduced from the dataset (i.e. it was empty).
        PowerGridError: if there was an internal error.

    Returns:
        A serialized string containing the dataset.
    """
    data = _map_to_component_types(data)
    if dataset_type is not None:
        dataset_type = _str_to_datatype(dataset_type)
    result = MsgpackSerializer(data=data, dataset_type=dataset_type).dump(use_compact_list=use_compact_list)
    assert_no_error()
    return result
