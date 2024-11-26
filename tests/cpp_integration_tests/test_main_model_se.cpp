// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/auxiliary/meta_data_gen.hpp>
#include <power_grid_model/main_model.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {
namespace {
using enum CalculationSymmetry;

constexpr double s3 = sqrt3;
constexpr double ph = 2.0 / 3.0 * pi;

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

    SUBCASE("State Estimation") {
        SUBCASE("Forbid Link Power Measurements") { // TODO(mgovers): This should be tested. maybe API test or in an
                                                    // isolated environment
            main_model.add_component<Node>({{1, 10e3}, {2, 10e3}});
            main_model.add_component<Link>({{3, 1, 2, 1, 1}});
            CHECK_THROWS_AS(main_model.add_component<SymPowerSensor>(
                                {{4, 3, MeasuredTerminalType::branch_from, 0, 0, 0, nan, nan}}),
                            InvalidMeasuredObject);
            CHECK_THROWS_WITH(main_model.add_component<SymPowerSensor>(
                                  {{4, 3, MeasuredTerminalType::branch_from, 0, 0, 0, nan, nan}}),
                              "PowerSensor measurement is not supported for object of type Link");
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

    SUBCASE(
        "Test incomplete input but complete update dataset") { // TODO(mgovers): this tests the same as the PF version
        std::vector<NodeInput> node_input{{1, 10e3}};

        std::vector<SourceInput> incomplete_source_input{{2, 1, 1, nan, nan, nan, nan, nan}};
        std::vector<SymVoltageSensorInput> incomplete_sym_sensor_input{{3, 1, 1e2, nan, nan}};
        std::vector<AsymVoltageSensorInput> incomplete_asym_sensor_input{
            {4, 1, 1e2, RealValue<asymmetric_t>{nan}, RealValue<asymmetric_t>{nan}}};

        std::vector<SourceUpdate> complete_source_update{{2, 1, 1.0, nan}};
        std::vector<SymVoltageSensorUpdate> complete_sym_sensor_update{{3, 1.0, 12.345e3, 0.1}};
        std::vector<AsymVoltageSensorUpdate> complete_asym_sensor_update{
            {4, 1.0, RealValue<asymmetric_t>{12.345e3}, RealValue<asymmetric_t>{0.1}}};

        ConstDataset input_data{false, 1, "input", meta_data::meta_data_gen::meta_data};
        input_data.add_buffer("node", node_input.size(), node_input.size(), nullptr, node_input.data());
        input_data.add_buffer("source", incomplete_source_input.size(), incomplete_source_input.size(), nullptr,
                              incomplete_source_input.data());
        input_data.add_buffer("sym_voltage_sensor", incomplete_sym_sensor_input.size(),
                              incomplete_sym_sensor_input.size(), nullptr, incomplete_sym_sensor_input.data());
        input_data.add_buffer("asym_voltage_sensor", incomplete_asym_sensor_input.size(),
                              incomplete_asym_sensor_input.size(), nullptr, incomplete_asym_sensor_input.data());

        ConstDataset update_data{true, 1, "update", meta_data::meta_data_gen::meta_data};
        update_data.add_buffer("source", complete_source_update.size(), complete_source_update.size(), nullptr,
                               complete_source_update.data());
        update_data.add_buffer("sym_voltage_sensor", complete_sym_sensor_update.size(),
                               complete_sym_sensor_update.size(), nullptr, complete_sym_sensor_update.data());
        update_data.add_buffer("asym_voltage_sensor", complete_asym_sensor_update.size(),
                               complete_asym_sensor_update.size(), nullptr, complete_asym_sensor_update.data());

        SUBCASE("State Estimation") {
            MainModel test_model{50.0, input_data};
            MainModel ref_model{50.0, input_data};
            ref_model.update_components<permanent_update_t>(update_data);

            SUBCASE("Symmetric Calculation") {
                std::vector<NodeOutput<symmetric_t>> test_node_output(1);
                std::vector<NodeOutput<symmetric_t>> ref_node_output(1);

                MutableDataset test_result_data{true, 1, "sym_output", meta_data::meta_data_gen::meta_data};
                MutableDataset ref_result_data{true, 1, "sym_output", meta_data::meta_data_gen::meta_data};
                test_result_data.add_buffer("node", test_node_output.size(), test_node_output.size(), nullptr,
                                            test_node_output.data());
                ref_result_data.add_buffer("node", ref_node_output.size(), ref_node_output.size(), nullptr,
                                           ref_node_output.data());

                options.calculation_symmetry = symmetric;
                test_model.calculate(options, test_result_data, update_data);
                ref_model.calculate(options, ref_result_data, update_data);

                CHECK(test_node_output[0].u == doctest::Approx(ref_node_output[0].u));
            }
            SUBCASE("Asymmetric Calculation") {
                std::vector<NodeOutput<asymmetric_t>> test_node_output(1);
                std::vector<NodeOutput<asymmetric_t>> ref_node_output(1);

                MutableDataset test_result_data{true, 1, "asym_output", meta_data::meta_data_gen::meta_data};
                MutableDataset ref_result_data{true, 1, "asym_output", meta_data::meta_data_gen::meta_data};

                test_result_data.add_buffer("node", test_node_output.size(), test_node_output.size(), nullptr,
                                            test_node_output.data());
                ref_result_data.add_buffer("node", ref_node_output.size(), ref_node_output.size(), nullptr,
                                           ref_node_output.data());

                options.calculation_symmetry = asymmetric;
                test_model.calculate(options, test_result_data, update_data);
                ref_model.calculate(options, ref_result_data, update_data);

                CHECK(test_node_output[0].u.x() == doctest::Approx(ref_node_output[0].u.x()));
                CHECK(test_node_output[0].u.y() == doctest::Approx(ref_node_output[0].u.y()));
                CHECK(test_node_output[0].u.z() == doctest::Approx(ref_node_output[0].u.z()));
            }
        }
    }
}

} // namespace power_grid_model
