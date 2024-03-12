// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

// define block matrix entry for several calculations
// the name of block matrix getter can be overwritten by sub matrix

#include "../common/common.hpp"
#include "../common/three_phase_tensor.hpp"

namespace power_grid_model::math_solver {

template <scalar_value T, symmetry_tag sym, bool is_tensor, int n_sub_block> struct block_trait {
    static constexpr int sub_block_size = is_symmetric_v<sym> ? 1 : 3;
    static constexpr int n_row = n_sub_block * sub_block_size;
    static constexpr int n_col = is_tensor ? n_sub_block * sub_block_size : 1;

    using ArrayType = Eigen::Array<T, n_row, n_col, Eigen::ColMajor>;
};

template <class T, symmetry_tag sym, bool is_tensor, int n_sub_block>
class Block : public block_trait<T, sym, is_tensor, n_sub_block>::ArrayType {
  public:
    using block_traits = block_trait<T, sym, is_tensor, n_sub_block>;
    using ArrayType = typename block_traits::ArrayType;
    using ArrayType::operator();

    // default zero
    Block() : ArrayType{ArrayType::Zero()} {};
    // eigen expression
    template <typename OtherDerived> explicit Block(Eigen::ArrayBase<OtherDerived> const& other) : ArrayType{other} {}
    template <typename OtherDerived> Block& operator=(Eigen::ArrayBase<OtherDerived> const& other) {
        this->ArrayType::operator=(other);
        return *this;
    }

    template <int start, int size, int n_total> static auto get_sequence() {
        static_assert(start >= 0);
        static_assert(size >= 0);
        static_assert(start + size <= n_total);
        return Eigen::seqN(Eigen::fix<start>, Eigen::fix<size>);
    }
    template <int block_start, int block_size, int n_total> static auto get_block_sequence() {
        constexpr auto size = block_size * block_traits::sub_block_size;
        constexpr auto start = block_start * size;
        return Block::get_sequence<start, size, n_total>();
    }
    template <int r, int r_size> static auto get_block_row_idx() {
        return Block::get_block_sequence<r, r_size, block_traits::n_row>();
    }
    template <int c, int c_size> static auto get_block_col_idx() {
        if constexpr (is_tensor) {
            return Block::get_block_sequence<c, c_size, block_traits::n_col>();
        } else {
            return Block::get_sequence<c, c_size, block_traits::n_col>();
        }
    }
    template <int r> static auto get_row_idx() { return Block::get_block_row_idx<r, 1>(); }
    template <int c> static auto get_col_idx() { return Block::get_block_col_idx<c, 1>(); }

    template <int r, int c>
    using GetterType = std::conditional_t<is_symmetric_v<sym>, T&,
                                          decltype(std::declval<ArrayType>()(get_row_idx<r>(), get_col_idx<c>()))>;

    template <int r, int c> GetterType<r, c> get_val() {
        if constexpr (is_symmetric_v<sym>) {
            return (*this)(r, c);
        } else {
            return (*this)(get_row_idx<r>(), get_col_idx<c>());
        }
    }

    template <int r, int c, int r_size, int c_size>
    using BlockGetterType = std::conditional_t<r_size == 1 && c_size == 1, GetterType<r, c>,
                                               decltype(std::declval<ArrayType>()(get_block_row_idx<r, r_size>(),
                                                                                  get_block_col_idx<c, c_size>()))>;

    template <int r, int c, int r_size, int c_size> BlockGetterType<r, c, r_size, c_size> get_block_val() {
        if constexpr (r_size == 1 && c_size == 1) {
            return get_val<r, c>();
        } else {
            return (*this)(get_block_row_idx<r, r_size>(), get_block_col_idx<c, c_size>());
        }
    }

    void clear() { *this = {}; }
};

} // namespace power_grid_model::math_solver
