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

if [[ $3 == "Coverage" ]]; then
  BUILD_COVERAGE=-DPOWER_GRID_MODEL_COVERAGE=1
else
  BUILD_COVERAGE=
fi

BUILD_DIR=cpp_build_$1_$2
echo "Build dir: ${BUILD_DIR}"

if [[ ! -z "${VCPKG_ROOT}" ]]; then
  PATH_FOR_CMAKE=-DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake
elif [[ ! -z "${CMAKE_PREFIX_PATH}" ]]; then
  PATH_FOR_CMAKE=-DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
else 
  PATH_FOR_CMAKE=
fi

echo ${PATH_FOR_CMAKE}

rm -rf ${BUILD_DIR}/
mkdir ${BUILD_DIR}
cd ${BUILD_DIR}
# generate
cmake .. -GNinja \
    -DCMAKE_BUILD_TYPE=$1 \
    -DPOWER_GRID_MODEL_SPARSE_SOLVER=$2 \
    ${PATH_FOR_CMAKE} \
    -DPOWER_GRID_MODEL_BUILD_BENCHMARK=1 \
    ${BUILD_COVERAGE}
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
if [[ "$1" = "Debug" ]] && [[ $3 == "Coverage" ]];  then
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
