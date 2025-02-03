// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

#pragma once

#include "three_phase_tensor.hpp"

namespace power_grid_model {

inline DoubleComplex average_of_diagonal_of_matrix(const ComplexTensor<asymmetric_t> &matrix) {
    return (matrix(0,0) + matrix(1,1) + matrix(2,2)) / 3.0;
}

inline DoubleComplex average_of_off_diagonal_of_matrix(const ComplexTensor<asymmetric_t> &matrix) {
    return (matrix(0,2) + matrix(1,1) + matrix(2,0)) / 3.0;
}

} // namespace power_grid_model
