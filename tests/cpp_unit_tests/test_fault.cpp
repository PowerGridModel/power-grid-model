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

    double const u_rated = 400.0;
    double const base_i = base_power_3p / (u_rated * sqrt3);

    SUBCASE("Test calc_param") {
        // Not connected to source
        FaultCalcParam param = fault.calc_param(u_rated, false);
        CHECK(param.math_fault_object == -1);
        CHECK(cabs(param.y_fault) == doctest::Approx(0.0));

        // Connected to source
        param = fault.calc_param(u_rated);
        double const base_y = base_i / (u_rated / sqrt(3));
        DoubleComplex const y_f = 1.0 / (3.0 + 1.0i * 4.0) / base_y;
        CHECK(param.math_fault_object == -1);
        CHECK(cabs(param.y_fault) == doctest::Approx(cabs(y_f)));
    }

    SUBCASE("Test calc param with nan impedance input") {
        Fault fault_nan_imp{{{1}, 2}};
        FaultCalcParam param = fault_nan_imp.calc_param(u_rated);
        CHECK(std::isinf(param.y_fault.real()));
        CHECK(std::isinf(param.y_fault.imag()));
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
        ComplexValue<true> const i_f_pu = 1.0 + 1.0i;
        ComplexValue<true> const i_f = i_f_pu * base_i;

        FaultShortCircuitOutput<true> output = fault.get_short_circuit_output<true>(i_f_pu, u_rated);
        CHECK(output.id == 1);
        CHECK(output.energized);
        CHECK(output.i_f == doctest::Approx(cabs(i_f)));
        CHECK(output.i_f_angle == doctest::Approx(0.25 * pi));
    }

    SUBCASE("Test get_short_circuit_output asym") {
        ComplexValue<false> i_f_pu{};
        i_f_pu << DoubleComplex(1.0, 1.0), DoubleComplex(0.0, 1.0), DoubleComplex(1.0, 0.0);
        ComplexValue<false> i_f = i_f_pu * base_i;

        FaultShortCircuitOutput<false> output = fault.get_short_circuit_output<false>(i_f_pu, u_rated);
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
        FaultUpdate const fault_update{{1}, 10};
        UpdateChange updated = fault.update(fault_update);

        CHECK(!updated.param);
        CHECK(!updated.topo);
        CHECK(fault.get_fault_object() == 10);
    }
}

}  // namespace power_grid_model