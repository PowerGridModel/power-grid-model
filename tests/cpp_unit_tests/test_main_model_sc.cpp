// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/main_model.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {
TEST_CASE("Test main model - short circuit") {
    MainModel main_model{50.0};

    SUBCASE("Single node + source") {
        double const u_rated = 10e3;
        double const u_ref = 1.0;
        double const sk = 100e6;
        double const rx_ratio = 0.1;
        double const z_ref_abs = u_rated * u_rated / sk;
        double const x_ref = z_ref_abs / sqrt(rx_ratio * rx_ratio + 1.0);
        double const r_ref = x_ref * rx_ratio;
        DoubleComplex const z_ref{r_ref, x_ref};
        double const r_f = 0.1;
        double const x_f = 0.1;
        DoubleComplex const z_f{r_f, x_f};

        main_model.add_component<Node>({{1, u_rated}});
        main_model.add_component<Source>({{2, 1, 1, u_ref, nan, sk, rx_ratio, nan}});

        SUBCASE("three phase fault - maximum voltage scaling") {
            ShortCircuitVoltageScaling const voltage_scaling = ShortCircuitVoltageScaling::maximum;
            constexpr double voltage_scaling_c = 1.1;
            main_model.add_component<Fault>({{3, 1, FaultType::three_phase, FaultPhase::default_value, 1, r_f, x_f}});
            main_model.set_construction_complete();

            double const u_source = u_rated * voltage_scaling_c / sqrt3;
            DoubleComplex const i_f = u_source / (z_ref + z_f);
            double const i_f_abs = cabs(i_f);
            DoubleComplex const u_node = i_f * z_f;
            double const u_node_abs = cabs(u_node);
            double const u_node_abs_pu = u_node_abs / (u_rated / sqrt3);

            SUBCASE("Symmetric Calculation") {
                std::vector<ShortCircuitMathOutput<true>> const math_output =
                    main_model.calculate_short_circuit<true>(voltage_scaling, CalculationMethod::iec60909);

                std::vector<FaultShortCircuitOutput> fault_output(1);
                main_model.output_result<Fault>(math_output, fault_output.begin());

                CHECK(fault_output[0].i_f(0) == doctest::Approx(i_f_abs));

                std::vector<NodeShortCircuitOutput> node_output(1);
                main_model.output_result<Node>(math_output, node_output.begin());
                CHECK(node_output[0].u_pu(0) == doctest::Approx(u_node_abs_pu));
            }

            SUBCASE("Asymmetric Calculation") {
                std::vector<ShortCircuitMathOutput<false>> const math_output =
                    main_model.calculate_short_circuit<false>(voltage_scaling, CalculationMethod::iec60909);

                std::vector<FaultShortCircuitOutput> fault_output(1);
                main_model.output_result<Fault>(math_output, fault_output.begin());
                CHECK(fault_output[0].i_f(0) == doctest::Approx(i_f_abs));

                std::vector<NodeShortCircuitOutput> node_output(1);
                main_model.output_result<Node>(math_output, node_output.begin());
                CHECK(node_output[0].u_pu(0) == doctest::Approx(u_node_abs_pu));
            }
        }
        SUBCASE("three phase fault - minimum voltage scaling") {
            ShortCircuitVoltageScaling const voltage_scaling = ShortCircuitVoltageScaling::minimum;
            constexpr double voltage_scaling_c = 1.0;
            main_model.add_component<Fault>({{3, 1, FaultType::three_phase, FaultPhase::default_value, 1, r_f, x_f}});
            main_model.set_construction_complete();

            double const u_source = u_rated * voltage_scaling_c / sqrt3;
            DoubleComplex const i_f = u_source / (z_ref + z_f);
            double const i_f_abs = cabs(i_f);
            DoubleComplex const u_node = i_f * z_f;
            double const u_node_abs = cabs(u_node);
            double const u_node_abs_pu = u_node_abs / (u_rated / sqrt3);

            SUBCASE("Symmetric Calculation") {
                std::vector<ShortCircuitMathOutput<true>> const math_output =
                    main_model.calculate_short_circuit<true>(voltage_scaling, CalculationMethod::iec60909);

                std::vector<FaultShortCircuitOutput> fault_output(1);
                main_model.output_result<Fault>(math_output, fault_output.begin());

                CHECK(fault_output[0].i_f(0) == doctest::Approx(i_f_abs));

                std::vector<NodeShortCircuitOutput> node_output(1);
                main_model.output_result<Node>(math_output, node_output.begin());
                CHECK(node_output[0].u_pu(0) == doctest::Approx(u_node_abs_pu));
            }

            SUBCASE("Asymmetric Calculation") {
                std::vector<ShortCircuitMathOutput<false>> const math_output =
                    main_model.calculate_short_circuit<false>(voltage_scaling, CalculationMethod::iec60909);

                std::vector<FaultShortCircuitOutput> fault_output(1);
                main_model.output_result<Fault>(math_output, fault_output.begin());
                CHECK(fault_output[0].i_f(0) == doctest::Approx(i_f_abs));

                std::vector<NodeShortCircuitOutput> node_output(1);
                main_model.output_result<Node>(math_output, node_output.begin());
                CHECK(node_output[0].u_pu(0) == doctest::Approx(u_node_abs_pu));
            }
        }
    }

    SUBCASE("Two nodes + branch + source") {
        ShortCircuitVoltageScaling const voltage_scaling = ShortCircuitVoltageScaling::maximum;
        constexpr double voltage_scaling_c = 1.1;

        main_model.add_component<Node>({{1, 10e4}, {2, 10e4}});
        main_model.add_component<Line>({{3, 1, 2, 1, 1, 10.0, 0.0, 0.0, 0.0, 10.0, 0.0, 0.0, 0.0, 1e3}});
        main_model.add_component<Source>({{4, 1, 1, 1.0, nan, nan, nan, nan}});

        SUBCASE("single phase to ground fault") {
            main_model.add_component<Fault>(
                {{5, 2, FaultType::single_phase_to_ground, FaultPhase::default_value, 1, nan, nan}});
            main_model.set_construction_complete();

            std::vector<ShortCircuitMathOutput<false>> const math_output =
                main_model.calculate_short_circuit<false>(voltage_scaling, CalculationMethod::iec60909);

            std::vector<FaultShortCircuitOutput> fault_output(1);
            main_model.output_result<Fault>(math_output, fault_output.begin());
            CHECK(fault_output[0].i_f(0) == doctest::Approx(voltage_scaling_c * 10e4 / sqrt3));

            std::vector<NodeShortCircuitOutput> node_output(2);
            main_model.output_result<Node>(math_output, node_output.begin());
            CHECK(node_output[0].u_pu(0) != doctest::Approx(voltage_scaling_c)); // influenced by fault
            CHECK(node_output[1].u_pu(0) == doctest::Approx(0.0));               // fault location

            CHECK(node_output[0].u_pu(1) == doctest::Approx(voltage_scaling_c));
            CHECK(node_output[0].u_pu(2) == doctest::Approx(voltage_scaling_c));
            CHECK(node_output[1].u_pu(1) == doctest::Approx(voltage_scaling_c));
            CHECK(node_output[1].u_pu(2) == doctest::Approx(voltage_scaling_c));
        }
    }
}

TEST_CASE("Test main model - short circuit - Dataset input") {
    SUBCASE("Two nodes + branch + source") {
        std::vector<NodeInput> node_input{{1, 10e4}, {2, 10e4}};
        std::vector<LineInput> line_input{{3, 1, 2, 1, 1, 10.0, 0.0, 0.0, 0.0, 10.0, 0.0, 0.0, 0.0, 1e3}};
        std::vector<SourceInput> source_input{{4, 1, 1, 1.0, nan, nan, nan, nan}};
        std::vector<FaultInput> fault_input{
            {5, 2, FaultType::single_phase_to_ground, FaultPhase::default_value, 1, nan, nan}};

        ConstDataset input_data;
        input_data["node"] = DataPointer<true>{node_input.data(), static_cast<Idx>(node_input.size())};
        input_data["line"] = DataPointer<true>{line_input.data(), static_cast<Idx>(line_input.size())};
        input_data["source"] = DataPointer<true>{source_input.data(), static_cast<Idx>(source_input.size())};
        input_data["fault"] = DataPointer<true>{fault_input.data(), static_cast<Idx>(fault_input.size())};

        MainModel model{50.0, input_data};

        std::vector<NodeShortCircuitOutput> node_output(2);

        Dataset result_data;
        result_data["node"] = DataPointer<false>{node_output.data(), static_cast<Idx>(node_output.size())};

        model.calculate_short_circuit(ShortCircuitVoltageScaling::maximum, CalculationMethod::iec60909, result_data);

        CHECK(node_output[0].u_pu(0) != doctest::Approx(1.0)); // influenced by fault
        CHECK(node_output[1].u_pu(0) == doctest::Approx(0.0)); // fault location
    }
}

} // namespace power_grid_model
