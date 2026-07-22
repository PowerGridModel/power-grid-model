# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import ctypes
import os
import platform
import sys
from pathlib import Path

from power_grid_model._core.power_grid_model_c.get_pgm_dll_path import get_pgm_dll_path


def get_pgm_cli_path() -> Path:
    """
    Returns the path to the PGM CLI backend shared library.
    """
    if os.environ.get("CONDA_PREFIX"):
        raise ImportError(
            "PGM CLI Python shim should not be called inside conda environment. Please call the executable directly."
        )

    dll_path = get_pgm_dll_path()
    cli_dll_file: Path
    platform_name = platform.uname().system
    if platform_name == "Windows":
        cli_dll_file = Path("power_grid_model_cli_backend.dll")
    elif platform_name == "Darwin":
        cli_dll_file = Path("libpower_grid_model_cli_backend.dylib")
    elif platform_name == "Linux":
        cli_dll_file = Path("libpower_grid_model_cli_backend.so")
    else:
        raise NotImplementedError(f"Unsupported platform: {platform_name}")

    return dll_path.parent / cli_dll_file


def _load_cli_backend() -> ctypes.CDLL:
    cli_path = get_pgm_cli_path()
    return ctypes.CDLL(str(cli_path))


def _build_argv() -> tuple[int, ctypes.Array[ctypes.c_char_p]]:
    argv = [arg.encode() for arg in sys.argv]
    c_argv = (ctypes.c_char_p * len(argv))(*argv)
    return len(argv), c_argv


def main() -> int:
    backend = _load_cli_backend()
    backend.PGM_cli_main.argtypes = [ctypes.c_int, ctypes.POINTER(ctypes.c_char_p)]
    backend.PGM_cli_main.restype = ctypes.c_int

    argc, argv = _build_argv()
    return backend.PGM_cli_main(argc, argv)
