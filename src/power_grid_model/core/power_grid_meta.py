# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Load meta data from C core and define numpy structured array

[deprecated]
"""

from power_grid_model._core.power_grid_meta import (  # pylint: disable=unused-import
    Any,
    AttributePtr,
    ComponentMetaData,
    ComponentPtr,
    ComponentTypeLike,
    ComponentTypeVar,
    DatasetMetaData,
    DatasetPtr,
    DatasetType,
    DatasetTypeLike,
    DenseBatchArray,
    IntEnum,
    PGMCType,
    PowerGridMetaData,
    SingleArray,
    dataclass,
    initialize_array,
    np,
    pgc,
    power_grid_meta_data,
)
