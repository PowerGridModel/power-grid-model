# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

find_package(doctest REQUIRED)

include("${doctest_DIR}/doctest.cmake")

add_subdirectory("native_api_tests")
add_subdirectory("cpp_unit_tests")
add_subdirectory("cpp_validation_tests")
add_subdirectory("benchmark_cpp")
