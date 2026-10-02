# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

add_library(power_grid_model INTERFACE)

target_link_libraries(
    power_grid_model
    INTERFACE
        Eigen3::Eigen
        Boost::headers
        msgpack-cxx
        nlohmann_json::nlohmann_json
)

if(Threads_FOUND)
    target_link_libraries(power_grid_model INTERFACE Threads::Threads)
endif()

file(
    GLOB_RECURSE pgm_public_headers
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

target_sources(
    power_grid_model
    INTERFACE
        FILE_SET pgm_public_headers
            TYPE HEADERS
            BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include/"
            FILES "${pgm_public_headers}"
)

# only use MPL version of eigen
target_compile_definitions(power_grid_model INTERFACE "EIGEN_MPL2_ONLY=1")

target_compile_options(
    power_grid_model
    BEFORE
    INTERFACE
        "$<$<CXX_COMPILER_ID:Clang>:-Wno-unknown-attributes>"
        "$<$<CXX_COMPILER_ID:MSVC>:/bigobj>"
)

set_target_properties(
    power_grid_model
    PROPERTIES VERIFY_INTERFACE_HEADER_SETS TRUE
)
