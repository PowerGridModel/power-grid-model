# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

set(PROJECT_SOURCES
    "test_entry_point.cpp"
    "test_api_meta_data.cpp"
    "test_api_buffer.cpp"
    "test_api_model.cpp"
    "test_api_model_update.cpp"
    "test_api_serialization.cpp"
    "test_api_utils.cpp"
)

add_executable(power_grid_model_api_tests ${PROJECT_SOURCES})

target_link_libraries(power_grid_model_api_tests
    PRIVATE
        power_grid_model_c
        power_grid_model_cpp
        doctest::doctest
)

# TODO: remove this when the API becomes stable
target_compile_definitions(power_grid_model_api_tests
    PRIVATE
    PGM_ENABLE_EXPERIMENTAL
)

doctest_discover_tests(power_grid_model_api_tests)
