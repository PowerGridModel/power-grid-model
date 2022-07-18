// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include "doctest/doctest.h"
#include "power_grid_model/main_model.hpp"

#define s3 (1.7320508075688773433)  // sqrt(3)
#define ph (2.0943951023931952219)  // 2/3 * pi

namespace power_grid_model {

TEST_CASE("Test Main Model") {
    MainModel main_model{50.0};
    SUBCASE("State Estimation") {
        SUBCASE("Single Node + Source") {
            main_model.add_component<Node>({{{1}, 10e3}});
            main_model.add_component<Source>({{{{2}, 1, true}, 1.0, nan, nan, nan, nan}});
            SUBCASE("Symmetric Voltage Sensor") {
                main_model.add_component<SymVoltageSensor>({{{{{3}, 1}, 1e2}, 12.345e3, 0.1}});
                main_model.set_construction_complete();
                SUBCASE("Symmetric Calculation") {
                    std::vector<MathOutput<true>> const math_output =
                        main_model.calculate_state_estimation<true>(1e-8, 20, CalculationMethod::iterative_linear);
                    std::vector<NodeOutput<true>> node_output(1);
                    main_model.output_result<true, Node>(math_output, node_output.begin());
                    CHECK(node_output[0].u == doctest::Approx(12.345e3));
                }
                SUBCASE("Asymmetric Calculation") {
                    std::vector<MathOutput<false>> const math_output =
                        main_model.calculate_state_estimation<false>(1e-8, 20, CalculationMethod::iterative_linear);
                    std::vector<NodeOutput<false>> node_output(1);
                    main_model.output_result<false, Node>(math_output, node_output.begin());
                    CHECK(node_output[0].u.x() == doctest::Approx(12.345e3 / s3));
                    CHECK(node_output[0].u.y() == doctest::Approx(12.345e3 / s3));
                    CHECK(node_output[0].u.z() == doctest::Approx(12.345e3 / s3));
                }
            }
            SUBCASE("Asymmetric Voltage Sensor") {
                main_model.add_component<AsymVoltageSensor>(
                    {{{{{3}, 1}, 1e2}, {12.345e3 / s3, 12.345e3 / s3, 12.345e3 / s3}, {0.1, 0.2 - ph, 0.3 + ph}}});
                main_model.set_construction_complete();
                SUBCASE("Symmetric Calculation") {
                    std::vector<MathOutput<true>> const math_output =
                        main_model.calculate_state_estimation<true>(1e-8, 20, CalculationMethod::iterative_linear);
                    std::vector<NodeOutput<true>> node_output(1);
                    main_model.output_result<true, Node>(math_output, node_output.begin());
                    double const u = (std::cos(0.1) + std::cos(0.2) + std::cos(0.3)) * 12.345e3;
                    double const v = (std::sin(0.1) + std::sin(0.2) + std::sin(0.3)) * 12.345e3;
                    double const expected_u = std::sqrt(u * u + v * v) / 3.0;
                    CHECK(node_output[0].u == doctest::Approx(expected_u));
                }
                SUBCASE("Asymmetric Calculation") {
                    std::vector<MathOutput<false>> const math_output =
                        main_model.calculate_state_estimation<false>(1e-8, 20, CalculationMethod::iterative_linear);
                    std::vector<NodeOutput<false>> node_output(1);
                    main_model.output_result<false, Node>(math_output, node_output.begin());
                    CHECK(node_output[0].u.x() == doctest::Approx(12.345e3 / s3));
                    CHECK(node_output[0].u.y() == doctest::Approx(12.345e3 / s3));
                    CHECK(node_output[0].u.z() == doctest::Approx(12.345e3 / s3));
                }
            }
        }
    }

    SUBCASE("Forbid Link Power Measurements") {
        main_model.add_component<Node>({{{1}, 10e3}, {{2}, 10e3}});
        main_model.add_component<Link>({{{{3}, 1, 2, true, true}}});
        CHECK_THROWS_AS(
            main_model.add_component<SymPowerSensor>({{{{{4}, 3}, MeasuredTerminalType::branch_from, 0}, 0, 0}}),
            InvalidMeasuredObject);
        CHECK_THROWS_WITH(
            main_model.add_component<SymPowerSensor>({{{{{4}, 3}, MeasuredTerminalType::branch_from, 0}, 0, 0}}),
            "PowerSensor is not supported for Link");
        CHECK_THROWS_AS(
            main_model.add_component<SymPowerSensor>({{{{{4}, 3}, MeasuredTerminalType::branch_to, 0}, 0, 0}}),
            InvalidMeasuredObject);
        CHECK_THROWS_AS(main_model.add_component<AsymPowerSensor>(
                            {{{{{4}, 3}, MeasuredTerminalType::branch_from, 0}, {0, 0, 0}, {0, 0, 0}}}),
                        InvalidMeasuredObject);
        CHECK_THROWS_AS(main_model.add_component<AsymPowerSensor>(
                            {{{{{4}, 3}, MeasuredTerminalType::branch_to, 0}, {0, 0, 0}, {0, 0, 0}}}),
                        InvalidMeasuredObject);
    }
}

}  // namespace power_grid_model
