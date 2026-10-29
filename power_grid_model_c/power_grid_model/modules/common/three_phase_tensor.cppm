// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

module;

#include <power_grid_model/common/three_phase_tensor.hpp>

export module power_grid_model.common.three_phase_tensor;

export namespace power_grid_model {
using power_grid_model::scalar_value;

namespace three_phase_tensor {
using power_grid_model::three_phase_tensor::DiagonalTensor;
using power_grid_model::three_phase_tensor::EigenDiagonalTensor3;
using power_grid_model::three_phase_tensor::EigenTensor3;
using power_grid_model::three_phase_tensor::EigenTensor4;
using power_grid_model::three_phase_tensor::EigenVector3;
using power_grid_model::three_phase_tensor::Tensor;
using power_grid_model::three_phase_tensor::Tensor4;
using power_grid_model::three_phase_tensor::Vector;
} // namespace three_phase_tensor

// three phase vector and tensor types
using power_grid_model::ComplexDiagonalTensor;
using power_grid_model::ComplexTensor;
using power_grid_model::ComplexTensor4;
using power_grid_model::ComplexTensorVector;
using power_grid_model::ComplexValue;
using power_grid_model::ComplexValueVector;
using power_grid_model::RealDiagonalTensor;
using power_grid_model::RealTensor;
using power_grid_model::RealTensorVector;
using power_grid_model::RealValue;
using power_grid_model::RealValueVector;

// concepts
using power_grid_model::column_vector;
using power_grid_model::column_vector_or_tensor;
using power_grid_model::rk2_tensor;

// basic operations (re-exported from std and Eigen)
using power_grid_model::arg;
using power_grid_model::conj;
using power_grid_model::exp;
using power_grid_model::imag;
using power_grid_model::real;

// functions
using power_grid_model::abs2;
using power_grid_model::add_diag;
using power_grid_model::all_zero;
using power_grid_model::any_zero;
using power_grid_model::as_diag;
using power_grid_model::cabs;
using power_grid_model::diag_mult;
using power_grid_model::dot;
using power_grid_model::get_sym_matrix;
using power_grid_model::get_sym_matrix_inv;
using power_grid_model::hermitian_transpose;
using power_grid_model::inv;
using power_grid_model::inv_sym_param;
using power_grid_model::is_inf;
using power_grid_model::is_nan;
using power_grid_model::is_normal;
using power_grid_model::max_val;
using power_grid_model::mean_val;
using power_grid_model::phase_mod_2pi;
using power_grid_model::phase_shift;
using power_grid_model::piecewise_complex_value;
using power_grid_model::pos_seq;
using power_grid_model::process_mean_val;
using power_grid_model::set_if_not_nan;
using power_grid_model::sum_row;
using power_grid_model::sum_val;
using power_grid_model::update_bool;
using power_grid_model::update_real_value;
using power_grid_model::vector_outer_product;
using power_grid_model::zero_tensor;
} // namespace power_grid_model

// stream output of Eigen types (e.g., for doctest)
export namespace Eigen {
using Eigen::operator<<;
using Eigen::abs2;
using Eigen::cos;
using Eigen::sin;
using Eigen::sqrt;
} // namespace Eigen
