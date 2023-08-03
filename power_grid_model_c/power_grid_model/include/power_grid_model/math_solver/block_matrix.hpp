// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_MATH_SOLVER_BLOCK_MATRIX_HPP
#define POWER_GRID_MODEL_MATH_SOLVER_BLOCK_MATRIX_HPP

// define block matrix entry for several calculations
// the name of block matrix getter can be overwritten by sub matrix

#include "../power_grid_model.hpp"
#include "../three_phase_tensor.hpp"

namespace power_grid_model {

// hide implementation in inside namespace
namespace math_model_impl {

template <scalar_value T, bool sym, bool is_tensor, int n_sub_block>
struct block_trait {
    static constexpr int n_row = sym ? n_sub_block : n_sub_block * 3;
    static constexpr int n_col = is_tensor ? (sym ? n_sub_block : n_sub_block * 3) : 1;
    using ArrayType = Eigen::Array<T, n_row, n_col, Eigen::ColMajor>;
};

template <class T, bool sym, bool is_tensor, int n_sub_block>
class Block : public block_trait<T, sym, is_tensor, n_sub_block>::ArrayType {
   public:
    using ArrayType = typename block_trait<T, sym, is_tensor, n_sub_block>::ArrayType;
    using ArrayType::operator();

    // default zero
    Block() : ArrayType{ArrayType::Zero()} {};
    // eigen expression
    template <typename OtherDerived>
    explicit Block(Eigen::ArrayBase<OtherDerived> const& other) : ArrayType{other} {
    }
    template <typename OtherDerived>
    Block& operator=(Eigen::ArrayBase<OtherDerived> const& other) {
        this->ArrayType::operator=(other);
        return *this;
    }

    template <int r>
    static auto get_asym_row_idx() {
        return Eigen::seqN(Eigen::fix<r * 3>, Eigen::fix<3>);
    }
    template <int c>
    static auto get_asym_col_idx() {
        if constexpr (is_tensor) {
            return Eigen::seqN(Eigen::fix<c * 3>, Eigen::fix<3>);
        }
        else {
            return Eigen::seqN(Eigen::fix<0>, Eigen::fix<1>);
        }
    }

    template <int r, int c>
    using GetterType =
        std::conditional_t<sym, T&, decltype(std::declval<ArrayType>()(get_asym_row_idx<r>(), get_asym_col_idx<c>()))>;

    template <int r, int c>
    GetterType<r, c> get_val() {
        if constexpr (sym) {
            return (*this)(r, c);
        }
        else {
            return (*this)(get_asym_row_idx<r>(), get_asym_col_idx<c>());
        }
    }
};

}  // namespace math_model_impl

}  // namespace power_grid_model

#endif