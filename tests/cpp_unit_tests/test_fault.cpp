// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/component/fault.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {

TEST_CASE("Test fault") {
    Fault fault{{{1}, 1, FaultType::two_phase_to_ground, FaultPhase::ab, 4, 3.0, 4.0}};
    CHECK(fault.math_model_type() == ComponentType::fault);
    CHECK(fault.status());
    CHECK(fault.get_fault_type() == FaultType::two_phase_to_ground);
    CHECK(fault.get_fault_phase() == FaultPhase::ab);
    CHECK(fault.get_fault_object() == 4);

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
        CHECK(param.fault_type == FaultType::two_phase_to_ground);
        CHECK(param.fault_phase == FaultPhase::ab);
    }

    SUBCASE("Test calc param with nan impedance input") {
        Fault fault_nan_imp{{{1}, 1, FaultType::two_phase_to_ground, FaultPhase::ab, 4, nan, nan}};
        FaultCalcParam param = fault_nan_imp.calc_param(u_rated);
        CHECK(std::isinf(param.y_fault.real()));
        CHECK(std::isinf(param.y_fault.imag()));
        CHECK(param.fault_type == FaultType::two_phase_to_ground);
        CHECK(param.fault_phase == FaultPhase::ab);
    }

    SUBCASE("Test calc param with other fault type") {
        Fault fault_nan_imp{{{1}, 1, FaultType::three_phase, FaultPhase::abc, 4, nan, nan}};
        FaultCalcParam param = fault_nan_imp.calc_param(u_rated);
        CHECK(std::isinf(param.y_fault.real()));
        CHECK(std::isinf(param.y_fault.imag()));
        CHECK(param.fault_type == FaultType::three_phase);
        CHECK(param.fault_phase == FaultPhase::abc);
    }

    SUBCASE("Test calc param with nan fault type") {
        Fault fault_nan_imp{{{1}, 1, FaultType::nan, FaultPhase::nan, 4, nan, nan}};
        CHECK_THROWS_AS((fault_nan_imp.calc_param(u_rated)), InvalidShortCircuitType);
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
        ComplexValue<false> const i_f_res{i_f_pu};
        FaultShortCircuitOutput output = fault.get_sc_output(i_f_pu, u_rated);
        CHECK(output.id == 1);
        CHECK(output.energized);
        CHECK((output.i_f - cabs(i_f_res) * base_i < numerical_tolerance).all());
        CHECK((output.i_f_angle - arg(i_f_res) < numerical_tolerance).all());
    }

    SUBCASE("Test get_short_circuit_output asym") {
        ComplexValue<false> i_f_pu{};
        i_f_pu << DoubleComplex(1.0, 1.0), DoubleComplex(0.0, 1.0), DoubleComplex(1.0, 0.0);
        FaultShortCircuitOutput output = fault.get_sc_output(i_f_pu, u_rated);
        CHECK(output.id == 1);
        CHECK(output.energized);
        CHECK((output.i_f - cabs(i_f_pu) * base_i < numerical_tolerance).all());
        CHECK((output.i_f_angle - arg(i_f_pu) < numerical_tolerance).all());
    }

    SUBCASE("Test energized") {
        CHECK(fault.energized(true));
        CHECK(!fault.energized(false));
    }

    SUBCASE("Check fault type getter") {
        using enum FaultType;

        auto create_fault = [](FaultType fault_type) {
            return Fault{{{1}, 1, fault_type, FaultPhase::nan, 4, 3.0, 4.0}};
        };

        CHECK((create_fault(three_phase).get_fault_type()) == three_phase);
        CHECK((create_fault(single_phase_to_ground).get_fault_type()) == single_phase_to_ground);
        CHECK((create_fault(two_phase).get_fault_type()) == two_phase);
        CHECK((create_fault(two_phase_to_ground).get_fault_type()) == two_phase_to_ground);
        CHECK_THROWS_AS((create_fault(FaultType::nan).get_fault_type()), InvalidShortCircuitType);
    }

    SUBCASE("Check fault phase getter") {
        using enum FaultPhase;

        auto create_fault = [](FaultType fault_type, FaultPhase fault_phase) {
            return Fault{{{1}, 1, fault_type, fault_phase, 4, 3.0, 4.0}};
        };

        SUBCASE("Fault phase fully specified") {
            CHECK(create_fault(FaultType::three_phase, abc).get_fault_phase() == abc);
            CHECK(create_fault(FaultType::single_phase_to_ground, a).get_fault_phase() == a);
            CHECK(create_fault(FaultType::single_phase_to_ground, b).get_fault_phase() == b);
            CHECK(create_fault(FaultType::single_phase_to_ground, c).get_fault_phase() == c);
            CHECK(create_fault(FaultType::two_phase, ab).get_fault_phase() == ab);
            CHECK(create_fault(FaultType::two_phase, ac).get_fault_phase() == ac);
            CHECK(create_fault(FaultType::two_phase, bc).get_fault_phase() == bc);
            CHECK(create_fault(FaultType::two_phase_to_ground, ab).get_fault_phase() == ab);
            CHECK(create_fault(FaultType::two_phase_to_ground, ac).get_fault_phase() == ac);
            CHECK(create_fault(FaultType::two_phase_to_ground, bc).get_fault_phase() == bc);
            CHECK((create_fault(FaultType::nan, abc).get_fault_phase()) == abc);
            CHECK((create_fault(FaultType::nan, a).get_fault_phase()) == a);
            CHECK((create_fault(FaultType::nan, b).get_fault_phase()) == b);
            CHECK((create_fault(FaultType::nan, c).get_fault_phase()) == c);
            CHECK((create_fault(FaultType::nan, ab).get_fault_phase()) == ab);
            CHECK((create_fault(FaultType::nan, ac).get_fault_phase()) == ac);
            CHECK((create_fault(FaultType::nan, bc).get_fault_phase()) == bc);
        }

        SUBCASE("Fault phase not specified") {
            for (auto fault_phase : {default_value, FaultPhase::nan}) {
                CHECK(create_fault(FaultType::three_phase, fault_phase).get_fault_phase() == abc);
                CHECK(create_fault(FaultType::single_phase_to_ground, fault_phase).get_fault_phase() == a);
                CHECK(create_fault(FaultType::two_phase, fault_phase).get_fault_phase() == bc);
                CHECK(create_fault(FaultType::two_phase_to_ground, fault_phase).get_fault_phase() == bc);
                CHECK_THROWS_AS((create_fault(FaultType::nan, fault_phase).get_fault_phase()), InvalidShortCircuitType);
            }
        }
    }

    SUBCASE("Test update") {
        FaultUpdate const fault_update{{1}, 0, FaultType::two_phase, FaultPhase::ac, 10};
        UpdateChange updated = fault.update(fault_update);

        CHECK(!updated.param);
        CHECK(!updated.topo);

        CHECK_FALSE(fault.status());
        CHECK(fault.get_fault_type() == FaultType::two_phase);
        CHECK(fault.get_fault_phase() == FaultPhase::ac);
        CHECK(fault.get_fault_object() == 10);

        // update without updating
        FaultUpdate const fault_update_nan{{1}, na_IntS, FaultType::nan, FaultPhase::nan, na_IntID};
        fault.update(fault_update_nan);
        CHECK_FALSE(fault.status());
        CHECK(fault.get_fault_type() == FaultType::two_phase);
        CHECK(fault.get_fault_phase() == FaultPhase::ac);
        CHECK(fault.get_fault_object() == 10);

        // default value does override
        FaultUpdate const fault_update_default_value{{1}, na_IntS, FaultType::nan, FaultPhase::default_value, na_IntID};
        fault.update(fault_update_default_value);
        CHECK_FALSE(fault.status());
        CHECK(fault.get_fault_type() == FaultType::two_phase);
        CHECK(fault.get_fault_phase() == FaultPhase::bc);  // bc is the default value for two_phase fault type
        CHECK(fault.get_fault_object() == 10);
    }

    SUBCASE("Check fault type/phase combination") {
        using enum FaultPhase;

        auto check_allowed = [&fault](FaultType fault_type, FaultPhase fault_phase) {
            CAPTURE(fault_type);
            CAPTURE(fault_phase);
            CHECK_NOTHROW((Fault{{{1}, 1, fault_type, fault_phase, 4, 3.0, 4.0}}));

            FaultUpdate const fault_update{{1}, 0, fault_type, fault_phase, 10};
            CHECK_NOTHROW(fault.update(fault_update));
        };

        auto check_not_allowed = [&fault](FaultType fault_type, FaultPhase fault_phase) {
            CAPTURE(fault_type);
            CAPTURE(fault_phase);
            CHECK_THROWS_AS((Fault{{{1}, 1, fault_type, fault_phase, 4, 3.0, 4.0}}), InvalidShortCircuitPhases);

            FaultUpdate const fault_update{{1}, 0, fault_type, fault_phase, 10};
            CHECK_THROWS_AS(fault.update(fault_update), InvalidShortCircuitPhases);
        };

        SUBCASE("Three phase fault type") {
            FaultType const fault_type = FaultType::three_phase;
            check_allowed(fault_type, default_value);
            check_allowed(fault_type, FaultPhase::nan);
            check_allowed(fault_type, abc);

            check_not_allowed(fault_type, a);
            check_not_allowed(fault_type, b);
            check_not_allowed(fault_type, c);
            check_not_allowed(fault_type, ab);
            check_not_allowed(fault_type, ac);
            check_not_allowed(fault_type, bc);
        }

        SUBCASE("Single phase to ground fault type") {
            FaultType const fault_type = FaultType::single_phase_to_ground;
            check_allowed(fault_type, default_value);
            check_allowed(fault_type, FaultPhase::nan);
            check_allowed(fault_type, a);
            check_allowed(fault_type, b);
            check_allowed(fault_type, c);

            check_not_allowed(fault_type, abc);
            check_not_allowed(fault_type, ab);
            check_not_allowed(fault_type, ac);
            check_not_allowed(fault_type, bc);
        }

        SUBCASE("Two phase fault type") {
            FaultType const fault_type = FaultType::two_phase;
            check_allowed(fault_type, default_value);
            check_allowed(fault_type, FaultPhase::nan);
            check_allowed(fault_type, ab);
            check_allowed(fault_type, ac);
            check_allowed(fault_type, bc);

            check_not_allowed(fault_type, abc);
            check_not_allowed(fault_type, a);
            check_not_allowed(fault_type, b);
            check_not_allowed(fault_type, c);
        }

        SUBCASE("Two phase to ground fault type") {
            FaultType const fault_type = FaultType::two_phase_to_ground;
            check_allowed(fault_type, default_value);
            check_allowed(fault_type, FaultPhase::nan);
            check_allowed(fault_type, ab);
            check_allowed(fault_type, ac);
            check_allowed(fault_type, bc);

            check_not_allowed(fault_type, abc);
            check_not_allowed(fault_type, a);
            check_not_allowed(fault_type, b);
            check_not_allowed(fault_type, c);
        }

        SUBCASE("Invalid fault type") {
            CHECK_THROWS_AS((Fault{{{1}, 1, static_cast<FaultType>(-127), FaultPhase::nan, 4, 3.0, 4.0}}),
                            InvalidShortCircuitType);

            FaultUpdate const fault_update{{1}, 0, static_cast<FaultType>(-127), FaultPhase::nan, 10};
            CHECK_THROWS_AS(fault.update(fault_update), InvalidShortCircuitType);
        }
    }
}

}  // namespace power_grid_model