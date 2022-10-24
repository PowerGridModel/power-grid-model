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
import Cython.Compiler.Main as CythonCompiler

# noinspection PyPackageRequirements
import numpy as np

# noinspection PyPackageRequirements
import requests
from pybuild_header_dependency import HeaderResolver
from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext

# determine platform, only windows or linux
if platform.system() == "Windows":
    if_win = True
elif platform.system() in ["Linux", "Darwin"]:
    if_win = False
else:
    raise SystemError("Only Windows, Linux, or MacOS is supported!")


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

        build_ext.build_extensions(self)


def get_ext_name(src_file: Path, pkg_dir: Path, pkg_name: str):
    module_name = str(src_file.relative_to(pkg_dir / "src" / pkg_name).with_suffix(""))
    module_name = module_name.replace("\\", ".").replace("/", ".")
    return module_name


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
    # include-folders
    include_dirs = [
        str(resolver.get_include()),
        np.get_include(),  # The include-folder of numpy header
        str(pkg_dir / "include"),  # The include-folder of the repo self
    ]
    # compiler and link flag
    cflags: List[str] = []
    lflags: List[str] = []
    library_dirs: List[str] = []
    libraries: List[str] = []
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
    bin_files = list(chain(pkg_bin_dir.rglob("*.so"), pkg_bin_dir.rglob("*.pyd")))
    for bin_file in bin_files:
        print(f"Remove binary file: {bin_file}")
        bin_file.unlink()

    # build steps for Windows and Linux
    # path of python env
    env_base_path = Path(get_paths()["data"])
    # different treat for windows and linux
    # determine platform specific options
    if if_win:
        # flag for C++17
        cflags += ["/std:c++17"]
        include_dirs += [str(env_base_path / "Library" / "include")]
        library_dirs += [str(env_base_path / "Library" / "lib")]
    else:
        include_dirs += [str(env_base_path / "include"), get_paths()["platinclude"], get_paths()["include"]]
        library_dirs += [str(env_base_path / "lib")]
        # flags for Linux and Mac
        cflags += [
            "-std=c++17",
            "-O3",
            "-fvisibility=hidden",
        ]
        lflags += ["-lpthread", "-O3"]
        # for linux/macos add visibility to the init function
        define_macros.append(("PyMODINIT_FUNC", 'extern "C" __attribute__((visibility ("default"))) PyObject*'))
        # # extra flag for Mac
        if platform.system() == "Darwin":
            # compiler flag to set version
            cflags.append("-mmacosx-version-min=10.15")

    # list of compiled cython files, without file extension
    cython_src = list(pkg_bin_dir.rglob(r"*.pyx"))
    cython_src = [x.with_suffix("") for x in cython_src]
    # compile cython
    cython_src_pyx = [x.with_suffix(".pyx") for x in cython_src]
    print("Compile Cython extensions")
    print(cython_src_pyx)
    CythonCompiler.compile(cython_src_pyx, cplus=True, language_level=3)
    cython_src_cpp = [x.with_suffix(".cpp") for x in cython_src]
    print("Generated cpp files")
    print(cython_src_cpp)

    # list of extensions of generated cpp files from cython
    exts = [
        Extension(
            name=get_ext_name(src_file=src_file, pkg_dir=pkg_dir, pkg_name=pkg_name),
            sources=[str(src_file.relative_to(pkg_dir))],
            include_dirs=include_dirs,
            library_dirs=library_dirs,
            libraries=libraries,
            extra_compile_args=cflags,
            extra_link_args=lflags,
            define_macros=define_macros,
            language="c++",
        )
        for src_file in cython_src_cpp
    ]

    # return dict of exts
    return dict(ext_package=pkg_name, ext_modules=exts, cmdclass={"build_ext": MyBuildExt})


def substitute_github_links(pkg_dir: Path):
    with open(pkg_dir / "README.md", "r") as f:
        raw_readme = f.read()
    if "GITHUB_SHA" not in os.environ:
        readme = raw_readme
    else:
        sha = os.environ["GITHUB_SHA"].lower()
        url = f"https://github.com/alliander-opensource/power-grid-model/blob/{sha}/"
        readme = re.sub(r"(\[[^\(\)\[\]]+\]\()((?!http)[^\(\)\[\]]+\))", f"\\1{url}\\2", raw_readme)
    with open("README.md", "w") as f:
        f.write(readme)


def set_version(pkg_dir: Path):
    with open(pkg_dir / "VERSION") as f:
        version = f.read().strip().strip("\n")
    major, minor = (int(x) for x in version.split("."))
    latest_major, latest_minor, latest_patch = get_pypi_latest()
    # get version
    version = get_new_version(major, minor, latest_major, latest_minor, latest_patch)
    # mutate version in GitHub Actions
    if ("GITHUB_SHA" in os.environ) and ("GITHUB_REF" in os.environ) and ("GITHUB_RUN_NUMBER" in os.environ):
        sha = os.environ["GITHUB_SHA"]
        ref = os.environ["GITHUB_REF"]
        build_number = os.environ["GITHUB_RUN_NUMBER"]
        # short hash number in numeric
        short_hash = f'{int(f"0x{sha[0:6]}", base=16):08}'

        if "main" in ref:
            # main branch
            # major.minor.patch
            # do nothing
            pass
        elif "release" in ref:
            # release branch
            # major.minor.patch rc 9 build_number short_hash
            # NOTE: the major.minor in release branch is usually higher than the main branch
            # this is the leading version if you enable test version in pip install
            version += f"rc9{build_number}{short_hash}"
        else:
            # feature branch
            # major.minor.patch a 1 build_number short_hash
            version += f"a1{build_number}{short_hash}"
    with open(pkg_dir / "PYPI_VERSION", "w") as f:
        f.write(version)


def get_pypi_latest():
    r = requests.get("https://pypi.org/pypi/power-grid-model/json")
    data = r.json()
    version: str = data["info"]["version"]
    return (int(x) for x in version.split("."))


def get_new_version(major, minor, latest_major, latest_minor, latest_patch):
    if (major > latest_major) or ((major == latest_major) and minor > latest_minor):
        # brand-new version with patch zero
        return f"{major}.{minor}.0"
    elif major == latest_major and minor == latest_minor:
        # current version, increment path
        return f"{major}.{minor}.{latest_patch + 1}"
    else:
        # does not allow building older version
        raise ValueError(
            "Invalid version number!\n"
            f"latest version: {latest_major}.{latest_minor}.{latest_patch}\n"
            f"to be built version: {major}.{minor}\n"
        )


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
