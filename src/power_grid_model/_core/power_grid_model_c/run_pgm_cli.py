# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

import os
import platform
import sys
from importlib.resources import files
from pathlib import Path


def get_pgm_cli_path() -> Path:
    """
    Returns the path to PGM CLI executable.
    """
    package_dir = Path(str(files(__package__)))
    bin_dir = package_dir / "bin"
    platform_name = platform.uname().system

    # determine executable file name
    if platform_name == "Windows":
        exe_file = Path("power-grid-model.exe")
    elif platform_name == "Darwin" or platform.system() == "Linux":
        exe_file = Path("power-grid-model")
    else:
        raise NotImplementedError(f"Unsupported platform: {platform_name}")
    bin_path = bin_dir / exe_file

    # determine editable path to the executable
    # __file__
    #   -> power_grid_model_c (..)
    #     -> _core (..)
    #       -> power_grid_model (..)
    #         -> src (..)
    #           -> repo_root (..)
    #             -> build
    #               -> bin
    editable_dir = Path(__file__).resolve().parent.parent.parent.parent.parent / "build" / "bin"
    editable_bin_path = editable_dir / exe_file

    # first try to load from bin_path
    # then editable_bin_path
    # then if it is inside conda, this Python shim should never be called, instead user calls the exe directly
    # then for anything else, raise an error
    if bin_path.exists():
        final_bin_path = bin_path
    elif editable_bin_path.exists():
        final_bin_path = editable_bin_path
    elif os.environ.get("CONDA_PREFIX"):
        raise ImportError(
            "PGM CLI Python shim should not be called inside conda environment. Please call the executable directly."
        )
    else:
        raise ImportError(f"Could not find executable: {exe_file}. Your PGM installation may be broken.")

    return final_bin_path


def main():
    exe_path = get_pgm_cli_path()
    os.execv(str(exe_path), [str(exe_path), *sys.argv[1:]])  # noqa: S606
