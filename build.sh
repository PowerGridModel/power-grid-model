#!/bin/bash

# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

set -e

usage() {
  echo "$0 {Debug/Release} {EIGEN,MKL,MKL_RUNTIME}"
}

if [ ! "$1" = "Debug" ] && [ ! "$1" = "Release" ]; then
  echo "Missing first argument"
  usage
  exit 1;
fi

if [ ! "$2" = "EIGEN" ] && [ ! "$2" = "MKL" ] && [ ! "$2" = "MKL_RUNTIME" ]; then
  echo "Missing second argument"
  usage
  exit 1;
fi

BUILD_DIR=cpp_build_$1_$2
echo "Build dir: ${BUILD_DIR}"

rm -rf ${BUILD_DIR}/
mkdir ${BUILD_DIR}
cd ${BUILD_DIR}
# generate
cmake .. -GNinja \
    -DCMAKE_BUILD_TYPE=$1 \
    -DPOWER_GRID_MODEL_SPARSE_SOLVER=$2 \
    -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake \
    -DPOWER_GRID_MODEL_BUILD_BENCHMARK=1
# build
VERBOSE=1 cmake --build .
# test

if [[ $2 == "MKL_RUNTIME" ]]; then
  LD_LIBRARY_PATH= ./tests/cpp_unit_tests/power_grid_model_unit_tests
  LD_LIBRARY_PATH= POWER_GRID_MODEL_SPARSE_SOLVER=MKL ./tests/cpp_unit_tests/power_grid_model_unit_tests
  LD_LIBRARY_PATH=${MKL_LIB} ./tests/cpp_unit_tests/power_grid_model_unit_tests
  LD_LIBRARY_PATH=${MKL_LIB} POWER_GRID_MODEL_SPARSE_SOLVER=MKL ./tests/cpp_unit_tests/power_grid_model_unit_tests
  POWER_GRID_MODEL_SPARSE_SOLVER=EIGEN ./tests/cpp_unit_tests/power_grid_model_unit_tests
else
  LD_LIBRARY_PATH=${MKL_LIB} ./tests/cpp_unit_tests/power_grid_model_unit_tests
fi


cd ..
# test coverage report for debug build and for linux
if [[ "$1" = "Debug" ]] && [[ ${CXX} == "g++"* ]] && [[ $3 == "Coverage" ]];  then
  echo "Generating coverage report..."
  lcov -q -c -d ${BUILD_DIR}/tests/cpp_unit_tests/CMakeFiles/power_grid_model_unit_tests.dir -b include --no-external --output-file cpp_coverage.info
  genhtml -q cpp_coverage.info --output-directory cpp_cov_html
  rm cpp_coverage.info
fi
