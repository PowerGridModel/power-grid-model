// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#ifdef PGM_USE_CXX_MODULES
import power_grid_model.common;
#else
#include <power_grid_model/common/calculation_info.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/component_list.hpp>
#include <power_grid_model/common/composite_logging.hpp>
#include <power_grid_model/common/counting_iterator.hpp>
#include <power_grid_model/common/dummy_logging.hpp>
#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/common/grouped_index_vector.hpp>
#include <power_grid_model/common/iterator_facade.hpp>
#include <power_grid_model/common/logging.hpp>
#include <power_grid_model/common/matrix_utils.hpp>
#include <power_grid_model/common/maybe_owning_view.hpp>
#include <power_grid_model/common/multi_threaded_logging.hpp>
#include <power_grid_model/common/small_vector.hpp>
#include <power_grid_model/common/statistics.hpp>
#include <power_grid_model/common/text_logger.hpp>
#include <power_grid_model/common/three_phase_tensor.hpp>
#include <power_grid_model/common/timer.hpp>
#include <power_grid_model/common/typing.hpp>
#endif

#include <doctest/doctest.h>

#include <complex>
#include <concepts>
#include <cstdint>
#include <type_traits>

namespace power_grid_model {
namespace {
// a sample of names from each of the sub-modules re-exported by power_grid_model.common
static_assert(std::is_class_v<CalculationInfo>);
static_assert(std::is_class_v<IdxRange>);
static_assert(std::is_class_v<ComponentList<>>);
static_assert(std::is_class_v<common::logging::MultiThreadedCompositeLogger>);
static_assert(std::is_class_v<common::logging::NoLogger>);
static_assert(std::is_enum_v<ComponentType>);
static_assert(std::derived_from<InvalidArguments, PowerGridError>);
static_assert(std::is_class_v<DenseGroupedIdxVector>);
static_assert(std::is_class_v<IteratorFacade>);
static_assert(std::is_enum_v<LogEvent>);
static_assert(std::is_class_v<MaybeOwningVector<Idx>>);
static_assert(std::is_class_v<common::logging::NoMultiThreadedLogger>);
static_assert(std::is_class_v<SmallVector<Idx, 2>>);
static_assert(std::is_class_v<UniformRealRandVar<symmetric_t>>);
static_assert(std::is_class_v<TextLogger>);
static_assert(std::is_class_v<RealValue<asymmetric_t>>);
static_assert(std::is_class_v<Timer>);
static_assert(std::same_as<decltype(narrow_cast<int32_t>(Idx{})), int32_t>);
} // namespace

TEST_CASE("Umbrella module") {
    ComplexTensor<asymmetric_t> const matrix{DoubleComplex{1.0, 0.0}};
    CHECK(average_of_diagonal_of_matrix(matrix) == DoubleComplex{1.0, 0.0});
}
} // namespace power_grid_model
