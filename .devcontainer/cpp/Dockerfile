# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

# Dedicated C++ development container for power-grid-model.
# Matches the Linux CI toolchain (ubuntu-24.04, gcc-14, clang-18) and the
# C++ dependencies listed in docs/advanced_documentation/build-guide.md.
FROM mcr.microsoft.com/devcontainers/base:ubuntu-24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        gcc-14 g++-14 \
        clang-18 lld-18 clang-format-18 clang-tidy-18 \
        cmake ninja-build gdb \
        libboost-dev libeigen3-dev nlohmann-json3-dev libmsgpack-cxx-dev doctest-dev \
        gcovr lcov pkg-config git curl ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# The CMake presets resolve `gcc`/`g++`/`clang`/`clang++` via PATH
# (same approach as CI, which symlinks the versioned binaries).
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-14 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-14 100 \
    && update-alternatives --install /usr/bin/clang clang /usr/bin/clang-18 100 \
    && update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-18 100 \
    && update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-18 100 \
    && update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-18 100

ENV CC=gcc CXX=g++
ENV DEBIAN_FRONTEND=dialog
