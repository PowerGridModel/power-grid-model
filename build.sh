#!/bin/bash

# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

set -e

usage() {
  echo "Usage: $0 -b <Debug|Release> [-c] [-s] [-e] [-i]" 1>&2
  echo "  -c option enables coverage"
  echo "  -s option enables sanitizer"
  echo "  -e option to run C API example"
  echo "  -i option to install package"
  exit 1
}

while getopts "b::csei" flag; do
  case "${flag}" in
    b)
      [ "${OPTARG}" == "Debug" -o "${OPTARG}" == "Release" ] || echo "Build type should be Debug or Release."
      BUILD_TYPE=${OPTARG}
    ;;
    c) BUILD_COVERAGE=-DPOWER_GRID_MODEL_COVERAGE=1;;
    s) BUILD_SANITIZER=-DPOWER_GRID_MODEL_SANITIZER=1;;
    e) C_API_EXAMPLE=1;;
    i) INSTALL=1;;
    *) usage ;;
  esac
done

if [ -z "${BUILD_TYPE}" ] ; then
  usage
fi

echo "BUILD_TYPE = ${BUILD_TYPE}"
echo "BUILD_COVERAGE = ${BUILD_COVERAGE}"
echo "BUILD_SANITIZER = ${BUILD_SANITIZER}"
echo "INSTALL = ${INSTALL}"

BUILD_DIR=cpp_build_script_${BUILD_TYPE}
echo "Build dir: ${BUILD_DIR}"

rm -rf ${BUILD_DIR}/
mkdir ${BUILD_DIR}
cd ${BUILD_DIR}
# generate
cmake .. -GNinja \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    ${BUILD_COVERAGE} \
    ${BUILD_SANITIZER}
# build
VERBOSE=1 cmake --build .
# test
ctest --test-dir . -E PGMExample
# example
if [[ "${C_API_EXAMPLE}" ]];  then
  ./bin/power_grid_model_c_example
fi

if [[ ${INSTALL} ]]; then
  echo
  cmake --build . --target install
fi

cd ..
# test coverage report for debug build and for linux
if [[ "${BUILD_TYPE}" = "Debug" ]] && [[ "${BUILD_COVERAGE}" ]];  then
  echo "Generating coverage report..."
  if [[ ${CXX} == "clang++"* ]]; then
    GCOV_TOOL="--gcov-tool llvm-gcov.sh"
  else
    GCOV_TOOL=
  fi

  PATH=${PATH}:${PWD} lcov -q -c \
    -d ${BUILD_DIR}/tests/cpp_unit_tests/CMakeFiles/power_grid_model_unit_tests.dir \
    -d ${BUILD_DIR}/tests/c_api_tests/CMakeFiles/power_grid_model_c_api_tests.dir \
    -d ${BUILD_DIR}/power_grid_model_c/power_grid_model_c/CMakeFiles/power_grid_model_c.dir \
    -b . \
    --no-external \
    --output-file cpp_coverage.info \
    ${GCOV_TOOL}
  genhtml -q cpp_coverage.info --output-directory cpp_cov_html
  rm cpp_coverage.info
fi
