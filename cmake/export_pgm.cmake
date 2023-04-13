# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

cmake_minimum_required (VERSION 3.9)

# export the license
install(FILES "LICENSE"
  DESTINATION "share"
)

# export the power grid model targets
install(EXPORT "PGMTargets"
  DESTINATION "lib/cmake/PGM"
  NAMESPACE PGM::
  COMPONENT PGM
)

# export configuration and version to enable find_package(PGM)
include(CMakePackageConfigHelpers)

configure_package_config_file("${CMAKE_SOURCE_DIR}/cmake/PGMConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/PGM/PGMConfig.cmake"
  INSTALL_DESTINATION "lib/cmake/PGM"
)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/PGM/PGMConfigVersion.cmake"
  VERSION ${PGM_VERSION}
  COMPATIBILITY SameMajorVersion
)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/PGM/PGMConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/PGM/PGMConfigVersion.cmake"
  DESTINATION "lib/cmake/PGM"
)