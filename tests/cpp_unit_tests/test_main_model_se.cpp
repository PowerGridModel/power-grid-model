// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/main_model.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {
namespace {
constexpr double s3 = sqrt3;
constexpr double ph = 2.0 / 3.0 * pi;

struct IterativeLinearCalculationMethod {
    static constexpr auto calculation_method = CalculationMethod::iterative_linear;
};
struct NewtonRaphsonCalculationMethod {
    static constexpr auto calculation_method = CalculationMethod::newton_raphson;
};
} // namespace

TEST_CASE_TEMPLATE("Test main model - state estimation", CalculationMethod, IterativeLinearCalculationMethod,
                   NewtonRaphsonCalculationMethod) {
    constexpr auto calculation_method = CalculationMethod::calculation_method;

    MainModel main_model{50.0};

    SUBCASE("State Estimation") {
        SUBCASE("Single Node + Source") {
            main_model.add_component<Node>({{1, 10e3}});
            main_model.add_component<Source>({{2, 1, 1, 1.0, nan, nan, nan, nan}});
            SUBCASE("Symmetric Voltage Sensor") {
                main_model.add_component<SymVoltageSensor>({{3, 1, 1e2, 12.345e3, 0.1}});
                main_model.set_construction_complete();
                SUBCASE("Symmetric Calculation") {
                    std::vector<MathOutput<true>> const math_output =
                        main_model.calculate_state_estimation<true>(1e-8, 20, calculation_method);
                    std::vector<NodeOutput<true>> node_output(1);
                    main_model.output_result<Node>(math_output, node_output.begin());
                    CHECK(node_output[0].u == doctest::Approx(12.345e3));
                    CHECK(node_output[0].u_angle == doctest::Approx(0.1));
                }
                SUBCASE("Asymmetric Calculation") {
                    std::vector<MathOutput<false>> const math_output =
                        main_model.calculate_state_estimation<false>(1e-8, 20, calculation_method);
                    std::vector<NodeOutput<false>> node_output(1);
                    main_model.output_result<Node>(math_output, node_output.begin());
                    CHECK(node_output[0].u.x() == doctest::Approx(12.345e3 / s3));
                    CHECK(node_output[0].u.y() == doctest::Approx(12.345e3 / s3));
                    CHECK(node_output[0].u.z() == doctest::Approx(12.345e3 / s3));
                    CHECK(node_output[0].u_angle.x() == doctest::Approx(0.1));
                    CHECK(node_output[0].u_angle.y() == doctest::Approx(0.1 - ph));
                    CHECK(node_output[0].u_angle.z() == doctest::Approx(0.1 + ph));
                }
            }
            SUBCASE("Symmetric Voltage Sensor - no angle") {
                main_model.add_component<SymVoltageSensor>({{3, 1, 1e2, 12.345e3, nan}});
                main_model.set_construction_complete();
                SUBCASE("Symmetric Calculation") {
                    std::vector<MathOutput<true>> const math_output =
                        main_model.calculate_state_estimation<true>(1e-8, 20, calculation_method);
                    std::vector<NodeOutput<true>> node_output(1);
                    main_model.output_result<Node>(math_output, node_output.begin());
                    CHECK(node_output[0].u == doctest::Approx(12.345e3));
                    CHECK(node_output[0].u_angle == doctest::Approx(0.0));
                }
                SUBCASE("Asymmetric Calculation") {
                    std::vector<MathOutput<false>> const math_output =
                        main_model.calculate_state_estimation<false>(1e-8, 20, calculation_method);
                    std::vector<NodeOutput<false>> node_output(1);
                    main_model.output_result<Node>(math_output, node_output.begin());
                    CHECK(node_output[0].u.x() == doctest::Approx(12.345e3 / s3));
                    CHECK(node_output[0].u.y() == doctest::Approx(12.345e3 / s3));
                    CHECK(node_output[0].u.z() == doctest::Approx(12.345e3 / s3));
                    CHECK(node_output[0].u_angle.x() == doctest::Approx(0.0));
                    CHECK(node_output[0].u_angle.y() == doctest::Approx(-ph));
                    CHECK(node_output[0].u_angle.z() == doctest::Approx(ph));
                }
            }
            SUBCASE("Asymmetric Voltage Sensor") {
                main_model.add_component<AsymVoltageSensor>(
                    {{3, 1, 1e2, {12.345e3 / s3, 12.345e3 / s3, 12.345e3 / s3}, {0.1, 0.2 - ph, 0.3 + ph}}});
                main_model.set_construction_complete();
                SUBCASE("Symmetric Calculation") {
                    std::vector<MathOutput<true>> const math_output =
                        main_model.calculate_state_estimation<true>(1e-8, 20, calculation_method);
                    std::vector<NodeOutput<true>> node_output(1);
                    main_model.output_result<Node>(math_output, node_output.begin());
                    double const u = (std::cos(0.1) + std::cos(0.2) + std::cos(0.3)) * 12.345e3;
                    double const v = (std::sin(0.1) + std::sin(0.2) + std::sin(0.3)) * 12.345e3;
                    double const expected_u = std::sqrt(u * u + v * v) / 3.0;
                    CHECK(node_output[0].u == doctest::Approx(expected_u));
                    CHECK(node_output[0].u_angle == doctest::Approx(0.2));
                }
                SUBCASE("Asymmetric Calculation") {
                    std::vector<MathOutput<false>> const math_output =
                        main_model.calculate_state_estimation<false>(1e-8, 20, calculation_method);
                    std::vector<NodeOutput<false>> node_output(1);
                    main_model.output_result<Node>(math_output, node_output.begin());
                    CHECK(node_output[0].u.x() == doctest::Approx(12.345e3 / s3));
                    CHECK(node_output[0].u.y() == doctest::Approx(12.345e3 / s3));
                    CHECK(node_output[0].u.z() == doctest::Approx(12.345e3 / s3));
                    CHECK(node_output[0].u_angle.x() == doctest::Approx(0.1));
                    CHECK(node_output[0].u_angle.y() == doctest::Approx(0.2 - ph));
                    CHECK(node_output[0].u_angle.z() == doctest::Approx(0.3 + ph));
                }
            }
            SUBCASE("Asymmetric Voltage Sensor - no angle") {
                main_model.add_component<AsymVoltageSensor>(
                    {{3, 1, 1e2, {12.345e3 / s3, 12.345e3 / s3, 12.345e3 / s3}, {nan, nan, nan}}});
                main_model.set_construction_complete();
                SUBCASE("Symmetric Calculation") {
                    std::vector<MathOutput<true>> const math_output =
                        main_model.calculate_state_estimation<true>(1e-8, 20, calculation_method);
                    std::vector<NodeOutput<true>> node_output(1);
                    main_model.output_result<Node>(math_output, node_output.begin());
                    CHECK(node_output[0].u == doctest::Approx(12.345e3));
                    CHECK(node_output[0].u_angle == doctest::Approx(0.0));
                }
                SUBCASE("Asymmetric Calculation") {
                    std::vector<MathOutput<false>> const math_output =
                        main_model.calculate_state_estimation<false>(1e-8, 20, calculation_method);
                    std::vector<NodeOutput<false>> node_output(1);
                    main_model.output_result<Node>(math_output, node_output.begin());
                    CHECK(node_output[0].u.x() == doctest::Approx(12.345e3 / s3));
                    CHECK(node_output[0].u.y() == doctest::Approx(12.345e3 / s3));
                    CHECK(node_output[0].u.z() == doctest::Approx(12.345e3 / s3));
                    CHECK(node_output[0].u_angle.x() == doctest::Approx(0.0));
                    CHECK(node_output[0].u_angle.y() == doctest::Approx(-ph));
                    CHECK(node_output[0].u_angle.z() == doctest::Approx(ph));
                }
            }
        }

        SUBCASE("Node Injection") {
            main_model.add_component<Node>({{1, 10e3}, {2, 10e3}});
            main_model.add_component<Link>({{3, 1, 2, 1, 1}});
            main_model.add_component<Source>({{4, 1, 1, 1.0, nan, nan, nan, nan}});
            main_model.add_component<AsymGenerator>(
                {{5, 2, 1, LoadGenType::const_pq, {nan, nan, nan}, {nan, nan, nan}}});
            main_model.add_component<AsymLoad>({{6, 2, 1, LoadGenType::const_pq, {nan, nan, nan}, {nan, nan, nan}}});
            main_model.add_component<SymVoltageSensor>({{11, 1, 1e2, 10.0e3, 0.0}});
            SUBCASE("Symmetric Power Sensor - Symmetric Calculation") {
                main_model.add_component<SymPowerSensor>(
                    {{15, 5, MeasuredTerminalType::generator, 1e2, 900.0, 90.0, nan, nan},
                     {16, 6, MeasuredTerminalType::load, 1e2, 1800.0, 180.0, nan, nan}});
                SUBCASE("Without Injection Sensor") {
                    main_model.set_construction_complete();
                    std::vector<MathOutput<true>> const math_output =
                        main_model.calculate_state_estimation<true>(1e-8, 20, calculation_method);

                    std::vector<SymApplianceOutput> gen_output(1);
                    std::vector<SymApplianceOutput> load_output(1);
                    std::vector<SymNodeOutput> node_output(2);
                    std::vector<SymPowerSensorOutput> power_sensor_output(2);
                    main_model.output_result<AsymGenerator>(math_output, gen_output.begin());
                    main_model.output_result<AsymLoad>(math_output, load_output.begin());
                    main_model.output_result<Node>(math_output, node_output.begin());
                    main_model.output_result<SymPowerSensor>(math_output, power_sensor_output.begin());

                    CHECK(gen_output[0].p == doctest::Approx(900.0).scale(1e3));
                    CHECK(gen_output[0].q == doctest::Approx(90.0).scale(1e3));

                    CHECK(load_output[0].p == doctest::Approx(1800.0).scale(1e3));
                    CHECK(load_output[0].q == doctest::Approx(180.0).scale(1e3));

                    CHECK(node_output[0].p == doctest::Approx(900.0).scale(1e3));
                    CHECK(node_output[0].q == doctest::Approx(90.0).scale(1e3));
                    CHECK(node_output[1].p == doctest::Approx(-900.0).scale(1e3));
                    CHECK(node_output[1].q == doctest::Approx(-90.0).scale(1e3));

                    CHECK(power_sensor_output[0].p_residual == doctest::Approx(0.0).scale(1e3)); // gen
                    CHECK(power_sensor_output[0].q_residual == doctest::Approx(0.0).scale(1e3)); // gen
                    CHECK(power_sensor_output[1].p_residual == doctest::Approx(0.0).scale(1e3)); // load
                    CHECK(power_sensor_output[1].q_residual == doctest::Approx(0.0).scale(1e3)); // load
                }
                SUBCASE("With Injection Sensor") {
                    main_model.add_component<SymPowerSensor>(
                        {{12, 2, MeasuredTerminalType::node, 2e2, -1200.0, -120.0, nan, nan}});
                    main_model.set_construction_complete();

                    std::vector<MathOutput<true>> const math_output =
                        main_model.calculate_state_estimation<true>(1e-8, 20, calculation_method);

                    std::vector<SymApplianceOutput> gen_output(1);
                    std::vector<SymApplianceOutput> load_output(1);
                    std::vector<SymNodeOutput> node_output(2);
                    std::vector<SymPowerSensorOutput> power_sensor_output(3);
                    main_model.output_result<AsymGenerator>(math_output, gen_output.begin());
                    main_model.output_result<AsymLoad>(math_output, load_output.begin());
                    main_model.output_result<Node>(math_output, node_output.begin());
                    main_model.output_result<SymPowerSensor>(math_output, power_sensor_output.begin());

                    CHECK(gen_output[0].p == doctest::Approx(850.0).scale(1e3));
                    CHECK(gen_output[0].q == doctest::Approx(85.0).scale(1e3));

                    CHECK(load_output[0].p == doctest::Approx(1850.0).scale(1e3));
                    CHECK(load_output[0].q == doctest::Approx(185.0).scale(1e3));

                    CHECK(node_output[0].p == doctest::Approx(1000.0).scale(1e3));
                    CHECK(node_output[0].q == doctest::Approx(100.0).scale(1e3));
                    CHECK(node_output[1].p == doctest::Approx(-1000.0).scale(1e3));
                    CHECK(node_output[1].q == doctest::Approx(-100.0).scale(1e3));

                    CHECK(power_sensor_output[0].p_residual == doctest::Approx(50.0).scale(1e3));   // gen
                    CHECK(power_sensor_output[0].q_residual == doctest::Approx(5.0).scale(1e3));    // gen
                    CHECK(power_sensor_output[1].p_residual == doctest::Approx(-50.0).scale(1e3));  // load
                    CHECK(power_sensor_output[1].q_residual == doctest::Approx(-5.0).scale(1e3));   // load
                    CHECK(power_sensor_output[2].p_residual == doctest::Approx(-200.0).scale(1e3)); // node
                    CHECK(power_sensor_output[2].q_residual == doctest::Approx(-20.0).scale(1e3));  // node
                }
            }
        }
        SUBCASE("Forbid Link Power Measurements") {
            main_model.add_component<Node>({{1, 10e3}, {2, 10e3}});
            main_model.add_component<Link>({{3, 1, 2, 1, 1}});
            CHECK_THROWS_AS(main_model.add_component<SymPowerSensor>(
                                {{4, 3, MeasuredTerminalType::branch_from, 0, 0, 0, nan, nan}}),
                            InvalidMeasuredObject);
            CHECK_THROWS_WITH(main_model.add_component<SymPowerSensor>(
                                  {{4, 3, MeasuredTerminalType::branch_from, 0, 0, 0, nan, nan}}),
                              "PowerSensor is not supported for Link");
            CHECK_THROWS_AS(
                main_model.add_component<SymPowerSensor>({{4, 3, MeasuredTerminalType::branch_to, 0, 0, 0, nan, nan}}),
                InvalidMeasuredObject);
            CHECK_THROWS_AS(main_model.add_component<AsymPowerSensor>({{4,
                                                                        3,
                                                                        MeasuredTerminalType::branch_from,
                                                                        0,
                                                                        {0, 0, 0},
                                                                        {0, 0, 0},
                                                                        {nan, nan, nan},
                                                                        {nan, nan, nan}}}),
                            InvalidMeasuredObject);
            CHECK_THROWS_AS(main_model.add_component<AsymPowerSensor>({{4,
                                                                        3,
                                                                        MeasuredTerminalType::branch_to,
                                                                        0,
                                                                        {0, 0, 0},
                                                                        {0, 0, 0},
                                                                        {nan, nan, nan},
                                                                        {nan, nan, nan}}}),
                            InvalidMeasuredObject);
        }
    }

    SUBCASE("Test incomplete input but complete update dataset") {
        std::vector<NodeInput> node_input{{1, 10e3}};

        std::vector<SourceInput> incomplete_source_input{{2, 1, 1, nan, nan, nan, nan, nan}};
        std::vector<SymVoltageSensorInput> incomplete_sym_sensor_input{{3, 1, 1e2, nan, nan}};
        std::vector<AsymVoltageSensorInput> incomplete_asym_sensor_input{
            {4, 1, 1e2, RealValue<false>{nan}, RealValue<false>{nan}}};

        std::vector<SourceUpdate> complete_source_update{{2, 1, 1.0, nan}};
        std::vector<SymVoltageSensorUpdate> complete_sym_sensor_update{{3, 1.0, 12.345e3, 0.1}};
        std::vector<AsymVoltageSensorUpdate> complete_asym_sensor_update{
            {4, 1.0, RealValue<false>{12.345e3}, RealValue<false>{0.1}}};

        ConstDataset input_data;
        input_data["node"] = DataPointer<true>{node_input.data(), static_cast<Idx>(node_input.size())};
        input_data["source"] =
            DataPointer<true>{incomplete_source_input.data(), static_cast<Idx>(incomplete_source_input.size())};
        input_data["sym_voltage_sensor"] =
            DataPointer<true>{incomplete_sym_sensor_input.data(), static_cast<Idx>(incomplete_sym_sensor_input.size())};
        input_data["asym_voltage_sensor"] = DataPointer<true>{incomplete_asym_sensor_input.data(),
                                                              static_cast<Idx>(incomplete_asym_sensor_input.size())};

        ConstDataset update_data;
        update_data["source"] =
            DataPointer<true>{complete_source_update.data(), static_cast<Idx>(complete_source_update.size())};
        update_data["sym_voltage_sensor"] =
            DataPointer<true>{complete_sym_sensor_update.data(), static_cast<Idx>(complete_sym_sensor_update.size())};
        update_data["asym_voltage_sensor"] =
            DataPointer<true>{complete_asym_sensor_update.data(), static_cast<Idx>(complete_asym_sensor_update.size())};

        SUBCASE("State Estimation") {
            MainModel test_model{50.0, input_data};
            MainModel ref_model{50.0, input_data};
            ref_model.update_component<MainModel::permanent_update_t>(update_data);

            SUBCASE("Symmetric Calculation") {
                std::vector<NodeOutput<true>> test_node_output(1);
                std::vector<NodeOutput<true>> ref_node_output(1);

                Dataset test_result_data;
                Dataset ref_result_data;
                test_result_data["node"] =
                    DataPointer<false>{test_node_output.data(), static_cast<Idx>(test_node_output.size())};
                ref_result_data["node"] =
                    DataPointer<false>{ref_node_output.data(), static_cast<Idx>(ref_node_output.size())};

                test_model.calculate_state_estimation<true>(1e-8, 20, calculation_method, test_result_data, update_data,
                                                            -1);
                ref_model.calculate_state_estimation<true>(1e-8, 20, calculation_method, ref_result_data, update_data,
                                                           -1);

                CHECK(test_node_output[0].u == doctest::Approx(ref_node_output[0].u));
            }
            SUBCASE("Asymmetric Calculation") {
                std::vector<NodeOutput<false>> test_node_output(1);
                std::vector<NodeOutput<false>> ref_node_output(1);

                Dataset test_result_data;
                Dataset ref_result_data;
                test_result_data["node"] =
                    DataPointer<false>{test_node_output.data(), static_cast<Idx>(test_node_output.size())};
                ref_result_data["node"] =
                    DataPointer<false>{ref_node_output.data(), static_cast<Idx>(ref_node_output.size())};

                test_model.calculate_state_estimation<false>(1e-8, 20, calculation_method, test_result_data,
                                                             update_data, -1);
                ref_model.calculate_state_estimation<false>(1e-8, 20, calculation_method, ref_result_data, update_data,
                                                            -1);

                CHECK(test_node_output[0].u.x() == doctest::Approx(ref_node_output[0].u.x()));
                CHECK(test_node_output[0].u.y() == doctest::Approx(ref_node_output[0].u.y()));
                CHECK(test_node_output[0].u.z() == doctest::Approx(ref_node_output[0].u.z()));
            }
        }
    }
}

} // namespace power_grid_model
