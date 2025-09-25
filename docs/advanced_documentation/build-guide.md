<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Build Guide

This document explains how you can build this library from source, including some examples of build environment.
In this repository there are three builds:

* A `power-grid-model` [pip](https://pypi.org/project/power-grid-model/) Python package with C++ extension as the
  calculation core.
* A [CMake](https://cmake.org/) project consisting of the C++ header-only calculation core, and the following build
  targets:
  * A dynamic library (`.dll` or `.so`) with stable pure C API/ABI which can be used by any application (enabled by
    default)
  * An install target that installs the package containing the dynamic library (enabled by default)
  * Native C++ unit tests
  * C API tests
  * A performance benchmark program
  * An example C program to call the shared library
* A separate example [CMake](https://cmake.org/) project with a small C++ program that shows how to find and use the
  installable package.

## Build Requirements

To build the library from source, you need to first prepare the compiler toolchains and the build dependencies.
In this section a list of general requirements are given.
After this section there are examples of setup in Linux (Ubuntu 24.04), Windows 11, and macOS (Sequoia).

### Architecture Support

This library is written and tested on `x86_64` and `arm64` architecture.
Building the library in `IA-32` might be working, but is not tested.

The source code is written with the mindset of ISO standard C++ only, i.e. avoid compiler-extension or platform-specific
features as much as possible.
In this way, minimum effort should be necessary to port the library to other platform/architecture.

### Compiler Support

You need a C++ compiler with C++23 support.
Below is a list of tested compilers:

#### Linux

* gcc >= 14.0
  * Version 14.x tested using the version in the `manylinux_2_28` container.
  * Version 14.x tested using the musllinux build with custom compiler
  * Version 14.x tested in CI
* Clang >= 18.0
  * Version 18.x tested in CI
  * Version 18.x tested in CI with code quality checks

You can define the environment variable `CXX` to for example `clang++` to specify the C++ compiler.

#### Windows

* MSVC >= 19.0
  * Latest release tested in CI (e.g. Visual Studio 2022, IDE or build tools)
* Clang CL >= 19.0
  * Latest release tested in CI (e.g. Visual Studio 2022, IDE or build tools)

#### macOS

* Clang >= 17.0
  * Latest XCode release tested in CI

### Build System for CMake Project

This repository uses [CMake](https://cmake.org/) (version 3.23 or later) as C++ build system.

### Build Dependencies

#### C++

The table below shows the C++ build dependencies

| Library name                                                        | Requirements to build Python package | Requirements to build CMake project         | Remark      | License                                                                                                      |
| ------------------------------------------------------------------- | ------------------------------------ | ------------------------------------------- | ----------- | ------------------------------------------------------------------------------------------------------------ |
| [boost](https://www.boost.org/)                                     | Will be installed automatically      | CMake needs to be able find `boost`         | header-only | [Boost Software License - Version 1.0](https://www.boost.org/LICENSE_1_0.txt)                                |
| [eigen3](https://eigen.tuxfamily.org/)                              | Will be installed automatically      | CMake needs to be able find `eigen3`        | header-only | [Mozilla Public License, version 2.0](https://www.mozilla.org/en-US/MPL/2.0/)                                |
| [nlohmann-json](https://github.com/nlohmann/json)                   | Will be installed automatically      | CMake needs to be able find `nlohmann_json` | header-only | [MIT](https://github.com/nlohmann/json/blob/develop/LICENSE.MIT)                                             |
| [msgpack-cxx](https://github.com/msgpack/msgpack-c/tree/cpp_master) | Will be installed automatically      | CMake needs to be able find `msgpack-cxx`   | header-only | [Boost Software License - Version 1.0](https://github.com/msgpack/msgpack-c/blob/cpp_master/LICENSE_1_0.txt) |
| [doctest](https://github.com/doctest/doctest)                       | None                                 | CMake needs to be able find `doctest`       | header-only | [MIT](https://github.com/doctest/doctest/blob/master/LICENSE.txt)                                            |

#### Python

The table below shows the Python dependencies

| Library name                                                           | Remark                 | License                                                                                    |
| ---------------------------------------------------------------------- | ---------------------- | ------------------------------------------------------------------------------------------ |
| [numpy](https://numpy.org/)                                            | runtime dependency     | [BSD-3](https://github.com/numpy/numpy/blob/main/LICENSE.txt)                              |
| [scikit-build-core](https://github.com/scikit-build/scikit-build-core) | build dependency       | [Apache](https://github.com/scikit-build/scikit-build-core/blob/main/LICENSE)              |
| [pytest](https://github.com/pytest-dev/pytest)                         | Development dependency | [MIT](https://github.com/pytest-dev/pytest/blob/main/LICENSE)                              |
| [pytest-cov](https://github.com/pytest-dev/pytest-cov)                 | Development dependency | [MIT](https://github.com/pytest-dev/pytest-cov/blob/master/LICENSE)                        |
| [msgpack-python](https://github.com/msgpack/msgpack-python)            | Development dependency | [Apache License, Version 2.0](https://github.com/msgpack/msgpack-python/blob/main/COPYING) |

## Build Python Package

Once you have prepared the build dependencies,
you can install the library from source in editable mode with the development dependency.
Go to the root folder of the repository.

```shell
pip install -e .[dev]
```

Then you can run the tests.

```shell
pytest
```

A basic `self_test` function is provided to check if the installation was successful and ensures there are no build
errors, segmentation violations, undefined symbols, etc.
It performs multiple C API calls, runs through the main data flow, and verifies the integrity of serialization and
deserialization.

```python
from power_grid_model.utils import self_test
self_test()
```

## Build CMake Project

### User build

If you are a C-API user of the library, you can build the CMake using all the default settings.
You can specifiy a standard [CMAKE_BUILD_TYPE](https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html).
This will only build the core C-API dynamic library.

```shell
cmake -DCMAKE_BUILD_TYPE=Release -B build/
cmake --build build/ --config Release
```

You can further install the C-API dynamic library in the default `CMAKE_INSTALL_PREFIX` or a local directory.
The command below installs the C-API dynamic library in a local directory `install`.

```shell
cmake --install build/ --config Release --prefix install/
```

In the repository there is a package test that consumes the C-API dynamic library.
We can configure, build, install, and run the package test.

```shell
cd tests/package_tests
cmake -DCMAKE_BUILD_TYPE=Release -Dpower_grid_model_DIR="../../install/lib/cmake/power_grid_model/" -B build/
cmake --build build/ --config Release
cmake --install build/ --config Release --prefix install/
./install/bin/power_grid_model_package_test
```

### Developer build

If you opt for a developer build of Power Grid Model,
you can use the pre-defined CMake presets to enable developer build, including all the tests, warnings, examples, and
benchmark.
In the presets the [Ninja](https://ninja-build.org/) generator is used.
In principle, you can use any C++ IDE with cmake and ninja support to develop the C++ project.
It is also possible to use the bare CMake CLI to set up the project.
Supported presets for your development platform can be listed using `cmake --list-presets`.

In the developer build the following build targets (directories) are enabled:

* `power_grid_model_c`: a dynamic library (`.dll` or `.so`) with stable pure C API/ABI which can be used by any
  application
* `tests/cpp_unit_tests`: the unit test target for the C++ core using the `doctest` framework.
* `tests/cpp_validation_tests`: the validation test target using the `doctest` framework
* `tests/native_api_tests`: the C API test target using the `doctest` framework
* `tests/benchmark_cpp`: the C++ benchmark target for performance measure.
* `power_grid_model_c_example`: an example C program to call the dynamic library

On Linux/macOS, the presets will use command `clang`/`clang++` or `gcc`/`g++` to find the relevant `clang` or `gcc`
compiler.
It is the developer's reponsiblity to properly define symbolic links (which should be discoverable through `PATH`
environment variable) of `clang` or `gcc` compiler in your system.
If you want to build with `clang-tidy`, you also need to define symbolic link of `clang-tidy` to point to the actual
`clang-tidy` executable of your system.

Similar also applies to Windows: the presets will use command `cl.exe` or `clang-cl.exe` to find the compiler.
Developer needs to make sure the they are discoverable in `PATH`.
For x64 Windows native development using MSVC or Clang CL, please use the `x64 Native Command Prompt`, which uses
`vcvarsall.bat` to set up the appropriate build environment.

## Visual Studio Code Support

You can use any IDE to develop this project.
As a popular cross-platform IDE, the settings for Visual Studio Code is preconfigured in the folder `.vscode`.
You can open the repository folder with VSCode and the configuration will be loaded automatically.

```{note}
VSCode (as well as some other IDEs) does not set its own build environment itself.
For optimal usage, open the folder using `code <project_dir>` from a terminal that has the environment set up.
See above section for tips.
```

## Build Script for Linux/macOS

There is a convenient shell script to build the cmake project in Linux or macOS:
{{ "[`build.sh`]({}/build.sh)".format(gh_link_head_blob) }}.
You can study the file and write your own build script.
The following options are supported in the build script.

```shell
Usage: ./build.sh -p <preset> [-c] [-e] [-i] [-t]
  -c option generates coverage if available
  -e option to run C API example
  -i option to install package
  -t option to run integration test (requires '-i')
```

To list the available presets, run `./build.sh -h`.

## Example Setup for Ubuntu 24.04 (in WSL or physical/virtual machine)

In this section an example is given for setup in Ubuntu 24.04.
You can use this example in Windows Subsystem for Linux (WSL), or in a physical/virtual machine.

### Environment variables

Append the following lines into the file `${HOME}/.bashrc`.

```shell
export CXX=clang++-18            # or g++-14
export CC=clang-18               # or gcc-14
export CMAKE_PREFIX_PATH=/home/linuxbrew/.linuxbrew
export LLVM_COV=llvm-cov-18
export CLANG_TIDY=clang-tidy-18  # only if you want to use one of the clang-tidy presets
```

### Ubuntu Software Packages

Install the following packages from Ubuntu.

```shell
sudo apt update && sudo apt -y upgrade
sudo apt install -y wget curl zip unzip tar git build-essential gcovr lcov gcc g++ clang-18 make gdb ninja-build pkg-config python3.12 python3.12-dev python3.12-venv python3-pip
```

### C++ packages

The recommended way to get C++ package is via [Homebrew](https://brew.sh/).

```{note}
Go to its website to follow the installation instruction.
```

Install the C++ dependencies

```shell
brew install boost eigen nlohmann-json msgpack-cxx doctest cmake
```

### Build Python Library from Source

It is recommended to create a virtual environment.
Clone repository, create and activate virtual environment.
Go to a root folder you prefer to save the repositories.

```shell
git clone https://github.com/PowerGridModel/power-grid-model.git
cd power-grid-model
python3.11 -m venv .venv
source ./.venv/bin/activate
```

Install from source in develop mode, and run `pytest`.

```shell
pip install -e .[dev]
pytest
```

### Build CMake Project

There is a convenient shell script to build the cmake project:
{{ "[`build.sh`]({}/build.sh)".format(gh_link_head_blob) }}.

As an example, go to the root folder of repo.
Use the following command to build the project in release mode:

```shell
./build.sh -p <preset>
```

To list the available presets, run `./build.sh -h`.

One can run the unit tests and C API example by:

```shell
ctest --preset <preset>
```

or

```shell
cpp_build/<preset>/bin/power_grid_model_unit_tests

cpp_build/<preset>/bin/power_grid_model_c_example
```

or install using

```shell
cmake --build --preset <preset> --target install
```

## Example Setup for Windows 11

Define the following environment variable user-wide:

| Name                | Value                            |
| ------------------- | -------------------------------- |
| `CMAKE_PREFIX_PATH` | `C:\conda_envs\cpp_pkgs\Library` |

### Software Toolchains

You need to install the MSVC compiler.
You can either install the whole Visual Studio IDE or just the build tools.

* [Visual Studio Build Tools](https://aka.ms/vs/17/release/vs_BuildTools.exe) (free)
  * Select C++ build tools
* Full [Visual Studio](https://visualstudio.microsoft.com/vs/) (All three versions are suitable.
  Check the license!)
  * Select Desktop Development with C++
    * [Optional] Select `C++ Clang tools for Windows`

Other toolchains:

* [Miniforge](https://github.com/conda-forge/miniforge), install Python 3 64-bit under user wide.
* [Git](https://git-scm.com/downloads)

```{note}
It is also possible to use any other `conda` provider like [Miniconda](https://docs.conda.io/en/latest/miniconda.html).
However, we recommend using [Miniforge](https://github.com/conda-forge/miniforge), because it is published under BSD
License and by default does not have any references to commercially licensed software.
```

```{note}
Long paths for (dependencies in) the installation environment might exceed the `maximum path length limitation` set by
Windows, causing the installation to fail.
It is possible to enable long paths in Windows by following the steps in the
[Microsoft documentation](https://learn.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation?tabs=registry)
```

### C++ packages

The recommended way to get C++ package is via `conda`.
Open a miniconda console.

```shell
conda create --yes -p C:\conda_envs\cpp_pkgs -c conda-forge libboost-headers eigen nlohmann_json msgpack-cxx doctest
```

```{note}
Long paths for (dependencies in) the installation environment might exceed the `maximum path length limitation` set by
Windows, causing the installation to fail.
It is possible to enable long paths in Windows by following the steps in the
[Microsoft documentation](https://learn.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation?tabs=registry)
```

### Build Python Library from Source

It is recommended to create a virtual environment.
Clone repository, create and activate virtual environment.
Go to a root folder you prefer to save the repositories, open a Git Bash Console.

```shell
git clone https://github.com/PowerGridModel/power-grid-model.git
```

Go to the repository folder.

```shell
python -m venv .venv
.venv\Script\activate
```

Install from source in develop mode, and run `pytest`.

```shell
pip install -e .[dev]
pytest
```

### Build CMake Project

If you have installed Visual Studio 2019/2022 (not the build tools), you can open the repo folder as a cmake project.
The IDE should be able to automatically detect the Visual Studio cmake configuration file `CMakePresets.json`.
Several configurations are pre-defined.
It includes debug and release builds.

* `msvc-debug`, displayed as `Debug (MSVC)`
* `msvc-release`, displayed as `Release (MSVC)`.
* `clang-cl-debug`, displayed as `Debug (Clang CL)`
* `clang-cl-release`, displayed as `Release (Clang CL)`

```{note}
- The `Release` presets are compiled with debug symbols.
- The `Clang CL` presets require `clang-cl` to be installed, e.g. by installing `C++ Clang tools for Windows`.
- When using an IDE that does not automatically set the toolchain environment using `vcvarsall.bat`, e.g. Visual Studio
  Code, make sure to open that IDE from a terminal that does so instead (e.g. `x64 Native Tools Command Prompt`).
```

## Example Setup for macOS (Sequoia)

In this section an example is given for setup in macOS Sequoia and Python 3.11.

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
brew install ninja cmake boost eigen nlohmann-json msgpack-cxx doctest
```

### Build Python Library from Source

It is recommended to create a virtual environment.
Clone repository, create and activate virtual environment, and install the build dependency.
Go to a root folder of your choice to save the repositories.

```shell
git clone https://github.com/PowerGridModel/power-grid-model.git 
cd power-grid-model
python3.11 -m venv .venv
source ./.venv/bin/activate
```

Install from source in develop mode, and run `pytest`.

```shell
pip install -e .[dev]
pytest
```

### Build CMake Project

There is a convenient shell script to build the cmake project:
{{ "[`build.sh`]({}/build.sh)".format(gh_link_head_blob) }}.

**Note: the test coverage option is not supported in macOS.**

As an example, go to the root folder of repo.
Use the following command to build the project in release mode:

```shell
./build.sh -p <preset>
```

To list the available presets, run `cmake --list-presets`.

One can run the unit tests and C API example by:

```shell
ctest --preset <preset>
```

or

```shell
cpp_build/<preset>/bin/power_grid_model_unit_tests

cpp_build/<preset>/bin/power_grid_model_c_example
```

or install using

```shell
cmake --build --preset <preset> --target install
```

## Package tests

The {{ "[package tests]({}/tests/package_tests)".format(gh_link_head_blob) }} project is a completely separate CMake
project contained in {{ "[`tests/package_tests`]({}/tests/package_tests)".format(gh_link_head_blob) }}.

This project is designed to test and illustrate finding and linking to the installed package from the Power Grid Model
project.
Setup of this project is done the same way as the setup of the main project mentioned in the above, but with the
{{ "[`tests/package_tests`]({}/tests/package_tests)".format(gh_link_head_blob) }} directory as its root folder.

```{note}
This project has the main project as a required dependency.
Configuration will fail if the main project has not been built and installed, e.g. using
`cmake --build --preset <preset> --target install` for the current preset.
```

## Documentation

The documentation is built in [Sphinx](https://github.com/sphinx-doc/sphinx).
It can be built locally in a Python environment.
The packages required for building it can be found under the `[doc]` optional dependencies.
In addition, the `power-grid-model` Python package needs to be built by following the steps mentioned
[above](#build-python-package).
After that, the documentation specific packages can be installed via:

```shell
pip install -e .[doc]
```

```{note}
The `pip install .` part of the command installs the complete package from scratch.
```

The C API documentation is generated using [Doxygen](https://www.doxygen.nl).
If you do not have Doxygen installed, it can also be temporarily bypassed by commenting out the `breathe` settings in
`docs/conf.py`.

The documentation can be built with the following commands, resulting in HTML files of the webpages which can be found
in `docs/_build/html` directory.

```shell
cd docs/doxygen
doxygen
cd ..
sphinx-build -b html . _build/html
```

```{note}
The user documentation generated by [Sphinx](https://github.com/sphinx-doc/sphinx) only contains the C API
documentation.
Doxygen, however, also builds the documentation for the Power Grid Model core implementation, which can be accessed
after building the documentation locally via `docs/_build/html/index.html`.
```
