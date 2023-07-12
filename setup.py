# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

import os
import platform
import shutil
from itertools import chain
from pathlib import Path
from typing import List

# noinspection PyPackageRequirements
from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext
from wheel.bdist_wheel import bdist_wheel

# determine platform, only windows or linux
if platform.system() == "Windows":
    if_win = True
elif platform.system() in ["Linux", "Darwin"]:
    if_win = False
else:
    raise SystemError("Only Windows, Linux, or MacOS is supported!")


def get_header_include() -> List[str]:
    """
    Get header files from pybuild_header_dependency, if it is installed

    Returns:
        either empty list or a list of header path
    """
    try:
        from pybuild_header_dependency import HeaderResolver

        resolver = HeaderResolver({"eigen": None, "boost": None})
        return [str(resolver.get_include())]
    except ImportError:
        return []


def get_conda_include() -> List[str]:
    """
    Get conda include path, if we are inside conda environment

    Returns:
        either empty list or a list of header paths
    """
    include_paths = []
    # in the conda build system the system root is defined in CONDA_PREFIX or BUILD_PREFIX
    for prefix in ["CONDA_PREFIX", "BUILD_PREFIX"]:
        if prefix in os.environ:
            conda_path = os.environ[prefix]
            if if_win:
                # windows has Library folder prefix
                include_paths.append(os.path.join(conda_path, "Library", "include"))
                include_paths.append(os.path.join(conda_path, "Library", "include", "eigen3"))
            else:
                include_paths.append(os.path.join(conda_path, "include"))
                include_paths.append(os.path.join(conda_path, "include", "eigen3"))
    return include_paths


# custom class for ctypes
class CTypesExtension(Extension):
    pass


class bdist_wheel_abi_none(bdist_wheel):
    def finalize_options(self):
        bdist_wheel.finalize_options(self)
        self.root_is_pure = False

    def get_tag(self):
        python, abi, plat = bdist_wheel.get_tag(self)
        return "py3", "none", plat


# custom compiler for linux
class MyBuildExt(build_ext):
    def build_extensions(self):
        if not if_win:
            if "CXX" in os.environ:
                cxx = os.environ["CXX"]
            else:
                cxx = self.compiler.compiler_cxx[0]
            # customize compiler and linker options
            self.compiler.compiler_so[0] = cxx
            self.compiler.linker_so[0] = cxx
            self.compiler.compiler_cxx = [cxx]
            # add link time optimization
            if "clang" in cxx:
                lto_flag = "-flto=thin"
            else:
                lto_flag = "-flto"
            self.compiler.compiler_so += [lto_flag]
            self.compiler.linker_so += [lto_flag]
            # remove -g and -O2
            self.compiler.compiler_so = [x for x in self.compiler.compiler_so if x not in ["-g", "-O2"]]
            self.compiler.linker_so = [x for x in self.compiler.linker_so if x not in ["-g", "-O2", "-Wl,-O1"]]

            print("-------compiler arguments----------")
            print(self.compiler.compiler_so)
            print("-------linker arguments----------")
            print(self.compiler.linker_so)
        return super().build_extensions()

    def get_export_symbols(self, ext):
        return ext.export_symbols

    def get_ext_filename(self, ext_name):
        return os.path.join(*ext_name.split(".")) + (".dll" if if_win else ".so")


def generate_build_ext(pkg_dir: Path, pkg_name: str):
    """
    Generate extension dict for setup.py
    the return value ext_dict, can be called in setup(**ext_dict)
    Args:
        pkg_dir:
        pkg_name:
    Returns:

    """
    # fetch dependent headers
    pgm = Path("power_grid_model")
    pgm_c = Path("power_grid_model_c")

    # include-folders
    include_dirs = [
        str(pkg_dir / pgm_c / pgm / "include"),  # The include-folder of the library
        str(pkg_dir / pgm_c / pgm_c / "include"),  # The include-folder of the C API self
    ]
    include_dirs += get_header_include()
    include_dirs += get_conda_include()
    # compiler and link flag
    cflags: List[str] = []
    lflags: List[str] = []
    library_dirs: List[str] = []
    libraries: List[str] = []
    sources = [
        str(pgm_c / pgm_c / "src" / "handle.cpp"),
        str(pgm_c / pgm_c / "src" / "meta_data.cpp"),
        str(pgm_c / pgm_c / "src" / "model.cpp"),
        str(pgm_c / pgm_c / "src" / "options.cpp"),
    ]
    # macro
    define_macros = [
        ("EIGEN_MPL2_ONLY", "1"),  # only MPL-2 part of eigen3
    ]
    pkg_bin_dir = pkg_dir / "src" / pkg_name

    # remove old extension build
    build_dir = pkg_dir / "build"
    if build_dir.exists():
        shutil.rmtree(build_dir)
    # remove binary
    bin_files = list(chain(pkg_bin_dir.rglob("*.so"), pkg_bin_dir.rglob("*.dll")))
    for bin_file in bin_files:
        print(f"Remove binary file: {bin_file}")
        bin_file.unlink()

    # build steps for Windows and Linux
    # different treat for windows and linux
    # determine platform specific options
    if if_win:
        # flag for C++20
        cflags += ["/std:c++20"]
    else:
        # flags for Linux and Mac
        cflags += ["-std=c++20", "-O3", "-fvisibility=hidden"]
        lflags += ["-lpthread", "-O3"]
        # extra flag for Mac
        if platform.system() == "Darwin":
            # compiler flag to set version
            cflags.append("-mmacosx-version-min=10.15")

    # list of extensions
    exts = [
        CTypesExtension(
            name="power_grid_model.core._power_grid_core",
            sources=sources,
            include_dirs=include_dirs,
            library_dirs=library_dirs,
            libraries=libraries,
            extra_compile_args=cflags,
            extra_link_args=lflags,
            define_macros=define_macros,
            language="c++",
        )
    ]

    # return dict of exts
    return dict(ext_modules=exts, cmdclass={"build_ext": MyBuildExt, "bdist_wheel": bdist_wheel_abi_none})


def set_version(pkg_dir: Path):
    # if PYPI_VERSION does not exist, copy from VERSION
    pypi_file = pkg_dir / "PYPI_VERSION"
    if not pypi_file.exists():
        with open(pkg_dir / "VERSION") as f:
            version = f.read().strip().strip("\n")
        with open(pypi_file, "w") as f:
            f.write(version)


def prepare_pkg(setup_file: Path) -> dict:
    """

    Args:
        setup_file:
    Returns:

    """
    print(f"Build wheel from {setup_file}")
    pkg_dir = setup_file.parent
    # package description
    pkg_pip_name = "power-grid-model"
    pkg_name = pkg_pip_name.replace("-", "_")
    set_version(pkg_dir)
    return generate_build_ext(pkg_dir=pkg_dir, pkg_name=pkg_name)


setup(
    **prepare_pkg(setup_file=Path(__file__).resolve()),
)
