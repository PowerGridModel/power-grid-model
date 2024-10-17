# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Power grid model buffer handler

[deprecated]
"""

from power_grid_model._core.buffer_handling import (  # pylint: disable=unused-import
    VALIDATOR_MSG,
    AttributeType,
    BufferProperties,
    CAttributeBuffer,
    CBuffer,
    ComponentData,
    ComponentMetaData,
    DenseBatchData,
    IdxC,
    IdxNp,
    IdxPtr,
    IndexPointer,
    SingleComponentData,
    SparseBatchArray,
    SparseBatchData,
    VoidPtr,
    cast,
    check_indptr_consistency,
    create_buffer,
    dataclass,
    get_buffer_properties,
    get_buffer_view,
    is_columnar,
    is_sparse,
    np,
    warnings,
)
