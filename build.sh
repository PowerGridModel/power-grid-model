#!/bin/bash

# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

set -e

usage() {
  echo "$0 {Debug/Release}"
}

if [ ! "$1" = "Debug" ] && [ ! "$1" = "Release" ]; then
  echo "Missing first argument"
  usage
  exit 1;
fi

if [[ $2 == "Coverage" ]]; then
  BUILD_COVERAGE=-DPOWER_GRID_MODEL_COVERAGE=1
else
  BUILD_COVERAGE=
fi

BUILD_DIR=cpp_build_$1
echo "Build dir: ${BUILD_DIR}"

rm -rf ${BUILD_DIR}/
mkdir ${BUILD_DIR}
cd ${BUILD_DIR}
# generate
cmake .. -GNinja \
    -DCMAKE_BUILD_TYPE=$1 \
    ${PATH_FOR_CMAKE} \
    ${BUILD_COVERAGE}
# build
VERBOSE=1 cmake --build .
# test
./tests/cpp_unit_tests/power_grid_model_unit_tests



cd ..
# test coverage report for debug build and for linux
if [[ "$1" = "Debug" ]] && [[ $2 == "Coverage" ]];  then
  echo "Generating coverage report..."
  if [[ ${CXX} == "clang++"* ]]; then
    GCOV_TOOL="--gcov-tool llvm-gcov.sh"
  else
    GCOV_TOOL=
  fi

  PATH=${PATH}:${PWD} lcov -q -c -d ${BUILD_DIR}/tests/cpp_unit_tests/CMakeFiles/power_grid_model_unit_tests.dir -b include --no-external --output-file cpp_coverage.info ${GCOV_TOOL}
  genhtml -q cpp_coverage.info --output-directory cpp_cov_html
  rm cpp_coverage.info
fi
