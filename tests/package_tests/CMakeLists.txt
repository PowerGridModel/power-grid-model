# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

cmake_minimum_required (VERSION 3.9)

project(power_grid_model_package_tests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

find_package(power_grid_model REQUIRED CONFIG)

set(PROJECT_SOURCES 
	"test_c_api_package.cpp"
)

# Link against shared power_grid_model object installed by main project
add_executable(power_grid_model_package_test ${PROJECT_SOURCES})
target_link_libraries(power_grid_model_package_test PRIVATE power_grid_model::power_grid_model_c)
set_target_properties(power_grid_model_package_test PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

install(TARGETS power_grid_model_package_test)
install(IMPORTED_RUNTIME_ARTIFACTS power_grid_model::power_grid_model_c)
