// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#ifdef PGM_USE_CXX_MODULES
import power_grid_model.common.common;
import power_grid_model.common.matrix_utils;
import power_grid_model.common.three_phase_tensor;
#else
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/matrix_utils.hpp>
#include <power_grid_model/common/three_phase_tensor.hpp>
#endif

#include <doctest/doctest.h>

#include <complex>

namespace power_grid_model {
TEST_CASE("Matrix utils") {
    ComplexTensor<asymmetric_t> const matrix{DoubleComplex{1.0, 3.0}, DoubleComplex{2.0, -1.0}};

    SUBCASE("average_of_diagonal_of_matrix") {
        CHECK(average_of_diagonal_of_matrix(matrix) == DoubleComplex{1.0, 3.0});
    }
    SUBCASE("average_of_off_diagonal_of_matrix") {
        CHECK(average_of_off_diagonal_of_matrix(matrix) == DoubleComplex{2.0, -1.0});
    }
}
} // namespace power_grid_model
