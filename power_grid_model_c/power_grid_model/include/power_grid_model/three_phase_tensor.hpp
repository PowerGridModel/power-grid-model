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
concept scalar_value = std::same_as<T, double> || std::same_as<T, DoubleComplex>;

namespace three_phase_tensor {

template <class T> using Eigen3Vector = Eigen::Array<T, 3, 1>;
template <class T> using Eigen3Tensor = Eigen::Array<T, 3, 3, Eigen::ColMajor>;
template <class T> using Eigen3DiagonalTensor = Eigen::DiagonalMatrix<T, 3>;

template <scalar_value T> class Vector : public Eigen3Vector<T> {
  public:
    Vector() { (*this) = Eigen3Vector<T>::Zero(); };
    // eigen expression
    template <typename OtherDerived> Vector(Eigen::ArrayBase<OtherDerived> const& other) : Eigen3Vector<T>{other} {}
    template <typename OtherDerived> Vector& operator=(Eigen::ArrayBase<OtherDerived> const& other) {
        this->Eigen3Vector<T>::operator=(other);
        return *this;
    }
    // constructor of single value
    // for complex number, rotate the single value by 120 and 240 degrees for 1st and 2nd entry
    // this will create a symmetric phasor based on one phasor
    explicit Vector(T const& x) {
        if constexpr (std::same_as<T, double>) {
            (*this) << x, x, x;
        } else {
            (*this) << x, (x * a2), (x * a);
        }
    }
    // piecewise constructor of single value
    // for both real and complex number, the value is repeated three times (without rotation)
    explicit Vector(std::piecewise_construct_t /* tag */, T const& x) { (*this) << x, x, x; }
    // constructor of three values
    Vector(T const& x1, T const& x2, T const& x3) { (*this) << x1, x2, x3; }
};

template <scalar_value T> class Tensor : public Eigen3Tensor<T> {
  public:
    Tensor() { (*this) = Eigen3Tensor<T>::Zero(); }
    // additional constructors
    explicit Tensor(T const& x) { (*this) << x, 0.0, 0.0, 0.0, x, 0.0, 0.0, 0.0, x; }
    explicit Tensor(T const& s, T const& m) { (*this) << s, m, m, m, s, m, m, m, s; }
    explicit Tensor(Vector<T> const& v) { (*this) << v(0), 0.0, 0.0, 0.0, v(1), 0.0, 0.0, 0.0, v(2); }
    // eigen expression
    template <typename OtherDerived> Tensor(Eigen::ArrayBase<OtherDerived> const& other) : Eigen3Tensor<T>{other} {}
    template <typename OtherDerived> Tensor& operator=(Eigen::ArrayBase<OtherDerived> const& other) {
        this->Eigen3Tensor<T>::operator=(other);
        return *this;
    }
};

template <scalar_value T> class DiagonalTensor : public Eigen3DiagonalTensor<T> {
  public:
    DiagonalTensor() { (*this) = Eigen3DiagonalTensor<T>::Zero(); }
    // additional constructors
    explicit DiagonalTensor(T const& x) : Eigen3DiagonalTensor<T>{x, x, x} {}
    explicit DiagonalTensor(Vector<T> const& v) : Eigen3DiagonalTensor<T>{v(0), v(1), v(2)} {}
    // eigen expression
    template <typename OtherDerived>
    DiagonalTensor(Eigen::ArrayBase<OtherDerived> const& other) : Eigen3DiagonalTensor<T>{other} {}

    template <typename OtherDerived> DiagonalTensor& operator=(Eigen::ArrayBase<OtherDerived> const& other) {
        this->Eigen3DiagonalTensor<T>::operator=(other);
        return *this;
    }
};
} // namespace three_phase_tensor

// three phase vector and tensor
template <bool sym> using RealTensor = std::conditional_t<sym, double, three_phase_tensor::Tensor<double>>;
template <bool sym>
using ComplexTensor = std::conditional_t<sym, DoubleComplex, three_phase_tensor::Tensor<DoubleComplex>>;

template <bool sym>
using RealDiagonalTensor = std::conditional_t<sym, double, three_phase_tensor::DiagonalTensor<double>>;
template <bool sym>
using ComplexDiagonalTensor = std::conditional_t<sym, DoubleComplex, three_phase_tensor::DiagonalTensor<DoubleComplex>>;

template <bool sym> using RealValue = std::conditional_t<sym, double, three_phase_tensor::Vector<double>>;
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
concept column_vector = (T::ColsAtCompileTime == 1);
template <class T>
concept rk2_tensor = (static_cast<Idx>(T::RowsAtCompileTime) ==
                      static_cast<Idx>(T::ColsAtCompileTime)); // rank 2 tensor
template <class T>
concept column_vector_or_tensor = column_vector<T> || rk2_tensor<T>;

// piecewise factory construction for complex vector
template <bool sym = false> inline ComplexValue<sym> piecewise_complex_value(DoubleComplex const& x) {
    if constexpr (sym) {
        return x;
    } else {
        return ComplexValue<false>{std::piecewise_construct, x};
    }
}

template <column_vector DerivedA>
inline ComplexValue<false> piecewise_complex_value(Eigen::ArrayBase<DerivedA> const& val) {
    return val;
}

// abs
inline double cabs(double x) { return std::abs(x); }
inline double cabs(DoubleComplex const& x) { return std::sqrt(std::norm(x)); }
inline double abs2(DoubleComplex const& x) { return std::norm(x); }
template <column_vector_or_tensor DerivedA> inline auto cabs(Eigen::ArrayBase<DerivedA> const& m) {
    return sqrt(abs2(m));
}

// calculate kron product of two vector
inline double vector_outer_product(double x, double y) { return x * y; }
template <column_vector DerivedA, column_vector DerivedB>
inline auto vector_outer_product(Eigen::ArrayBase<DerivedA> const& x, Eigen::ArrayBase<DerivedB> const& y) {
    return (x.matrix() * y.matrix().transpose()).array();
}

namespace detail {
template <column_vector_or_tensor Derived> inline auto const& dot_prepare(Eigen::DiagonalBase<Derived> const& x) {
    return x;
}
template <column_vector_or_tensor Derived> inline auto const& dot_prepare(Eigen::MatrixBase<Derived> const& x) {
    return x;
}
template <column_vector_or_tensor Derived> inline auto dot_prepare(Eigen::ArrayBase<Derived> const& x) {
    return x.matrix();
}
} // namespace detail

// calculate matrix multiply, dot
inline double dot(double x, double y) { return x * y; }
inline DoubleComplex dot(DoubleComplex const& x, DoubleComplex const& y) { return x * y; }

template <scalar_value... T> inline auto dot(T const&... x) { return (... * x); }

template <column_vector_or_tensor... Derived> inline auto dot(Derived const&... x) {
    using detail::dot_prepare;
    return (... * dot_prepare(x)).array();
}

template <column_vector_or_tensor... Derived>
    requires(std::derived_from<Derived, Eigen::DiagonalBase<Derived>> && ...)
inline auto dot(Derived const&... x) {
    return (... * detail::dot_prepare(x));
}

// max of a vector
inline double max_val(double val) { return val; }
template <column_vector DerivedA> inline double max_val(Eigen::ArrayBase<DerivedA> const& val) {
    return val.maxCoeff();
}

// function to sum rows of tensor
template <rk2_tensor DerivedA> inline auto sum_row(Eigen::ArrayBase<DerivedA> const& m) { return m.rowwise().sum(); }
// overload for double
inline double sum_row(double d) { return d; }

// function to sum vector
template <column_vector DerivedA> inline auto sum_val(Eigen::ArrayBase<DerivedA> const& m) { return m.sum(); }
// overload for double and complex
inline double sum_val(double d) { return d; }
inline DoubleComplex sum_val(DoubleComplex const& z) { return z; }

// function to mean vector
template <column_vector DerivedA> inline auto mean_val(Eigen::ArrayBase<DerivedA> const& m) { return m.mean(); }
inline DoubleComplex mean_val(DoubleComplex const& z) { return z; }
inline double mean_val(double z) { return z; }

template <bool sym, class T> inline auto process_mean_val(const T& m) {
    if constexpr (sym) {
        return mean_val(m);
    } else {
        return m;
    }
}

template <column_vector Derived> inline auto as_diag(Eigen::ArrayBase<Derived> const& x) {
    return x.matrix().asDiagonal();
}
inline auto as_diag(double x) { return x; }

// diagonal multiply
template <column_vector DerivedA, rk2_tensor DerivedB, column_vector DerivedC>
inline auto diag_mult(Eigen::ArrayBase<DerivedA> const& x, Eigen::ArrayBase<DerivedB> const& y,
                      Eigen::ArrayBase<DerivedC> const& z) {
    return (as_diag(x) * y.matrix() * as_diag(z)).array();
}
// double overload
inline auto diag_mult(double x, double y, double z) { return x * y * z; }

// calculate positive sequence
template <column_vector Derived> inline DoubleComplex pos_seq(Eigen::ArrayBase<Derived> const& val) {
    return (val(0) + a * val(1) + a2 * val(2)) / 3.0;
}

inline auto pos_seq(DoubleComplex const& val) { return val; }

// inverse of tensor
inline auto inv(double val) { return 1.0 / val; }
inline auto inv(DoubleComplex const& val) { return 1.0 / val; }
inline auto inv(ComplexTensor<false> const& val) { return val.matrix().inverse().array(); }

// add_diag
inline void add_diag(double& x, double y) { x += y; }
inline void add_diag(DoubleComplex& x, DoubleComplex const& y) { x += y; }
template <rk2_tensor DerivedA, column_vector DerivedB>
inline void add_diag(Eigen::ArrayBase<DerivedA>& x, Eigen::ArrayBase<DerivedB> const& y) {
    x.matrix().diagonal() += y.matrix();
}
template <rk2_tensor DerivedA, column_vector DerivedB>
inline void add_diag(Eigen::ArrayBase<DerivedA>&& x, Eigen::ArrayBase<DerivedB> const& y) {
    x.matrix().diagonal() += y.matrix();
}

// zero tensor
template <bool sym> inline const ComplexTensor<sym> zero_tensor = ComplexTensor<sym>{0.0};

// inverse symmetric param
inline std::pair<DoubleComplex, DoubleComplex> inv_sym_param(DoubleComplex const& s, DoubleComplex const& m) {
    DoubleComplex const det_1 = 1.0 / (s * s + s * m - 2.0 * m * m);
    return {(s + m) * det_1, -m * det_1};
}

// is nan
template <class Derived> inline bool is_nan(Eigen::ArrayBase<Derived> const& x) { return x.isNaN().all(); }
inline bool is_nan(std::floating_point auto x) { return std::isnan(x); }
inline bool is_nan(ID x) { return x == na_IntID; }
inline bool is_nan(IntS x) { return x == na_IntS; }
template <class Enum>
    requires std::same_as<std::underlying_type_t<Enum>, IntS>
inline bool is_nan(Enum x) {
    return static_cast<IntS>(x) == na_IntS;
}

// is normal
inline auto is_normal(std::floating_point auto value) { return std::isnormal(value); }
inline auto is_normal(RealValue<false> const& value) {
    return is_normal(value(0)) && is_normal(value(1)) && is_normal(value(2));
}

// isinf
inline auto is_inf(std::floating_point auto value) { return std::isinf(value); }
inline auto is_inf(RealValue<false> const& value) { return is_inf(value(0)) || is_inf(value(1)) || is_inf(value(2)); }

// any_zero
inline auto any_zero(std::floating_point auto value) { return value == 0.0; }
inline auto any_zero(RealValue<false> const& value) { return (value == RealValue<false>{0.0}).any(); }

// update real values
//
// RealValue is only updated when the update value is not nan
//
// symmetric:  update 1.0 with nan -> 1.0
//             update 1.0 with 2.0 -> 2.0
//
// asymmetric: update [1.0, nan, nan] with [nan, nan, 2.0] -> [1.0, nan, 2.0]
//
// The function assumes that the current value is normalized and new value should be normalized with scalar
template <bool sym, class Proxy>
inline void update_real_value(RealValue<sym> const& new_value, Proxy&& current_value, double scalar) {
    if constexpr (sym) {
        if (!is_nan(new_value)) {
            current_value = scalar * new_value;
        }
    } else {
        for (size_t i = 0; i != 3; ++i) {
            if (!is_nan(new_value(i))) {
                current_value(i) = scalar * new_value(i);
            }
        }
    }
}

// update a value if the existing value is not nan
//
// contrary to update_real_value, this function retains nan in the target
template <typename T> inline void set_if_not_nan(T& target, T const& value) {
    if (!is_nan(target)) {
        target = value;
    }
};
inline void set_if_not_nan(RealValue<false>& target, RealValue<false> const& value) {
    for (Idx i = 0; i != 3; ++i) {
        set_if_not_nan(target(i), value(i));
    }
};

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
inline DoubleComplex hermitian_transpose(DoubleComplex const& z) { return conj(z); }
inline double hermitian_transpose(double x) { return x; }
template <rk2_tensor Derived> inline auto hermitian_transpose(Eigen::ArrayBase<Derived> const& x) {
    return x.matrix().adjoint().array();
}

// vector of values
template <bool sym> using RealValueVector = std::vector<RealValue<sym>>;
template <bool sym> using ComplexValueVector = std::vector<ComplexValue<sym>>;
template <bool sym> using RealTensorVector = std::vector<RealTensor<sym>>;
template <bool sym> using ComplexTensorVector = std::vector<ComplexTensor<sym>>;

} // namespace power_grid_model

#endif
