# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

project (power_grid_model_tests)

find_package(doctest REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

include("${doctest_DIR}/doctest.cmake")

add_subdirectory("c_api_tests")
add_subdirectory("cpp_unit_tests")
add_subdirectory("cpp_validation_tests")
add_subdirectory("benchmark_cpp")
