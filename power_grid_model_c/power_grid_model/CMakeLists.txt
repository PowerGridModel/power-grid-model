# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

add_library(power_grid_model INTERFACE)

target_link_libraries(power_grid_model
	INTERFACE Eigen3::Eigen Boost::headers
)

if(Threads_FOUND)
  target_link_libraries(power_grid_model INTERFACE Threads::Threads)
endif()

target_include_directories(power_grid_model INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

# only use MPL version of eigen
target_compile_definitions(power_grid_model INTERFACE "EIGEN_MPL2_ONLY=1")
