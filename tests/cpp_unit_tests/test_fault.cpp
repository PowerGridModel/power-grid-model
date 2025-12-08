// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/component/fault.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {
namespace {
void check_nan_preserving_equality(std::floating_point auto actual, std::floating_point auto expected) {
    if (is_nan(expected)) {
        is_nan(actual);
    } else {
        CHECK(actual == doctest::Approx(expected));
    }
}
} // namespace

TEST_CASE("Test fault") {
    Fault fault{{.id = 1,
                 .status = 1,
                 .fault_type = FaultType::two_phase_to_ground,
                 .fault_phase = FaultPhase::ab,
                 .fault_object = 4,
                 .r_f = 3.0,
                 .x_f = 4.0}};

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
        CHECK(cabs(param.y_fault) == doctest::Approx(0.0));

        // Connected to source
        param = fault.calc_param(u_rated);
        double const base_y = base_i / (u_rated / sqrt3);
        DoubleComplex const y_f = 1.0 / (3.0 + 1.0i * 4.0) / base_y;
        CHECK(cabs(param.y_fault - y_f) < numerical_tolerance);
        CHECK(param.fault_type == FaultType::two_phase_to_ground);
        CHECK(param.fault_phase == FaultPhase::ab);
    }

    SUBCASE("Test calc param with nan impedance input") {
        Fault const fault_nan_imp{{.id = 1,
                                   .status = 1,
                                   .fault_type = FaultType::two_phase_to_ground,
                                   .fault_phase = FaultPhase::ab,
                                   .fault_object = 4,
                                   .r_f = nan,
                                   .x_f = nan}};
        FaultCalcParam const param = fault_nan_imp.calc_param(u_rated);
        CHECK(std::isinf(param.y_fault.real()));
        CHECK(std::isinf(param.y_fault.imag()));
        CHECK(param.fault_type == FaultType::two_phase_to_ground);
        CHECK(param.fault_phase == FaultPhase::ab);
    }

    SUBCASE("Test calc param with other fault type") {
        Fault const fault_nan_imp{{.id = 1,
                                   .status = 1,
                                   .fault_type = FaultType::three_phase,
                                   .fault_phase = FaultPhase::abc,
                                   .fault_object = 4,
                                   .r_f = nan,
                                   .x_f = nan}};
        FaultCalcParam const param = fault_nan_imp.calc_param(u_rated);
        CHECK(std::isinf(param.y_fault.real()));
        CHECK(std::isinf(param.y_fault.imag()));
        CHECK(param.fault_type == FaultType::three_phase);
        CHECK(param.fault_phase == FaultPhase::abc);
    }

    SUBCASE("Test calc param with nan fault type") {
        Fault const fault_nan_imp{{.id = 1,
                                   .status = 1,
                                   .fault_type = FaultType::nan,
                                   .fault_phase = FaultPhase::nan,
                                   .fault_object = 4,
                                   .r_f = nan,
                                   .x_f = nan}};
        CHECK_THROWS_AS((fault_nan_imp.calc_param(u_rated)), InvalidShortCircuitType);
    }

    SUBCASE("Test get_null_output") {
        FaultOutput const output = fault.get_null_output();
        CHECK(output.id == 1);
        CHECK(!output.energized);
    }

    SUBCASE("Test get_null_sc_output") {
        FaultShortCircuitOutput output = fault.get_null_sc_output();
        CHECK(output.id == 1);
        CHECK(!output.energized);
        CHECK(output.i_f(0) == doctest::Approx(0.0));
        CHECK(output.i_f(1) == doctest::Approx(0.0));
        CHECK(output.i_f(2) == doctest::Approx(0.0));
        CHECK(output.i_f_angle(0) == doctest::Approx(0.0));
        CHECK(output.i_f_angle(1) == doctest::Approx(0.0));
        CHECK(output.i_f_angle(2) == doctest::Approx(0.0));
    }

    SUBCASE("Test get_output") {
        FaultOutput const output = fault.get_output();
        CHECK(output.id == 1);
        CHECK(!output.energized);
    }

    SUBCASE("Test get_short_circuit_output sym") {
        ComplexValue<symmetric_t> const i_f_pu = 1.0 + 1.0i;
        ComplexValue<asymmetric_t> const i_f_res{i_f_pu};
        FaultShortCircuitOutput const output = fault.get_sc_output(i_f_pu, u_rated);
        CHECK(output.id == 1);
        CHECK(output.energized);
        CHECK((output.i_f - cabs(i_f_res) * base_i < numerical_tolerance).all());
        CHECK((output.i_f_angle - arg(i_f_res) < numerical_tolerance).all());
    }

    SUBCASE("Test get_short_circuit_output asym") {
        ComplexValue<asymmetric_t> i_f_pu{};
        i_f_pu << DoubleComplex(1.0, 1.0), DoubleComplex(0.0, 1.0), DoubleComplex(1.0, 0.0);
        FaultShortCircuitOutput const output = fault.get_sc_output(i_f_pu, u_rated);
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
            return Fault{{.id = 1,
                          .status = 1,
                          .fault_type = fault_type,
                          .fault_phase = FaultPhase::nan,
                          .fault_object = 4,
                          .r_f = 3.0,
                          .x_f = 4.0}};
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
            return Fault{{.id = 1,
                          .status = 1,
                          .fault_type = fault_type,
                          .fault_phase = fault_phase,
                          .fault_object = 4,
                          .r_f = 3.0,
                          .x_f = 4.0}};
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
        FaultUpdate const fault_update{.id = 1,
                                       .status = 0,
                                       .fault_type = FaultType::two_phase,
                                       .fault_phase = FaultPhase::ac,
                                       .fault_object = 10,
                                       .r_f = nan,
                                       .x_f = nan};
        UpdateChange const updated = fault.update(fault_update);

        CHECK(!updated.param);
        CHECK(!updated.topo);

        CHECK_FALSE(fault.status());
        CHECK(fault.get_fault_type() == FaultType::two_phase);
        CHECK(fault.get_fault_phase() == FaultPhase::ac);
        CHECK(fault.get_fault_object() == 10);

        // update without updating
        FaultUpdate const fault_update_nan{.id = 1,
                                           .status = na_IntS,
                                           .fault_type = FaultType::nan,
                                           .fault_phase = FaultPhase::nan,
                                           .fault_object = na_IntID,
                                           .r_f = nan,
                                           .x_f = nan};
        fault.update(fault_update_nan);
        CHECK_FALSE(fault.status());
        CHECK(fault.get_fault_type() == FaultType::two_phase);
        CHECK(fault.get_fault_phase() == FaultPhase::ac);
        CHECK(fault.get_fault_object() == 10);

        // default value does override
        FaultUpdate const fault_update_default_value{.id = 1,
                                                     .status = na_IntS,
                                                     .fault_type = FaultType::nan,
                                                     .fault_phase = FaultPhase::default_value,
                                                     .fault_object = na_IntID,
                                                     .r_f = nan,
                                                     .x_f = nan};
        fault.update(fault_update_default_value);
        CHECK_FALSE(fault.status());
        CHECK(fault.get_fault_type() == FaultType::two_phase);
        CHECK(fault.get_fault_phase() == FaultPhase::bc); // bc is the default value for two_phase fault type
        CHECK(fault.get_fault_object() == 10);
    }

    SUBCASE("Check fault type/phase combination") {
        using FaultPhase::a;
        using FaultPhase::ab;
        using FaultPhase::abc;
        using FaultPhase::ac;
        using FaultPhase::b;
        using FaultPhase::bc;
        using FaultPhase::c;
        using FaultPhase::default_value;

        auto check_allowed = [&fault](FaultType fault_type, FaultPhase fault_phase) {
            CAPTURE(fault_type);
            CAPTURE(fault_phase);
            CHECK_NOTHROW((Fault{{1, 1, fault_type, fault_phase, 4, 3.0, 4.0}}));

            FaultUpdate const fault_update{.id = 1,
                                           .status = 0,
                                           .fault_type = fault_type,
                                           .fault_phase = fault_phase,
                                           .fault_object = 10,
                                           .r_f = nan,
                                           .x_f = nan};
            CHECK_NOTHROW(fault.update(fault_update));
        };

        auto check_not_allowed = [&fault](FaultType fault_type, FaultPhase fault_phase) {
            CAPTURE(fault_type);
            CAPTURE(fault_phase);
            CHECK_THROWS_AS((Fault{{1, 1, fault_type, fault_phase, 4, 3.0, 4.0}}), InvalidShortCircuitPhases);

            FaultUpdate const fault_update{.id = 1,
                                           .status = 0,
                                           .fault_type = fault_type,
                                           .fault_phase = fault_phase,
                                           .fault_object = 10,
                                           .r_f = nan,
                                           .x_f = nan};
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
            constexpr auto bad_value{
                static_cast<FaultType>(-127)}; // NOLINT(clang-analyzer-optin.core.EnumCastOutOfRange)

            CHECK_THROWS_AS((Fault{{1, 1, bad_value, FaultPhase::nan, 4, 3.0, 4.0}}), InvalidShortCircuitType);

            FaultUpdate const fault_update{.id = 1,
                                           .status = 0,
                                           .fault_type = bad_value,
                                           .fault_phase = FaultPhase::nan,
                                           .fault_object = 10,
                                           .r_f = nan,
                                           .x_f = nan};
            CHECK_THROWS_AS(fault.update(fault_update), InvalidShortCircuitType);
        }
    }

    SUBCASE("Update fault r, x") {
        FaultUpdate const fault_update_rx{.id = 1,
                                          .status = na_IntS,
                                          .fault_type = FaultType::nan,
                                          .fault_phase = FaultPhase::nan,
                                          .fault_object = na_IntID,
                                          .r_f = 10.0,
                                          .x_f = 20.0};
        fault.update(fault_update_rx);
        FaultCalcParam const param = fault.calc_param(u_rated);
        double const base_y = base_i / (u_rated / sqrt3);
        DoubleComplex const y_f = 1.0 / (10.0 + 20.0i) / base_y;
        CHECK(cabs(param.y_fault - y_f) < numerical_tolerance);
        CHECK(param.fault_type == FaultType::two_phase_to_ground);
        CHECK(param.fault_phase == FaultPhase::ab);
    }

    SUBCASE("Update inverse") {
        FaultUpdate fault_update{.id = 1,
                                 .status = na_IntS,
                                 .fault_type = FaultType::nan,
                                 .fault_phase = FaultPhase::nan,
                                 .fault_object = na_IntID,
                                 .r_f = nan,
                                 .x_f = nan};
        auto expected = fault_update;

        SUBCASE("Identical") {
            // default values
        }

        SUBCASE("Status") {
            SUBCASE("same") { fault_update.status = status_to_int(fault.status()); }
            SUBCASE("different") { fault_update.status = IntS{0}; }
            expected.status = status_to_int(fault.status());
        }

        SUBCASE("Fault type") {
            SUBCASE("Same") { fault_update.fault_type = fault.get_fault_type(); }
            SUBCASE("different") { fault_update.fault_type = FaultType::three_phase; }
            expected.fault_type = fault.get_fault_type();
        }

        SUBCASE("Fault phase") {
            SUBCASE("Same") { fault_update.fault_phase = fault.get_fault_phase(); }
            SUBCASE("different") { fault_update.fault_phase = FaultPhase::abc; }
            expected.fault_phase = fault.get_fault_phase();
        }

        SUBCASE("Fault object") {
            SUBCASE("Same") { fault_update.fault_object = fault.get_fault_object(); }
            SUBCASE("different") { fault_update.fault_object = 100; }
            expected.fault_object = fault.get_fault_object();
        }

        SUBCASE("r_f, x_f") {
            fault_update.r_f = 6.0;
            fault_update.x_f = 7.0;
            expected.r_f = 3.0;
            expected.x_f = 4.0;
        }

        SUBCASE("multiple") {
            fault_update.status = IntS{0};
            fault_update.fault_type = FaultType::three_phase;
            fault_update.fault_phase = FaultPhase::abc;
            fault_update.fault_object = 100;
            expected.status = status_to_int(fault.status());
            expected.fault_type = fault.get_fault_type();
            expected.fault_phase = fault.get_fault_phase();
            expected.fault_object = fault.get_fault_object();
        }

        auto const inv = fault.inverse(fault_update);

        CHECK(inv.id == expected.id);
        CHECK(inv.status == expected.status);
        CHECK(inv.fault_type == expected.fault_type);
        CHECK(inv.fault_phase == expected.fault_phase);
        CHECK(inv.fault_object == expected.fault_object);

        check_nan_preserving_equality(inv.r_f, expected.r_f);
        check_nan_preserving_equality(inv.x_f, expected.x_f);
    }
}

} // namespace power_grid_model
