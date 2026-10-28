# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

add_executable(
    power_grid_model_unit_tests_common
    "../test_entry_point.cpp"
    "test_common.cpp"
    "test_exceptions.cpp"
    "test_component_list.cpp"
    "test_grouped_index_vector.cpp"
    "test_counting_iterator.cpp"
    "test_statistics.cpp"
    "test_three_phase_tensor.cpp"
    "test_typing.cpp"
    "test_iterator_facade.cpp"
    "test_maybe_owning_view.cpp"
    "test_small_vector.cpp"
    "test_matrix_utils.cpp"
    "test_logging.cpp"
    "test_dummy_logging.cpp"
    "test_multi_threaded_logging.cpp"
    "test_composite_logging.cpp"
    "test_text_logger.cpp"
    "test_calculation_info.cpp"
    "test_timer.cpp"
    "test_common_module.cpp"
)

target_link_libraries(
    power_grid_model_unit_tests_common
    PRIVATE power_grid_model doctest::doctest
)

if(PGM_ENABLE_CXX_MODULES)
    target_link_libraries(
        power_grid_model_unit_tests_common
        PRIVATE power_grid_model_cxx_modules
    )
    set_target_properties(
        power_grid_model_unit_tests_common
        PROPERTIES CXX_SCAN_FOR_MODULES ON
    )
endif()

doctest_discover_tests(power_grid_model_unit_tests_common)
