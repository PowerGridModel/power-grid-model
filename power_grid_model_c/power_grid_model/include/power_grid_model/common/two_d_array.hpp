// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"

// eigen properties
#include <Eigen/Dense>

#include <concepts>

namespace power_grid_model::two_d_array {
template <class T> using Eigen3Vector = Eigen::Array<T, 3, 1>;
template <class T> using Eigen3Tensor = Eigen::Array<T, 3, 3, Eigen::ColMajor>;
template <class T> using Eigen4Tensor = Eigen::Array<T, 4, 4, Eigen::ColMajor>;
template <class T> using Eigen3DiagonalTensor = Eigen::DiagonalMatrix<T, 3>;
} // namespace power_grid_model::two_d_array
