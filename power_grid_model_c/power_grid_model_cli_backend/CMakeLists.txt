# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

add_library(
    power_grid_model_cli_backend
    SHARED
    "src/cli_main.cpp"
    "src/cli_options.cpp"
    "src/pgm_calculation.cpp"
)

target_include_directories(
    power_grid_model_cli_backend
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

file(
    GLOB_RECURSE pgm_cli_backend_public_headers
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

target_link_libraries(power_grid_model_cli_backend PRIVATE power_grid_model_c)
target_link_libraries(power_grid_model_cli_backend PRIVATE power_grid_model_cpp)
target_link_libraries(power_grid_model_cli_backend PRIVATE CLI11::CLI11)

target_compile_definitions(
    power_grid_model_cli_backend
    PRIVATE
        PGM_CLI_BACKEND_EXPORTS
        PGM_ENABLE_EXPERIMENTAL
)

target_sources(
    power_grid_model_cli_backend
    PUBLIC
        FILE_SET pgm_cli_backend_public_headers
            TYPE HEADERS
            BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include/"
            FILES "${pgm_cli_backend_public_headers}"
)

set_target_properties(
    power_grid_model_cli_backend
    PROPERTIES VERSION ${PGM_VERSION} SOVERSION ${PGM_VERSION_MAJOR}
)

set_property(TARGET power_grid_model_cli_backend PROPERTY INSTALL_RPATH_USE_LINK_PATH FALSE)
if(APPLE)
    set_property(TARGET power_grid_model_cli_backend PROPERTY INSTALL_RPATH "@loader_path")
elseif(UNIX) # Linux, BSD (not Windows/macOS)
    set_property(TARGET power_grid_model_cli_backend PROPERTY INSTALL_RPATH "$ORIGIN")
endif()

if(NOT PGM_MSVC_REPRODUCIBLE_BUILD)
    set_target_properties(
        power_grid_model_cli_backend
        PROPERTIES
            INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
            INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE
    )
endif()

install(
    TARGETS power_grid_model_cli_backend
    EXPORT power_grid_modelTargets
    COMPONENT power_grid_model
    FILE_SET pgm_cli_backend_public_headers
)