// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/main_model.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {
TEST_CASE("Test main model - short circuit") {
    MainModel main_model{50.0};

    SUBCASE("Single node + source") {
        main_model.add_component<Node>({{{1}, 10e4}});
        main_model.add_component<Source>({{{{2}, 1, true}, 1.0, nan, nan, nan, nan}});

        SUBCASE("three phase fault") {
            main_model.add_component<Fault>({{{3}, 1, FaultType::three_phase, FaultPhase::default_value, 1, nan, nan}});
            main_model.set_construction_complete();

            SUBCASE("Symmetric Calculation") {
                std::vector<ShortCircuitMathOutput<true>> const math_output =
                    main_model.calculate_short_circuit<true>(1.0, CalculationMethod::iec60909);

                std::vector<FaultShortCircuitOutput> fault_output(1);
                main_model.output_result<Fault>(math_output, fault_output.begin());

                // abs(source.y1_ref) * 1e6 / 10e4 / sqrt3
                CHECK(fault_output[0].i_f(0) == doctest::Approx(57735.026918962572175));

                std::vector<NodeShortCircuitOutput> node_output(1);
                main_model.output_result<Node>(math_output, node_output.begin());
                CHECK(node_output[0].u_pu(0) == doctest::Approx(0.0));
            }

            SUBCASE("Asymmetric Calculation") {
                std::vector<ShortCircuitMathOutput<false>> const math_output =
                    main_model.calculate_short_circuit<false>(1.0, CalculationMethod::iec60909);
                std::vector<FaultShortCircuitOutput> fault_output(1);
                main_model.output_result<Fault>(math_output, fault_output.begin());
                CHECK(fault_output[0].i_f(0) == doctest::Approx(57735.026918962572175));

                std::vector<NodeShortCircuitOutput> node_output(1);
                main_model.output_result<Node>(math_output, node_output.begin());
                CHECK(node_output[0].u_pu(0) == doctest::Approx(0.0));
            }
        }
    }
}

}  // namespace power_grid_model
