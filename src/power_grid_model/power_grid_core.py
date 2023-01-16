# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0


from ctypes import CDLL, c_void_p, c_size_t, c_char_p, POINTER
from functools import partial

from power_grid_model.index_integer import Idx_c
from pathlib import Path
from typing import Optional, List, Callable
import platform

# integer index
IdxPtr = POINTER(Idx_c)


class HandlePtr(c_void_p):
    pass


def _load_core() -> CDLL:
    """

    Returns:

    """
    if platform.system() == "Windows":
        dll_file = "_power_grid_core.dll"
    else:
        dll_file = "_power_grid_core.so"
    cdll = CDLL(str(Path(__file__).parent / dll_file))
    # assign return types
    # handle
    cdll.PGM_create_handle.argtypes = []
    cdll.PGM_create_handle.restype = HandlePtr
    cdll.PGM_destroy_handle.argtypes = [HandlePtr]
    cdll.PGM_destroy_handle.restype = None
    cdll.PGM_err_code.argtypes = [HandlePtr]
    cdll.PGM_err_code.restype = Idx_c
    cdll.PGM_err_msg.argtypes = [HandlePtr]
    cdll.PGM_err_msg.restype = c_char_p
    # meta data
    cdll.PGM_meta_n_datasets.argtypes = [HandlePtr]
    cdll.PGM_meta_n_datasets.restype = Idx_c
    cdll.PGM_meta_dataset_name.argtypes = [HandlePtr, Idx_c]
    cdll.PGM_meta_dataset_name.restype = c_char_p
    cdll.PGM_meta_n_classes.argtypes = [HandlePtr, c_char_p]
    cdll.PGM_meta_n_classes.restype = Idx_c
    cdll.PGM_meta_class_name.argtypes = [HandlePtr, c_char_p, Idx_c]
    cdll.PGM_meta_class_name.restype = c_char_p
    cdll.PGM_meta_class_size.argtypes = [HandlePtr, c_char_p, c_char_p]
    cdll.PGM_meta_class_size.restype = c_size_t
    cdll.PGM_meta_class_alignment.argtypes = [HandlePtr, c_char_p, c_char_p]
    cdll.PGM_meta_class_alignment.restype = c_size_t
    cdll.PGM_meta_n_attributes.argtypes = [HandlePtr, c_char_p, c_char_p]
    cdll.PGM_meta_n_attributes.restype = Idx_c
    cdll.PGM_meta_attribute_name.argtypes = [HandlePtr, c_char_p, c_char_p, Idx_c]
    cdll.PGM_meta_attribute_name.restype = c_char_p
    cdll.PGM_meta_attribute_ctype.argtypes = [HandlePtr, c_char_p, c_char_p, c_char_p]
    cdll.PGM_meta_attribute_ctype.restype = c_char_p
    cdll.PGM_meta_attribute_offset.argtypes = [HandlePtr, c_char_p, c_char_p, c_char_p]
    cdll.PGM_meta_attribute_offset.restype = c_size_t
    cdll.PGM_is_little_endian.argtypes = [HandlePtr]
    cdll.PGM_is_little_endian.restype = Idx_c
    # return
    return cdll


class PowerGridCore:
    _cdll: CDLL
    _handle: HandlePtr
    methods: List[Callable]

    def __new__(cls, *args, **kwargs):
        instance = super().__new__(cls, *args, **kwargs)
        instance._handle = HandlePtr()
        instance._cdll = _load_core()
        return instance

    def __init__(self):
        keys = [x for x in self._cdll.__dict__.keys() if x.startswith("PGM_")]
        for key in keys:
            func = getattr(self._cdll, key)
            if len(func.argtypes) > 0 and func.argtypes[0]:
                setattr(self, key[len("PGM_"):], partial(func, self._handle))

    def __del__(self):
        self._cdll.PGM_destroy_handle(self._handle)


# make one instance
power_grid_core = PowerGridCore()
