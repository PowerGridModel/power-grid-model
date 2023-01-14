# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0


from ctypes import CDLL, c_void_p, c_int64, c_size_t, c_char_p, POINTER
from pathlib import Path
import platform

# integer index
Idx = c_int64
IdxPtr = POINTER(Idx)


class PGMHandle(c_void_p):
    pass


HandlePtr = POINTER(PGMHandle)


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
    cdll.PGM_destroy_handle.argtypes = []
    cdll.PGM_destroy_handle.restype = HandlePtr

    cdll.PGM_create_handle.argtypes = [HandlePtr]
    cdll.PGM_create_handle.restype = None

    cdll.PGM_err_code.argtypes = [HandlePtr]
    cdll.PGM_err_code.restype = Idx

    cdll.PGM_err_msg.argtypes = [HandlePtr]
    cdll.PGM_err_msg.restype = c_char_p

    return cdll
