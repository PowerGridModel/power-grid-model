# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0
cmake_minimum_required (VERSION 3.9)

project(power_grid_model_package_tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

find_package(PGM REQUIRED)

get_target_property(LIB_INCLUDES PGM::power_grid_model_c INTERFACE_INCLUDE_DIRECTORIES)
message(STATUS "here  ${LIB_INCLUDES}")

set(PROJECT_SOURCES 
	"test_c_api_package.cpp"
)

# Link against shared PGM object installed by main project
add_executable(power_grid_model_package_test ${PROJECT_SOURCES})
target_link_libraries(power_grid_model_package_test PRIVATE PGM::power_grid_model_c)

install(TARGETS power_grid_model_package_test)
install(IMPORTED_RUNTIME_ARTIFACTS PGM::power_grid_model_c)
