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
