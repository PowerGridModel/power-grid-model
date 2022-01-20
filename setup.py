# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

import os
from glob import glob
import shutil
import numpy as np
import platform
from sysconfig import get_paths
from setuptools import Extension
from distutils.command.build_ext import build_ext
from setuptools import setup, find_packages
import Cython.Compiler.Main as CythonCompiler
from pathlib import Path
from typing import List, Dict
import datetime


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
            if "clang" in cxx:
                lto_flag = "-flto=thin"
            else:
                lto_flag = "-flto"
            # customize compiler and linker options
            self.compiler.compiler_so = [
                cxx,
                "-std=c++17",
                "-m64",
                "-DNDEBUG",
                "-fPIC",
                "-Wall",
                lto_flag,
                "-O3",
                "-fvisibility=hidden",
            ]
            self.compiler.compiler_cxx = [cxx]
            self.compiler.linker_so = [cxx, "-lpthread", "-ldl", "-fPIC", "-shared", lto_flag, "-O3"]

            # extra flag for Mac
            if platform.system() == "Darwin":
                # compiler flag
                self.compiler.compiler_so.append("-mmacosx-version-min=10.14")
                # linker flag
                self.compiler.linker_so += ["-undefined", "dynamic_lookup"]

        build_ext.build_extensions(self)


def get_ext_name(src_file, pkg_dir, pkg_name):
    base_name = os.path.dirname(os.path.relpath(src_file, os.path.join(pkg_dir, pkg_name)))
    base_name = base_name.replace("\\", ".").replace("/", ".")
    module_name = Path(src_file).resolve().stem
    return f"{base_name}.{module_name}"


def generate_build_ext(
    pkg_dir: str,
    pkg_name: str,
    cflags: list = None,
    lflags: list = None,
    include_dirs: list = None,
    library_dirs: list = None,
    libraries: list = None,
    define_macros: list = None,
    cpp_exts: Dict[str, List[str]] = None,
):
    """
    Generate extension dict for setup.py
    the return value ext_dict, can be called in setup(**ext_dict)
    Args:
        pkg_dir:
        pkg_name:
        cflags:
        lflags:
        include_dirs:
        library_dirs:
        libraries:
        define_macros:
        cpp_exts: a dict of hand-made C++ extentions, each entry is an extension module
            key: name of the extension module, including relative pakcage path
            value: list of C++ source files

    Returns:

    """
    # compiler and link flag
    cflags = cflags if cflags is not None else []
    lflags = lflags if lflags is not None else []
    include_dirs = include_dirs if include_dirs is not None else []
    # The include folder of numpy header is always present
    include_dirs += [np.get_include()]
    # The include folder of the repo self is always present
    include_dirs += [os.path.join(pkg_dir, "include")]
    library_dirs = library_dirs if library_dirs is not None else []
    libraries = libraries if libraries is not None else []

    # remove old extension build
    shutil.rmtree(os.path.join(pkg_dir, "build"), ignore_errors=True)
    # remove binary
    bin_files = glob(os.path.join(pkg_dir, pkg_name, "**", r"*.so"), recursive=True) + glob(
        os.path.join(pkg_dir, pkg_name, "**", r"*.pyd"), recursive=True
    )
    for bin_file in bin_files:
        os.remove(bin_file)

    # build steps for Windows and Linux
    # path of python env
    env_base_path = get_paths()["data"]
    # different treat for windows and linux
    # determine platform specific options
    if if_win:
        # flag for C++17
        cflags += ["/std:c++17"]
        include_dirs += [os.path.join(env_base_path, "Library", "include")]
        library_dirs += [os.path.join(env_base_path, "Library", "lib")]
    else:
        include_dirs += [os.path.join(env_base_path, "include"), get_paths()["platinclude"], get_paths()["include"]]
        library_dirs += [os.path.join(env_base_path, "lib")]

    # list of compiled cython files, without file extension
    cython_src = glob(os.path.join(pkg_dir, pkg_name, "**", r"*.pyx"), recursive=True)
    cython_src = [os.path.splitext(x)[0] for x in cython_src]
    # compile cython
    cython_src_pyx = [f"{x}.pyx" for x in cython_src]
    print("Compile Cython extensions")
    print(cython_src_pyx)
    CythonCompiler.compile(cython_src_pyx, cplus=True, language_level=3)
    cython_src_cpp = [f"{x}.cpp" for x in cython_src]
    print("Generated cpp files")
    print(cython_src_cpp)
    # for linux add visibility to the init function
    if not if_win:
        for cpp_src in cython_src_cpp:
            with open(cpp_src) as f:
                cpp_code = f.read()
            cpp_code_modified = cpp_code.replace(
                "#define __Pyx_PyMODINIT_FUNC PyMODINIT_FUNC",
                '#define __Pyx_PyMODINIT_FUNC extern "C" __attribute__((visibility ("default"))) PyObject* ',
            )
            with open(cpp_src, "w") as f:
                f.write(cpp_code_modified)

    # list of extensions of generated cpp files from cython
    exts = [
        Extension(
            name=get_ext_name(src_file=src_file, pkg_dir=pkg_dir, pkg_name=pkg_name),
            sources=[os.path.relpath(src_file, pkg_dir)],
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
    # add hand-made extensions
    if cpp_exts is not None:
        for cpp_ext_name, cpp_ext_src in cpp_exts.items():
            exts.append(
                Extension(
                    name=cpp_ext_name,
                    sources=cpp_ext_src,
                    include_dirs=include_dirs,
                    library_dirs=library_dirs,
                    libraries=libraries,
                    extra_compile_args=cflags,
                    extra_link_args=lflags,
                    define_macros=define_macros,
                    language="c++",
                )
            )

    # return dict of exts
    return dict(ext_package=pkg_name, ext_modules=exts, cmdclass={"build_ext": MyBuildExt})


def package_files(directory):
    include_extensions = {".json"}
    paths = []
    for (path, directories, filenames) in os.walk(directory):
        for filename in filenames:
            if os.path.splitext(filename)[1] in include_extensions:
                paths.append(os.path.join("..", path, filename))
    return paths


def get_version(pkg_dir):
    with open(os.path.join(pkg_dir, "VERSION")) as f:
        version = f.read().strip().strip("\n")
    if ("GITHUB_SHA" in os.environ) and ("GITHUB_REF" in os.environ) and ("GITHUB_RUN_NUMBER" in os.environ):
        sha = os.environ["GITHUB_SHA"]
        ref = os.environ["GITHUB_REF"]
        build_number = os.environ["GITHUB_RUN_NUMBER"]
        # short hash number in numeric
        short_hash = f'{int(f"0x{sha[0:6]}", base=16):08}'

        if "main" in ref:
            # main branch
            # major.minor.build_number
            version += f".{build_number}"
        elif "release" in ref:
            # release branch
            # major.minor.build_number rc short_hash
            # NOTE: the major.minor in release branch is usually higher than the main branch
            # this is the leading version if you enable test version in pip install
            version += f".{build_number}rc{short_hash}"
        else:
            # feature branch
            # major.minor.0a build_number short_hash
            version += f".0a{build_number}{short_hash}"
    return version


def build_pkg(setup_file, author, author_email, description, url, entry_points: dict = None, **ext_args):
    """

    Args:
        setup_file:
        author:
        author_email:
        description:
        url:
        entry_points: entry points of scripts
        **ext_args: extra argument for extension build

    Returns:

    """
    pkg_dir = os.path.dirname(os.path.realpath(setup_file))
    # package description
    pkg_pip_name = "power-grid-model"
    pkg_name = pkg_pip_name.replace("-", "_")
    with open(os.path.join(pkg_dir, "README.md")) as f:
        long_description = f.read()
    with open(os.path.join(pkg_dir, "requirements.txt")) as f:
        required = f.read().splitlines()
        required = [x for x in required if "#" not in x]
    version = get_version(pkg_dir)

    resource_package = package_files(os.path.join(pkg_name, "resources"))
    if entry_points is None:
        entry_points = {}

    setup(
        name=pkg_pip_name,
        version=version,
        author=author,
        author_email=author_email,
        description=description,
        long_description=long_description,
        long_description_content_type="text/markdown",
        url=url,
        packages=find_packages(),
        package_data={pkg_name: resource_package},
        license="MPL-2.0",
        classifiers=[
            "Programming Language :: Python :: 3",
            "Programming Language :: Python :: 3.8",
            "Programming Language :: Python :: 3.9",
            "Programming Language :: Python :: 3.10",
            "Programming Language :: Python :: Implementation :: CPython",
            "Programming Language :: C++",
            "Programming Language :: Cython",
            r"Development Status :: 5 - Production/Stable",
            "Intended Audience :: Developers",
            "Intended Audience :: Science/Research",
            r"License :: OSI Approved :: Mozilla Public License 2.0 (MPL 2.0)",
            "Operating System :: Microsoft :: Windows",
            "Operating System :: POSIX :: Linux",
            "Operating System :: MacOS",
            "Topic :: Scientific/Engineering :: Physics",
        ],
        install_requires=required,
        entry_points=entry_points,
        **generate_build_ext(pkg_dir=pkg_dir, pkg_name=pkg_name, **ext_args),
    )


build_pkg(
    setup_file=__file__,
    author="Alliander Dynamic Grid Calculation",
    author_email="dynamic.grid.calculation@alliander.com",
    description="Python/C++ library for distribution power system analysis",
    url="https://github.com/alliander-opensource/power-grid-model",
    include_dirs=[os.environ["EIGEN_INCLUDE"], os.environ["BOOST_INCLUDE"]],
    define_macros=[("EIGEN_MPL2_ONLY", "1"), ("POWER_GRID_MODEL_USE_MKL_AT_RUNTIME", 1)],
)
