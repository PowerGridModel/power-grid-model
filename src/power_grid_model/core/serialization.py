# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Power grid model (de)serialization

[deprecated]
"""

from power_grid_model._core.serialization import (  # pylint: disable=unused-import
    ABC,
    CConstDataset,
    CharPtr,
    ComponentAttributeMapping,
    CWritableDataset,
    Dataset,
    DatasetType,
    Deserializer,
    DeserializerPtr,
    IdxC,
    IntEnum,
    JsonDeserializer,
    JsonSerializer,
    MsgpackDeserializer,
    MsgpackSerializer,
    PowerGridSerializationError,
    SerializationType,
    Serializer,
    SerializerPtr,
    WritableDatasetPtr,
    abstractmethod,
    assert_no_error,
    byref,
    json_deserialize,
    json_serialize,
    msgpack_deserialize,
    msgpack_serialize,
    pgc,
)
