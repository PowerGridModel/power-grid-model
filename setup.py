# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

import os
import re
from glob import glob
import shutil
import numpy as np
import platform
from sysconfig import get_paths
from setuptools import Extension
from setuptools.command.build_ext import build_ext
from setuptools import setup, find_packages
import Cython.Compiler.Main as CythonCompiler
from pathlib import Path


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
            self.compiler.compiler_so[0] = cxx
            self.compiler.compiler_so += [lto_flag]
            self.compiler.linker_so[0] = cxx
            self.compiler.linker_so += [lto_flag]
            self.compiler.compiler_cxx = [cxx]
            # remove -g and -O2
            self.compiler.compiler_so = [x for x in self.compiler.compiler_so if x not in ["-g", "-O2"]]
            self.compiler.linker_so = [x for x in self.compiler.linker_so if x not in ["-g", "-O2", "-Wl,-O1"]]

            print("-------compiler arguments----------")
            print(self.compiler.compiler_so)
            print("-------linker arguments----------")
            print(self.compiler.linker_so)

        build_ext.build_extensions(self)


def get_ext_name(src_file, pkg_dir, pkg_name):
    base_name = os.path.dirname(os.path.relpath(src_file, os.path.join(pkg_dir, "src", pkg_name)))
    base_name = base_name.replace("\\", ".").replace("/", ".")
    module_name = Path(src_file).resolve().stem
    return f"{base_name}.{module_name}"


def generate_build_ext(pkg_dir: Path, pkg_name: str):
    """
    Generate extension dict for setup.py
    the return value ext_dict, can be called in setup(**ext_dict)
    Args:
        pkg_dir:
        pkg_name:
    Returns:

    """
    # include-folders
    include_dirs = [
        np.get_include(),  # The include-folder of numpy header
        str(pkg_dir / "include"),  # The include-folder of the repo self
        os.environ["EIGEN_INCLUDE"],  # eigen3 library
        os.environ["BOOST_INCLUDE"],  # boost library
    ]
    # compiler and link flag
    cflags = []
    lflags = []
    library_dirs = []
    libraries = []
    # macro
    define_macros = [
        ("EIGEN_MPL2_ONLY", "1"),  # only MPL-2 part of eigen3
        ("POWER_GRID_MODEL_USE_MKL_AT_RUNTIME", 1),  # use mkl runtime loading
    ]

    # remove old extension build
    shutil.rmtree(pkg_dir / "build", ignore_errors=True)
    # remove binary
    bin_files = glob(str(pkg_dir / "src" / pkg_name / "**" / r"*.so"), recursive=True) + glob(
        str(pkg_dir / "src" / pkg_name / "**" / r"*.pyd"), recursive=True
    )
    for bin_file in bin_files:
        os.remove(bin_file)

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
            "-m64",
            "-O3",
            "-fvisibility=hidden",
        ]
        lflags += ["-lpthread", "-ldl", "-O3"]
        # # extra flag for Mac
        if platform.system() == "Darwin":
            # compiler flag to set version
            cflags.append("-mmacosx-version-min=10.15")

    # list of compiled cython files, without file extension
    cython_src = glob(str(pkg_dir / "src" / pkg_name / "**" / r"*.pyx"), recursive=True)
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

    # return dict of exts
    return dict(ext_package=pkg_name, ext_modules=exts, cmdclass={"build_ext": MyBuildExt})


def convert_long_description(raw_readme: str):
    if "GITHUB_SHA" not in os.environ:
        return raw_readme
    else:
        sha = os.environ["GITHUB_SHA"].lower()
        url = f"https://github.com/alliander-opensource/power-grid-model/blob/{sha}/"
        return re.sub(r"(\[[^\(\)\[\]]+\]\()((?!http)[^\(\)\[\]]+\))", f"\\1{url}\\2", raw_readme)


def get_version(pkg_dir: Path):
    with open(pkg_dir / "VERSION") as f:
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


def build_pkg(setup_file: Path, author, author_email, description, url):
    """

    Args:
        setup_file:
        author:
        author_email:
        description:
        url:
    Returns:

    """
    pkg_dir = setup_file.parent
    # package description
    pkg_pip_name = "power-grid-model"
    pkg_name = pkg_pip_name.replace("-", "_")
    with open(pkg_dir / "README.md") as f:
        long_description = f.read()
        long_description = convert_long_description(long_description)
    with open(pkg_dir / "requirements.txt") as f:
        required = f.read().splitlines()
        required = [x for x in required if "#" not in x]
    version = get_version(pkg_dir)

    setup(
        name=pkg_pip_name,
        version=version,
        author=author,
        author_email=author_email,
        description=description,
        long_description=long_description,
        long_description_content_type="text/markdown",
        url=url,
        package_dir={"": "src"},
        packages=find_packages("src"),
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
        python_requires=">=3.8",
        **generate_build_ext(pkg_dir=pkg_dir, pkg_name=pkg_name),
    )


build_pkg(
    setup_file=Path(__file__),
    author="Alliander Dynamic Grid Calculation",
    author_email="dynamic.grid.calculation@alliander.com",
    description="Python/C++ library for distribution power system analysis",
    url="https://github.com/alliander-opensource/power-grid-model",
)
