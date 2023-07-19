// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/main_model.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {
TEST_CASE("Test Main Model - short circuit") {
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
                CHECK(std::isinf(fault_output[0].i_f(0)));
            }
        }
    }
}

}  // namespace power_grid_model
