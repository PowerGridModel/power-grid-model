// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/auxiliary/meta_data_gen.hpp>
#include <power_grid_model/main_model.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {
namespace {
using enum CalculationSymmetry;

struct IterativeLinearCalculationMethod {
    static constexpr auto calculation_method = CalculationMethod::iterative_linear;
};
struct NewtonRaphsonCalculationMethod {
    static constexpr auto calculation_method = CalculationMethod::newton_raphson;
};
} // namespace

TEST_CASE_TEMPLATE("Test main model - state estimation", CalcMethod, IterativeLinearCalculationMethod,
                   NewtonRaphsonCalculationMethod) {
    constexpr auto calculation_method = CalcMethod::calculation_method;

    MainModel main_model{50.0, meta_data::meta_data_gen::meta_data};

    auto options = MainModel::Options{.calculation_type = CalculationType::state_estimation,
                                      .calculation_symmetry = symmetric,
                                      .calculation_method = calculation_method,
                                      .err_tol = 1e-8,
                                      .max_iter = 20};

    SUBCASE("Forbid Link Power Measurements") { // TODO(mgovers): This should be tested. maybe API test or in an
                                                // isolated environment
        main_model.add_component<Node>({{1, 10e3}, {2, 10e3}});
        main_model.add_component<Link>({{3, 1, 2, 1, 1}});
        CHECK_THROWS_AS(
            main_model.add_component<SymPowerSensor>({{4, 3, MeasuredTerminalType::branch_from, 0, 0, 0, nan, nan}}),
            InvalidMeasuredObject);
        CHECK_THROWS_WITH(
            main_model.add_component<SymPowerSensor>({{4, 3, MeasuredTerminalType::branch_from, 0, 0, 0, nan, nan}}),
            "PowerSensor measurement is not supported for object of type Link");
        CHECK_THROWS_AS(
            main_model.add_component<SymPowerSensor>({{4, 3, MeasuredTerminalType::branch_to, 0, 0, 0, nan, nan}}),
            InvalidMeasuredObject);
        CHECK_THROWS_AS(
            main_model.add_component<AsymPowerSensor>(
                {{4, 3, MeasuredTerminalType::branch_from, 0, {0, 0, 0}, {0, 0, 0}, {nan, nan, nan}, {nan, nan, nan}}}),
            InvalidMeasuredObject);
        CHECK_THROWS_AS(
            main_model.add_component<AsymPowerSensor>(
                {{4, 3, MeasuredTerminalType::branch_to, 0, {0, 0, 0}, {0, 0, 0}, {nan, nan, nan}, {nan, nan, nan}}}),
            InvalidMeasuredObject);
    }
}

} // namespace power_grid_model
