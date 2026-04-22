// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/component/branch.hpp>
#include <power_grid_model/component/link.hpp>

#include <power_grid_model/auxiliary/input.hpp>
#include <power_grid_model/auxiliary/output.hpp>
#include <power_grid_model/auxiliary/update.hpp>
#include <power_grid_model/calculation_parameters.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/common/three_phase_tensor.hpp>
#include <power_grid_model/component/base.hpp>

#include <doctest/doctest.h>

#include <cmath>
#include <complex>

namespace power_grid_model {

using namespace std::complex_literals;

TEST_CASE("Test link") {
    LinkInput input{.id = 1, .from_node = 2, .to_node = 3, .from_status = 1, .to_status = 1};
    Link link{input, 10e3, 50e3};
    Branch& branch = link;
    double const base_i_from = base_power_1p / (10.0e3 / sqrt3);
    double const base_i_to = base_power_1p / (50.0e3 / sqrt3);
    ComplexValue<asymmetric_t> const uaf{1.0};
    ComplexValue<asymmetric_t> const uat{0.9};

    // Short circuit results
    DoubleComplex const if_sc{1.0, 1.0};
    DoubleComplex const it_sc{2.0, 2.0 * sqrt3};
    ComplexValue<asymmetric_t> const if_sc_asym{1.0 + 1.0i};
    ComplexValue<asymmetric_t> const it_sc_asym{2.0 + (2.0i * sqrt3)};

    CHECK(link.math_model_type() == ComponentType::branch);

    SUBCASE("General") {
        CHECK(branch.status(BranchSide::from) == branch.from_status());
        CHECK(branch.status(BranchSide::to) == branch.to_status());
        CHECK(branch.base_i_from() == doctest::Approx(base_i_from));
        CHECK(branch.base_i_to() == doctest::Approx(base_i_to));
        CHECK(!branch.is_param_mutable());
        CHECK(branch.phase_shift() == 0.0);
    }

    SUBCASE("Invalid branch") {
        input.to_node = 2;
        CHECK_THROWS_AS(Link(input, 10e3, 50e3), InvalidBranch);
    }

    SUBCASE("Symmetric parameters") {
        // double connected
        BranchCalcParam<symmetric_t> param = branch.calc_param<symmetric_t>();
        CHECK(cabs(param.yff() - y_link) < numerical_tolerance);
        CHECK(cabs(param.ytt() - y_link) < numerical_tolerance);
        CHECK(cabs(param.ytf() + y_link) < numerical_tolerance);
        CHECK(cabs(param.yft() + y_link) < numerical_tolerance);
        // single connected
        CHECK(branch.set_status(false, na_IntS));
        param = branch.calc_param<symmetric_t>();
        CHECK(cabs(param.yff() - 0.0) < numerical_tolerance);
        CHECK(cabs(param.ytt() - 0.0) < numerical_tolerance);
        CHECK(cabs(param.ytf() - 0.0) < numerical_tolerance);
        CHECK(cabs(param.yft() - 0.0) < numerical_tolerance);
    }

    SUBCASE("Short circuit asym results") {
        BranchShortCircuitOutput const asym_output = branch.get_sc_output(if_sc_asym, it_sc_asym);
        CHECK(asym_output.id == 1);
        CHECK(asym_output.energized);
        CHECK(asym_output.i_from(0) == doctest::Approx(cabs(if_sc) * base_i_from));
        CHECK(asym_output.i_to(1) == doctest::Approx(cabs(it_sc) * base_i_to));
        CHECK(asym_output.i_from_angle(1) == doctest::Approx(pi / 4 - 2 * pi / 3));
        CHECK(asym_output.i_to_angle(2) == doctest::Approx(pi));
    }

    SUBCASE("Short circuit sym results") {
        BranchShortCircuitOutput const sym_output = branch.get_sc_output(if_sc, it_sc);
        BranchShortCircuitOutput const asym_output = branch.get_sc_output(if_sc_asym, it_sc_asym);
        CHECK(sym_output.id == asym_output.id);
        CHECK(sym_output.energized == asym_output.energized);
        CHECK(sym_output.i_from(0) == doctest::Approx(asym_output.i_from(0)));
        CHECK(sym_output.i_to(1) == doctest::Approx(asym_output.i_to(1)));
        CHECK(sym_output.i_from_angle(1) == doctest::Approx(asym_output.i_from_angle(1)));
        CHECK(sym_output.i_to_angle(2) == doctest::Approx(asym_output.i_to_angle(2)));
    }

    SUBCASE("Update inverse") {
        BranchUpdate branch_update{.id = 1, .from_status = na_IntS, .to_status = na_IntS};
        auto expected = branch_update;

        SUBCASE("Identical") {
            // default values
        }

        SUBCASE("From status") {
            SUBCASE("same") { branch_update.from_status = status_to_int(link.from_status()); }
            SUBCASE("different") { branch_update.from_status = IntS{0}; }
            expected.from_status = status_to_int(link.from_status());
        }

        SUBCASE("To status") {
            SUBCASE("same") { branch_update.to_status = status_to_int(link.to_status()); }
            SUBCASE("different") { branch_update.to_status = IntS{0}; }
            expected.to_status = status_to_int(link.to_status());
        }

        SUBCASE("multiple") {
            branch_update.from_status = IntS{0};
            branch_update.to_status = IntS{0};
            expected.from_status = status_to_int(link.from_status());
            expected.to_status = status_to_int(link.to_status());
        }

        auto const inv = link.inverse(branch_update);

        CHECK(inv.id == expected.id);
        CHECK(inv.from_status == expected.from_status);
        CHECK(inv.to_status == expected.to_status);
    }
}

} // namespace power_grid_model
