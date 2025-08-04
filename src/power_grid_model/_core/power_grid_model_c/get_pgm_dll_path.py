# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import os
import platform
from importlib.resources import files
from pathlib import Path


def get_pgm_dll_path() -> Path:
    """
    Returns the path to PGM dynamic library.
    """
    package_dir = Path(str(files(__package__)))
    platform_name = platform.uname().system
    if platform_name == "Windows":
        lib_dir_1 = package_dir / "bin"
        lib_dir_2 = package_dir / "bin"
    else:
        lib_dir_1 = package_dir / "lib"
        lib_dir_2 = package_dir / "lib64"

    # determine DLL file name
    if platform_name == "Windows":
        dll_file = Path("power_grid_model_c.dll")
    elif platform_name == "Darwin":
        dll_file = Path("libpower_grid_model_c.dylib")
    elif platform.system() == "Linux":
        dll_file = Path("libpower_grid_model_c.so")
    else:
        raise NotImplementedError(f"Unsupported platform: {platform_name}")
    lib_dll_path_1 = lib_dir_1 / dll_file
    lib_dll_path_2 = lib_dir_2 / dll_file

    # determine editable path to the DLL
    # __file__
    #   -> power_grid_model_c (..)
    #     -> _core (..)
    #       -> power_grid_model (..)
    #         -> src (..)
    #           -> repo_root (..)
    #             -> build
    #               -> bin
    editable_dir = Path(__file__).resolve().parent.parent.parent.parent.parent / "build" / "bin"
    editable_dll_path = editable_dir / dll_file

    # first try to load from lib_dll_path
    # then editable_dll_path
    # then if it is inside conda, just load dll_file, the system tries to find it in the PATH
    # then for anything else, raise an error
    if lib_dll_path_1.exists():
        final_dll_path = lib_dll_path_1
    elif lib_dll_path_2.exists():
        final_dll_path = lib_dll_path_2
    elif editable_dll_path.exists():
        final_dll_path = editable_dll_path
    elif os.environ.get("CONDA_PREFIX"):
        final_dll_path = dll_file
    else:
        raise ImportError(f"Could not find shared library: {dll_file}. Your PGM installation may be broken.")

    return final_dll_path
