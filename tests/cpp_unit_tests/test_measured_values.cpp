// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/math_solver/iterative_linear_se_solver.hpp>

#include <doctest/doctest.h>

namespace power_grid_model::math_solver {
namespace {
template <bool sym> void check_close(auto const& x, auto const& y, auto const& tolerance) {
    if constexpr (sym) {
        CHECK(cabs((x) - (y)) < (tolerance));
    } else {
        CHECK((cabs((x) - (y)) < (tolerance)).all());
    }
}

template <bool sym> void check_close(auto const& x, auto const& y) { check_close<sym>(x, y, numerical_tolerance); }
void check_close(auto const& x, auto const& y, auto const& tolerance) { check_close<true>(x, y, tolerance); }
void check_close(auto const& x, auto const& y) { check_close<true>(x, y); }
} // namespace

TEST_CASE("Measured Values") {
    SUBCASE("Accumulate single injection power sensor - sym") {
        auto topo = MathModelTopology{};
        topo.phase_shift = {0.0};
        topo.shunts_per_bus = {from_dense, {}, 1};
        topo.load_gens_per_bus = {from_dense, {0}, 1};
        topo.sources_per_bus = {from_dense, {}, 1};
        topo.power_sensors_per_load_gen = {from_dense, {0}, 1};

        StateEstimationInput<true> input{};
        input.measured_load_gen_power = {{1.0 + 0.1i, 0.3, 0.1}};
        input.load_gen_status = {1};

        MeasuredValues<true> const values{std::make_shared<MathModelTopology const>(std::move(topo)), input};

        CHECK(values.has_bus_injection(0));
        auto const& injection = values.bus_injection(0);
        check_close(injection.value, 1.0 + 0.1i);
        check_close(injection.p_variance, 0.75);
        check_close(injection.q_variance, 0.25);
    }

    SUBCASE("Accumulate single injection power sensor - asym") {
        auto topo = MathModelTopology{};
        topo.phase_shift = {0.0};
        topo.shunts_per_bus = {from_dense, {}, 1};
        topo.load_gens_per_bus = {from_dense, {0}, 1};
        topo.sources_per_bus = {from_dense, {}, 1};
        topo.power_sensors_per_load_gen = {from_dense, {0}, 1};

        StateEstimationInput<false> input{};
        input.measured_load_gen_power = {{{1.0, 1.1 + 0.1i, 1.2 - 0.2i}, {0.3, 0.6, 0.65}, {0.1, 0.2, 0.05}}};
        input.load_gen_status = {1};

        MeasuredValues<false> const values{std::make_shared<MathModelTopology const>(std::move(topo)), input};

        CHECK(values.has_bus_injection(0));
        auto const& injection = values.bus_injection(0);
        check_close<false>(injection.value, ComplexValue<false>{1.0 + 0.0i, 1.1 + 0.1i, 1.2 - 0.2i});
        check_close<false>(injection.p_variance, RealValue<false>{0.75, 1.5, 1.625});
        check_close<false>(injection.q_variance, RealValue<false>{0.25, 0.5, 0.125});
    }

    SUBCASE("Accumulate two power sensors on same injection - sym") {
        auto topo = MathModelTopology{};
        topo.phase_shift = {0.0};
        topo.shunts_per_bus = {from_dense, {}, 1};
        topo.load_gens_per_bus = {from_dense, {0}, 1};
        topo.sources_per_bus = {from_dense, {}, 1};
        topo.power_sensors_per_load_gen = {from_dense, {0, 0}, 1};

        StateEstimationInput<true> input{};
        input.measured_load_gen_power = {{1.0 + 1.5i, 1.0, 5.0}, {4.0 + 0.7i, 2.0, 3.0}};
        input.load_gen_status = {1};

        MeasuredValues<true> const values{std::make_shared<MathModelTopology const>(std::move(topo)), input};

        CHECK(values.has_bus_injection(0));
        auto const& injection = values.bus_injection(0);
        check_close(injection.value, 2.0 + 1.0i);
        check_close(injection.p_variance, 16.0 / 61.0);
        check_close(injection.q_variance, 45.0 / 61.0);
    }

    SUBCASE("Accumulate power sensors on two injections on same bus - sym") {
        auto topo = MathModelTopology{};
        topo.phase_shift = {0.0};
        topo.shunts_per_bus = {from_dense, {}, 1};
        topo.load_gens_per_bus = {from_dense, {0, 0}, 1};
        topo.sources_per_bus = {from_dense, {}, 1};
        topo.power_sensors_per_load_gen = {from_dense, {0, 1}, 1};

        StateEstimationInput<true> input{};
        input.measured_load_gen_power = {{1.0 + 1.5i, 1.0, 4.0}, {4.0 + 0.7i, 2.0, 3.0}};
        input.load_gen_status = {1, 1};

        MeasuredValues<true> const values{std::make_shared<MathModelTopology const>(std::move(topo)), input};

        CHECK(values.has_bus_injection(0));
        auto const& injection = values.bus_injection(0);
        check_close(injection.value, 5.0 + 2.2i);
        check_close(injection.p_variance, 0.3);
        check_close(injection.q_variance, 0.7);
    }

    SUBCASE("Accumulate power sensors on two injections on same bus - asym") {
        auto topo = MathModelTopology{};
        topo.phase_shift = {0.0};
        topo.shunts_per_bus = {from_dense, {}, 1};
        topo.load_gens_per_bus = {from_dense, {0, 0}, 1};
        topo.sources_per_bus = {from_dense, {}, 1};
        topo.power_sensors_per_load_gen = {from_dense, {0, 1}, 1};

        StateEstimationInput<false> input{};
        input.measured_load_gen_power = {{{1.0 + 1.5i, 0.5 + 1.0i, 2.0 + 0.5i}, {1.0, 0.5, 3.0}, {4.0, 3.5, 1.5}},
                                         {{4.0 + 0.7i, -0.4 - 0.8i, -1.2 + 2.5i}, {2.0, 1.5, 5.5}, {3.0, 5.0, 0.5}}};
        input.load_gen_status = {1, 1};

        MeasuredValues<false> const values{std::make_shared<MathModelTopology const>(std::move(topo)), input};

        CHECK(values.has_bus_injection(0));
        auto const& injection = values.bus_injection(0);
        check_close<false>(injection.value, ComplexValue<false>{5.0 + 2.2i, 0.1 + 0.2i, 0.8 + 3.0i});
        check_close<false>(injection.p_variance, RealValue<false>{0.3, 0.2, 0.85});
        check_close<false>(injection.q_variance, RealValue<false>{0.7, 0.85, 0.2});
    }
}

} // namespace power_grid_model::math_solver