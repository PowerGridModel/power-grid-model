# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

set(PROJECT_SOURCES
    "test_entry_point.cpp"
    "test_cpp_api_meta_data.cpp"
    "test_cpp_api_model.cpp"
    "test_cpp_api_serialization.cpp"
)

add_executable(power_grid_model_c_api_tests ${PROJECT_SOURCES})

target_link_libraries(power_grid_model_c_api_tests
    PRIVATE
        power_grid_model_c
        power_grid_model_cpp
        doctest::doctest
)

doctest_discover_tests(power_grid_model_c_api_tests)
