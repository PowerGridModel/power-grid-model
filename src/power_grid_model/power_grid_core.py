# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0


import platform
from ctypes import CDLL, POINTER, c_char_p, c_size_t, c_void_p
from pathlib import Path
from typing import Callable, List

from power_grid_model.index_integer import Idx_c

# integer index
IdxPtr = POINTER(Idx_c)

# functions with size_t return
_FUNC_SIZE_T_RES = {"meta_class_size", "meta_class_alignment", "meta_attribute_offset"}
_ARGS_TYPE_MAPPING = {
    str: c_char_p,
    int: Idx_c,
}


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
    return cdll


class WrapperFunc:
    def __init__(self, cdll: CDLL, handle: HandlePtr, name: str, c_argtypes: List, c_restype):
        """

        Args:
            name:
            c_argtypes:
        """
        self._cfunc = getattr(cdll, f"PGM_{name}")
        self._handle = handle
        self._name = name
        self._c_argtypes = c_argtypes
        self._c_restype = c_restype

    def __call__(self, *args):
        if "destroy" in self._name:
            c_inputs = []
        else:
            c_inputs = [self._handle]
        for arg, arg_type in zip(args, self._c_argtypes):
            if arg_type == c_char_p:
                c_inputs.append(arg.encode())
            else:
                c_inputs.append(arg)
        # call
        res = self._cfunc(*c_inputs)
        # convert to string for c_char_p
        if self._c_restype == c_char_p:
            res = res.decode()
        return res


class PowerGridCore:
    _cdll: CDLL
    _handle: HandlePtr
    # error handling
    err_code: Callable[[], int]
    err_msg: Callable[[], str]
    # meta data
    meta_n_datasets: Callable[[], int]
    meta_dataset_name: Callable[[int], str]
    meta_n_classes: Callable[[str], int]
    meta_class_name: Callable[[str, int], str]
    meta_class_size: Callable[[str, str], int]
    meta_class_alignment: Callable[[str, str], int]
    meta_n_attributes: Callable[[str, str], int]
    meta_attribute_name: Callable[[str, str, int], str]
    meta_attribute_ctype: Callable[[str, str, str], str]
    meta_attribute_offset: Callable[[str, str, str], int]
    is_little_endian: Callable[[], int]

    def __new__(cls, *args, **kwargs):
        instance = super().__new__(cls, *args, **kwargs)
        instance._handle = HandlePtr()
        instance._cdll = _load_core()
        return instance

    def __init__(self):
        for name, function in PowerGridCore.__annotations__.items():
            if name.startswith("_"):
                continue
            # get and convert types
            py_argtypes = function.__args__[:-1]
            py_restype = function.__args__[-1]
            c_argtypes = [_ARGS_TYPE_MAPPING.get(x, x) for x in py_argtypes]
            c_restype = _ARGS_TYPE_MAPPING.get(py_restype, py_restype)
            if c_restype == Idx_c and name in _FUNC_SIZE_T_RES:
                c_restype = c_size_t
            # set argument in dll
            # mostly with handle pointer, except destroy function
            is_destroy_func = "destroy" in name
            if is_destroy_func:
                getattr(self._cdll, f"PGM_{name}").argtypes = c_argtypes
            else:
                getattr(self._cdll, f"PGM_{name}").argtypes = [HandlePtr] + c_argtypes
            getattr(self._cdll, f"PGM_{name}").restype = c_restype
            # set wrapper functor to instance
            setattr(
                self,
                name,
                WrapperFunc(
                    cdll=self._cdll, handle=self._handle, name=name, c_argtypes=c_argtypes, c_restype=c_restype
                ),
            )

    def __del__(self):
        self._cdll.PGM_destroy_handle(self._handle)


# make one instance
power_grid_core = PowerGridCore()
