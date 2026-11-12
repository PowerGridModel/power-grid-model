#!/usr/bin/env bash
# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

set -eu

pgm_build_prefixes="$(/opt/pgm-build-deps/bin/pgm-build-setup-local-prefix)"
if [ -z "$pgm_build_prefixes" ]; then
    echo "pgm-build-deps returned no CMake prefixes" >&2
    exit 1
fi

if [ -n "${CMAKE_PREFIX_PATH:-}" ]; then
    export CMAKE_PREFIX_PATH="${pgm_build_prefixes}:${CMAKE_PREFIX_PATH}"
else
    export CMAKE_PREFIX_PATH="$pgm_build_prefixes"
fi

exec /usr/bin/cmake "$@"
