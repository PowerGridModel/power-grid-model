// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "../common/three_phase_tensor.hpp"

namespace power_grid_model {

inline ComplexTensor<asymmetric_t> kron_reduction(const ComplexTensor4& matrix_to_reduce) {
    ComplexTensor4 Y = matrix_to_reduce;
    ComplexTensor<asymmetric_t> Y_aa =
        ComplexTensor<asymmetric_t>(Y(0, 0), Y(1, 1), Y(2, 2), Y(1, 0), Y(2, 0), Y(2, 1));
    ComplexValue<asymmetric_t> Y_ab(Y(0, 3), Y(1, 3), Y(2, 3));
    ComplexValue<asymmetric_t> Y_ba(Y(3, 0), Y(3, 1), Y(3, 2));
    DoubleComplex Y_bb_inv = 1.0 / Y(3, 3);
    return Y_aa - vector_outer_product(Y_ba, Y_ab) * Y_bb_inv;
}

} // namespace power_grid_model
