#!/bin/bash

# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

set -e

usage() {
  echo "Usage: $0 -p <preset> [-c] [-e]" 1>&2
  echo "  -c option generates coverage if available"
  echo "  -e option to run C API example"
  cmake --list-presets
  exit 1
}

while getopts "p::ce" flag; do
  case "${flag}" in
    p)
      PRESET=${OPTARG}
    ;;
    c) COVERAGE=1;;
    e) C_API_EXAMPLE=1;;
    *) usage ;;
  esac
done

if [ -z "${PRESET}" ] ; then
  usage
fi

echo "PRESET = ${PRESET}"

BUILD_DIR=cpp_build/${PRESET}
echo "Build dir: ${BUILD_DIR}"

rm -rf ${BUILD_DIR}/
# generate
cmake --preset ${PRESET}
# build
cmake --build --preset ${PRESET} --verbose
# test
${BUILD_DIR}/bin/power_grid_model_unit_tests
# example
if [[ "${C_API_EXAMPLE}" ]];  then
  ${BUILD_DIR}/bin/power_grid_model_c_example
fi

# test coverage report for debug build and for linux
if [[ "${COVERAGE}" ]];  then
  echo "Generating coverage report..."
  if [[ ${CXX} == "clang++"* ]]; then
    GCOV_TOOL="--gcov-tool llvm-gcov.sh"
  else
    GCOV_TOOL=
  fi

  PATH=${PATH}:${PWD} lcov -q -c -d ${BUILD_DIR}/tests/cpp_unit_tests/CMakeFiles/power_grid_model_unit_tests.dir -d ${BUILD_DIR}/power_grid_model_c/CMakeFiles//power_grid_model_c.dir -b . --no-external --output-file cpp_coverage.info ${GCOV_TOOL}
  genhtml -q cpp_coverage.info --output-directory cpp_cov_html
  rm cpp_coverage.info
fi
