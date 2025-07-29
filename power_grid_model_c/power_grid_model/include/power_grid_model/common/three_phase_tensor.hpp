// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"

// eigen properties
#include <Eigen/Dense>

#include <cmath>
#include <complex>

namespace power_grid_model {

// enable scalar
template <class T>
concept scalar_value = std::same_as<T, double> || std::same_as<T, DoubleComplex>;

namespace three_phase_tensor {

template <class T> using Eigen3Vector = Eigen::Array<T, 3, 1>;
template <class T> using Eigen3Tensor = Eigen::Array<T, 3, 3, Eigen::ColMajor>;
template <class T> using Eigen4Tensor = Eigen::Array<T, 4, 4, Eigen::ColMajor>;
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
        if constexpr (std::floating_point<T>) {
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
    // for complex, it is possible to construct from real part and imaginary part
    template <std::floating_point U>
        requires std::same_as<T, std::complex<U>>
    Vector(Vector<U> real_part, Vector<U> imag_part)
        : Vector{{real_part(0), imag_part(0)}, {real_part(1), imag_part(1)}, {real_part(2), imag_part(2)}} {}
};

template <scalar_value T> class Tensor : public Eigen3Tensor<T> {
  public:
    Tensor() { (*this) = Eigen3Tensor<T>::Zero(); }
    // additional constructors
    explicit Tensor(T const& x) { (*this) << x, T{0}, T{0}, T{0}, x, T{0}, T{0}, T{0}, x; }
    explicit Tensor(T const& s, T const& m) { (*this) << s, m, m, m, s, m, m, m, s; }
    explicit Tensor(T const& s1, T const& s2, T const& s3, T const& m12, T const& m13, T const& m23) {
        (*this) << s1, m12, m13, m12, s2, m23, m13, m23, s3;
    }
    explicit Tensor(Vector<T> const& v) {
        assert(v.size() == 3);
        (*this) << v(0), T{0}, T{0}, T{0}, v(1), T{0}, T{0}, T{0}, v(2);
    }
    // eigen expression
    template <typename OtherDerived> Tensor(Eigen::ArrayBase<OtherDerived> const& other) : Eigen3Tensor<T>{other} {}
    template <typename OtherDerived> Tensor& operator=(Eigen::ArrayBase<OtherDerived> const& other) {
        this->Eigen3Tensor<T>::operator=(other);
        return *this;
    }
};

template <scalar_value T> class Tensor4 : public Eigen4Tensor<T> {
  public:
    Tensor4() { (*this) = Eigen4Tensor<T>::Zero(); }
    // additional constructors
    explicit Tensor4(T const& x) {
        (*this) << x, T{0}, T{0}, T{0}, T{0}, x, T{0}, T{0}, T{0}, T{0}, x, T{0}, T{0}, T{0}, T{0}, x;
    }
    explicit Tensor4(T const& s, T const& m) { (*this) << s, m, m, m, m, s, m, m, m, m, s, m, m, m, m, s; }
    explicit Tensor4(T const& s1, T const& s2, T const& s3, T const& s4, T const& m12, T const& m13, T const& m14,
                     T const& m23, T const& m24, T const& m34) {
        (*this) << s1, m12, m13, m14, m12, s2, m23, m24, m13, m23, s3, m34, m14, m24, m34, s4;
    }
    explicit Tensor4(Vector<T> const& v) {
        assert(v.size() == 4);
        (*this) << v(0), T{0}, T{0}, T{0}, T{0}, v(1), T{0}, T{0}, T{0}, T{0}, v(2), T{0}, T{0}, T{0}, T{0}, v(3);
    }
    // eigen expression
    template <typename OtherDerived> Tensor4(Eigen::ArrayBase<OtherDerived> const& other) : Eigen4Tensor<T>{other} {}
    template <typename OtherDerived> Tensor4& operator=(Eigen::ArrayBase<OtherDerived> const& other) {
        this->Eigen4Tensor<T>::operator=(other);
        return *this;
    }
};

template <scalar_value T> class DiagonalTensor : public Eigen3DiagonalTensor<T> {
  public:
    DiagonalTensor() { (*this).setZero(); }
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
template <symmetry_tag sym>
using RealTensor = std::conditional_t<is_symmetric_v<sym>, double, three_phase_tensor::Tensor<double>>;
template <symmetry_tag sym>
using ComplexTensor = std::conditional_t<is_symmetric_v<sym>, DoubleComplex, three_phase_tensor::Tensor<DoubleComplex>>;
using ComplexTensor4 = three_phase_tensor::Tensor4<DoubleComplex>;

template <symmetry_tag sym>
using RealDiagonalTensor = std::conditional_t<is_symmetric_v<sym>, double, three_phase_tensor::DiagonalTensor<double>>;
template <symmetry_tag sym>
using ComplexDiagonalTensor =
    std::conditional_t<is_symmetric_v<sym>, DoubleComplex, three_phase_tensor::DiagonalTensor<DoubleComplex>>;
template <symmetry_tag sym>
using RealValue = std::conditional_t<is_symmetric_v<sym>, double, three_phase_tensor::Vector<double>>;
template <symmetry_tag sym>
using ComplexValue = std::conditional_t<is_symmetric_v<sym>, DoubleComplex, three_phase_tensor::Vector<DoubleComplex>>;

// asserts to ensure alignment
static_assert(sizeof(RealTensor<asymmetric_t>) == sizeof(double[9]));   // NOLINT(hicpp-avoid-c-arrays)
static_assert(alignof(RealTensor<asymmetric_t>) == alignof(double[9])); // NOLINT(hicpp-avoid-c-arrays)
static_assert(std::is_standard_layout_v<RealTensor<asymmetric_t>>);
static_assert(std::is_trivially_destructible_v<RealTensor<asymmetric_t>>);
static_assert(sizeof(ComplexTensor<asymmetric_t>) == sizeof(double[18]));   // NOLINT(hicpp-avoid-c-arrays)
static_assert(alignof(ComplexTensor<asymmetric_t>) >= alignof(double[18])); // NOLINT(hicpp-avoid-c-arrays)
static_assert(std::is_standard_layout_v<ComplexTensor<asymmetric_t>>);
static_assert(std::is_trivially_destructible_v<ComplexTensor<asymmetric_t>>);
static_assert(sizeof(RealValue<asymmetric_t>) == sizeof(double[3]));   // NOLINT(hicpp-avoid-c-arrays)
static_assert(alignof(RealValue<asymmetric_t>) == alignof(double[3])); // NOLINT(hicpp-avoid-c-arrays)
static_assert(std::is_standard_layout_v<RealValue<asymmetric_t>>);
static_assert(std::is_trivially_destructible_v<RealValue<asymmetric_t>>);
static_assert(sizeof(ComplexValue<asymmetric_t>) == sizeof(double[6]));   // NOLINT(hicpp-avoid-c-arrays)
static_assert(alignof(ComplexValue<asymmetric_t>) >= alignof(double[6])); // NOLINT(hicpp-avoid-c-arrays)
static_assert(std::is_standard_layout_v<ComplexValue<asymmetric_t>>);
static_assert(std::is_trivially_destructible_v<ComplexValue<asymmetric_t>>);

// enabler
template <class T>
concept column_vector = (T::ColsAtCompileTime == 1);
template <class T>
concept rk2_tensor =
    (static_cast<Idx>(T::RowsAtCompileTime) == static_cast<Idx>(T::ColsAtCompileTime)); // rank 2 tensor
template <class T>
concept column_vector_or_tensor = column_vector<T> || rk2_tensor<T>;

// piecewise factory construction for complex vector
template <symmetry_tag sym = asymmetric_t> inline ComplexValue<sym> piecewise_complex_value(DoubleComplex const& x) {
    if constexpr (is_symmetric_v<sym>) {
        return x;
    } else {
        return ComplexValue<asymmetric_t>{std::piecewise_construct, x};
    }
}

template <column_vector Derived, template <typename> typename T>
    requires std::convertible_to<T<Derived>, Eigen::ArrayBase<Derived>>
inline ComplexValue<asymmetric_t> piecewise_complex_value(T<Derived> const& x) {
    return x;
}

// piecewise factory construction for complex vector
inline ComplexValue<asymmetric_t> piecewise_complex_value(ComplexValue<asymmetric_t> const& x) { return x; }

// abs
template <std::floating_point Float> inline Float cabs(Float x) { return std::abs(x); }
template <std::floating_point Float> inline Float abs2(Float x) { return x * x; }
template <std::floating_point Float> inline Float cabs(std::complex<Float> const& x) { return std::sqrt(std::norm(x)); }
template <std::floating_point Float> inline Float abs2(std::complex<Float> const& x) { return std::norm(x); }

template <column_vector_or_tensor DerivedA>
inline auto cabs(Eigen::ArrayBase<DerivedA> const& m)
    requires(std::same_as<typename DerivedA::Scalar, DoubleComplex>)
{
    return sqrt(abs2(m));
}
template <column_vector_or_tensor DerivedA>
inline auto cabs(Eigen::ArrayBase<DerivedA> const& m)
    requires(std::floating_point<typename DerivedA::Scalar>)
{
    return m.abs();
}

// phase_shift(x) = e^{i arg(x)} = x / |x|
template <std::floating_point Float> inline std::complex<Float> phase_shift(std::complex<Float> const x) {
    if (auto const abs_x = cabs(x); abs_x > 0.0) {
        return x / abs_x;
    }
    return std::complex<Float>{1.0};
}
inline ComplexValue<asymmetric_t> phase_shift(ComplexValue<asymmetric_t> const& m) {
    return {phase_shift(m(0)), phase_shift(m(1)), phase_shift(m(2))};
}

// arg(e^(i * phase)) = phase (mod 2pi). By convention restrict to [-pi, pi].
template <std::floating_point Float> inline auto phase_mod_2pi(Float phase) {
    return RealValue<symmetric_t>{arg(std::complex<Float>{exp(std::complex<Float>{Float{0.0}, phase})})};
}
inline auto phase_mod_2pi(RealValue<asymmetric_t> const& phase) {
    return RealValue<asymmetric_t>{arg(ComplexValue<asymmetric_t>{exp(1.0i * phase)})};
}

// calculate kron product of two vector
template <std::floating_point Float> constexpr Float vector_outer_product(Float x, Float y) { return x * y; }
template <std::floating_point Float>
constexpr std::complex<Float> vector_outer_product(std::complex<Float> x, std::complex<Float> y) {
    return x * y;
}
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
template <std::floating_point Float> constexpr Float dot(Float x, Float y) { return x * y; }
template <std::floating_point Float>
constexpr std::complex<Float> dot(std::complex<Float> const& x, std::complex<Float> const& y) {
    return x * y;
}

template <scalar_value... T> constexpr auto dot(T const&... x) { return (... * x); }

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
template <std::floating_point Float> constexpr Float max_val(Float val) { return val; }
template <column_vector DerivedA> inline auto max_val(Eigen::ArrayBase<DerivedA> const& val) { return val.maxCoeff(); }

// function to sum rows of tensor
template <rk2_tensor DerivedA> inline auto sum_row(Eigen::ArrayBase<DerivedA> const& m) { return m.rowwise().sum(); }
template <typename T>
    requires std::floating_point<T> || std::same_as<T, DoubleComplex>
constexpr T sum_row(T d) {
    return d;
}

// function to sum vector
template <column_vector DerivedA> inline auto sum_val(Eigen::ArrayBase<DerivedA> const& m) { return m.sum(); }
// overload for floating-point and complex
template <std::floating_point Float> constexpr Float sum_val(Float d) { return d; }
template <std::floating_point Float> constexpr std::complex<Float> sum_val(std::complex<Float> const& z) { return z; }

// function to mean vector
template <column_vector DerivedA> inline auto mean_val(Eigen::ArrayBase<DerivedA> const& m) { return m.mean(); }
template <std::floating_point Float> constexpr std::complex<Float> mean_val(std::complex<Float> const& z) { return z; }
template <std::floating_point Float> constexpr Float mean_val(Float z) { return z; }

template <symmetry_tag sym, class T> inline auto process_mean_val(T const& m) {
    if constexpr (is_symmetric_v<sym>) {
        return mean_val(m);
    } else {
        return m;
    }
}

template <column_vector Derived> inline auto as_diag(Eigen::ArrayBase<Derived> const& x) {
    return x.matrix().asDiagonal();
}
constexpr auto as_diag(std::floating_point auto x) { return x; }

// diagonal multiply
template <column_vector DerivedA, rk2_tensor DerivedB, column_vector DerivedC>
inline auto diag_mult(Eigen::ArrayBase<DerivedA> const& x, Eigen::ArrayBase<DerivedB> const& y,
                      Eigen::ArrayBase<DerivedC> const& z) {
    return (as_diag(x) * y.matrix() * as_diag(z)).array();
}
// floating-point overload
template <std::floating_point Float> constexpr auto diag_mult(Float x, Float y, Float z) { return x * y * z; }

// calculate positive sequence
template <column_vector Derived> inline DoubleComplex pos_seq(Eigen::ArrayBase<Derived> const& val) {
    return (val(0) + a * val(1) + a2 * val(2)) / 3.0;
}

constexpr auto pos_seq(DoubleComplex const& val) { return val; }

// inverse of tensor
template <std::floating_point Float> constexpr auto inv(Float val) { return Float{1.0} / val; }
template <std::floating_point Float> constexpr auto inv(std::complex<Float> const& val) { return Float{1.0} / val; }
inline auto inv(ComplexTensor<asymmetric_t> const& val) { return val.matrix().inverse().array(); }

// add_diag
template <std::floating_point Float> constexpr void add_diag(Float& x, Float y) { x += y; }
template <std::floating_point Float> constexpr void add_diag(std::complex<Float>& x, std::complex<Float> const& y) {
    x += y;
}
template <rk2_tensor DerivedA, column_vector DerivedB>
inline void add_diag(Eigen::ArrayBase<DerivedA>& x, Eigen::ArrayBase<DerivedB> const& y) {
    x.matrix().diagonal() += y.matrix();
}
template <rk2_tensor DerivedA, column_vector DerivedB>
inline void add_diag(Eigen::ArrayBase<DerivedA>&& x, Eigen::ArrayBase<DerivedB> const& y) {
    std::move(x).matrix().diagonal() += y.matrix();
}

// zero tensor
template <symmetry_tag sym> inline const ComplexTensor<sym> zero_tensor = ComplexTensor<sym>{0.0};

// inverse symmetric param
template <std::floating_point Float>
constexpr std::pair<std::complex<Float>, std::complex<Float>> inv_sym_param(std::complex<Float> const& s,
                                                                            std::complex<Float> const& m) {
    std::complex<Float> const det_1 = Float{1} / (s * s + s * m - Float{2} * m * m);
    return {(s + m) * det_1, -m * det_1};
}

// is nan
template <class Derived> inline bool is_nan(Eigen::ArrayBase<Derived> const& x) { return x.isNaN().all(); }
inline bool is_nan(std::floating_point auto x) { return std::isnan(x); }
template <std::floating_point Float> inline bool is_nan(std::complex<Float> const& x) {
    return is_nan(x.real()) || is_nan(x.imag());
}
constexpr bool is_nan(ID x) { return x == na_IntID; }
constexpr bool is_nan(IntS x) { return x == na_IntS; }
template <class Enum>
    requires std::same_as<std::underlying_type_t<Enum>, IntS>
constexpr bool is_nan(Enum x) {
    return static_cast<IntS>(x) == na_IntS;
}
constexpr bool is_nan(Idx x) { return x == na_Idx; }

// is normal
inline auto is_normal(std::floating_point auto value) { return std::isnormal(value); }
template <std::floating_point Float> inline auto is_normal(std::complex<Float> const& value) {
    if (value.real() == Float{0}) {
        return is_normal(value.imag());
    }
    if (value.imag() == Float{0}) {
        return is_normal(value.real());
    }
    return is_normal(value.real()) && is_normal(value.imag());
}
template <class Derived> inline auto is_normal(Eigen::ArrayBase<Derived> const& value) {
    return is_normal(value(0)) && is_normal(value(1)) && is_normal(value(2));
}

// isinf
inline auto is_inf(std::floating_point auto value) { return std::isinf(value); }
inline auto is_inf(RealValue<asymmetric_t> const& value) {
    return is_inf(value(0)) || is_inf(value(1)) || is_inf(value(2));
}

// any_zero
template <std::floating_point Float> constexpr auto any_zero(Float value) { return value == Float{0}; }
inline auto any_zero(RealValue<asymmetric_t> const& value) { return (value == RealValue<asymmetric_t>{0.0}).any(); }

// all_zero
template <std::floating_point Float> constexpr auto all_zero(Float value) { return value == Float{0}; }
inline auto all_zero(RealValue<asymmetric_t> const& value) { return (value == RealValue<asymmetric_t>{0.0}).all(); }

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
template <symmetry_tag sym, class Proxy>
inline void update_real_value(RealValue<sym> const& new_value, Proxy&& current_value, RealValue<symmetric_t> scalar) {
    if constexpr (is_symmetric_v<sym>) {
        if (!is_nan(new_value)) {
            std::forward<Proxy>(current_value) = scalar * new_value;
        } else {
            capturing::into_the_void<Proxy>(std::forward<Proxy>(current_value));
        }
    } else {
        for (size_t i = 0; i != 3; ++i) {
            if (!is_nan(new_value(i))) {
                current_value(i) = scalar * new_value(i); // can't forward due to runtime element access
            }
        }
        capturing::into_the_void<Proxy>(std::forward<Proxy>(current_value));
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
inline void set_if_not_nan(RealValue<asymmetric_t>& target, RealValue<asymmetric_t> const& value) {
    for (Idx i = 0; i != 3; ++i) {
        set_if_not_nan(target(i), value(i));
    }
};

// symmetric component matrix
inline ComplexTensor<asymmetric_t> get_sym_matrix() {
    ComplexTensor<asymmetric_t> m;
    m << 1.0, 1.0, 1.0, 1.0, a2, a, 1.0, a, a2;
    return m;
}
inline ComplexTensor<asymmetric_t> get_sym_matrix_inv() {
    ComplexTensor<asymmetric_t> m;
    m << 1.0, 1.0, 1.0, 1.0, a, a2, 1.0, a2, a;
    m = m / 3.0;
    return m;
}

// conjugate (hermitian) transpose
template <std::floating_point Float> constexpr std::complex<Float> hermitian_transpose(std::complex<Float> const& z) {
    return conj(z);
}
template <std::floating_point Float> constexpr Float hermitian_transpose(Float x) { return x; }
template <rk2_tensor Derived> inline auto hermitian_transpose(Eigen::ArrayBase<Derived> const& x) {
    return x.matrix().adjoint().array();
}

// vector of values
template <symmetry_tag sym> using RealValueVector = std::vector<RealValue<sym>>;
template <symmetry_tag sym> using ComplexValueVector = std::vector<ComplexValue<sym>>;
template <symmetry_tag sym> using RealTensorVector = std::vector<RealTensor<sym>>;
template <symmetry_tag sym> using ComplexTensorVector = std::vector<ComplexTensor<sym>>;

} // namespace power_grid_model
