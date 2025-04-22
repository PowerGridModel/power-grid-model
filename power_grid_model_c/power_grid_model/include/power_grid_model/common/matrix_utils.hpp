// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "three_phase_tensor.hpp"

namespace power_grid_model {

inline DoubleComplex average_of_diagonal_of_matrix(const ComplexTensor<asymmetric_t>& matrix) {
    return (matrix(0, 0) + matrix(1, 1) + matrix(2, 2)) / 3.0;
}

inline DoubleComplex average_of_off_diagonal_of_matrix(const ComplexTensor<asymmetric_t>& matrix) {
    return (matrix(0, 1) + matrix(1, 2) + matrix(1, 0) + matrix(1, 2) + matrix(2, 0) + matrix(2, 1)) / 6.0;
}

} // namespace power_grid_model
