// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include "doctest/doctest.h"
#include "power_grid_model//component/fault.hpp"

namespace power_grid_model {

TEST_CASE("Test fault") {
    Fault fault{{{1}, 2, 3.0, 4.0}};
    CHECK(fault.math_model_type() == ComponentType::fault);
    CHECK(fault.get_fault_object() == 2);

    SUBCASE("Test calc_param") {
        // Not connected to source
        FaultCalcParam param = fault.calc_param(400.0, false);
        CHECK(param.math_fault_object == -1);
        CHECK(cabs(param.y_fault) == doctest::Approx(0.0));

        // Connected to source
        param = fault.calc_param(400.0);
        double const base_y = base_power_3p / (400.0 * 400.0);
        DoubleComplex y_f = 1.0 / (3.0 + 1.0i * 4.0) / base_y;
        CHECK(param.math_fault_object == -1);
        CHECK(cabs(param.y_fault) == doctest::Approx(cabs(y_f)));
    }

    SUBCASE("Test get_null_output") {
        FaultOutput output = fault.get_null_output();
        CHECK(output.id == 1);
        CHECK(!output.energized);
    }

    SUBCASE("Test get_output") {
        FaultOutput output = fault.get_output();
        CHECK(output.id == 1);
        CHECK(!output.energized);
    }

    SUBCASE("Test get_short_circuit_output sym") {
        ComplexValue<true> i_f_pu = 1.0 + 1.0i;
        double const base_i = base_power_3p / (400.0 * sqrt3);
        ComplexValue<true> i_f = i_f_pu * base_i;

        FaultShortCircuitOutput<true> output = fault.get_short_circuit_output<true>(i_f_pu, 400.0);
        CHECK(output.id == 1);
        CHECK(output.energized);
        CHECK(output.i_f == doctest::Approx(cabs(i_f)));
        CHECK(output.i_f_angle == doctest::Approx(0.25 * pi));
    }

    SUBCASE("Test get_short_circuit_output asym") {
        ComplexValue<false> i_f_pu{};
        i_f_pu << DoubleComplex(1.0, 1.0), DoubleComplex(0.0, 1.0), DoubleComplex(1.0, 0.0);
        double const base_i = base_power_3p / (400.0 * sqrt3);
        ComplexValue<false> i_f = i_f_pu * base_i;

        FaultShortCircuitOutput<false> output = fault.get_short_circuit_output<false>(i_f_pu, 400.0);
        CHECK(output.id == 1);
        CHECK(output.energized);
        CHECK((output.i_f - cabs(i_f) < numerical_tolerance).all());
        CHECK((output.i_f_angle - arg(i_f) < numerical_tolerance).all());
    }

    SUBCASE("Test energized") {
        CHECK(fault.energized(true));
        CHECK(!fault.energized(false));
    }

    SUBCASE("Test update") {
        FaultUpdate fault_update{{1}, 10};
        UpdateChange updated = fault.update(fault_update);

        CHECK(!updated.param);
        CHECK(!updated.topo);
        CHECK(fault.get_fault_object() == 10);
    }
}

}  // namespace power_grid_model