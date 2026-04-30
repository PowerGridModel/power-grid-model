<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Build Guide

This document explains how you can build this library from source, including some examples of build environments.
In this repository there are three builds:

* A `power-grid-model` ([PyPI](https://pypi.org/project/power-grid-model/)) Python package with a C++ extension as the
  calculation core.
* A [CMake](https://cmake.org/) project consisting of the C++ header-only calculation core, and the following build
  targets:
  * A dynamic library (`.dll` on Windows, `.so` on Linux, or `.dylib` on macOS) with a stable pure C API/ABI which can
    be used by any application (enabled by default).
  * An install target that installs the package containing the dynamic library (enabled by default).
  * Native C++ unit tests.
  * C API tests.
  * A performance benchmark program.
  * An example C program to call the shared library.
* A separate example [CMake](https://cmake.org/) project with a small C++ program that shows how to find and use the
  installable package.

```{contents}
```

## Build Requirements

To build the library from source, you need to first prepare the compiler toolchains and the build dependencies.
In this section a list of general requirements are given.
After this section there are examples of setup in Linux (Ubuntu 24.04), Windows 11, and macOS (Tahoe).

### Architecture Support

This library is written and tested on `x86_64` and `arm64` architectures.
Building the library in `IA-32` might work, but this is not tested.

The source code is written with the mindset of ISO standard C++ only, i.e. we avoid compiler-extension or
platform-specific features as much as possible.
In this way, minimum effort should be necessary to port the library to other platforms/architectures.

### Compiler Support

You need a C++ compiler with C++23 support.
Below is a list of tested compilers:

#### Linux

* gcc >= 14.0:
  * Version 14.x tested using the version in the `manylinux_2_28` container.
  * Version 14.x tested using the `musllinux` build with custom compiler.
  * Version 14.x tested in CI.
* Clang >= 18.0:
  * Version 18.x tested in CI.
  * Version 18.x tested in CI with code quality checks.

```{Admonition} Additional information
Wheel builds for Linux are done inside containers using `cibuildwheel`:
- **manylinux_2_28**: glibc-based Linux distributions (Ubuntu, Debian, Fedora, etc.)
- **musllinux_1_2**: musl-based Linux distributions (Alpine Linux, etc.)

These are handled automatically in CI. For local development, use your system's native compiler.
```

#### Windows

* MSVC >= 19.0:
  * Latest release tested in CI (e.g. Visual Studio 2022, IDE or build tools).
* Clang CL >= 19.0:
  * Latest release tested in CI (e.g. Visual Studio 2022, IDE or build tools).

#### macOS

* Apple Clang >= 21.0:
  * Latest XCode release tested in CI.

```{note}
Once your compiler of choice is installed, you need to define the environment variables `CC` and `CXX` to specify the compiler. For example `export CC=clang-18` and `export CXX=clang++-18` to select the `clang` compiler in Ubuntu.
```

### Build System for CMake Project

This repository uses [CMake](https://cmake.org/) (version 3.23 or later) as its C++ build system.

### Build Dependencies

#### C++

The table below shows the C++ build dependencies.

```{note}
The C++ dependencies below are **build-time only**. When building the Python package from source (via `uv sync`), they are automatically downloaded and used during the build — you do not need to install them manually. Manual installation is only required for standalone CMake builds.
```

| Library name                                                        | Requirements to build Python package | Requirements to build CMake project         | Remark      | License                                                                                                      |
| ------------------------------------------------------------------- | ------------------------------------ | ------------------------------------------- | ----------- | ------------------------------------------------------------------------------------------------------------ |
| [boost](https://www.boost.org/)                                     | Installed automatically              | CMake needs to be able find `boost`         | header-only | [Boost Software License - Version 1.0](https://www.boost.org/LICENSE_1_0.txt)                                |
| [eigen3](https://eigen.tuxfamily.org/)                              | Installed automatically              | CMake needs to be able find `eigen3`        | header-only | [Mozilla Public License, version 2.0](https://www.mozilla.org/en-US/MPL/2.0/)                                |
| [nlohmann-json](https://github.com/nlohmann/json)                   | Installed automatically              | CMake needs to be able find `nlohmann_json` | header-only | [MIT](https://github.com/nlohmann/json/blob/develop/LICENSE.MIT)                                             |
| [msgpack-cxx](https://github.com/msgpack/msgpack-c/tree/cpp_master) | Installed automatically              | CMake needs to be able find `msgpack-cxx`   | header-only | [Boost Software License - Version 1.0](https://github.com/msgpack/msgpack-c/blob/cpp_master/LICENSE_1_0.txt) |
| [doctest](https://github.com/doctest/doctest)                       | None                                 | CMake needs to be able find `doctest`       | header-only | [MIT](https://github.com/doctest/doctest/blob/master/LICENSE.txt)                                            |

To install the C++ dependencies for a CMake build, use your platform's package manager of choice.
In the platform-specific examples below, we will give some suggestions.

```{note}
Commonly used package managers include `brew` on macOS, `brew` or `apt` on Linux, and `conda` on Windows.
Set `CMAKE_PREFIX_PATH` to the installation prefix of your package manager so CMake can locate the libraries.
```

#### Python

The table below shows the Python dependencies.

| Library name                                                           | Remark                 | License                                                                                    |
|------------------------------------------------------------------------|------------------------|--------------------------------------------------------------------------------------------|
| [numpy](https://numpy.org/)                                            | Runtime dependency     | [BSD-3](https://github.com/numpy/numpy/blob/main/LICENSE.txt)                              |
| [scikit-build-core](https://github.com/scikit-build/scikit-build-core) | Build dependency       | [Apache](https://github.com/scikit-build/scikit-build-core/blob/main/LICENSE)              |
| [pytest](https://github.com/pytest-dev/pytest)                         | Development dependency | [MIT](https://github.com/pytest-dev/pytest/blob/main/LICENSE)                              |
| [pytest-cov](https://github.com/pytest-dev/pytest-cov)                 | Development dependency | [MIT](https://github.com/pytest-dev/pytest-cov/blob/master/LICENSE)                        |
| [msgpack-python](https://github.com/msgpack/msgpack-python)            | Development dependency | [Apache License, Version 2.0](https://github.com/msgpack/msgpack-python/blob/main/COPYING) |
| [uv](https://github.com/astral-sh/uv)                                  | Development dependency | [Apache License, Version 2.0](https://github.com/astral-sh/uv/blob/main/LICENSE-APACHE)    |

## Build Python Package

Once you have prepared the build dependencies,
you can install the library from source in editable mode (with the development dependency group by default).
Go to the root folder of the repository.

```shell
uv sync
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
You can specify a standard [CMAKE_BUILD_TYPE](https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html).
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

* `power_grid_model_c`: a dynamic library (`.so` on Linux, `.dylib` on macOS, `.dll` on Windows) with stable pure
C API/ABI which can be used by any
  application.
* `tests/cpp_unit_tests_*`: the different unit test targets for the C++ core using the `doctest` framework.
* `tests/cpp_validation_tests`: the validation test target using the `doctest` framework.
* `tests/native_api_tests`: the C API test target using the `doctest` framework.
* `tests/benchmark_cpp`: the C++ benchmark target for performance measure.
* `power_grid_model_c_example`: an example C program to call the dynamic library.

On Linux/macOS, the presets will use command `clang`/`clang++` or `gcc`/`g++` to find the relevant `clang` or `gcc`
compiler.
It is the developer's responsibility to properly define symbolic links (which should be discoverable through `PATH`
environment variable) of `clang` or `gcc` compiler in your system.
If you want to build with `clang-tidy`, you also need to define symbolic link of `clang-tidy` to point to the actual
`clang-tidy` executable of your system.

The same also applies to Windows: the presets will use command `cl.exe` or `clang-cl.exe` to find the compiler.
The developer needs to make sure they are discoverable in `PATH`.
For x64 Windows native development using MSVC or Clang CL, please use the `x64 Native Command Prompt`, which uses
`vcvarsall.bat` to set up the appropriate build environment.

### Build Script for Linux/macOS

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

### Ubuntu Software Packages

Install the minimum required packages:

```shell
sudo apt update && sudo apt -y upgrade
sudo apt install -y build-essential gcc g++ clang-18 make ninja-build pkg-config
```

The following packages are optional depending on your use case:

```shell
sudo apt install -y gcovr lcov # For coverage reports
sudo apt install -y gdb # For debugging
sudo apt install -y wget curl zip unzip tar git # General use tools
```

### C++ Dependencies for CMake

The recommended way to get the [C++ packages](#c) and `uv` is via [Homebrew](https://brew.sh):

```shell
brew install boost eigen nlohmann-json msgpack-cxx doctest cmake uv
```

### Environment variables

Append the following lines into the file `${HOME}/.bashrc`.

```shell
export CXX=clang++-18            # or g++-14
export CC=clang-18               # or gcc-14
export CMAKE_PREFIX_PATH=/home/linuxbrew/.linuxbrew  # only needed for CMake builds
export LLVM_COV=llvm-cov-18      # only if you want to use one of the llvm features
export CLANG_TIDY=clang-tidy-18  # only if you want to use one of the clang-tidy presets
```

### Build Python Library from Source

Go to the root folder you prefer to save the repositories.

```shell
git clone https://github.com/PowerGridModel/power-grid-model.git
cd power-grid-model
```

Install from source in develop mode and run the tests:

```shell
uv sync
uv run pytest
```

### Build CMake Project

Refer to the [Build CMake Project](#build-cmake-project) section above for full details.
As a quick start, from the root of the repository:

```shell
./build.sh -p <preset>   # list available presets with: ./build.sh -h
```

## Example Setup for Windows 11

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

### Install uv

You can install `uv` by the following command as suggested in their [website](https://github.com/astral-sh/uv).

```shell
powershell -ExecutionPolicy ByPass -c "irm https://astral.sh/uv/install.ps1 | iex"
```

### C++ Dependencies for CMake

The recommended way to get [C++ packages](#c) is via `conda`. Open a miniforge console.

```shell
conda create --yes -p C:\conda_envs\cpp_pkgs -c conda-forge libboost-headers eigen nlohmann_json msgpack-cxx doctest
```

### Environment variables

Set `CMAKE_PREFIX_PATH` so CMake can locate the C++ libraries. In PowerShell:

```powershell
$env:CMAKE_PREFIX_PATH = C:\conda_envs\cpp_pkgs\Library
```

To make it persistent across sessions:

```powershell
[System.Environment]::SetEnvironmentVariable("CMAKE_PREFIX_PATH", "C:\conda_envs\cpp_pkgs\Library", "User")
```

### Build Python Library from Source

Clone repository, go to a root folder you prefer to save the repositories, open a Git Bash Console.

```powershell
git clone https://github.com/PowerGridModel/power-grid-model.git
```

Go to the repository folder in your powershell, build project from source and run `pytest`.

```powershell
uv sync
uv run pytest
```

#### Build Python Library from Source with Native Debug Symbols

For debugging purposes, it may be useful to build the Power Grid Model with debug symbols in the shared native library.
To do so, the following command can be used to override the default build settings.

```powershell
uv sync --config-settings=cmake.build-type="RelWithDebInfo"
```

It is also possible to install the Power Grid Model as a full debug build, including extra sanity checks and with a
lower degree of optimizations. Note this may come with a significant impact on the performance.

```powershell
uv sync --config-settings=cmake.build-type="Debug"
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

## Example Setup for macOS (Tahoe)

In this section, an example is given for setup in macOS Tahoe and the project's default Python.

### C++ Dependencies for CMake

The recommended way to get the [C++ packages](#c) and `uv` is via [Homebrew](https://brew.sh):

```shell
brew install ninja cmake boost eigen nlohmann-json msgpack-cxx doctest uv
```

### Environment variables

Append the following lines into the file `${HOME}/.zshrc` (or `${HOME}/.bashrc` if using bash).

```shell
export CXX=clang++
export CC=clang
export CMAKE_PREFIX_PATH=/usr/local  # only needed for CMake builds
```

### Build Python Library from Source

Go to a root folder of your choice to save the repositories.

```shell
git clone https://github.com/PowerGridModel/power-grid-model.git
cd power-grid-model
```

Install from source in develop mode and run the tests:

```shell
uv sync
uv run pytest
```

### Build CMake Project

Refer to the [Build CMake Project](#build-cmake-project) section above for full details.
As a quick start, from the root of the repository:

```shell
./build.sh -p <preset>   # list available presets with: ./build.sh -h
```

```{note}
Test coverage is not supported on macOS.
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

## Visual Studio Code Support

You can use any IDE to develop this project.
As a popular cross-platform IDE, the settings for Visual Studio Code is preconfigured in the folder `.vscode`.
You can open the repository folder with VSCode and the configuration will be loaded automatically.

```{note}
VSCode (as well as some other IDEs) does not set its own build environment itself.
For optimal usage, open the folder using `code <project_dir>` from a terminal that has the environment set up.
See the platform-specific setup sections above for guidance.
```

For automatic formatting of JSON(C) files, you will need to have [Node.js](https://nodejs.org/) installed.
You may also need to update the recommended extension Biome to the latest version using
`manage > install specific version` for optimal up-to-date support.
Alternatively, as usual, you can also use
{{ "[`pre-commit`]({}/CONTRIBUTING.md#pre-commit-hooks)".format(gh_link_head_blob) }} to keep all files
correctly formatted before committing.

## Documentation

The documentation is built in [Sphinx](https://github.com/sphinx-doc/sphinx).
It can be built locally in a Python environment.
The packages required for building it can be found under the `[doc]` optional dependencies.
In addition, the `power-grid-model` Python package needs to be built by following the steps mentioned
[above](#build-python-package).
After that, the documentation specific packages can be installed via:

```shell
uv sync --group doc
```

```{note}
The `uv sync` part of the command installs the complete package from scratch.
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
