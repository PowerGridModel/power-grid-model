# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

# Dedicated C++ development container for power-grid-model.
# Matches the Linux CI toolchain (Ubuntu 26.04, GCC/G++ 16, Clang 22) and uses
# the same header-only C++ dependency provisioner as Linux CI.
# clang-tidy remains on version 18 on purpose: CI does the same (see PR #1631).
FROM mcr.microsoft.com/devcontainers/base:resolute

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        gcc-16 g++-16 \
        clang-22 lld-22 libclang-rt-22-dev clang-tidy-18 \
        cmake ninja-build gdb libcli11-dev \
        gcovr lcov pkg-config git curl ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# The CMake presets resolve `gcc`/`g++`/`clang`/`clang++` via PATH
# (same approach as CI, which symlinks the versioned binaries).
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-16 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-16 100 \
    && update-alternatives --install /usr/bin/clang clang /usr/bin/clang-22 100 \
    && update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-22 100 \
    && update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-18 100

# uv provides the Python runtime for the repository's uv-managed tooling.
# pre-commit, REUSE and clang-format are installed from uv.lock by the
# postCreateCommand, so they stay in sync with CI instead of being duplicated
# as global tools here.
ENV UV_INSTALL_DIR=/usr/local/bin
RUN curl -LsSf https://astral.sh/uv/install.sh | sh

# Match CI's dependency provisioner. The CMake wrapper resolves its package
# roots at invocation time, independent of the uv-managed Python install path.
RUN UV_TOOL_DIR=/opt/pgm-build-deps/tools \
    UV_TOOL_BIN_DIR=/opt/pgm-build-deps/bin \
    uv tool install pgm-build-deps \
    && chmod -R a+rX /opt/pgm-build-deps

COPY --chmod=0755 cmake-wrapper.sh /usr/local/bin/cmake

ENV CC=gcc CXX=g++
ENV DEBIAN_FRONTEND=dialog
