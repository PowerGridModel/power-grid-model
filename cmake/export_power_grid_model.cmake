# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

cmake_minimum_required (VERSION 3.23)

# export the license and readme
configure_file("${PROJECT_SOURCE_DIR}/cmake/README.md.in" "${CMAKE_CURRENT_BINARY_DIR}/README.md")
install(FILES "LICENSE" "${CMAKE_CURRENT_BINARY_DIR}/README.md"
  DESTINATION "share"
)

# export the power grid model targets
install(EXPORT "power_grid_modelTargets"
  DESTINATION "lib/cmake/power_grid_model"
  NAMESPACE power_grid_model::
  COMPONENT power_grid_model
)

# export configuration and version to enable find_package(power_grid_model)
include(CMakePackageConfigHelpers)

configure_package_config_file("${PROJECT_SOURCE_DIR}/cmake/power_grid_modelConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/power_grid_model/power_grid_modelConfig.cmake"
  INSTALL_DESTINATION "lib/cmake/power_grid_model"
)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/power_grid_model/power_grid_modelConfigVersion.cmake"
  VERSION ${PGM_VERSION}
  COMPATIBILITY SameMajorVersion
)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/power_grid_model/power_grid_modelConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/power_grid_model/power_grid_modelConfigVersion.cmake"
  DESTINATION "lib/cmake/power_grid_model"
)