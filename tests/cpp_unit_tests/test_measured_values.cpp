// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/math_solver/iterative_linear_se_solver.hpp>

#include <doctest/doctest.h>

namespace power_grid_model::math_solver {
namespace {
template <symmetry_tag sym> void check_close(auto const& x, auto const& y, auto const& tolerance) {
    if constexpr (is_symmetric_v<sym>) {
        CHECK(cabs((x) - (y)) < (tolerance));
    } else {
        CHECK((cabs((x) - (y)) < (tolerance)).all());
    }
}

template <symmetry_tag sym> void check_close(auto const& x, auto const& y) {
    check_close<sym>(x, y, numerical_tolerance);
}
void check_close(auto const& x, auto const& y, auto const& tolerance) { check_close<symmetric_t>(x, y, tolerance); }
void check_close(auto const& x, auto const& y) { check_close<symmetric_t>(x, y); }
} // namespace

TEST_CASE("Measured Values") {
    SUBCASE("Accumulate single injection power sensor - sym") {
        auto topo = MathModelTopology{};
        topo.phase_shift = {0.0};
        topo.shunts_per_bus = {from_dense, {}, 1};
        topo.load_gens_per_bus = {from_dense, {0}, 1};
        topo.sources_per_bus = {from_dense, {}, 1};
        topo.power_sensors_per_load_gen = {from_dense, {0}, 1};

        StateEstimationInput<symmetric_t> input{};
        input.measured_load_gen_power = {
            {.real_component = {.value = 1.0, .variance = 0.3}, .imag_component = {.value = 0.1, .variance = 0.1}}};
        input.load_gen_status = {1};

        MeasuredValues<symmetric_t> const values{std::make_shared<MathModelTopology const>(std::move(topo)), input};

        REQUIRE(values.has_bus_injection(0));
        auto const& injection = values.bus_injection(0);
        check_close(injection.value(), 1.0 + 0.1i);
        check_close(injection.real_component.variance, 0.75);
        check_close(injection.imag_component.variance, 0.25);
    }

    SUBCASE("Accumulate single injection power sensor - asym") {
        auto topo = MathModelTopology{};
        topo.phase_shift = {0.0};
        topo.shunts_per_bus = {from_dense, {}, 1};
        topo.load_gens_per_bus = {from_dense, {0}, 1};
        topo.sources_per_bus = {from_dense, {}, 1};
        topo.power_sensors_per_load_gen = {from_dense, {0}, 1};

        StateEstimationInput<asymmetric_t> input{};
        input.measured_load_gen_power = {{.real_component = {.value = {1.0, 1.1, 1.2}, .variance = {0.3, 0.6, 0.65}},
                                          .imag_component = {.value = {0.0, 0.1, -0.2}, .variance = {0.1, 0.2, 0.05}}}};
        input.load_gen_status = {1};

        MeasuredValues<asymmetric_t> const values{std::make_shared<MathModelTopology const>(std::move(topo)), input};

        REQUIRE(values.has_bus_injection(0));
        auto const& injection = values.bus_injection(0);
        check_close<asymmetric_t>(injection.value(), ComplexValue<asymmetric_t>{1.0 + 0.0i, 1.1 + 0.1i, 1.2 - 0.2i});
        check_close<asymmetric_t>(injection.real_component.variance, RealValue<asymmetric_t>{0.75, 1.5, 1.625});
        check_close<asymmetric_t>(injection.imag_component.variance, RealValue<asymmetric_t>{0.25, 0.5, 0.125});
    }

    SUBCASE("Accumulate two power sensors on same injection - sym") {
        auto topo = MathModelTopology{};
        topo.phase_shift = {0.0};
        topo.shunts_per_bus = {from_dense, {}, 1};
        topo.load_gens_per_bus = {from_dense, {0}, 1};
        topo.sources_per_bus = {from_dense, {}, 1};
        topo.power_sensors_per_load_gen = {from_dense, {0, 0}, 1};

        StateEstimationInput<symmetric_t> input{};
        input.measured_load_gen_power = {
            {.real_component = {.value = 1.0, .variance = 1.0}, .imag_component = {.value = 1.5, .variance = 5.0}},
            {.real_component = {.value = 4.0, .variance = 2.0}, .imag_component = {.value = 0.7, .variance = 3.0}}};
        input.load_gen_status = {1};

        MeasuredValues<symmetric_t> const values{std::make_shared<MathModelTopology const>(std::move(topo)), input};

        REQUIRE(values.has_bus_injection(0));
        auto const& injection = values.bus_injection(0);
        check_close(injection.value(), 2.0 + 1.0i);
        check_close(injection.real_component.variance, 16.0 / 61.0);
        check_close(injection.imag_component.variance, 45.0 / 61.0);
    }

    SUBCASE("Accumulate power sensors on two injections on same bus - sym") {
        auto topo = MathModelTopology{};
        topo.phase_shift = {0.0};
        topo.shunts_per_bus = {from_dense, {}, 1};
        topo.load_gens_per_bus = {from_dense, {0, 0}, 1};
        topo.sources_per_bus = {from_dense, {}, 1};
        topo.power_sensors_per_load_gen = {from_dense, {0, 1}, 1};

        StateEstimationInput<symmetric_t> input{};
        input.measured_load_gen_power = {
            {.real_component = {.value = 1.0, .variance = 1.0}, .imag_component = {.value = 1.5, .variance = 4.0}},
            {.real_component = {.value = 4.0, .variance = 2.0}, .imag_component = {.value = 0.7, .variance = 3.0}}};
        input.load_gen_status = {1, 1};

        MeasuredValues<symmetric_t> const values{std::make_shared<MathModelTopology const>(std::move(topo)), input};

        REQUIRE(values.has_bus_injection(0));
        auto const& injection = values.bus_injection(0);
        check_close(injection.value(), 5.0 + 2.2i);
        check_close(injection.real_component.variance, 0.3);
        check_close(injection.imag_component.variance, 0.7);
    }

    SUBCASE("Accumulate power sensors on two injections on same bus - asym") {
        auto topo = MathModelTopology{};
        topo.phase_shift = {0.0};
        topo.shunts_per_bus = {from_dense, {}, 1};
        topo.load_gens_per_bus = {from_dense, {0, 0}, 1};
        topo.sources_per_bus = {from_dense, {}, 1};
        topo.power_sensors_per_load_gen = {from_dense, {0, 1}, 1};

        StateEstimationInput<asymmetric_t> input{};
        input.measured_load_gen_power = {{.real_component = {.value = {1.0, 0.5, 2.0}, .variance = {1.0, 0.5, 3.0}},
                                          .imag_component = {.value = {1.5, 1.0, 0.5}, .variance = {4.0, 3.5, 1.5}}},
                                         {.real_component = {.value = {4.0, -0.4, -1.2}, .variance = {2.0, 1.5, 5.5}},
                                          .imag_component = {.value = {0.7, -0.8, 2.5}, .variance = {3.0, 5.0, 0.5}}}};
        input.load_gen_status = {1, 1};

        MeasuredValues<asymmetric_t> const values{std::make_shared<MathModelTopology const>(std::move(topo)), input};

        REQUIRE(values.has_bus_injection(0));
        auto const& injection = values.bus_injection(0);
        check_close<asymmetric_t>(injection.value(), ComplexValue<asymmetric_t>{5.0 + 2.2i, 0.1 + 0.2i, 0.8 + 3.0i});
        check_close<asymmetric_t>(injection.real_component.variance, RealValue<asymmetric_t>{0.3, 0.2, 0.85});
        check_close<asymmetric_t>(injection.imag_component.variance, RealValue<asymmetric_t>{0.7, 0.85, 0.2});
    }

    SUBCASE("Accumulate branch flow sensors") {
        using enum AngleMeasurementType;

        auto const get_single_branch_topology = [] {
            auto topo = MathModelTopology{};
            topo.phase_shift = {0.0, 0.0};
            topo.branch_bus_idx = {{0, 1}};
            topo.shunts_per_bus = {from_dense, {}, 1};
            topo.load_gens_per_bus = {from_dense, {}, 1};
            topo.sources_per_bus = {from_dense, {}, 1};
            return topo;
        };
        DecomposedComplexRandVar<symmetric_t> const measurement_a{.real_component = {.value = 1.0, .variance = 1.0},
                                                                  .imag_component = {.value = 1.5, .variance = 4.0}};
        DecomposedComplexRandVar<symmetric_t> const measurement_b{.real_component = {.value = 4.0, .variance = 2.0},
                                                                  .imag_component = {.value = 0.7, .variance = 3.0}};
        auto const check_accumulated = [](DecomposedComplexRandVar<symmetric_t> const& value) {
            check_close(value.value(), 2.0 + (73.0i / 70.0));
            check_close(value.real_component.variance, 7.0 / 25.0);
            check_close(value.imag_component.variance, 18.0 / 25.0);
        };

        SUBCASE("Accumulate power sensors on branch - sym") {
            auto topo = get_single_branch_topology();
            topo.power_sensors_per_branch_from = {from_dense, {0, 0}, 1};

            StateEstimationInput<symmetric_t> input{};
            input.measured_branch_from_power = {measurement_a, measurement_b};

            MeasuredValues<symmetric_t> const values{std::make_shared<MathModelTopology const>(std::move(topo)), input};

            REQUIRE(values.has_branch_from_power(0));
            CHECK_FALSE(values.has_branch_from_current(0));
            CHECK_FALSE(values.has_branch_to_power(0));
            CHECK_FALSE(values.has_branch_to_current(0));

            check_accumulated(values.branch_from_power(0));
        }

        SUBCASE("Accumulate local angle current sensors on branch - sym") {
            auto topo = get_single_branch_topology();
            topo.current_sensors_per_branch_from = {from_dense, {0, 0}, 1};

            StateEstimationInput<symmetric_t> input{};
            input.measured_branch_from_current = {
                {.angle_measurement_type = local_angle, .measurement = measurement_a},
                {.angle_measurement_type = local_angle, .measurement = measurement_b}};

            MeasuredValues<symmetric_t> const values{std::make_shared<MathModelTopology const>(std::move(topo)), input};

            REQUIRE(values.has_branch_from_current(0));
            CHECK_FALSE(values.has_branch_from_power(0));
            CHECK_FALSE(values.has_branch_to_power(0));
            CHECK_FALSE(values.has_branch_to_current(0));

            CHECK(values.branch_from_current(0).angle_measurement_type == local_angle);
            check_accumulated(values.branch_from_current(0).measurement);
        }

        SUBCASE("Accumulate global angle current sensors on branch - sym") {
            auto topo = get_single_branch_topology();
            topo.current_sensors_per_branch_from = {from_dense, {0, 0}, 1};
            topo.voltage_sensors_per_bus = {from_dense, {0}, 2};

            StateEstimationInput<symmetric_t> input{};
            input.measured_voltage = {{.value = DoubleComplex{1.0, 1.0}, .variance = 10.0}};
            input.measured_branch_from_current = {
                {.angle_measurement_type = global_angle, .measurement = measurement_a},
                {.angle_measurement_type = global_angle, .measurement = measurement_b}};

            MeasuredValues<symmetric_t> const values{std::make_shared<MathModelTopology const>(std::move(topo)), input};

            REQUIRE(values.has_branch_from_current(0));
            CHECK_FALSE(values.has_branch_from_power(0));
            CHECK_FALSE(values.has_branch_to_power(0));
            CHECK_FALSE(values.has_branch_to_current(0));

            CHECK(values.branch_from_current(0).angle_measurement_type == global_angle);
            check_accumulated(values.branch_from_current(0).measurement);
        }

        SUBCASE("Cannot accumulate different angle measurement types on same terminal") {
            auto topo = get_single_branch_topology();
            topo.current_sensors_per_branch_from = {from_dense, {0, 0}, 1};

            StateEstimationInput<symmetric_t> input{};
            input.measured_branch_from_current = {
                {.angle_measurement_type = AngleMeasurementType::local_angle, .measurement = measurement_a},
                {.angle_measurement_type = AngleMeasurementType::global_angle, .measurement = measurement_b}};

            auto const create_measured_values = [&topo, &input] {
                return MeasuredValues<symmetric_t>{std::make_shared<MathModelTopology const>(topo), input};
            };
            CHECK_THROWS_AS(create_measured_values(), ConflictingAngleMeasurementType);
        }
    }
}

} // namespace power_grid_model::math_solver
