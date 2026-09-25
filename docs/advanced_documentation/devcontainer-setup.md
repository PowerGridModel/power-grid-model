<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Development Container Setup Guide

This document explains how you can use our development container (dev container) setups to contribute to
`power-grid-model`.
There are two separately selectable configurations:

* The default container (`.devcontainer/devcontainer.json`) for the _Python_ side of `power-grid-model`.
* A dedicated container (`.devcontainer/cpp/devcontainer.json`) for _C++_ development (see
  [C++ Development Container](#c-development-container)).

```{note}
A [development container](https://containers.dev/) is a pre-configured
[container](https://en.wikipedia.org/wiki/Containerization_(computing)).
It includes all necessary dependencies for an immediate development start.
Most modern IDEs support development containers natively.
This allows you to use your preferred editor and personal extensions directly inside the isolated environment.
```

## Prerequisites

To start developing in the development container, you must clone the repository, install a container engine, and
_(optionally)_ configure an IDE or editor that supports remote container connections.

First, **clone the repository:**

```shell
git clone https://github.com/PowerGridModel/power-grid-model.git
```

For the container engine and the IDE/editor you have several options:

**Container Engine:**

* [Podman](#podman): _Recommended_ (free for both private and commercial use)
* [Docker](#docker): (larger enterprises require a paid subscription)

**IDE/Editor:**

This is a _non-exhaustive_ list, as other IDEs and editors support development containers too.

* [Visual Studio Code](#visual-studio-code): _Fully supported_
* [PyCharm](#pycharm)

## Container Engine

You _must_ install _one_ of the container engines listed below to use the development container.

### Podman

[Podman](https://podman.io/) is an open-source container management tool featuring a command-line interface and an
optional graphical user interface called `Podman Desktop`.
Both tools are free for both private _and commercial_ use.
A key security advantage of Podman over other engines is its _rootless_ architecture, meaning it operates entirely
without root (admin) privileges.

* **Installation:** Follow the official guides for [Podman](https://podman.io/docs/installation) _or_
[Podman Desktop](https://podman-desktop.io/docs/installation).
* **Initialization:** If using `Podman Desktop`, open the application and click `initialize and start`.
Once successful, the status will change to a green `RUNNING` indicator.

### Docker

[Docker](https://www.docker.com/) consists of the core `Docker` engine and an optional graphical interface,
`Docker Desktop`.
Note that professional use of Docker Desktop in larger enterprises _requires a paid subscription._

* **Installation:** Follow the official guides for [Docker](https://docs.docker.com/engine/install/) Engine _or_
[Docker Desktop](https://docs.docker.com/get-started/get-docker/).

## IDE/Editor

The following IDEs provide development container integration.
Other editors that support development containers may also work but are not explicitly listed here.

### Visual Studio Code

[Visual Studio Code](https://code.visualstudio.com/docs/devcontainers/containers) offers full, native integration with
development containers.

#### Setup

* Install the [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)
  extension.
* Open the cloned `power-grid-model` folder in VS Code.
* Click the green `Remote Indicator` button in the bottom-left corner or open the Command Palette
  (`Ctrl+Shift+P` / `Cmd+Shift+P`)
* Select `Dev Containers: Reopen in Container`.

#### Usage

##### Testing

Visual Studio Code has built-in [support](https://code.visualstudio.com/docs/python/testing) for the Python tests in
`power-grid-model`.
You can open the test view via `View / Testing`, which lists all available tests.
From there, you can run or [debug](https://code.visualstudio.com/docs/python/debugging) individual tests, or execute the
entire test suite at once.

To debug a test, set a breakpoint and click the debug icon next to the test; the IDE will pause execution at your
breakpoint.
You can also launch tests directly from the source code by clicking the test icon in the gutter next to the test
definition line.

#### Pre-Commit

`power-grid-model` automatically installs [pre-commit](https://pre-commit.com/) inside the development container.
This framework runs automated checks before each commit to ensure all tests pass, the source code is properly formatted,
and certain quality standards are met.

#### Linting

The development container utilizes Visual Studio Code extensions to seamlessly integrate code linting for Python, TOML,
Markdown, JSON, and other formats directly into the IDE.

### PyCharm

[PyCharm Professional](https://www.jetbrains.com/help/pycharm/dev-containers-starting-page.html) provides native support
for development containers.
Note that the _Professional_ edition is required to have native development container support in PyCharm.

#### Setup

* Open the [welcome screen](https://www.jetbrains.com/help/pycharm/welcome-screen.html) of PyCharm.
* Click `Remote Development`, choose `Dev Containers` and click the `Create Dev Containers` button.
* Select your local path to the cloned repository and choose your container engine.

## C++ Development Container

The dedicated C++ container (`.devcontainer/cpp/devcontainer.json`) provides the Linux C++ toolchain for
`power-grid-model`: `gcc-14`/`g++-14` plus `clang-18`, `CMake` (>= 3.23), `Ninja`, `gdb`, `clang-format`/`clang-tidy`,
and the C++ dependencies from the [build guide](./build-guide.md) (`boost`, `eigen3`, `nlohmann-json`, `msgpack-cxx`,
`doctest`).
The Python container remains the default and is unchanged; combined Python/C++ and docs-building environments are
out of scope.

### Selecting the C++ container in VS Code

* Install the [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)
  extension and open the cloned `power-grid-model` folder.
* Click the green `Remote Indicator` in the bottom-left corner or open the Command Palette
  (`Ctrl+Shift+P` / `Cmd+Shift+P`), then select `Dev Containers: Reopen in Container`.
* When prompted, pick the `power-grid-model (C++)` configuration (`.devcontainer/cpp/devcontainer.json`).

### Configure and build (CMake preset)

The container defaults to the existing `gcc-debug` CMake preset (`cmake.configurePreset`); use `clang-debug`
instead if you want the Clang build.
From a VS Code terminal inside the container, or any terminal with the environment set up:

```shell
cmake --preset gcc-debug
cmake --build --preset gcc-debug
```

Alternatively, use the CMake Tools extension preset picker, or the shortcut `./build.sh -p gcc-debug`.

### Run the C++ tests

```shell
ctest --preset gcc-debug --output-on-failure
```

The `doctest` suites are also discoverable in the VS Code Test Explorer via the Catch2 test adapter.

### Debug a C++ test in the IDE

* Build with a Debug preset as above so binaries carry symbols.
* Set a breakpoint, then start the preconfigured `Debug validation test [Unix]` launch configuration
  (`.vscode/launch.json`, `gdb`-based), or right-click an individual test in the Test Explorer and debug it.
* The launch configuration resolves the test binary via `${command:cmake.buildDirectory}`
  (e.g. `cpp_build/gcc-debug/bin/power_grid_model_validation_tests`).

### Returning to the Python container

Reopen the folder in the default container (`Dev Containers: Reopen in Container`, then pick the
`power-grid-model` configuration), or `Dev Containers: Reopen Folder Locally` first and then reopen in the
Python container.
