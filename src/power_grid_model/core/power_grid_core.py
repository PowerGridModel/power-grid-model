# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

"""
Loader for the dynamic library

[deprecated]
"""

from power_grid_model._core.power_grid_core import (  # pylint: disable=unused-import
    CDLL,
    POINTER,
    AttributePtr,
    Callable,
    CharDoublePtr,
    CharPtr,
    ComponentPtr,
    ConstDatasetPtr,
    CStr,
    CStrPtr,
    DatasetInfoPtr,
    DatasetPtr,
    DeserializerPtr,
    HandlePtr,
    IdC,
    IDPtr,
    IdxC,
    IdxDoublePtr,
    IdxPtr,
    ModelPtr,
    MutableDatasetPtr,
    Optional,
    OptionsPtr,
    Path,
    PowerGridCore,
    SerializerPtr,
    VoidDoublePtr,
    VoidPtr,
    WritableDatasetPtr,
    c_char,
    c_char_p,
    c_double,
    c_size_t,
    c_void_p,
    chain,
    make_c_binding,
    platform,
    power_grid_core,
    signature,
)
