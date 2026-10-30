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

target_include_directories(
    power_grid_model
    INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# only use MPL version of eigen
target_compile_definitions(power_grid_model INTERFACE "EIGEN_MPL2_ONLY=1")

# C++20 named modules wrapping the headers (not installed nor exported)
if(PGM_ENABLE_CXX_MODULES)
    add_library(power_grid_model_cxx_modules STATIC)
    target_sources(
        power_grid_model_cxx_modules
        PUBLIC
            FILE_SET CXX_MODULES
                BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/modules"
                FILES
                    "modules/auxiliary.cppm"
                    "modules/auxiliary/input.cppm"
                    "modules/auxiliary/meta_data.cppm"
                    "modules/auxiliary/meta_data_gen.cppm"
                    "modules/auxiliary/dataset_fwd.cppm"
                    "modules/auxiliary/dataset.cppm"
                    "modules/auxiliary/output.cppm"
                    "modules/auxiliary/update.cppm"
                    "modules/auxiliary/serialization/deserializer.cppm"
                    "modules/auxiliary/serialization/serializer.cppm"
                    "modules/common.cppm"
                    "modules/common/calculation_info.cppm"
                    "modules/common/common.cppm"
                    "modules/common/component_list.cppm"
                    "modules/common/composite_logging.cppm"
                    "modules/common/counting_iterator.cppm"
                    "modules/common/dummy_logging.cppm"
                    "modules/common/enums.cppm"
                    "modules/common/exception.cppm"
                    "modules/common/grouped_index_vector.cppm"
                    "modules/common/iterator_facade.cppm"
                    "modules/common/logging.cppm"
                    "modules/common/matrix_utils.cppm"
                    "modules/common/maybe_owning_view.cppm"
                    "modules/common/multi_threaded_logging.cppm"
                    "modules/common/small_vector.cppm"
                    "modules/common/statistics.cppm"
                    "modules/common/text_logger.cppm"
                    "modules/common/three_phase_tensor.cppm"
                    "modules/common/timer.cppm"
                    "modules/common/typing.cppm"
                    "modules/component.cppm"
                    "modules/component/appliance.cppm"
                    "modules/component/asym_line.cppm"
                    "modules/component/base.cppm"
                    "modules/component/branch.cppm"
                    "modules/component/branch3.cppm"
                    "modules/component/component.cppm"
                    "modules/component/current_sensor.cppm"
                    "modules/component/edge.cppm"
                    "modules/component/fault.cppm"
                    "modules/component/generic_branch.cppm"
                    "modules/component/line.cppm"
                    "modules/component/link.cppm"
                    "modules/component/load_gen.cppm"
                    "modules/component/node.cppm"
                    "modules/component/power_sensor.cppm"
                    "modules/component/regulator.cppm"
                    "modules/component/sensor.cppm"
                    "modules/component/shunt.cppm"
                    "modules/component/source.cppm"
                    "modules/component/three_winding_transformer.cppm"
                    "modules/component/transformer_tap_regulator.cppm"
                    "modules/component/transformer_utils.cppm"
                    "modules/component/transformer.cppm"
                    "modules/component/voltage_regulator.cppm"
                    "modules/component/voltage_sensor.cppm"
                    "modules/main_core.cppm"
                    "modules/main_core/calculation_input_preparation.cppm"
                    "modules/main_core/container_queries.cppm"
                    "modules/main_core/core_utils.cppm"
                    "modules/main_core/input.cppm"
                    "modules/main_core/main_model_type.cppm"
                    "modules/main_core/math_output_queries.cppm"
                    "modules/main_core/math_state.cppm"
                    "modules/main_core/output.cppm"
                    "modules/main_core/state_queries.cppm"
                    "modules/main_core/state.cppm"
                    "modules/main_core/topological_node_output.cppm"
                    "modules/main_core/topology.cppm"
                    "modules/main_core/update.cppm"
                    "modules/main_core/y_bus.cppm"
                    "modules/math_solver.cppm"
                    "modules/math_solver/block_matrix.cppm"
                    "modules/math_solver/iterative_current_pf_solver.cppm"
                    "modules/math_solver/iterative_linear_se_solver.cppm"
                    "modules/math_solver/iterative_pf_solver.cppm"
                    "modules/math_solver/linear_pf_solver.cppm"
                    "modules/math_solver/math_solver_dispatch.cppm"
                    "modules/math_solver/math_solver.cppm"
                    "modules/math_solver/measured_values.cppm"
                    "modules/math_solver/newton_raphson_pf_solver.cppm"
                    "modules/math_solver/newton_raphson_se_solver.cppm"
                    "modules/math_solver/observability.cppm"
                    "modules/math_solver/short_circuit_solver.cppm"
                    "modules/math_solver/sparse_lu_solver.cppm"
                    "modules/math_solver/y_bus.cppm"
                    "modules/optimizer.cppm"
                    "modules/optimizer/base_optimizer.cppm"
                    "modules/optimizer/optimizer.cppm"
                    "modules/optimizer/tap_position_optimizer.cppm"
                    "modules/all_components.cppm"
                    "modules/batch_parameters.cppm"
                    "modules/calculation_parameters.cppm"
                    "modules/calculation_preparation.cppm"
                    "modules/calculation.cppm"
                    "modules/container_fwd.cppm"
                    "modules/container.cppm"
                    "modules/index_mapping.cppm"
                    "modules/job_adapter.cppm"
                    "modules/job_dispatch.cppm"
                    "modules/job_interface.cppm"
                    "modules/link_solver.cppm"
                    "modules/main_model_fwd.cppm"
                    "modules/main_model_impl.cppm"
                    "modules/main_model.cppm"
                    "modules/sparse_ordering.cppm"
                    "modules/supernodes.cppm"
                    "modules/topology.cppm"
                    "modules/power_grid_model.cppm"
    )
    target_link_libraries(power_grid_model_cxx_modules PUBLIC power_grid_model)
    target_compile_definitions(
        power_grid_model_cxx_modules
        INTERFACE PGM_USE_CXX_MODULES
    )
endif()

target_compile_options(
    power_grid_model
    BEFORE
    INTERFACE
        "$<$<CXX_COMPILER_ID:Clang>:-Wno-unknown-attributes>"
        "$<$<CXX_COMPILER_ID:MSVC>:/bigobj>"
)
