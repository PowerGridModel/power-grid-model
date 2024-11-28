// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

// In this unit test the powerflow solvers are tested

#include <power_grid_model/calculation_parameters.hpp>
#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/common/three_phase_tensor.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {
template <symmetry_tag sym> inline void check_close(auto const& x, auto const& y, auto const& tolerance) {
    if constexpr (is_symmetric_v<sym>) {
        CHECK(cabs((x) - (y)) < (tolerance));
    } else {
        CHECK((cabs((x) - (y)) < (tolerance)).all());
    }
}

template <symmetry_tag sym> inline void check_close(auto const& x, auto const& y) {
    check_close<sym>(x, y, numerical_tolerance);
}
inline void check_close(auto const& x, auto const& y, auto const& tolerance) {
    check_close<symmetric_t>(x, y, tolerance);
}
inline void check_close(auto const& x, auto const& y) { check_close<symmetric_t>(x, y); }

template <symmetry_tag sym>
inline void assert_output(SolverOutput<sym> const& output, SolverOutput<sym> const& output_ref,
                          bool normalize_phase = false, double tolerance = numerical_tolerance) {
    DoubleComplex const phase_offset = normalize_phase ? std::exp(1.0i / 180.0 * pi) : 1.0;
    for (size_t i = 0; i != output.u.size(); ++i) {
        check_close<sym>(output.u[i], output_ref.u[i] * phase_offset, tolerance);
    }
    for (size_t i = 0; i != output.bus_injection.size(); ++i) {
        check_close<sym>(output.bus_injection[i], output_ref.bus_injection[i], tolerance);
    }
    for (size_t i = 0; i != output.branch.size(); ++i) {
        check_close<sym>(output.branch[i].s_f, output_ref.branch[i].s_f, tolerance);
        check_close<sym>(output.branch[i].s_t, output_ref.branch[i].s_t, tolerance);
        check_close<sym>(output.branch[i].i_f, output_ref.branch[i].i_f * phase_offset, tolerance);
        check_close<sym>(output.branch[i].i_t, output_ref.branch[i].i_t * phase_offset, tolerance);
    }
    for (size_t i = 0; i != output.source.size(); ++i) {
        check_close<sym>(output.source[i].s, output_ref.source[i].s, tolerance);
        check_close<sym>(output.source[i].i, output_ref.source[i].i * phase_offset, tolerance);
    }
    for (size_t i = 0; i != output.load_gen.size(); ++i) {
        check_close<sym>(output.load_gen[i].s, output_ref.load_gen[i].s, tolerance);
        check_close<sym>(output.load_gen[i].i, output_ref.load_gen[i].i * phase_offset, tolerance);
    }
    for (size_t i = 0; i != output.shunt.size(); ++i) {
        check_close<sym>(output.shunt[i].s, output_ref.shunt[i].s, tolerance);
        check_close<sym>(output.shunt[i].i, output_ref.shunt[i].i * phase_offset, tolerance);
    }
}

} // namespace power_grid_model
