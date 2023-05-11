# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

import os
import platform
import re
import shutil
from itertools import chain
from pathlib import Path
from sysconfig import get_paths
from typing import List

# noinspection PyPackageRequirements
from pybuild_header_dependency import HeaderResolver
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
            # add optional link time optimization
            if os.environ.get("POWER_GRID_MODEL_ENABLE_LTO", "OFF") == "ON":
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
    resolver = HeaderResolver({"eigen": None, "boost": None})
    pgm = Path("power_grid_model")
    pgm_c = Path("power_grid_model_c")

    # include-folders
    include_dirs = [
        str(resolver.get_include()),
        str(pkg_dir / pgm_c / pgm / "include"),  # The include-folder of the library
        str(pkg_dir / pgm_c / pgm_c / "include"),  # The include-folder of the C API self
    ]
    # compiler and link flag
    cflags: List[str] = []
    lflags: List[str] = []
    library_dirs: List[str] = []
    libraries: List[str] = []
    sources = [str(pgm_c / pgm_c / pgm_c.with_suffix(".cpp"))]
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
    # path of python env
    env_base_path = Path(get_paths()["data"])
    # different treat for windows and linux
    # determine platform specific options
    if if_win:
        # flag for C++20
        cflags += ["/std:c++20"]
        include_dirs += [str(env_base_path / "Library" / "include")]
        library_dirs += [str(env_base_path / "Library" / "lib")]
    else:
        include_dirs += [str(env_base_path / "include"), get_paths()["platinclude"], get_paths()["include"]]
        library_dirs += [str(env_base_path / "lib")]
        # flags for Linux and Mac
        cflags += [
            "-std=c++20",
            "-O3",
            "-fvisibility=hidden",
        ]
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


def substitute_github_links(pkg_dir: Path):
    with open(pkg_dir / "README.md", "r") as f:
        raw_readme = f.read()
    if "GITHUB_SHA" not in os.environ:
        readme = raw_readme
    else:
        sha = os.environ["GITHUB_SHA"].lower()
        url = f"https://github.com/PowerGridModel/power-grid-model/blob/{sha}/"
        readme = re.sub(r"(\[[^\(\)\[\]]+\]\()((?!http)[^\(\)\[\]]+\))", f"\\1{url}\\2", raw_readme)
    with open("README.md", "w") as f:
        f.write(readme)


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
    substitute_github_links(pkg_dir)
    return generate_build_ext(pkg_dir=pkg_dir, pkg_name=pkg_name)


setup(
    **prepare_pkg(setup_file=Path(__file__).resolve()),
)
