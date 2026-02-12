# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

set(PROJECT_SOURCES benchmark.cpp)

add_executable(
    power_grid_model_benchmark_cpp
    EXCLUDE_FROM_ALL
    ${PROJECT_SOURCES}
)
target_link_libraries(power_grid_model_benchmark_cpp PRIVATE power_grid_model)
target_compile_definitions(
    power_grid_model_benchmark_cpp
    PRIVATE "POWER_GRID_MODEL_CPP_BENCHMARK=1"
)
# set_target_properties(power_grid_model_benchmark_cpp PROPERTIES LINK_WHAT_YOU_USE TRUE)

# Observability benchmark
set(OBSERVABILITY_BENCHMARK_SOURCES observability_benchmark.cpp)

add_executable(
    power_grid_model_observability_benchmark
    EXCLUDE_FROM_ALL
    ${OBSERVABILITY_BENCHMARK_SOURCES}
)
target_link_libraries(power_grid_model_observability_benchmark PRIVATE power_grid_model nlohmann_json::nlohmann_json)
target_compile_definitions(
    power_grid_model_observability_benchmark
    PRIVATE "POWER_GRID_MODEL_OBSERVABILITY_BENCHMARK=1"
)

# Install rule for observability benchmark
install(
    TARGETS power_grid_model_observability_benchmark
    RUNTIME DESTINATION bin
    COMPONENT benchmark
)

# Convenience target to build all benchmarks
add_custom_target(
    benchmarks
    DEPENDS power_grid_model_benchmark_cpp power_grid_model_observability_benchmark
)
