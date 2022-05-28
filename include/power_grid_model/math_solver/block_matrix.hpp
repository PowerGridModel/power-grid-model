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

template <class T, bool sym, bool is_tensor, int n_sub_block, class = std::enable_if_t<check_scalar_v<T>>>
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
    Block(Eigen::ArrayBase<OtherDerived> const& other) : ArrayType{other} {
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
            return Eigen::fix<0>;
        }
    }

    template <int r, int c>
    using GetterType = std::conditional_t<sym, T&, decltype(Block{}(get_asym_row_idx<r>(), get_asym_col_idx<c>()))>;

    template <int r, int c>
    GetterType get_val() {
        if constexpr (sym) {
            return (*this)(Eigen::fix<row>, Eigen::fix<col>);
        }
        else {
            return (*this)(get_asym_row_idx<r>(), get_asym_col_idx<c>());
        }
    }
};

template <class T, bool sym, int n_sub_block = 2,
          class = std::enable_if_t<std::is_same_v<T, double> || std::is_same_v<T, DoubleComplex>>>
class BlockEntry {
   public:
    static constexpr int scalar_size = sym ? 1 : 3;
    static constexpr int block_size = scalar_size * n_sub_block;
    static constexpr int size = block_size * block_size;
    static constexpr int size_in_double = (std::is_same_v<T, double> ? 1 : 2) * size;

    using ArrayType = Eigen::Array<T, block_size, block_size, Eigen::ColMajor>;
    using GetterType = std::conditional_t<sym, T&, Eigen::Block<ArrayType, scalar_size, scalar_size>>;

   protected:
    // get position, can be 4 values
    // 0, 0: upper left
    // 0, 1: upper right
    // 1, 0: lower left
    // 1, 1: lower right
    template <int row, int col>
    GetterType get_val() {
        if constexpr (sym) {
            return data_(row, col);
        }
        else {
            return data_.template block<scalar_size, scalar_size>(row * scalar_size, col * scalar_size);
        }
    }

   private:
    ArrayType data_{ArrayType::Zero()};
};

template <template <bool> class BlockType>
struct block_entry_trait {
    template <bool sym>
    struct internal_trait {
        static_assert(sizeof(BlockType<sym>) == sizeof(double[BlockType<sym>::size_in_double]));
        static_assert(alignof(BlockType<sym>) >= alignof(double[BlockType<sym>::size_in_double]));
        static_assert(std::is_standard_layout_v<BlockType<sym>>);
    };
    static constexpr internal_trait<true> sym_trait{};
    static constexpr internal_trait<false> asym_trait{};
};

}  // namespace math_model_impl

}  // namespace power_grid_model

#endif