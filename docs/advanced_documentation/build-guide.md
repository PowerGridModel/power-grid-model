<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

# Build Guide

This document explains how you can build this library from source, including some examples of build environment. In this
repository there are two builds:

* A `power-grid-model` [pip](https://pypi.org/project/power-grid-model/) Python package with C++ extension as the calculation core.
* A [CMake](https://cmake.org/) C++ project to build native C++ unit tests and/or performance benchmark.
* In the future, the following two libraries might be provided:
    * A C++ header-only library of the calculation core
    * A C/C++ dynamic shared library (`.dll` or `.so`) with pure C ABI interface.

## Build Requirements

To build the library from source, you need to first prepare the compiler toolchains and the build dependencies. In this
section a list of general requirements are given. After this section there are examples of setup in Linux (Ubuntu 22.04)
, Windows 10, and macOS (Big Sur).

### Architecture Support

This library is written and tested on `x86_64` and `arm64` architecture. Building the library in `IA-32` might be working, but is
not tested.

The source code is written with the mindset of ISO standard C++ only, i.e. avoid compiler-extension or platform-specific
features as much as possible. In this way the effort to port the library to other platform/architecture might be
minimum.

### Compiler Support

You need a C++ compiler with C++17 support. Below is a list of tested compilers:

**Linux**

* gcc >= 9.3
* clang >= 9.0

You can define the environment variable `CXX` to for example `clang++` to specify the C++ compiler.

**Windows**

* MSVC >= 14.2 (Visual Studio 2019, IDE or build tools)

**macOS**

* clang >= 12.0

### Build System for CMake Project

This repository uses [CMake](https://cmake.org/) and [Ninja](https://ninja-build.org/) as C++ build system.

### Build Dependencies

#### C++

The table below shows the C++ build dependencies

| Library name                                      | Requirements to build Python package | Requirements to build CMake project         | Remark      | License                                                                       |
|---------------------------------------------------|--------------------------------------|---------------------------------------------|-------------|-------------------------------------------------------------------------------|
| [boost](https://www.boost.org/)                   | Will be installed automatically      | CMake needs to be able find `boost`         | header-only | [Boost Software License - Version 1.0](https://www.boost.org/LICENSE_1_0.txt) |
| [eigen3](https://eigen.tuxfamily.org/)            | Will be installed automatically      | CMake needs to be able find `eigen3`        | header-only | [Mozilla Public License, version 2.0](https://www.mozilla.org/en-US/MPL/2.0/) |
| [doctest](https://github.com/doctest/doctest)     | None                                 | CMake needs to be able find `doctest`       | header-only | [MIT](https://github.com/doctest/doctest/blob/master/LICENSE.txt)            |
| [nlohmann-json](https://github.com/nlohmann/json) | None                                 | CMake needs to be able find `nlohmann_json` | header-only | [MIT](https://github.com/nlohmann/json/blob/develop/LICENSE.MIT)              |

#### Python

The table below shows the Python dependencies

| Library name                                                                            | Remark                   | License                                                                               |
|-----------------------------------------------------------------------------------------|--------------------------|---------------------------------------------------------------------------------------|
| [cython](https://cython.org/)                                                           | build dependency         | [Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0.html)                |
| [pybuild-header-dependency](https://github.com/TonyXiang8787/pybuild-header-dependency) | build dependency         | [BSD-3](https://github.com/TonyXiang8787/pybuild-header-dependency/blob/main/LICENSE) |
| [numpy](https://numpy.org/)                                                             | build/runtime dependency | [BSD-3](https://github.com/numpy/numpy/blob/main/LICENSE.txt)                         |
| [wheel](https://github.com/pypa/wheel)                                                  | build dependency         | [MIT](https://github.com/pypa/wheel/blob/main/LICENSE.txt)                            | 
| [pytest](https://github.com/pytest-dev/pytest)                                          | Development dependency   | [MIT](https://github.com/pytest-dev/pytest/blob/main/LICENSE)                         |
| [pytest-cov](https://github.com/pytest-dev/pytest-cov)                                  | Development dependency   | [MIT](https://github.com/pytest-dev/pytest-cov/blob/master/LICENSE)                   |

## Build Python Package

Once you have prepared the build dependencies, 
you can install the library from source in develop mode with the development dependency. 
Go to the root folder of the repository.

```
pip install -e .[dev]
```

Then you can run the tests.

```
pytest
```

## Build CMake Project

There is a root cmake file in the root folder of the repo [`CMakeLists.txt`](../../CMakeLists.txt). It specifies
dependencies and the build options for the project. The core algorithm is implemented in the header-only
library `power_grid_model`. There are two sub-projects defined in the root cmake file:

* [`tests/cpp_unit_tests/CMakeLists.txt`](../../tests/cpp_unit_tests/CMakeLists.txt): the unit test project using `doctest`
  framework.
* [`tests/benchmark_cpp/CMakeLists.txt`](../../tests/benchmark_cpp/CMakeLists.txt): the C++ benchmark project for
  performance measure.

In principle, you can use any C++ IDE with cmake and ninja support to develop the C++ project. When you
use `cmake build` for the root cmake file, the following additional options are available besides the standard cmake
option.

| Option                             | Description                                                                                                 |
|------------------------------------|-------------------------------------------------------------------------------------------------------------|
| `POWER_GRID_MODEL_COVERAGE`        | When set to `1`, build with test coverage. This is only applicable for Linux.                               |

## Visual Studio Code Support

You can use any IDE to develop this project. As a popular cross-platform IDE, the settings for Visual Studio Code is preconfigured in the folder `.vscode`. You can open the repository folder with VSCode and the configuration will be loaded automatically.

## Example Setup for Ubuntu 22.04 (in WSL or physical/virtual machine)

In this section an example is given for setup in Ubuntu 22.04. You can use this example in Windows Subsystem for Linux (
WSL), or in a physical/virtual machine.

### Environment variables

Append the following lines into the file `${HOME}/.bashrc`.

```shell
export CXX=clang++-14  # or g++-11
export CC=clang-14  # gcc-11
export CMAKE_PREFIX_PATH=/home/linuxbrew/.linuxbrew
export LLVM_COV=llvm-cov-14
```

### Ubuntu Software Packages

Install the following packages from Ubuntu.

```shell
sudo apt update && sudo apt -y upgrade
sudo apt install -y wget curl zip unzip tar git build-essential gcovr lcov gcc g++ clang make cmake gdb ninja-build pkg-config python3.10 python3.10-dev python3.10-venv python3-pip
```

### C++ packages

The recommended way to get C++ package is via [Homebrew](https://brew.sh/). 
Go to its website to follow the installation instruction.

Install the C++ dependencies

```shell
brew install boost eigen nlohmann-json doctest
```

### Build Python Library from Source

It is recommended to create a virtual environment. 
Clone repository, create and activate virtual environment. 
Go to a root folder you prefer to save the repositories.

```shell
git clone https://github.com/alliander-opensource/power-grid-model.git
cd power-grid-model
python3.10 -m venv .venv
source ./.venv/bin/activate
```

Install from source in develop mode, and run `pytest`.

```shell
pip install -e .[dev]
pytest
```

### Build CMake Project

There is a convenient shell script to build the cmake project:
{{ "[`build.sh`]({}/build.sh)".format(gh_link_head_blob) }}. You can study the file and write your own build script. Four configurations are pre-defined
for two input arguments, which will be passed into `cmake`. It includes debug or release build, as well as the option to
build test coverage or not.

* Option 1
    * Debug
    * Release
* Option 2 (optional)
    * Coverage (if specified, the test coverage will be run and a web report will be generated in `cpp_cov_html` folder.)

As an example, go to the root folder of repo. Use the following command to build the project in release mode:

```shell
./build.sh Release
```

One can run the unit tests and benchmark by:

```shell
./cpp_build_Release/tests/cpp_unit_tests/power_grid_model_unit_tests

./cpp_build_Release/tests/benchmark_cpp/power_grid_model_benchmark_cpp
```

## Example Setup for Windows 10

### Environment variables

Define the following environment variables in user wide.

| Name                      | Value                                                |
|---------------------------|------------------------------------------------------|
| PreferredToolArchitecture | x64                                                  |
| CMAKE_PREFIX_PATH         | C:\conda_envs\cpp_pkgs\Library                       |

### Software Toolchains

You need to install the MSVC compiler. You can either install the whole Visual Studio IDE or just the build tools.

* [Visual Studio Build Tools](https://aka.ms/vs/17/release/vs_BuildTools.exe) (free)
    * Select C++ build tools
* Full [Visual Studio](https://visualstudio.microsoft.com/vs/) (All three versions are suitable. Check the license!)
    * Select Desktop Development with C++

Other toolchains:

* [Miniconda](https://docs.conda.io/en/latest/miniconda.html), install Python 3 64-bit under user wide.
* [Git](https://git-scm.com/downloads)

### C++ packages

The recommended way to get C++ package is via `conda`. Open a miniconda console.

```shell
conda create --yes -p C:\conda_envs\cpp_pkgs -c conda-forge boost-cpp eigen nlohmann_json doctest
```

### Build Python Library from Source

It is recommended to create a `conda` environment. 
Clone repository, create and activate `conda` environment. 
Go to a root folder you prefer to save the repositories, open a Git Bash Console.

```shell
git clone https://github.com/alliander-opensource/power-grid-model.git
```

Then open a Miniconda PowerShell Prompt, go to the repository folder.

```shell
conda create -n power-grid-env python=3.10
conda activate power-grid-env
```

Install from source in develop mode, and run `pytest`.

```shell
pip install -e .[dev]
pytest
```

### Build CMake Project

If you have installed Visual Studio 2019/2022 (not the build tools), you can open the repo folder as a cmake project.
The IDE should be able to automatically detect the Visual Studio cmake configuration file
[`CMakeSettings.json`](../../CMakeSettings.json). Two configurations are pre-defined. It includes debug or release build.

* `x64-Debug`
* `x64-Release`

## Example Setup for macOS (Big Sur)

In this section an example is given for setup in macOS Big Sur and Python 3.10.

### Environment variables

Append the following lines into the file `${HOME}/.bashrc`.

```shell
export CXX=clang++
export CC=clang
export CMAKE_PREFIX_PATH=/usr/local
```

### macOS Software Packages and C++ libraries

Install the following packages with [Homebrew](https://brew.sh/).

```shell
brew install ninja cmake boost eigen nlohmann-json doctest
```

### Build Python Library from Source

It is recommended to create a virtual environment. Clone repository, create and activate virtual environment, and
install the build dependency. go to a root folder you prefer to save the repositories.

```shell
git clone https://github.com/alliander-opensource/power-grid-model.git 
cd power-grid-model
python3.10 -m venv .venv
source ./.venv/bin/activate
```

Install from source in develop mode, and run `pytest`.

```shell
pip install -e .[dev]
pytest
```

### Build CMake Project

There is a convenient shell script to build the cmake project:
[`build.sh`](../../build.sh). You can study the file and write your own build script. Two configurations are pre-defined
with one input argument, which will be passed into `cmake`. It includes debug or release build.

* Option 1
    * Debug
    * Release

**Note: the test coverage option is not supported in macOS.**

As an example, go to the root folder of repo. Use the following command to build the project in release mode:

```shell
./build.sh Release
```

One can run the unit tests and benchmark by:

```shell
./cpp_build_Release/tests/cpp_unit_tests/power_grid_model_unit_tests

./cpp_build_Release/tests/benchmark_cpp/power_grid_model_benchmark_cpp
```