# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

add_executable(
    power_grid_model_unit_tests_auxiliary
    "../test_entry_point.cpp"
    "test_deserializer.cpp"
    "test_serializer.cpp"
    "test_dataset.cpp"
    "test_meta_data_generation.cpp"
)

target_link_libraries(
    power_grid_model_unit_tests_auxiliary
    PRIVATE power_grid_model doctest::doctest
)

if(PGM_ENABLE_CXX_MODULES)
    target_link_libraries(
        power_grid_model_unit_tests_auxiliary
        PRIVATE power_grid_model_cxx_modules
    )
    set_target_properties(
        power_grid_model_unit_tests_auxiliary
        PROPERTIES CXX_SCAN_FOR_MODULES ON
    )
endif()

doctest_discover_tests(power_grid_model_unit_tests_auxiliary)
