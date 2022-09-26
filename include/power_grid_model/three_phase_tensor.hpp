// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_THREE_PHASE_TENSOR_HPP
#define POWER_GRID_MODEL_THREE_PHASE_TENSOR_HPP

#include "power_grid_model.hpp"

// eigen properties
#include <Eigen/Dense>
#include <cmath>
#include <complex>
#include <tuple>
#include <utility>

namespace power_grid_model {

// enable scalar
template <class T>
constexpr bool check_scalar_v = std::is_same_v<T, double> || std::is_same_v<T, DoubleComplex>;
template <class T>
using enable_scalar_t = std::enable_if_t<check_scalar_v<T>>;

namespace three_phase_tensor {

template <class T>
using Eigen3Vector = Eigen::Array<T, 3, 1>;
template <class T>
using Eigen3Tensor = Eigen::Array<T, 3, 3, Eigen::ColMajor>;

template <class T, class = enable_scalar_t<T>>
class Vector : public Eigen3Vector<T> {
   public:
    Vector() {
        (*this) = Eigen3Vector<T>::Zero();
    };
    // eigen expression
    template <typename OtherDerived>
    Vector(Eigen::ArrayBase<OtherDerived> const& other) : Eigen3Vector<T>{other} {
    }
    template <typename OtherDerived>
    Vector& operator=(Eigen::ArrayBase<OtherDerived> const& other) {
        this->Eigen3Vector<T>::operator=(other);
        return *this;
    }
    // constructor of single value
    // for complex number, rotate the single value by 120 and 240 degrees for 1st and 2nd entry
    // this will create a symmetric phasor based on one phasor
    explicit Vector(T const& x) {
        if constexpr (std::is_same_v<T, double>) {
            (*this) << x, x, x;
        }
        else {
            (*this) << x, (x * a2), (x * a);
        }
    }
    // piecewise constructor of single value
    // for both real and complex number, the value is repeated three times (without rotation)
    explicit Vector(std::piecewise_construct_t, T const& x) {
        (*this) << x, x, x;
    }
    // constructor of three values
    Vector(T const& x1, T const& x2, T const& x3) {
        (*this) << x1, x2, x3;
    }
};

template <class T, class = enable_scalar_t<T>>
class Tensor : public Eigen3Tensor<T> {
   public:
    Tensor() {
        (*this) = Eigen3Tensor<T>::Zero();
    }
    // additional constructors
    explicit Tensor(T const& x) {
        (*this) << x, 0.0, 0.0, 0.0, x, 0.0, 0.0, 0.0, x;
    }
    explicit Tensor(T const& s, T const& m) {
        (*this) << s, m, m, m, s, m, m, m, s;
    }
    explicit Tensor(Vector<T> const& v) {
        (*this) << v(0), 0.0, 0.0, 0.0, v(1), 0.0, 0.0, 0.0, v(2);
    }
    // eigen expression
    template <typename OtherDerived>
    Tensor(Eigen::ArrayBase<OtherDerived> const& other) : Eigen3Tensor<T>{other} {
    }
    template <typename OtherDerived>
    Tensor& operator=(Eigen::ArrayBase<OtherDerived> const& other) {
        this->Eigen3Tensor<T>::operator=(other);
        return *this;
    }
};

}  // namespace three_phase_tensor

// three phase vector and tensor
template <bool sym>
using RealTensor = std::conditional_t<sym, double, three_phase_tensor::Tensor<double>>;
template <bool sym>
using ComplexTensor = std::conditional_t<sym, DoubleComplex, three_phase_tensor::Tensor<DoubleComplex>>;
template <bool sym>
using RealValue = std::conditional_t<sym, double, three_phase_tensor::Vector<double>>;
template <bool sym>
using ComplexValue = std::conditional_t<sym, DoubleComplex, three_phase_tensor::Vector<DoubleComplex>>;
// asserts to ensure alignment
static_assert(sizeof(RealTensor<false>) == sizeof(double[9]));
static_assert(alignof(RealTensor<false>) == alignof(double[9]));
static_assert(std::is_standard_layout_v<RealTensor<false>>);
static_assert(std::is_trivially_destructible_v<RealTensor<false>>);
static_assert(sizeof(ComplexTensor<false>) == sizeof(double[18]));
static_assert(alignof(ComplexTensor<false>) >= alignof(double[18]));
static_assert(std::is_standard_layout_v<ComplexTensor<false>>);
static_assert(std::is_trivially_destructible_v<ComplexTensor<false>>);
static_assert(sizeof(RealValue<false>) == sizeof(double[3]));
static_assert(alignof(RealValue<false>) == alignof(double[3]));
static_assert(std::is_standard_layout_v<RealValue<false>>);
static_assert(std::is_trivially_destructible_v<RealValue<false>>);
static_assert(sizeof(ComplexValue<false>) == sizeof(double[6]));
static_assert(alignof(ComplexValue<false>) >= alignof(double[6]));
static_assert(std::is_standard_layout_v<ComplexValue<false>>);
static_assert(std::is_trivially_destructible_v<ComplexValue<false>>);

// enabler
template <class T>
constexpr bool check_vector_v = T::ColsAtCompileTime == 1;
template <class T>
constexpr bool check_tensor_v = (Idx)T::RowsAtCompileTime == (Idx)T::ColsAtCompileTime;
template <class T>
constexpr bool check_all_v = check_tensor_v<T> || check_vector_v<T>;
template <class T>
using enable_vector_t = std::enable_if_t<check_vector_v<T>>;
template <class T>
using enable_tensor_t = std::enable_if_t<check_tensor_v<T>>;
template <class T>
using enable_all_t = std::enable_if_t<check_all_v<T>>;

// piecewise factory construction for complex vector
template <bool sym = false>
inline ComplexValue<sym> piecewise_complex_value(DoubleComplex const& x) {
    if constexpr (sym) {
        return x;
    }
    else {
        return ComplexValue<false>{std::piecewise_construct, x};
    }
}

template <class DerivedA, class = enable_vector_t<DerivedA>>
inline ComplexValue<false> piecewise_complex_value(Eigen::ArrayBase<DerivedA> const& val) {
    return val;
}

// abs
inline double cabs(double x) {
    return std::abs(x);
}
inline double cabs(DoubleComplex const& x) {
    return std::sqrt(std::norm(x));
}
inline double abs2(DoubleComplex const& x) {
    return std::norm(x);
}
template <class DerivedA, class = enable_all_t<DerivedA>>
inline auto cabs(Eigen::ArrayBase<DerivedA> const& m) {
    return sqrt(abs2(m));
}

// calculate kron product of two vector
inline double vector_outer_product(double x, double y) {
    return x * y;
}
template <class DerivedA, class DerivedB, class = enable_vector_t<DerivedA>, class = enable_vector_t<DerivedB>>
inline auto vector_outer_product(Eigen::ArrayBase<DerivedA> const& x, Eigen::ArrayBase<DerivedB> const& y) {
    return (x.matrix() * y.matrix().transpose()).array();
}

// calculate matrix multiply, dot
inline double dot(double x, double y) {
    return x * y;
}
inline DoubleComplex dot(DoubleComplex const& x, DoubleComplex const& y) {
    return x * y;
}

template <class... T, class = std::enable_if_t<(check_scalar_v<T> && ...)>>
inline auto dot(T const&... x) {
    return (... * x);
}

template <class... Derived, class = std::enable_if_t<(check_all_v<Derived> && ...)>>
inline auto dot(Eigen::ArrayBase<Derived> const&... x) {
    auto res_mat = (... * x.matrix());
    return res_mat.array();
}

// max of a vector
inline double max_val(double val) {
    return val;
}
template <class DerivedA, class = enable_vector_t<DerivedA>>
inline double max_val(Eigen::ArrayBase<DerivedA> const& val) {
    return val.maxCoeff();
}

// function to sum rows of tensor
template <class DerivedA, class = enable_tensor_t<DerivedA>>
inline auto sum_row(Eigen::ArrayBase<DerivedA> const& m) {
    return m.rowwise().sum();
}
// overload for double
inline double sum_row(double d) {
    return d;
}

// function to sum vector
template <class DerivedA, class = enable_vector_t<DerivedA>>
inline auto sum_val(Eigen::ArrayBase<DerivedA> const& m) {
    return m.sum();
}
// overload for double and complex
inline double sum_val(double d) {
    return d;
}
inline DoubleComplex sum_val(DoubleComplex const& z) {
    return z;
}

// function to mean vector
template <class DerivedA, class = enable_vector_t<DerivedA>>
inline auto mean_val(Eigen::ArrayBase<DerivedA> const& m) {
    return m.mean();
}
inline DoubleComplex mean_val(DoubleComplex const& z) {
    return z;
}
inline double mean_val(double z) {
    return z;
}

template <bool sym, class T>
inline auto process_mean_val(T&& m) {
    if constexpr (sym) {
        return mean_val(m);
    }
    else {
        return m;
    }
}

// diagonal multiply
template <class DerivedA, class DerivedB, class DerivedC, class = enable_vector_t<DerivedA>,
          class = enable_tensor_t<DerivedB>, class = enable_vector_t<DerivedC>>
inline auto diag_mult(Eigen::ArrayBase<DerivedA> const& x, Eigen::ArrayBase<DerivedB> const& y,
                      Eigen::ArrayBase<DerivedC> const& z) {
    return (x.matrix().asDiagonal() * y.matrix() * z.matrix().asDiagonal()).array();
}
// double overload
inline double diag_mult(double x, double y, double z) {
    return x * y * z;
}

// calculate positive sequence
template <class Derived, class = enable_vector_t<Derived>>
inline DoubleComplex pos_seq(Eigen::ArrayBase<Derived> const& val) {
    return (val(0) + a * val(1) + a2 * val(2)) / 3.0;
}

inline DoubleComplex pos_seq(DoubleComplex const& val) {
    return val;
}

// inverse of tensor
inline DoubleComplex inv(DoubleComplex const& val) {
    return 1.0 / val;
}
inline auto inv(ComplexTensor<false> const& val) {
    return val.matrix().inverse().array();
}

// add_diag
inline void add_diag(double& x, double y) {
    x += y;
}
inline void add_diag(DoubleComplex& x, DoubleComplex const& y) {
    x += y;
}
template <class DerivedA, class DerivedB, class = enable_tensor_t<DerivedA>, class = enable_vector_t<DerivedB>>
inline void add_diag(Eigen::ArrayBase<DerivedA>& x, Eigen::ArrayBase<DerivedB> const& y) {
    x.matrix().diagonal() += y.matrix();
}
template <class DerivedA, class DerivedB, class = enable_tensor_t<DerivedA>, class = enable_vector_t<DerivedB>>
inline void add_diag(Eigen::ArrayBase<DerivedA>&& x, Eigen::ArrayBase<DerivedB> const& y) {
    x.matrix().diagonal() += y.matrix();
}

// zero tensor
template <bool sym>
inline const ComplexTensor<sym> zero_tensor = ComplexTensor<sym>{0.0};

// inverse symmetric param
inline std::pair<DoubleComplex, DoubleComplex> inv_sym_param(DoubleComplex const& s, DoubleComplex const& m) {
    DoubleComplex const det_1 = 1.0 / (s * s + s * m - 2.0 * m * m);
    return {(s + m) * det_1, -m * det_1};
}

// is nan
template <class Derived>
inline bool is_nan(Eigen::ArrayBase<Derived> const& x) {
    return x.isNaN().all();
}
inline bool is_nan(double x) {
    return std::isnan(x);
}
inline bool is_nan(ID x) {
    return x == na_IntID;
}
inline bool is_nan(IntS x) {
    return x == na_IntS;
}

/* update real values

   RealValue is only updated when the update value is not nan

   symmetric:  update 1.0 with nan -> 1.0
               update 1.0 with 2.0 -> 2.0

   asymmetric: update [1.0, nan, nan] with [nan, nan, 2.0] -> [1.0, nan, 2.0]

   The function assumes that the current value is normalized and new value should be normalized with scalar
*/
template <bool sym, class Proxy>
void update_real_value(RealValue<sym> const& new_value, Proxy&& current_value, double scalar) {
    if constexpr (sym) {
        if (!is_nan(new_value)) {
            current_value = scalar * new_value;
        }
    }
    else {
        for (size_t i = 0; i != 3; ++i) {
            if (!is_nan(new_value(i))) {
                current_value(i) = scalar * new_value(i);
            }
        }
    }
}

// symmetric component matrix
inline ComplexTensor<false> get_sym_matrix() {
    ComplexTensor<false> m;
    m << 1.0, 1.0, 1.0, 1.0, a2, a, 1.0, a, a2;
    return m;
}
inline ComplexTensor<false> get_sym_matrix_inv() {
    ComplexTensor<false> m;
    m << 1.0, 1.0, 1.0, 1.0, a, a2, 1.0, a2, a;
    m = m / 3.0;
    return m;
}

// conjugate (hermitian) transpose
inline DoubleComplex hermitian_transpose(DoubleComplex const& z) {
    return conj(z);
}
inline double hermitian_transpose(double x) {
    return x;
}
template <class Derived, class = enable_tensor_t<Derived>>
inline auto hermitian_transpose(Eigen::ArrayBase<Derived> const& x) {
    return x.matrix().adjoint().array();
}

// vector of values
template <bool sym>
using RealValueVector = std::vector<RealValue<sym>>;
template <bool sym>
using ComplexValueVector = std::vector<ComplexValue<sym>>;
template <bool sym>
using RealTensorVector = std::vector<RealTensor<sym>>;
template <bool sym>
using ComplexTensorVector = std::vector<ComplexTensor<sym>>;

}  // namespace power_grid_model

#endif
