# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import platform
from importlib.resources import files
from pathlib import Path


def get_pgm_dll_path() -> Path:
    """
    Returns the path to PGM dynamic library.
    """
    package_dir = files(__package__)
    if platform.system() == "Windows":
        lib_dir = package_dir / "bin"
    else:
        lib_dir = package_dir / "lib"
    # determine DLL file name
    if platform.system() == "Windows":
        dll_file = Path("power_grid_model_c.dll")
    elif platform.system() == "Darwin":
        dll_file = Path("libpower_grid_model_c.dylib")
    elif platform.system() == "Linux":
        dll_file = Path("libpower_grid_model_c.so")
    else:
        raise NotImplementedError(f"Unsupported platform: {platform.system()}")
    lib_dll_path = lib_dir / dll_file

    # first try to load from lib_dll_path
    # then just load dll_file, the system tries to find it in the PATH
    if lib_dll_path.exists():
        final_dll_path = lib_dll_path
    else:
        final_dll_path = dll_file

    return final_dll_path
