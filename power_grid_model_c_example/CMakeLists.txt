# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

add_executable(power_grid_model_c_example main.c)

target_link_libraries(power_grid_model_c_example
	PRIVATE power_grid_model_c
)

add_test(PGMExample ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/power_grid_model_c_example)
