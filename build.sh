#!/bin/bash

# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

set -e

usage() {
  echo "Usage: $0 -p <preset> [-c] [-e] [-i] [-t]" 1>&2
  echo "  -c option generates coverage if available"
  echo "  -e option to run C API example"
  echo "  -i option to install package"
  echo "  -t option to run integration test (requires '-i')"
  cmake --list-presets
  exit 1
}

while getopts "p::ceit" flag; do
  case "${flag}" in
    p)
      PRESET=${OPTARG}
    ;;
    c) COVERAGE=1;;
    e) C_API_EXAMPLE=1;;
    i) INSTALL=1;;
    t) INTEGRATION_TEST=1;;
    *) usage ;;
  esac
done

if [ -z "${PRESET}" ] ; then
  usage
fi

echo "PRESET = ${PRESET}"
echo "INSTALL = ${INSTALL}"

BUILD_DIR=cpp_build/${PRESET}
INSTALL_DIR=install/${PRESET}
echo "Build dir: ${BUILD_DIR}"
echo "Install dir: ${INSTALL_DIR}"

rm -rf ${BUILD_DIR}/

# generate
cmake --preset ${PRESET}

# build
cmake --build --preset ${PRESET} --verbose -j1

# test
ctest --preset ${PRESET} -E PGMExample --output-on-failure

# example
if [[ "${C_API_EXAMPLE}" ]];  then
  ctest --preset ${PRESET} -R PGMExample --output-on-failure
fi

# test coverage report for debug build and for linux
if [[ "${COVERAGE}" ]];  then
  echo "Generating coverage report..."
  if [[ ${CXX} == "clang++"* ]]; then
    GCOV_TOOL="--gcov-tool llvm-gcov.sh"
  else
    GCOV_TOOL=
  fi

  PATH=${PATH}:${PWD} lcov -q -c \
    -d ${BUILD_DIR}/tests/cpp_unit_tests/CMakeFiles/power_grid_model_unit_tests.dir \
    -d ${BUILD_DIR}/tests/cpp_validation_tests/CMakeFiles/power_grid_model_validation_tests.dir \
    -d ${BUILD_DIR}/tests/native_api_tests/CMakeFiles/power_grid_model_api_tests.dir \
    -d ${BUILD_DIR}/power_grid_model_c/power_grid_model_c/CMakeFiles/power_grid_model_c.dir \
    -b . \
    --no-external \
    --output-file cpp_coverage.info \
    ${GCOV_TOOL}
  genhtml -q cpp_coverage.info --output-directory cpp_cov_html
  rm cpp_coverage.info
fi

# install
if [[ ${INSTALL} ]]; then
  cmake --build --preset ${PRESET} --target install
  
  # end-to-end test
  if [[ ${INTEGRATION_TEST} ]]; then
    cd tests/package_tests
    cmake --preset ${PRESET}
    cmake --build --preset ${PRESET} --verbose -j1
    cmake --build --preset ${PRESET} --verbose -j1 --target install
    install/${PRESET}/bin/power_grid_model_package_test
    cd ../..
  fi
fi
