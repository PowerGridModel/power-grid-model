# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0


"""
Power grid model (de)serialization
"""

from enum import IntEnum
from typing import Dict, Union

import numpy as np

from power_grid_model.core.data_handling import create_dataset_from_info
from power_grid_model.core.error_handling import assert_no_error
from power_grid_model.core.power_grid_core import power_grid_core as pgc
from power_grid_model.core.power_grid_dataset import DatasetInfo
from power_grid_model.core.power_grid_meta import prepare_cpp_array
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

        self._info_ptr = pgc.dataset_writable_get_info(self._dataset_ptr)
        assert_no_error()

        self._info = DatasetInfo(self._info_ptr)
        assert_no_error()

        # TODO(mgovers): we do not support sparse batches yet
        if any(count == -1 for count in self._info.elements_per_scenario.values()):
            raise PowerGridSerializationError("Sparse batch data sets are not supported.")

    def __del__(self):
        pgc.destroy_deserializer(self._deserializer)
        assert_no_error()

    def load(self) -> Dict[str, np.ndarray]:
        """Load the deserialized data to a new data set."""
        dataset = create_dataset_from_info(self._info)
        self._deserialize(target_dataset=dataset)
        return dataset

    def _deserialize(self, target_dataset: Dict[str, np.ndarray]):
        raw_dataset_view = prepare_cpp_array(data_type=self._info.name, array_dict=target_dataset)
        for component, buffer in raw_dataset_view.dataset.items():
            pgc.dataset_writable_set_buffer(self._dataset_ptr, component, buffer.indptr, buffer.data)

        pgc.deserializer_parse_to_buffer(self._deserializer)


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
