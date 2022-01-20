<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

# Build Guide

This document explains how you can build this library from source, including some examples of build environment.
In this repository there are two builds:

* A `power-grid-model` [pip](https://pip.pypa.io/en/stable/) Python package with C++ extension as the calculation core.
* A [CMake](https://cmake.org/) C++ project to build native C++ unit tests and/or performance benchmark.
* In the future, the following two libraries might be provided:
    * A C++ header-only library of the calculation core
    * A C/C++ dynamic shared library (`.dll` or `.so`) with pure C ABI interface.

# Build Requirements

To build the library from source, you need to first prepare the compiler toolchains
and the build dependencies. In this section a list of general requirements are given.
After this section there are examples of setup in Linux (Ubuntu 20.04), Windows 10, and macOS (Big Sur).

## Architecture Support

This library is written and tested on `x86_64` architecture.
Building the library in `x86_32` might be working, but is not tested.
It is in theory also possible to build the library in other architectures,
but the MKL library is only available for `x86`. So only Eigen sparse solver is available.

The source code is written with the mindset of ISO standard C++ only,
i.e. avoid compiler-extension or platform-specific features as much as possible.
In this way the effort to port the library to other platform/architecture might be minimum.

## Compiler Support

You need a C++ compiler with C++17 support. Below is a list of tested compilers:

**Linux**
* gcc >= 9.3
* clang >= 9.0

You can define the environment variable `CXX` to for example `clang++` to specify the C++ compiler.

**Windows**
* MSVC >= 14.2 (Visual Studio 2019, IDE or build tools)

**macOS**
* clang >= 12.0


## Build System for CMake Project

This repository uses [CMake](https://cmake.org/) and [Ninja](https://ninja-build.org/) as C++ build system.

## Build Dependencies

### C++

The table below shows the C++ build dependencies

| Library name                                                                                    | Requirements to build Python package                                                                                | Requirements to build CMake project                                                                                                                                                                                                                                                                   | Remark                                                                                                                                                                                                                                   | License                                                                                                                         |
|-------------------------------------------------------------------------------------------------|---------------------------------------------------------------------------------------------------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------|
| [boost](https://www.boost.org/)                                                                 | Define environment variable `BOOST_INCLUDE` to the include folder of `boost`\*                                      | CMake needs to be able find `boost`                                                                                                                                                                                                                                                                   | header-only                                                                                                                                                                                                                              | [Boost Software License - Version 1.0](https://www.boost.org/LICENSE_1_0.txt)                                                                                           |
| [eigen3](https://eigen.tuxfamily.org/)                                                          | Define environment variable `EIGEN_INCLUDE` to the include folder of `eigen3`\*                                     | CMake needs to be able find `eigen3`                                                                                                                                                                                                                                                                  | header-only                                                                                                                                                                                                                              | [Mozilla Public License, version 2.0](https://www.mozilla.org/en-US/MPL/2.0/)                                                                                                                           |
| [Catch2](https://github.com/catchorg/Catch2)                                                    | None                                                                                                                | CMake needs to be able find `Catch2`                                                                                                                                                                                                                                                                  | header-only                                                                                                                                                                                                                              | [Boost Software License 1.0](https://github.com/catchorg/Catch2/blob/devel/LICENSE.txt)                                                                                                                           |
| [nlohmann-json](https://github.com/nlohmann/json)                                               | None                                                                                                                | CMake needs to be able find `nlohmann_json`                                                                                                                                                                                                                                                           | header-only                                                                                                                                                                                                                              | [MIT](https://github.com/nlohmann/json/blob/develop/LICENSE.MIT)                                                                                                                             |
| [MKL](https://software.intel.com/content/www/us/en/develop/tools/oneapi/components/onemkl.html) | Add path to MKL runtime library (`libmkl_rt.so` or `mkl_rt.dll`) into `PATH` (Windows) or `LD_LIBRARY_PATH` (Linux) | <ul><li> Define environment variable `MKL_INCLUDE` to the include folder </li><li> Define environment variable `MKL_LIB` to the lib folder (`.lib` or `.so`) </li><li> Add path to MKL runtime library (`libmkl_rt.so` or `mkl_rt.dll`) into `PATH` (Windows) or `LD_LIBRARY_PATH` (Linux) </li></ul> | Optional for use of MKL [PARDISO](https://software.intel.com/content/www/us/en/develop/documentation/onemkl-developer-reference-c/top/sparse-solver-routines/onemkl-pardiso-parallel-direct-sparse-solver-interface.html) sparse solver. | [Intel Simplified Software License (proprietary license)](https://www.intel.com/content/www/us/en/developer/articles/license/onemkl-license-faq.html) |

\* The environment variables should point to the root include folder of the library, not a subfolder.
For example in the path `BOOST_INCLUDE` there should be a folder called `boost` which has all the `boost` header files.

### Python

The table below shows the Python dependencies

| Library name                                           | Remark                   | License            |
|--------------------------------------------------------|--------------------------|--------------------|
| [cython](https://cython.org/)                          | build dependency         | [Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0.html) |
 | [numpy](https://numpy.org/)                            | build/runtime dependency | [BSD-3](https://github.com/numpy/numpy/blob/main/LICENSE.txt)              |
 | [wheel](https://github.com/pypa/wheel)                 | build dependency         | [MIT](https://github.com/pypa/wheel/blob/main/LICENSE.txt)                | 
 | [pytest](https://github.com/pytest-dev/pytest)         | Development dependency   | [MIT](https://github.com/pytest-dev/pytest/blob/main/LICENSE)                |
 | [pytest-cov](https://github.com/pytest-dev/pytest-cov) | Development dependency   | [MIT](https://github.com/pytest-dev/pytest-cov/blob/master/LICENSE)                |
 | [twine](https://github.com/pypa/twine)                 | Development dependency   | [Apache License 2.0](https://github.com/pypa/twine/blob/main/LICENSE) |

You can install them using `pip`.

```
pip install numpy cython wheel pytest pytest-cov twine
```

# Build Python Package

Once you have prepared the build dependencies,
you can install the library from source in develop mode.
Go to the root folder of the repository.

```
python setup.py develop
```

If you have `pytest` installed in your Python environment, you can run the tests.

```
pytest
```

# Build CMake Project

There is a root cmake file in the root folder of the repo [`CMakeLists.txt`](../CMakeLists.txt).
It specifies dependencies and the build options for the project.
The core algorithm is implemented in the header-only library `power_grid_model`.
There are two sub-project defined in the root cmake file:

* [`tests/cpp_unit_tests/CMakeLists.txt`](../tests/cpp_unit_tests/CMakeLists.txt): the unit test project using `Catch2` framework.
* [`tests/benchmark_cpp/CMakeLists.txt`](../tests/benchmark_cpp/CMakeLists.txt): the C++ benchmark project for performance measure.

In principle, you can use any C++ IDE with cmake and ninja support to develop the C++ project.
When you use `cmake build` for the root cmake file,
the following additional options are available besides the standard cmake option.
If no option is defined, the cmake project will build the unit tests with *Eigen sparse solver*.

| Option                             | Description                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |
|------------------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `POWER_GRID_MODEL_SPARSE_SOLVER`   | Specify which sparse solver to use during the build. There are three options: <br> `EIGEN`: use built-in `SparseLU` solver of `eigen3`. <br> `MKL`: use MKL PARDISO solver and link MKL library at link-time. <br> `MKL_RUNTIME`:  build the project using both Eigen sparse solver and MKL PARDISO solver. The MKL library `mkl_rt` is loaded at runtime. If the MKL library cannot be found, it falls back to the built-in Eigen sparse solver. This is the build option for Python package. |
| `POWER_GRID_MODEL_BUILD_BENCHMARK` | When set to `1`, build the both sub-projects: unit test and benchmarks. Otherwise only build the unit test.                                                                                                                                                                                                                                                                                                                                                                                    |

# Example Setup for Ubuntu 20.04 (in WSL or physical/virtual machine)

In this section an example is given for setup in Ubuntu 20.04.
You can use this example in Windows Subsystem for Linux (WSL),
or in a physical/virtual machine.

## Environment variables

Append the following lines into the file `${HOME}/.bashrc`.

```shell
export CXX=clang++-11  # or clang++-10, or g++-9, or g++-10
export CC=clang-11  # or clang-10, or gcc-9, or gcc-10
export VCPKG_FEATURE_FLAGS=-binarycaching
export VCPKG_ROOT=${HOME}/vcpkg
export EIGEN_INCLUDE=${VCPKG_ROOT}/installed/x64-linux/include/eigen3
export BOOST_INCLUDE=${VCPKG_ROOT}/installed/x64-linux/include
export MKL_THREADING_LAYER=SEQUENTIAL
export MKL_INTERFACE_LAYER=LP64
export LD_LIBRARY_PATH=${HOME}/.local/lib:${LD_LIBRARY_PATH}
export MKL_LIB=${HOME}/.local/lib
export MKL_INCLUDE=${HOME}/.local/include
```

## Ubuntu Software Packages

Install the following packages from Ubuntu.
In the `.bashrc` you can set environment variable `CXX` to select a C++ compiler.

```shell
sudo apt update && sudo apt -y upgrade
sudo apt install -y curl zip unzip tar git build-essential lcov gcc g++ gcc-10 g++-10 clang clang-11 make cmake gdb ninja-build pkg-config python3.9 python3.9-dev python3.9-venv python3-pip
```

## C++ package manager: vcpkg

The recommended way to get C++ package is via [vcpkg](https://github.com/microsoft/vcpkg).

```shell
cd ${HOME}
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
./vcpkg install eigen3 catch2 boost nlohmann-json
```

**The installation of `boost` will take a long time, be patient**

## MKL

The easiest way to install `mkl` is through `pip`.
Since you can have multiple Python environment which may depend on a single `mkl` library.
It is better to install `mkl` without virtual environment.
Make sure your current shell is **NOT** in a virtual environment (no parentheses before the prompt).
Without `sudo`, it will install the `mkl` into the folder `${HOME}/.local`.

```shell
python3.9 -m pip install mkl mkl-include mkl-devel
```

## Build Python Library from Source

It is recommended to create a virtual environment.
Clone repository, create and activate virtual environment, and install the build dependency.
go to a root folder you prefer to save the repositories.

```shell
git clone https://github.com/alliander-opensource/power-grid-model.git  # or git@github.com:alliander-opensource/power-grid-model.git
cd power-grid-model
python3.9 -m venv .venv
source ./.venv/bin/activate
pip install -r dev-requirements.txt
```

Install from source in develop mode, and run `pytest`.

```shell
python setup.py develop
pytest
```

## Build CMake Project

There is a convenient shell script to build the cmake project:
[`build.sh`](../build.sh).
You can study the file and write your own build script.
Six configurations are pre-defined for two input arguments, which will be passed into `cmake`.
It includes debug or release build,
as well as the option to use MKL at link-time, or at runtime, or not at all.

* Option 1
  * Debug
  * Release
* Option 2
  * EIGEN
  * MKL
  * MKL_RUNTIME

As an example, go to the root folder of repo.
Use the following command to build the project with MKL at link-time and benchmark:

```shell
./build.sh Release MKL
```

One can run the unit tests and benchmark by:

```shell
./cpp_build_Relase_MKL/tests/cpp_unit_tests/power_grid_model_unit_tests

./cpp_build_Relase_MKL/tests/benchmark_cpp/power_grid_model_benchmark_cpp
```


# Example Setup for Windows 10

## Environment variables

Define the following environment variables in user wide.

| Name                      | Value                                                       |
|---------------------------|-------------------------------------------------------------|
| PreferredToolArchitecture | x64                                                         |
| VCPKG_ROOT                | <ROOT_FOLDER_OF_VCPKG>                                      |
| VCPKG_DEFAULT_TRIPLET     | x64-windows                                                 |
| VCPKG_FEATURE_FLAGS       | -binarycaching                                              |
| EIGEN_INCLUDE             | <ROOT_FOLDER_OF_VCPKG>\installed\x64-windows\include\eigen3 |
| BOOST_INCLUDE             | <ROOT_FOLDER_OF_VCPKG>\installed\x64-windows\include        |
| MKL_LIB                   | <ROOT_FOLDER_OF_MINICONDA>\envs\mkl-base\Library\lib        |
| MKL_INCLUDE               | <ROOT_FOLDER_OF_MINICONDA>\envs\mkl-base\Library\include    |
| MKL_THREADING_LAYER       | SEQUENTIAL                                                  |
| MKL_INTERFACE_LAYER       | LP64                                                        |

Further, append `<ROOT_FOLDER_OF_MINICONDA>\envs\mkl-base\Library\bin` to environment variable `PATH` in user wide.

## Software Toolchains

You need to install the MSVC compiler. You can either install the whole Visual Studio IDE or just the build tools.

* [Visual Studio Build Tools](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2019) (free)
    * Select C++ build tools
* Full [Visual Studio](https://visualstudio.microsoft.com/vs/) (All three versions are suitable. Check the license!)
    * Select Desktop Development with C++

Other toolchains:

* [Miniconda](https://docs.conda.io/en/latest/miniconda.html), install Python 3 64-bit under user wide.
* [Git](https://git-scm.com/downloads)

## C++ package manager: vcpkg

The recommended way to get C++ package is via [vcpkg](https://github.com/microsoft/vcpkg).
Open a PowerShell console, go to the parent folder of your desired <ROOT_FOLDER_OF_VCPKG>.

```shell
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.bat
./vcpkg install eigen3 catch2 boost nlohmann-json
```

## MKL

You can install `mkl` from `conda`.
Since you might want to use `mkl` in different projects,
it is recommended to (only) install `mkl` in a separate conda environment.
In this example the environment is called `mkl-base`. Open a Miniconda PowerShell Prompt.

```shell
conda create -n mkl-base python=3.9
conda activate mkl-base
conda install mkl mkl-include mkl-devel
```

## Build Python Library from Source

It is recommended to create a `conda` environment.
Clone repository, create and activate `conda` environment, and install the build dependency.
go to a root folder you prefer to save the repositories, open a Git Bash Console.

```shell
git clone https://github.com/alliander-opensource/power-grid-model.git
```

Then open a Miniconda PowerShell Prompt, go to the repository folder.

```shell
conda create -n power-grid-env python=3.9
conda activate power-grid-env
pip install -r dev-requirements.txt
```

Install from source in develop mode, and run `pytest`.

```shell
python setup.py develop
pytest
```

## Build CMake Project

If you have installed Visual Studio 2019/2022 (not the build tools),
you can open the repo folder as a cmake project.
The IDE should be able to automatically detect the Visual Studio cmake configuration file
[`CMakeSettings.json`](../CMakeSettings.json).
Six configurations are pre-defined. It includes debug or release build,
as well as the option to use MKL at link-time, or at runtime, or not at all.

* `x64-Debug`
* `x64-Debug-MKL`
* `x64-Debug-MKL-at-runtime`
* `x64-Release`
* `x64-Release-MKL`
* `x64-Release-MKL-at-runtime`



# Example Setup for macOS (Big Sur)

In this section an example is given for setup in macOS Big Sur and Python 3.10.
It is likely that other versions (3.8+) of Python are also supported, 
though you'll need to change some paths accordingly.

## Environment variables

Append the following lines into the file `${HOME}/.bashrc`.

```shell
export CXX=clang++
export CC=clang
export VCPKG_FEATURE_FLAGS=-binarycaching
export VCPKG_ROOT=${HOME}/vcpkg
export EIGEN_INCLUDE=${VCPKG_ROOT}/installed/x64-osx/include/eigen3
export BOOST_INCLUDE=${VCPKG_ROOT}/installed/x64-osx/include
export MKL_THREADING_LAYER=SEQUENTIAL
export MKL_INTERFACE_LAYER=LP64
export LD_LIBRARY_PATH=/Library/Frameworks/Python.framework/Versions/3.9/lib:${LD_LIBRARY_PATH}
export MKL_LIB=/Library/Frameworks/Python.framework/Versions/3.9/lib
export MKL_INCLUDE=/Library/Frameworks/Python.framework/Versions/3.9/include
```


## macOS Software Packages

Install the following packages with [Homebrew](https://brew.sh/).

```shell
brew install ninja cmake
```

## C++ package manager: vcpkg

The recommended way to get C++ package is via [vcpkg](https://github.com/microsoft/vcpkg).

```shell
cd ${HOME}
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
./vcpkg install eigen3 catch2 boost nlohmann-json
```

**The installation of `boost` will take a long time, be patient**

## MKL

The easiest way to install `mkl` is through `pip`.
Since you can have multiple Python environment which may depend on a single `mkl` library.
It is better to install `mkl` without virtual environment.
Make sure your current shell is **NOT** in a virtual environment (no parentheses before the prompt).
Without `sudo`, it will install the `mkl` into the folder `${HOME}/.local`.

```shell
python3.9 -m pip install mkl mkl-include mkl-devel
```

## Build Python Library from Source

It is recommended to create a virtual environment.
Clone repository, create and activate virtual environment, and install the build dependency.
go to a root folder you prefer to save the repositories.

```shell
git clone https://github.com/alliander-opensource/power-grid-model.git  # or git@github.com:alliander-opensource/power-grid-model.git
cd power-grid-model
python3.9 -m venv .venv
source ./.venv/bin/activate
pip install -r dev-requirements.txt
```

Install from source in develop mode, and run `pytest`.

```shell
python setup.py develop
pytest
```

## Build CMake Project

There is a convenient shell script to build the cmake project:
[`build.sh`](../build.sh).
You can study the file and write your own build script.
Six configurations are pre-defined for two input arguments, which will be passed into `cmake`.
It includes debug or release build,
as well as the option to use MKL at link-time, or at runtime, or not at all.

* Option 1
  * Debug
  * Release
* Option 2
  * EIGEN
  * MKL
  * MKL_RUNTIME

As an example, go to the root folder of repo.
Use the following command to build the project with MKL at link-time and benchmark:

```shell
./build.sh Release MKL
```

One can run the unit tests and benchmark by:

```shell
./cpp_build_Release_MKL/tests/cpp_unit_tests/power_grid_model_unit_tests

./cpp_build_Release_MKL/tests/benchmark_cpp/power_grid_model_benchmark_cpp
```
