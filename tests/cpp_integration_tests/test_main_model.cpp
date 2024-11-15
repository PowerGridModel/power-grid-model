// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/auxiliary/meta_data_gen.hpp>
#include <power_grid_model/main_model.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {
namespace {
using CalculationType::power_flow;
using enum CalculationSymmetry;

constexpr auto get_default_options(CalculationSymmetry calculation_symmetry, CalculationMethod calculation_method,
                                   Idx threading = -1) {
    return MainModel::Options{.calculation_type = power_flow,
                              .calculation_symmetry = calculation_symmetry,
                              .calculation_method = calculation_method,
                              .err_tol = 1e-8,
                              .max_iter = 20,
                              .threading = threading};
}

struct regular_update {
    using update_type = permanent_update_t;
};

struct cached_update {
    using update_type = cached_update_t;
};

namespace test {
constexpr double z_bus_2 = 1.0 / (0.015 + 0.5e6 / 10e3 / 10e3 * 2);
constexpr double z_total = z_bus_2 + 10.0;
constexpr double u1 = 1.05 * z_bus_2 / (z_bus_2 + 10.0);
constexpr double i = 1.05 * 10e3 / z_total / sqrt3;
constexpr double i_shunt = 0.015 / 0.025 * i;
constexpr double i_load = 0.005 / 0.025 * i;
} // namespace test

struct State {
    // TODO(mgovers): values identical to power_flow/dummy-test/input.json validation case
    std::vector<NodeInput> node_input{{1, 10e3}, {2, 10e3}, {3, 10e3}};
    std::vector<LineInput> line_input{{4, 1, 2, 1, 1, 10.0, 0.0, 0.0, 0.0, 10.0, 0.0, 0.0, 0.0, 1e3}};
    std::vector<LinkInput> link_input{{5, 2, 3, 1, 1}};
    std::vector<SourceInput> source_input{{6, 1, 1, 1.05, nan, 1e12, nan, nan}, {10, 3, 0, 1.05, 0.0, 1e12, nan, nan}};
    std::vector<SymLoadGenInput> sym_load_input{{7, 3, 1, LoadGenType::const_y, 0.5e6, 0.0}};
    std::vector<AsymLoadGenInput> asym_load_input{
        {8, 3, 1, LoadGenType::const_y, RealValue<asymmetric_t>{0.5e6 / 3.0}, RealValue<asymmetric_t>{0.0}}};
    std::vector<ShuntInput> shunt_input{{9, 3, 1, 0.015, 0.0, 0.015, 0.0}};

    // TODO(mgovers): only power_flow/pandapower pf validation cases have sensor input and they differ from here
    // {{{id}, measured_object}, measured_terminal_type, power_sigma, p_measured, q_measured}
    std::vector<SymPowerSensorInput> sym_power_sensor_input{
        {11, 4, MeasuredTerminalType::branch_from, 0.02, 1.1e6, 1.1e3, nan, nan},
        {13, 6, MeasuredTerminalType::source, 0.02, 1.3e6, 1.3e3, nan, nan},
        {14, 6, MeasuredTerminalType::source, 0.02, 1.4e6, 1.4e3, nan, nan},
        {15, 9, MeasuredTerminalType::shunt, 0.02, 1.5e6, 1.5e3, nan, nan},
        {16, 7, MeasuredTerminalType::load, 0.02, 1.6e6, 1.6e3, nan, nan},
        {17, 8, MeasuredTerminalType::load, 0.02, 1.7e6, 1.7e3, nan, nan},
        {28, 3, MeasuredTerminalType::node, 0.02, 3.0e6, 3.0e3, nan, nan}};

    // TODO(mgovers): only power_flow/pandapower pf validation cases have sensor input and they differ from here
    // {{{id}, measured_object}, measured_terminal_type, power_sigma, p_measured, q_measured}
    std::vector<AsymPowerSensorInput> asym_power_sensor_input{{18,
                                                               4,
                                                               MeasuredTerminalType::branch_from,
                                                               0.02,
                                                               {2.11e6, 2.12e6, 2.13e6},
                                                               {2.11e3, 2.12e3, 2.13e3},
                                                               {nan, nan, nan},
                                                               {nan, nan, nan}},
                                                              {20,
                                                               6,
                                                               MeasuredTerminalType::source,
                                                               0.02,
                                                               {2.31e6, 2.32e6, 2.33e6},
                                                               {2.31e3, 2.32e3, 2.33e3},
                                                               {nan, nan, nan},
                                                               {nan, nan, nan}},
                                                              {21,
                                                               6,
                                                               MeasuredTerminalType::source,
                                                               0.02,
                                                               {2.41e6, 2.42e6, 2.43e6},
                                                               {2.41e3, 2.42e3, 2.43e3},
                                                               {nan, nan, nan},
                                                               {nan, nan, nan}},
                                                              {22,
                                                               9,
                                                               MeasuredTerminalType::shunt,
                                                               0.02,
                                                               {2.51e6, 2.52e6, 2.53e6},
                                                               {2.51e3, 2.52e3, 2.53e3},
                                                               {nan, nan, nan},
                                                               {nan, nan, nan}},
                                                              {23,
                                                               7,
                                                               MeasuredTerminalType::load,
                                                               0.02,
                                                               {2.61e6, 2.62e6, 2.63e6},
                                                               {2.61e3, 2.62e3, 2.63e3},
                                                               {nan, nan, nan},
                                                               {nan, nan, nan}},
                                                              {24,
                                                               8,
                                                               MeasuredTerminalType::load,
                                                               0.02,
                                                               {2.71e6, 2.72e6, 2.73e6},
                                                               {2.71e3, 2.72e3, 2.73e3},
                                                               {nan, nan, nan},
                                                               {nan, nan, nan}},
                                                              {29,
                                                               3,
                                                               MeasuredTerminalType::node,
                                                               0.02,
                                                               {5.01e6, 5.02e6, 5.03e6},
                                                               {5.01e3, 5.02e3, 5.03e3},
                                                               {nan, nan, nan},
                                                               {nan, nan, nan}}};

    // TODO(mgovers): only power_flow/pandapower pf validation cases have sensor input and they differ from here
    // {{{id}, measured_object}, u_sigma, u_measured, u_angle_measured}
    std::vector<SymVoltageSensorInput> sym_voltage_sensor_input{{25, 1, 105.0, 10.1e3, 0.1},
                                                                {26, 2, 105.0, 10.2e3, 0.2}};

    // TODO(mgovers): only power_flow/pandapower pf validation cases have sensor input and they differ from here
    // {{{id}, measured_object}, u_sigma, u_measured, u_angle_measured}
    std::vector<AsymVoltageSensorInput> asym_voltage_sensor_input{
        {27, 3, 105.0, {10.31e3 / sqrt3, 10.32e3 / sqrt3, 10.33e3 / sqrt3}, {0.0, -deg_120, -deg_240}}};

    // TODO(mgovers): only used for updating (and proving no different output). nowhere else used in powerflow
    // TODO(mgovers): no powerflow validation cases have fault input
    std::vector<FaultInput> fault_input{{30, 1, FaultType::single_phase_to_ground, FaultPhase::a, 3, 0.1, 0.1}};

    // output vector
    std::vector<NodeOutput<symmetric_t>> sym_node = std::vector<NodeOutput<symmetric_t>>(3);
    std::vector<BranchOutput<symmetric_t>> sym_branch = std::vector<BranchOutput<symmetric_t>>(2);
    std::vector<ApplianceOutput<symmetric_t>> sym_appliance = std::vector<ApplianceOutput<symmetric_t>>(5);
    std::vector<NodeOutput<asymmetric_t>> asym_node = std::vector<NodeOutput<asymmetric_t>>(3);
    std::vector<BranchOutput<asymmetric_t>> asym_branch = std::vector<BranchOutput<asymmetric_t>>(2);
    std::vector<ApplianceOutput<asymmetric_t>> asym_appliance = std::vector<ApplianceOutput<asymmetric_t>>(5);

    // individual symmetric
    std::vector<BranchOutput<symmetric_t>> sym_line = std::vector<BranchOutput<symmetric_t>>(1);
    std::vector<BranchOutput<symmetric_t>> sym_link = std::vector<BranchOutput<symmetric_t>>(1);
    std::vector<ApplianceOutput<symmetric_t>> sym_load_sym = std::vector<ApplianceOutput<symmetric_t>>(1);
    std::vector<ApplianceOutput<symmetric_t>> sym_load_asym = std::vector<ApplianceOutput<symmetric_t>>(1);
    std::vector<ApplianceOutput<symmetric_t>> sym_source = std::vector<ApplianceOutput<symmetric_t>>(2);
    std::vector<ApplianceOutput<symmetric_t>> sym_shunt = std::vector<ApplianceOutput<symmetric_t>>(1);
    std::vector<VoltageSensorOutput<symmetric_t>> sym_voltage_sensor = std::vector<VoltageSensorOutput<symmetric_t>>(2);
    std::vector<VoltageSensorOutput<symmetric_t>> asym_voltage_sensor_sym_output =
        std::vector<VoltageSensorOutput<symmetric_t>>(1);
    std::vector<PowerSensorOutput<symmetric_t>> sym_power_sensor = std::vector<PowerSensorOutput<symmetric_t>>(7);
    std::vector<PowerSensorOutput<symmetric_t>> asym_power_sensor_sym_output =
        std::vector<PowerSensorOutput<symmetric_t>>(7);

    // individual asymmetric
    std::vector<BranchOutput<asymmetric_t>> asym_line = std::vector<BranchOutput<asymmetric_t>>(1);
    std::vector<BranchOutput<asymmetric_t>> asym_link = std::vector<BranchOutput<asymmetric_t>>(1);
    std::vector<ApplianceOutput<asymmetric_t>> asym_load_sym = std::vector<ApplianceOutput<asymmetric_t>>(1);
    std::vector<ApplianceOutput<asymmetric_t>> asym_load_asym = std::vector<ApplianceOutput<asymmetric_t>>(1);
    std::vector<ApplianceOutput<asymmetric_t>> asym_source = std::vector<ApplianceOutput<asymmetric_t>>(2);
    std::vector<ApplianceOutput<asymmetric_t>> asym_shunt = std::vector<ApplianceOutput<asymmetric_t>>(1);
    std::vector<VoltageSensorOutput<asymmetric_t>> asym_voltage_sensor =
        std::vector<VoltageSensorOutput<asymmetric_t>>(1);
    std::vector<VoltageSensorOutput<asymmetric_t>> sym_voltage_sensor_asym_output =
        std::vector<VoltageSensorOutput<asymmetric_t>>(2);
    std::vector<PowerSensorOutput<asymmetric_t>> asym_power_sensor = std::vector<PowerSensorOutput<asymmetric_t>>(7);
    std::vector<PowerSensorOutput<asymmetric_t>> sym_power_sensor_asym_output =
        std::vector<PowerSensorOutput<asymmetric_t>>(7);

    // update vector
    std::vector<SymLoadGenUpdate> sym_load_update{{7, 1, 1.0e6, nan}};
    std::vector<AsymLoadGenUpdate> asym_load_update{{8, 0, RealValue<asymmetric_t>{nan}, RealValue<asymmetric_t>{nan}}};
    std::vector<ShuntUpdate> shunt_update{{9, 0, nan, 0.02, nan, 0.02}};
    std::vector<ShuntUpdate> shunt_update_2{{6, 0, nan, 0.01, nan, 0.01}}; // used for test case alternate compute mode
    std::vector<SourceUpdate> source_update{{10, 1, test::u1, nan}};
    std::vector<BranchUpdate> link_update{{5, 1, 0}};
    std::vector<FaultUpdate> fault_update{{30, 1, FaultType::three_phase, FaultPhase::abc, 1, nan, nan}};

    // batch update vector
    std::vector<SymLoadGenUpdate> batch_sym_load_update{{7, 1, 1.0e6, nan}, {7}, {7}, {7}, {7}};
    std::vector<AsymLoadGenUpdate> batch_asym_load_update{
        {8, 0, RealValue<asymmetric_t>{nan}, RealValue<asymmetric_t>{nan}}, {8}, {8}, {8}, {8}};
    std::vector<ShuntUpdate> batch_shunt_update{{9, 0, nan, 0.02, nan, 0.02}, {9}, {9}, {9}, {9}};
    std::vector<SourceUpdate> batch_source_update{{10, 1, test::u1, nan}, {10}, {10}, {10}, {10}};
    std::vector<BranchUpdate> batch_link_update{{5, 1, 0}, {5}, {5}, {5}, {5}};
    std::vector<FaultUpdate> batch_fault_update{
        {30, 1, FaultType::three_phase, FaultPhase::abc, 1, nan, nan}, {30}, {30}, {30}, {30}};
};

auto default_model(State const& state) -> MainModel {
    MainModel main_model{50.0, meta_data::meta_data_gen::meta_data};
    main_model.add_component<Node>(state.node_input);
    main_model.add_component<Line>(state.line_input);
    main_model.add_component<Link>(state.link_input);
    main_model.add_component<Source>(state.source_input);
    main_model.add_component<AsymLoad>(state.asym_load_input);
    main_model.add_component<SymLoad>(state.sym_load_input);
    main_model.add_component<Shunt>(state.shunt_input);
    main_model.add_component<SymPowerSensor>(state.sym_power_sensor_input);
    main_model.add_component<AsymPowerSensor>(state.asym_power_sensor_input);
    main_model.add_component<SymVoltageSensor>(state.sym_voltage_sensor_input);
    main_model.add_component<AsymVoltageSensor>(state.asym_voltage_sensor_input);
    main_model.add_component<Fault>(state.fault_input);
    main_model.set_construction_complete();
    return main_model;
}
} // namespace

TEST_CASE("Test main model - power flow") {
    State state;
    auto main_model = default_model(state);

    SUBCASE("Test get indexer") { // TODO(mgovers): needed
        std::vector<ID> const node_id{2, 1, 3, 2};
        IdxVector const expected_indexer{1, 0, 2, 1};
        IdxVector indexer(4);
        main_model.get_indexer("node", node_id.data(), 4, indexer.data());
        CHECK(indexer == expected_indexer);
    }

    SUBCASE("Test duplicated id") { // TODO(mgovers): needed; captured in Python test; maybe move to
                                    // test_main_core_input.cpp
        MainModel main_model2{50.0, meta_data::meta_data_gen::meta_data};
        state.node_input[1].id = 1;
        CHECK_THROWS_AS(main_model2.add_component<Node>(state.node_input), ConflictID);
    }

    SUBCASE("Test no existing id") { // TODO(mgovers): needed; captured in Python test; maybe move to
                                     // test_main_core_input.cpp
        MainModel main_model2{50.0, meta_data::meta_data_gen::meta_data};
        state.line_input[0].from_node = 100;
        main_model2.add_component<Node>(state.node_input);
        CHECK_THROWS_AS(main_model2.add_component<Line>(state.line_input), IDNotFound);
    }

    SUBCASE("Test id for wrong type") { // TODO(mgovers): needed; captured in Python test but not all flavors; maybe
                                        // move to test_main_core_input.cpp
        MainModel main_model2{50.0, meta_data::meta_data_gen::meta_data};

        state.link_input[0].from_node = 4;
        main_model2.add_component<Node>(state.node_input); // 1 2 3
        main_model2.add_component<Line>(state.line_input); // 4
        CHECK_THROWS_AS(main_model2.add_component<Link>(state.link_input), IDWrongType);

        // Fix link input, retry
        state.link_input[0].from_node = 2;
        main_model2.add_component<Link>(state.link_input); // 5

        main_model2.add_component<Source>(state.source_input);      // 6 10
        main_model2.add_component<SymLoad>(state.sym_load_input);   // 7
        main_model2.add_component<AsymLoad>(state.asym_load_input); // 8
        main_model2.add_component<Shunt>(state.shunt_input);        // 9

        // voltage sensor with a measured id which is not a node (link)
        state.sym_voltage_sensor_input[0].measured_object = 5;
        CHECK_THROWS_AS(main_model2.add_component<SymVoltageSensor>(state.sym_voltage_sensor_input), IDWrongType);

        // Test for all MeasuredTerminalType instances
        using enum MeasuredTerminalType;
        std::vector<MeasuredTerminalType> const mt_types{branch_from, branch_to, generator, load, shunt, source};

        // power sensor with terminal branch, with a measured id which is not a branch (node)
        for (auto const& mt_type : mt_types) {
            state.sym_power_sensor_input[0].measured_object = 1;
            state.sym_power_sensor_input[0].measured_terminal_type = mt_type;
            CHECK_THROWS_AS(main_model2.add_component<SymPowerSensor>(state.sym_power_sensor_input), IDWrongType);
        }
    }
}

TEST_CASE("Test main model - individual output (symmetric)") {
    State state;
    auto main_model = default_model(state);

    auto const res = main_model.calculate<power_flow_t, symmetric_t>(
        get_default_options(symmetric, CalculationMethod::newton_raphson));

    SUBCASE("SymVoltageSensor, sym output") { // TODO(mgovers): sensor output for powerflow calculations are not tested
                                              // elsewhere => validation case
        main_model.output_result<Node>(res, state.sym_node);
        main_model.output_result<SymVoltageSensor>(res, state.sym_voltage_sensor);

        CHECK(state.sym_voltage_sensor[0].u_residual == doctest::Approx(1.01 * 10.0e3 - state.sym_node[0].u));
        CHECK(state.sym_voltage_sensor[1].u_residual == doctest::Approx(1.02 * 10.0e3 - state.sym_node[1].u));
        CHECK(state.sym_voltage_sensor[0].u_angle_residual == doctest::Approx(0.1 - state.sym_node[0].u_angle));
        CHECK(state.sym_voltage_sensor[1].u_angle_residual == doctest::Approx(0.2 - state.sym_node[1].u_angle));
    }

    SUBCASE("SymPowerSensor, sym output") { // TODO(mgovers): sensor output for powerflow calculations are not tested
                                            // elsewhere => validation case
        main_model.output_result<Line>(res, state.sym_line);
        main_model.output_result<Link>(res, state.sym_link);
        main_model.output_result<Source>(res, state.sym_source);
        main_model.output_result<SymLoad>(res, state.sym_load_sym);
        main_model.output_result<AsymLoad>(res, state.sym_load_asym);
        main_model.output_result<Shunt>(res, state.sym_shunt);
        main_model.output_result<SymPowerSensor>(res, state.sym_power_sensor);

        CHECK(state.sym_power_sensor[0].p_residual == doctest::Approx(1.1e6 - state.sym_line[0].p_from));
        CHECK(state.sym_power_sensor[0].q_residual == doctest::Approx(1.1e3 - state.sym_line[0].q_from));
        CHECK(state.sym_power_sensor[1].p_residual == doctest::Approx(1.3e6 - state.sym_source[0].p));
        CHECK(state.sym_power_sensor[1].q_residual == doctest::Approx(1.3e3 - state.sym_source[0].q));
        CHECK(state.sym_power_sensor[2].p_residual == doctest::Approx(1.4e6 - state.sym_source[0].p));
        CHECK(state.sym_power_sensor[2].q_residual == doctest::Approx(1.4e3 - state.sym_source[0].q));
        CHECK(state.sym_power_sensor[3].p_residual == doctest::Approx(1.5e6 - state.sym_shunt[0].p));
        CHECK(state.sym_power_sensor[3].q_residual == doctest::Approx(1.5e3 - state.sym_shunt[0].q));
        CHECK(state.sym_power_sensor[4].p_residual == doctest::Approx(1.6e6 - state.sym_load_sym[0].p));
        CHECK(state.sym_power_sensor[4].q_residual == doctest::Approx(1.6e3 - state.sym_load_sym[0].q));
        CHECK(state.sym_power_sensor[5].p_residual == doctest::Approx(1.7e6 - state.sym_load_asym[0].p));
        CHECK(state.sym_power_sensor[5].q_residual == doctest::Approx(1.7e3 - state.sym_load_asym[0].q));
        CHECK(state.sym_power_sensor[6].p_residual ==
              doctest::Approx(3.0e6 - (state.sym_source[1].p - state.sym_load_sym[0].p - state.sym_load_asym[0].p)));
        CHECK(state.sym_power_sensor[6].q_residual ==
              doctest::Approx(3.0e3 - (state.sym_source[1].q - state.sym_load_sym[0].q - state.sym_load_asym[0].q)));
    }

    SUBCASE("AsymVoltageSensor, sym output") { // TODO(mgovers): sensor output for powerflow calculations are not tested
                                               // elsewhere => validation case
        main_model.output_result<Node>(res, state.sym_node);
        main_model.output_result<AsymVoltageSensor>(res, state.asym_voltage_sensor_sym_output);

        CHECK(state.asym_voltage_sensor_sym_output[0].u_residual == doctest::Approx(10.32e3 - state.sym_node[2].u));
        CHECK(state.asym_voltage_sensor_sym_output[0].u_angle_residual ==
              doctest::Approx(0.0 - state.sym_node[2].u_angle));
    }

    SUBCASE("AsymPowerSensor, sym output") { // TODO(mgovers): sensor output for powerflow calculations are not tested
                                             // elsewhere => validation case
        main_model.output_result<Line>(res, state.sym_line);
        main_model.output_result<Link>(res, state.sym_link);
        main_model.output_result<Source>(res, state.sym_source);
        main_model.output_result<SymLoad>(res, state.sym_load_sym);
        main_model.output_result<AsymLoad>(res, state.sym_load_asym);
        main_model.output_result<Shunt>(res, state.sym_shunt);
        main_model.output_result<AsymPowerSensor>(res, state.asym_power_sensor_sym_output);

        CHECK(state.asym_power_sensor_sym_output[0].p_residual ==
              doctest::Approx(3 * 2.12e6 - state.sym_line[0].p_from));
        CHECK(state.asym_power_sensor_sym_output[0].q_residual ==
              doctest::Approx(3 * 2.12e3 - state.sym_line[0].q_from));
        CHECK(state.asym_power_sensor_sym_output[1].p_residual == doctest::Approx(3 * 2.32e6 - state.sym_source[0].p));
        CHECK(state.asym_power_sensor_sym_output[1].q_residual == doctest::Approx(3 * 2.32e3 - state.sym_source[0].q));
        CHECK(state.asym_power_sensor_sym_output[2].p_residual == doctest::Approx(3 * 2.42e6 - state.sym_source[0].p));
        CHECK(state.asym_power_sensor_sym_output[2].q_residual == doctest::Approx(3 * 2.42e3 - state.sym_source[0].q));
        CHECK(state.asym_power_sensor_sym_output[3].p_residual == doctest::Approx(3 * 2.52e6 - state.sym_shunt[0].p));
        CHECK(state.asym_power_sensor_sym_output[3].q_residual == doctest::Approx(3 * 2.52e3 - state.sym_shunt[0].q));
        CHECK(state.asym_power_sensor_sym_output[4].p_residual ==
              doctest::Approx(3 * 2.62e6 - state.sym_load_sym[0].p));
        CHECK(state.asym_power_sensor_sym_output[4].q_residual ==
              doctest::Approx(3 * 2.62e3 - state.sym_load_sym[0].q));
        CHECK(state.asym_power_sensor_sym_output[5].p_residual ==
              doctest::Approx(3 * 2.72e6 - state.sym_load_asym[0].p));
        CHECK(state.asym_power_sensor_sym_output[5].q_residual ==
              doctest::Approx(3 * 2.72e3 - state.sym_load_asym[0].q));
        CHECK(
            state.asym_power_sensor_sym_output[6].p_residual ==
            doctest::Approx(3 * 5.02e6 - (state.sym_source[1].p - state.sym_load_sym[0].p - state.sym_load_asym[0].p)));
        CHECK(
            state.asym_power_sensor_sym_output[6].q_residual ==
            doctest::Approx(3 * 5.02e3 - (state.sym_source[1].q - state.sym_load_sym[0].q - state.sym_load_asym[0].q)));
    }
}

TEST_CASE("Test main model - individual output (asymmetric)") {
    State state;
    auto main_model = default_model(state);

    auto const res = main_model.calculate<power_flow_t, asymmetric_t>(
        get_default_options(asymmetric, CalculationMethod::newton_raphson));

    SUBCASE("AsymVoltageSensor, asym output") { // TODO(mgovers): sensor output for powerflow calculations are not
                                                // tested elsewhere => validation case
        main_model.output_result<Node>(res, state.asym_node);
        main_model.output_result<AsymVoltageSensor>(res, state.asym_voltage_sensor);

        CHECK(state.asym_voltage_sensor[0].u_residual[0] ==
              doctest::Approx(1.031 / sqrt3 * 10.0e3 - state.asym_node[2].u[0]));
        CHECK(state.asym_voltage_sensor[0].u_residual[1] ==
              doctest::Approx(1.032 / sqrt3 * 10.0e3 - state.asym_node[2].u[1]));
        CHECK(state.asym_voltage_sensor[0].u_residual[2] ==
              doctest::Approx(1.033 / sqrt3 * 10.0e3 - state.asym_node[2].u[2]));
        CHECK(state.asym_voltage_sensor[0].u_angle_residual[0] == doctest::Approx(0.0 - state.asym_node[2].u_angle[0]));
        CHECK(state.asym_voltage_sensor[0].u_angle_residual[1] ==
              doctest::Approx(-deg_120 - state.asym_node[2].u_angle[1]));
        CHECK(state.asym_voltage_sensor[0].u_angle_residual[2] ==
              doctest::Approx(-deg_240 - state.asym_node[2].u_angle[2]));
    }

    SUBCASE("SymVoltageSensor, asym output") { // TODO(mgovers): sensor output for powerflow calculations are not tested
                                               // elsewhere => validation case
        main_model.output_result<Node>(res, state.asym_node);
        main_model.output_result<SymVoltageSensor>(res, state.sym_voltage_sensor_asym_output);

        CHECK(state.sym_voltage_sensor_asym_output[0].u_residual[0] ==
              doctest::Approx(10.1e3 / sqrt3 - state.asym_node[0].u[0]));
        CHECK(state.sym_voltage_sensor_asym_output[0].u_residual[1] ==
              doctest::Approx(10.1e3 / sqrt3 - state.asym_node[0].u[1]));
        CHECK(state.sym_voltage_sensor_asym_output[0].u_residual[2] ==
              doctest::Approx(10.1e3 / sqrt3 - state.asym_node[0].u[2]));
        CHECK(state.sym_voltage_sensor_asym_output[0].u_angle_residual[0] ==
              doctest::Approx(0.1 - state.asym_node[0].u_angle[0]));
        CHECK(state.sym_voltage_sensor_asym_output[0].u_angle_residual[1] ==
              doctest::Approx(0.1 - state.asym_node[0].u_angle[1]));
        CHECK(state.sym_voltage_sensor_asym_output[0].u_angle_residual[2] ==
              doctest::Approx(0.1 - state.asym_node[0].u_angle[2]));
        CHECK(state.sym_voltage_sensor_asym_output[1].u_residual[0] ==
              doctest::Approx(10.2e3 / sqrt3 - state.asym_node[1].u[0]));
        CHECK(state.sym_voltage_sensor_asym_output[1].u_residual[1] ==
              doctest::Approx(10.2e3 / sqrt3 - state.asym_node[1].u[1]));
        CHECK(state.sym_voltage_sensor_asym_output[1].u_residual[2] ==
              doctest::Approx(10.2e3 / sqrt3 - state.asym_node[1].u[2]));
        CHECK(state.sym_voltage_sensor_asym_output[1].u_angle_residual[0] ==
              doctest::Approx(0.2 - state.asym_node[1].u_angle[0]));
        CHECK(state.sym_voltage_sensor_asym_output[1].u_angle_residual[1] ==
              doctest::Approx(0.2 - state.asym_node[1].u_angle[1]));
        CHECK(state.sym_voltage_sensor_asym_output[1].u_angle_residual[2] ==
              doctest::Approx(0.2 - state.asym_node[1].u_angle[2]));
    }

    // Note that only 1/3 of the values is being checked
    SUBCASE("AsymPowerSensor, asym output") { // TODO(mgovers): sensor output for powerflow calculations are not tested
                                              // elsewhere
        main_model.output_result<Line>(res, state.asym_line);
        main_model.output_result<Link>(res, state.asym_link);
        main_model.output_result<Source>(res, state.asym_source);
        main_model.output_result<SymLoad>(res, state.asym_load_sym);
        main_model.output_result<AsymLoad>(res, state.asym_load_asym);
        main_model.output_result<Shunt>(res, state.asym_shunt);
        main_model.output_result<AsymPowerSensor>(res, state.asym_power_sensor);

        CHECK(state.asym_power_sensor[0].p_residual[0] == doctest::Approx(2.11e6 - state.asym_line[0].p_from[0]));
        CHECK(state.asym_power_sensor[0].q_residual[1] == doctest::Approx(2.12e3 - state.asym_line[0].q_from[1]));
        CHECK(state.asym_power_sensor[1].p_residual[1] == doctest::Approx(2.32e6 - state.asym_source[0].p[1]));
        CHECK(state.asym_power_sensor[1].q_residual[2] == doctest::Approx(2.33e3 - state.asym_source[0].q[2]));
        CHECK(state.asym_power_sensor[2].p_residual[0] == doctest::Approx(2.41e6 - state.asym_source[0].p[0]));
        CHECK(state.asym_power_sensor[2].q_residual[1] == doctest::Approx(2.42e3 - state.asym_source[0].q[1]));
        CHECK(state.asym_power_sensor[3].p_residual[2] == doctest::Approx(2.53e6 - state.asym_shunt[0].p[2]));
        CHECK(state.asym_power_sensor[3].q_residual[0] == doctest::Approx(2.51e3 - state.asym_shunt[0].q[0]));
        CHECK(state.asym_power_sensor[4].p_residual[1] == doctest::Approx(2.62e6 - state.asym_load_sym[0].p[1]));
        CHECK(state.asym_power_sensor[4].q_residual[2] == doctest::Approx(2.63e3 - state.asym_load_sym[0].q[2]));
        CHECK(state.asym_power_sensor[5].p_residual[0] == doctest::Approx(2.71e6 - state.asym_load_asym[0].p[0]));
        CHECK(state.asym_power_sensor[5].q_residual[1] == doctest::Approx(2.72e3 - state.asym_load_asym[0].q[1]));
        CHECK(state.asym_power_sensor[6].p_residual[0] ==
              doctest::Approx(
                  5.01e6 - (state.asym_source[1].p[0] - state.asym_load_sym[0].p[0] - state.asym_load_asym[0].p[0])));
        CHECK(state.asym_power_sensor[6].q_residual[1] ==
              doctest::Approx(
                  5.02e3 - (state.asym_source[1].q[1] - state.asym_load_sym[0].q[1] - state.asym_load_asym[0].q[1])));
    }

    SUBCASE("SymPowerSensor, asym output") { // TODO(mgovers): sensor output for powerflow calculations are not tested
                                             // elsewhere
        main_model.output_result<Line>(res, state.asym_line);
        main_model.output_result<Link>(res, state.asym_link);
        main_model.output_result<Source>(res, state.asym_source);
        main_model.output_result<SymLoad>(res, state.asym_load_sym);
        main_model.output_result<AsymLoad>(res, state.asym_load_asym);
        main_model.output_result<Shunt>(res, state.asym_shunt);
        main_model.output_result<SymPowerSensor>(res, state.sym_power_sensor_asym_output);

        CHECK(state.sym_power_sensor_asym_output[0].p_residual[0] ==
              doctest::Approx(1.1e6 / 3 - state.asym_line[0].p_from[0]));
        CHECK(state.sym_power_sensor_asym_output[0].q_residual[1] ==
              doctest::Approx(1.1e3 / 3 - state.asym_line[0].q_from[1]));
        CHECK(state.sym_power_sensor_asym_output[1].p_residual[1] ==
              doctest::Approx(1.3e6 / 3 - state.asym_source[0].p[1]));
        CHECK(state.sym_power_sensor_asym_output[1].q_residual[2] ==
              doctest::Approx(1.3e3 / 3 - state.asym_source[0].q[2]));
        CHECK(state.sym_power_sensor_asym_output[2].p_residual[0] ==
              doctest::Approx(1.4e6 / 3 - state.asym_source[0].p[0]));
        CHECK(state.sym_power_sensor_asym_output[2].q_residual[1] ==
              doctest::Approx(1.4e3 / 3 - state.asym_source[0].q[1]));
        CHECK(state.sym_power_sensor_asym_output[3].p_residual[2] ==
              doctest::Approx(1.5e6 / 3 - state.asym_shunt[0].p[2]));
        CHECK(state.sym_power_sensor_asym_output[3].q_residual[0] ==
              doctest::Approx(1.5e3 / 3 - state.asym_shunt[0].q[0]));
        CHECK(state.sym_power_sensor_asym_output[4].p_residual[1] ==
              doctest::Approx(1.6e6 / 3 - state.asym_load_sym[0].p[1]));
        CHECK(state.sym_power_sensor_asym_output[4].q_residual[2] ==
              doctest::Approx(1.6e3 / 3 - state.asym_load_sym[0].q[2]));
        CHECK(state.sym_power_sensor_asym_output[5].p_residual[0] ==
              doctest::Approx(1.7e6 / 3 - state.asym_load_asym[0].p[0]));
        CHECK(state.sym_power_sensor_asym_output[5].q_residual[1] ==
              doctest::Approx(1.7e3 / 3 - state.asym_load_asym[0].q[1]));
        CHECK(state.sym_power_sensor_asym_output[6].p_residual[0] ==
              doctest::Approx(3.0e6 / 3 - (state.asym_source[1].p[0] - state.asym_load_sym[0].p[0] -
                                           state.asym_load_asym[0].p[0])));
        CHECK(state.sym_power_sensor_asym_output[6].q_residual[1] ==
              doctest::Approx(3.0e3 / 3 - (state.asym_source[1].q[1] - state.asym_load_sym[0].q[1] -
                                           state.asym_load_asym[0].q[1])));
    }
}

TEST_CASE("Test main model - linear calculation") {
    State state;
    auto main_model = default_model(state);

    SUBCASE("Symmetrical") { // TODO(mgovers): tested above and in power_flow/dummy-test/sym_output.json
        auto const solver_output =
            main_model.calculate<power_flow_t, symmetric_t>(get_default_options(symmetric, CalculationMethod::linear));
        main_model.output_result<Node>(solver_output, state.sym_node);
        main_model.output_result<Branch>(solver_output, state.sym_branch);
        main_model.output_result<Appliance>(solver_output, state.sym_appliance);
        CHECK(state.sym_node[0].u_pu == doctest::Approx(1.05));
        CHECK(state.sym_node[1].u_pu == doctest::Approx(test::u1));
        CHECK(state.sym_node[2].u_pu == doctest::Approx(test::u1));
        CHECK(state.sym_branch[0].i_from == doctest::Approx(test::i));
        CHECK(state.sym_appliance[0].i == doctest::Approx(test::i));
        CHECK(state.sym_appliance[1].i == doctest::Approx(0.0));
        CHECK(state.sym_appliance[2].i == doctest::Approx(test::i_load));
        CHECK(state.sym_appliance[3].i == doctest::Approx(test::i_load));
        CHECK(state.sym_appliance[4].i == doctest::Approx(test::i_shunt));
    }
    SUBCASE("Asymmetrical") { // TODO(mgovers): tested above and in power_flow/dummy-test/asym_output.json
        auto const solver_output = main_model.calculate<power_flow_t, asymmetric_t>(
            get_default_options(asymmetric, CalculationMethod::linear));
        main_model.output_result<Node>(solver_output, state.asym_node);
        main_model.output_result<Branch>(solver_output, state.asym_branch);
        main_model.output_result<Appliance>(solver_output, state.asym_appliance);
        CHECK(state.asym_node[0].u_pu(0) == doctest::Approx(1.05));
        CHECK(state.asym_node[1].u_pu(1) == doctest::Approx(test::u1));
        CHECK(state.asym_node[2].u_pu(2) == doctest::Approx(test::u1));
        CHECK(state.asym_branch[0].i_from(0) == doctest::Approx(test::i));
        CHECK(state.asym_appliance[0].i(1) == doctest::Approx(test::i));
        CHECK(state.asym_appliance[1].i(2) == doctest::Approx(0.0));
        CHECK(state.asym_appliance[2].i(0) == doctest::Approx(test::i_load));
        CHECK(state.asym_appliance[3].i(1) == doctest::Approx(test::i_load));
        CHECK(state.asym_appliance[4].i(2) == doctest::Approx(test::i_shunt));
    }
}

TEST_CASE_TEMPLATE("Test main model - unknown id", settings, regular_update,
                   cached_update) { // TODO(mgovers): we need this test
    State const state;
    auto main_model = default_model(state);

    std::vector<SourceUpdate> const source_update2{SourceUpdate{100, true, nan, nan}};
    ConstDataset update_data{false, 1, "update", meta_data::meta_data_gen::meta_data};
    update_data.add_buffer("source", source_update2.size(), source_update2.size(), nullptr, source_update2.data());
    CHECK_THROWS_AS((main_model.update_component<typename settings::update_type>(update_data)), IDNotFound);
}

TEST_CASE_TEMPLATE(
    "Test main model - update only load", settings, regular_update,
    cached_update) { // TODO(mgovers): we should whitebox-test this instead; values not reproduced by validation tests
    State state;
    auto main_model = default_model(state);

    ConstDataset update_data{false, 1, "update", meta_data::meta_data_gen::meta_data};
    update_data.add_buffer("sym_load", state.sym_load_update.size(), state.sym_load_update.size(), nullptr,
                           state.sym_load_update.data());
    update_data.add_buffer("asym_load", state.asym_load_update.size(), state.asym_load_update.size(), nullptr,
                           state.asym_load_update.data());
    main_model.update_component<typename settings::update_type>(update_data);

    SUBCASE("Symmetrical") {
        auto const solver_output =
            main_model.calculate<power_flow_t, symmetric_t>(get_default_options(symmetric, CalculationMethod::linear));
        main_model.output_result<Node>(solver_output, state.sym_node);
        main_model.output_result<Branch>(solver_output, state.sym_branch);
        main_model.output_result<Appliance>(solver_output, state.sym_appliance);
        CHECK(state.sym_node[0].u_pu == doctest::Approx(1.05));
        CHECK(state.sym_node[1].u_pu == doctest::Approx(test::u1));
        CHECK(state.sym_node[2].u_pu == doctest::Approx(test::u1));
        CHECK(state.sym_branch[0].i_from == doctest::Approx(test::i));
        CHECK(state.sym_appliance[0].i == doctest::Approx(test::i));
        CHECK(state.sym_appliance[1].i == doctest::Approx(0.0));
        CHECK(state.sym_appliance[2].i == doctest::Approx(test::i_load * 2));
        CHECK(state.sym_appliance[3].i == doctest::Approx(0.0));
        CHECK(state.sym_appliance[4].i == doctest::Approx(test::i_shunt));
    }
    SUBCASE("Asymmetrical") {
        auto const solver_output = main_model.calculate<power_flow_t, asymmetric_t>(
            get_default_options(asymmetric, CalculationMethod::linear));
        main_model.output_result<Node>(solver_output, state.asym_node);
        main_model.output_result<Branch>(solver_output, state.asym_branch);
        main_model.output_result<Appliance>(solver_output, state.asym_appliance);
        CHECK(state.asym_node[0].u_pu(0) == doctest::Approx(1.05));
        CHECK(state.asym_node[1].u_pu(1) == doctest::Approx(test::u1));
        CHECK(state.asym_node[2].u_pu(2) == doctest::Approx(test::u1));
        CHECK(state.asym_branch[0].i_from(0) == doctest::Approx(test::i));
        CHECK(state.asym_appliance[0].i(1) == doctest::Approx(test::i));
        CHECK(state.asym_appliance[1].i(2) == doctest::Approx(0.0));
        CHECK(state.asym_appliance[2].i(0) == doctest::Approx(test::i_load * 2));
        CHECK(state.asym_appliance[3].i(1) == doctest::Approx(0.0));
        CHECK(state.asym_appliance[4].i(2) == doctest::Approx(test::i_shunt));
    }
}

TEST_CASE_TEMPLATE(
    "Test main model - update load and shunt param", settings, regular_update,
    cached_update) { // TODO(mgovers): we should whitebox-test this instead; values not reproduced by validation tests
    State state;
    auto main_model = default_model(state);

    state.sym_load_update[0].p_specified = 2.5e6;
    ConstDataset update_data{false, 1, "update", meta_data::meta_data_gen::meta_data};
    update_data.add_buffer("sym_load", state.sym_load_update.size(), state.sym_load_update.size(), nullptr,
                           state.sym_load_update.data());
    update_data.add_buffer("asym_load", state.asym_load_update.size(), state.asym_load_update.size(), nullptr,
                           state.asym_load_update.data());
    update_data.add_buffer("shunt", state.shunt_update.size(), state.shunt_update.size(), nullptr,
                           state.shunt_update.data());
    main_model.update_component<typename settings::update_type>(update_data);

    SUBCASE("Symmetrical") {
        auto const solver_output =
            main_model.calculate<power_flow_t, symmetric_t>(get_default_options(symmetric, CalculationMethod::linear));
        main_model.output_result<Node>(solver_output, state.sym_node);
        main_model.output_result<Branch>(solver_output, state.sym_branch);
        main_model.output_result<Appliance>(solver_output, state.sym_appliance);
        CHECK(state.sym_node[0].u_pu == doctest::Approx(1.05));
        CHECK(state.sym_node[1].u_pu == doctest::Approx(test::u1));
        CHECK(state.sym_node[2].u_pu == doctest::Approx(test::u1));
        CHECK(state.sym_branch[0].i_from == doctest::Approx(test::i));
        CHECK(state.sym_appliance[0].i == doctest::Approx(test::i));
        CHECK(state.sym_appliance[1].i == doctest::Approx(0.0));
        CHECK(state.sym_appliance[2].i == doctest::Approx(test::i_load * 2 + test::i_shunt));
        CHECK(state.sym_appliance[3].i == doctest::Approx(0.0));
        CHECK(state.sym_appliance[4].i == doctest::Approx(0.0));
    }
    SUBCASE("Asymmetrical") {
        auto const solver_output = main_model.calculate<power_flow_t, asymmetric_t>(
            get_default_options(asymmetric, CalculationMethod::linear));
        main_model.output_result<Node>(solver_output, state.asym_node);
        main_model.output_result<Branch>(solver_output, state.asym_branch);
        main_model.output_result<Appliance>(solver_output, state.asym_appliance);
        CHECK(state.asym_node[0].u_pu(0) == doctest::Approx(1.05));
        CHECK(state.asym_node[1].u_pu(1) == doctest::Approx(test::u1));
        CHECK(state.asym_node[2].u_pu(2) == doctest::Approx(test::u1));
        CHECK(state.asym_branch[0].i_from(0) == doctest::Approx(test::i));
        CHECK(state.asym_appliance[0].i(1) == doctest::Approx(test::i));
        CHECK(state.asym_appliance[1].i(2) == doctest::Approx(0.0));
        CHECK(state.asym_appliance[2].i(0) == doctest::Approx(test::i_load * 2 + test::i_shunt));
        CHECK(state.asym_appliance[3].i(1) == doctest::Approx(0.0));
        CHECK(state.asym_appliance[4].i(2) == doctest::Approx(0.0));
    }
}

TEST_CASE_TEMPLATE(
    "Test main model - all updates", settings, regular_update,
    cached_update) { // TODO(mgovers): we should whitebox-test this instead; values not reproduced by validation tests
    State state;
    auto main_model = default_model(state);

    state.sym_load_update[0].p_specified = 2.5e6;
    ConstDataset update_data{false, 1, "update", meta_data::meta_data_gen::meta_data};
    update_data.add_buffer("sym_load", state.sym_load_update.size(), state.sym_load_update.size(), nullptr,
                           state.sym_load_update.data());
    update_data.add_buffer("asym_load", state.asym_load_update.size(), state.asym_load_update.size(), nullptr,
                           state.asym_load_update.data());
    update_data.add_buffer("shunt", state.shunt_update.size(), state.shunt_update.size(), nullptr,
                           state.shunt_update.data());
    update_data.add_buffer("source", state.source_update.size(), state.source_update.size(), nullptr,
                           state.source_update.data());
    update_data.add_buffer("link", state.link_update.size(), state.link_update.size(), nullptr,
                           state.link_update.data());
    update_data.add_buffer("fault", state.fault_update.size(), state.fault_update.size(), nullptr,
                           state.fault_update.data());

    main_model.update_component<typename settings::update_type>(update_data);

    SUBCASE("Symmetrical") {
        auto const solver_output =
            main_model.calculate<power_flow_t, symmetric_t>(get_default_options(symmetric, CalculationMethod::linear));
        main_model.output_result<Node>(solver_output, state.sym_node);
        main_model.output_result<Branch>(solver_output, state.sym_branch);
        main_model.output_result<Appliance>(solver_output, state.sym_appliance);
        CHECK(state.sym_node[0].u_pu == doctest::Approx(1.05));
        CHECK(state.sym_node[1].u_pu == doctest::Approx(1.05));
        CHECK(state.sym_node[2].u_pu == doctest::Approx(test::u1));
        CHECK(state.sym_branch[0].i_from == doctest::Approx(0.0).epsilon(1e-6));
        CHECK(state.sym_appliance[0].i == doctest::Approx(0.0).epsilon(1e-6));
        CHECK(state.sym_appliance[1].i == doctest::Approx(test::i));
        CHECK(state.sym_appliance[2].i == doctest::Approx(test::i));
        CHECK(state.sym_appliance[3].i == doctest::Approx(0.0));
        CHECK(state.sym_appliance[4].i == doctest::Approx(0.0));
    }
    SUBCASE("Asymmetrical") {
        auto const solver_output = main_model.calculate<power_flow_t, asymmetric_t>(
            get_default_options(asymmetric, CalculationMethod::linear));
        main_model.output_result<Node>(solver_output, state.asym_node);
        main_model.output_result<Branch>(solver_output, state.asym_branch);
        main_model.output_result<Appliance>(solver_output, state.asym_appliance);
        CHECK(state.asym_node[0].u_pu(0) == doctest::Approx(1.05));
        CHECK(state.asym_node[1].u_pu(1) == doctest::Approx(1.05));
        CHECK(state.asym_node[2].u_pu(2) == doctest::Approx(test::u1));
        CHECK(state.asym_branch[0].i_from(0) == doctest::Approx(0.0).epsilon(1e-6));
        CHECK(state.asym_appliance[0].i(1) == doctest::Approx(0.0).epsilon(1e-6));
        CHECK(state.asym_appliance[1].i(2) == doctest::Approx(test::i));
        CHECK(state.asym_appliance[2].i(0) == doctest::Approx(test::i));
        CHECK(state.asym_appliance[3].i(1) == doctest::Approx(0.0));
        CHECK(state.asym_appliance[4].i(2) == doctest::Approx(0.0));
    }
}

TEST_CASE_TEMPLATE("Test main model - single permanent update from batch", settings, regular_update,
                   cached_update) { // TODO(mgovers): we should whitebox-test this instead
    State state;
    auto main_model = default_model(state);

    state.batch_sym_load_update[0].p_specified = 2.5e6;
    ConstDataset update_data{true, 5, "update", meta_data::meta_data_gen::meta_data};
    update_data.add_buffer("sym_load", 1, state.batch_sym_load_update.size(), nullptr,
                           state.batch_sym_load_update.data());
    update_data.add_buffer("asym_load", 1, state.batch_asym_load_update.size(), nullptr,
                           state.batch_asym_load_update.data());
    update_data.add_buffer("shunt", 1, state.batch_shunt_update.size(), nullptr, state.batch_shunt_update.data());
    update_data.add_buffer("source", 1, state.batch_source_update.size(), nullptr, state.batch_source_update.data());
    update_data.add_buffer("link", 1, state.batch_link_update.size(), nullptr, state.batch_link_update.data());
    update_data.add_buffer("fault", 1, state.batch_fault_update.size(), nullptr, state.batch_fault_update.data());

    main_model.update_component<typename settings::update_type>(update_data);

    SUBCASE("Symmetrical") {
        auto const solver_output =
            main_model.calculate<power_flow_t, symmetric_t>(get_default_options(symmetric, CalculationMethod::linear));
        main_model.output_result<Node>(solver_output, state.sym_node);
        main_model.output_result<Branch>(solver_output, state.sym_branch);
        main_model.output_result<Appliance>(solver_output, state.sym_appliance);
        CHECK(state.sym_node[0].u_pu == doctest::Approx(1.05));
        CHECK(state.sym_node[1].u_pu == doctest::Approx(1.05));
        CHECK(state.sym_node[2].u_pu == doctest::Approx(test::u1));
        CHECK(state.sym_branch[0].i_from == doctest::Approx(0.0).epsilon(1e-6));
        CHECK(state.sym_appliance[0].i == doctest::Approx(0.0).epsilon(1e-6));
        CHECK(state.sym_appliance[1].i == doctest::Approx(test::i));
        CHECK(state.sym_appliance[2].i == doctest::Approx(test::i));
        CHECK(state.sym_appliance[3].i == doctest::Approx(0.0));
        CHECK(state.sym_appliance[4].i == doctest::Approx(0.0));
    }
    SUBCASE("Asymmetrical") {
        auto const solver_output = main_model.calculate<power_flow_t, asymmetric_t>(
            get_default_options(asymmetric, CalculationMethod::linear));
        main_model.output_result<Node>(solver_output, state.asym_node);
        main_model.output_result<Branch>(solver_output, state.asym_branch);
        main_model.output_result<Appliance>(solver_output, state.asym_appliance);
        CHECK(state.asym_node[0].u_pu(0) == doctest::Approx(1.05));
        CHECK(state.asym_node[1].u_pu(1) == doctest::Approx(1.05));
        CHECK(state.asym_node[2].u_pu(2) == doctest::Approx(test::u1));
        CHECK(state.asym_branch[0].i_from(0) == doctest::Approx(0.0).epsilon(1e-6));
        CHECK(state.asym_appliance[0].i(1) == doctest::Approx(0.0).epsilon(1e-6));
        CHECK(state.asym_appliance[1].i(2) == doctest::Approx(test::i));
        CHECK(state.asym_appliance[2].i(0) == doctest::Approx(test::i));
        CHECK(state.asym_appliance[3].i(1) == doctest::Approx(0.0));
        CHECK(state.asym_appliance[4].i(2) == doctest::Approx(0.0));
    }
}

TEST_CASE_TEMPLATE("Test main model - restore components", settings, regular_update,
                   cached_update) { // TODO(mgovers): either whitebox (as a sub-part of batch impl) or otherwise drop
                                    // entirely (tested by batch update)
    State state;
    auto main_model = default_model(state);

    auto const solver_output_orig =
        main_model.calculate<power_flow_t, symmetric_t>(get_default_options(symmetric, CalculationMethod::linear));

    ConstDataset update_data{false, 1, "update", meta_data::meta_data_gen::meta_data};
    update_data.add_buffer("sym_load", state.sym_load_update.size(), state.sym_load_update.size(), nullptr,
                           state.sym_load_update.data());
    update_data.add_buffer("asym_load", state.asym_load_update.size(), state.asym_load_update.size(), nullptr,
                           state.asym_load_update.data());

    main_model.update_component<typename settings::update_type>(update_data);
    main_model.restore_components(update_data);

    SUBCASE("Symmetrical") {
        auto const solver_output_result =
            main_model.calculate<power_flow_t, symmetric_t>(get_default_options(symmetric, CalculationMethod::linear));
        main_model.output_result<Node>(solver_output_result, state.sym_node);
        main_model.output_result<Branch>(solver_output_result, state.sym_branch);
        main_model.output_result<Appliance>(solver_output_result, state.sym_appliance);

        CHECK(state.sym_node[0].u_pu == doctest::Approx(1.05));
        CHECK(state.sym_node[1].u_pu == doctest::Approx(test::u1));
        CHECK(state.sym_node[2].u_pu == doctest::Approx(test::u1));
        CHECK(state.sym_branch[0].i_from == doctest::Approx(test::i));
        CHECK(state.sym_appliance[0].i == doctest::Approx(test::i));
        CHECK(state.sym_appliance[1].i == doctest::Approx(0.0));
        if constexpr (settings::update_type::value) {
            CHECK(state.sym_appliance[2].i == doctest::Approx(test::i_load));
            CHECK(state.sym_appliance[3].i == doctest::Approx(test::i_load));
        } else {
            CHECK(state.sym_appliance[2].i == doctest::Approx(test::i_load * 2));
            CHECK(state.sym_appliance[3].i == doctest::Approx(0.0));
        }
        CHECK(state.sym_appliance[4].i == doctest::Approx(test::i_shunt));
    }
    SUBCASE("Asymmetrical") {
        auto const solver_output = main_model.calculate<power_flow_t, asymmetric_t>(
            get_default_options(asymmetric, CalculationMethod::linear));
        main_model.output_result<Node>(solver_output, state.asym_node);
        main_model.output_result<Branch>(solver_output, state.asym_branch);
        main_model.output_result<Appliance>(solver_output, state.asym_appliance);

        CHECK(state.asym_node[0].u_pu(0) == doctest::Approx(1.05));
        CHECK(state.asym_node[1].u_pu(1) == doctest::Approx(test::u1));
        CHECK(state.asym_node[2].u_pu(2) == doctest::Approx(test::u1));
        CHECK(state.asym_branch[0].i_from(0) == doctest::Approx(test::i));
        CHECK(state.asym_appliance[0].i(1) == doctest::Approx(test::i));
        CHECK(state.asym_appliance[1].i(2) == doctest::Approx(0.0));
        if constexpr (settings::update_type::value) {
            CHECK(state.asym_appliance[2].i(0) == doctest::Approx(test::i_load));
            CHECK(state.asym_appliance[3].i(1) == doctest::Approx(test::i_load));
        } else {
            CHECK(state.asym_appliance[2].i(0) == doctest::Approx(test::i_load * 2));
            CHECK(state.asym_appliance[3].i(1) == doctest::Approx(0.0));
        }
        CHECK(state.asym_appliance[4].i(2) == doctest::Approx(test::i_shunt));
    }
}

TEST_CASE_TEMPLATE("Test main model - updates w/ alternating compute mode", settings, regular_update,
                   cached_update) { // TODO(mgovers): we need to keep this one way or another
    constexpr auto check_sym = [](MainModel const& model_, auto const& math_output_) {
        State state_;
        model_.output_result<Node>(math_output_, state_.sym_node);
        model_.output_result<Branch>(math_output_, state_.sym_branch);
        model_.output_result<Appliance>(math_output_, state_.sym_appliance);

        CHECK(state_.sym_node[0].u_pu == doctest::Approx(1.05));
        CHECK(state_.sym_node[1].u_pu == doctest::Approx(test::u1));
        CHECK(state_.sym_node[2].u_pu == doctest::Approx(test::u1));
        CHECK(state_.sym_branch[0].i_from == doctest::Approx(test::i));
        CHECK(state_.sym_appliance[0].i == doctest::Approx(test::i));
        CHECK(state_.sym_appliance[1].i == doctest::Approx(0.0));
        CHECK(state_.sym_appliance[2].i == doctest::Approx(test::i_load * 2 + test::i_shunt));
        CHECK(state_.sym_appliance[3].i == doctest::Approx(0.0));
        CHECK(state_.sym_appliance[4].i == doctest::Approx(0.0));
    };
    constexpr auto check_asym = [](MainModel const& model_, auto const& math_output_) {
        State state_;
        model_.output_result<Node>(math_output_, state_.asym_node);
        model_.output_result<Branch>(math_output_, state_.asym_branch);
        model_.output_result<Appliance>(math_output_, state_.asym_appliance);
        CHECK(state_.asym_node[0].u_pu(0) == doctest::Approx(1.05));
        CHECK(state_.asym_node[1].u_pu(1) == doctest::Approx(test::u1));
        CHECK(state_.asym_node[2].u_pu(2) == doctest::Approx(test::u1));
        CHECK(state_.asym_branch[0].i_from(0) == doctest::Approx(test::i));
        CHECK(state_.asym_appliance[0].i(1) == doctest::Approx(test::i));
        CHECK(state_.asym_appliance[1].i(2) == doctest::Approx(0.0));
        CHECK(state_.asym_appliance[2].i(0) == doctest::Approx(test::i_load * 2 + test::i_shunt));
        CHECK(state_.asym_appliance[3].i(1) == doctest::Approx(0.0));
        CHECK(state_.asym_appliance[4].i(2) == doctest::Approx(0.0));
    };

    State state;
    auto main_model = default_model(state);

    state.sym_load_update[0].p_specified = 2.5e6;

    ConstDataset update_data{false, 1, "update", meta_data::meta_data_gen::meta_data};
    update_data.add_buffer("sym_load", state.sym_load_update.size(), state.sym_load_update.size(), nullptr,
                           state.sym_load_update.data());
    update_data.add_buffer("asym_load", state.asym_load_update.size(), state.asym_load_update.size(), nullptr,
                           state.asym_load_update.data());
    update_data.add_buffer("shunt", state.shunt_update.size(), state.shunt_update.size(), nullptr,
                           state.shunt_update.data());

    // This will lead to no topo change but param change
    main_model.update_component<typename settings::update_type>(update_data);

    auto const math_output_sym_1 =
        main_model.calculate<power_flow_t, symmetric_t>(get_default_options(symmetric, CalculationMethod::linear));
    check_sym(main_model, math_output_sym_1);

    auto const math_output_asym_1 =
        main_model.calculate<power_flow_t, asymmetric_t>(get_default_options(asymmetric, CalculationMethod::linear));
    check_asym(main_model, math_output_asym_1);

    SUBCASE("No new update") {
        // Math state may be fully cached
    }
    if constexpr (std::same_as<typename settings::update_type, regular_update>) {
        SUBCASE("No new parameter change") {
            // Math state may be fully cached due to no change
            main_model.update_component<typename settings::update_type>(update_data);
        }
    }
    SUBCASE("With parameter change") {
        // Restore to original state and re-apply same update: causes param change for cached update
        main_model.restore_components(update_data);
        main_model.update_component<typename settings::update_type>(update_data);
    }

    auto const math_output_asym_2 =
        main_model.calculate<power_flow_t, asymmetric_t>(get_default_options(asymmetric, CalculationMethod::linear));
    check_asym(main_model, math_output_asym_2);

    auto const math_output_sym_2 =
        main_model.calculate<power_flow_t, symmetric_t>(get_default_options(symmetric, CalculationMethod::linear));
    check_sym(main_model, math_output_sym_2);

    main_model.restore_components(update_data);
}

TEST_CASE("Test main model - runtime dispatch") {
    using CalculationMethod::newton_raphson;

    State state;
    auto main_model = default_model(state);

    ConstDataset input_data{false, 1, "input", meta_data::meta_data_gen::meta_data};
    input_data.add_buffer("node", state.node_input.size(), state.node_input.size(), nullptr, state.node_input.data());
    input_data.add_buffer("line", state.line_input.size(), state.line_input.size(), nullptr, state.line_input.data());
    input_data.add_buffer("link", state.link_input.size(), state.link_input.size(), nullptr, state.link_input.data());
    input_data.add_buffer("source", state.source_input.size(), state.source_input.size(), nullptr,
                          state.source_input.data());
    input_data.add_buffer("sym_load", state.sym_load_input.size(), state.sym_load_input.size(), nullptr,
                          state.sym_load_input.data());
    input_data.add_buffer("asym_load", state.asym_load_input.size(), state.asym_load_input.size(), nullptr,
                          state.asym_load_input.data());
    input_data.add_buffer("shunt", state.shunt_input.size(), state.shunt_input.size(), nullptr,
                          state.shunt_input.data());

    SUBCASE("Single-size batches") { // TODO(mgovers): can be removed: validation cases already do single +
                                     // multi-threaded; whitebox test should be isolated
        ConstDataset update_data{true, 1, "update", meta_data::meta_data_gen::meta_data};
        update_data.add_buffer("sym_load", state.sym_load_update.size(), state.sym_load_update.size(), nullptr,
                               state.sym_load_update.data());
        update_data.add_buffer("asym_load", state.asym_load_update.size(), state.asym_load_update.size(), nullptr,
                               state.asym_load_update.data());
        update_data.add_buffer("shunt", state.shunt_update.size(), state.shunt_update.size(), nullptr,
                               state.shunt_update.data());
        update_data.add_buffer("source", state.source_update.size(), state.source_update.size(), nullptr,
                               state.source_update.data());
        update_data.add_buffer("link", state.link_update.size(), state.link_update.size(), nullptr,
                               state.link_update.data());

        MutableDataset sym_result_data{true, 1, "sym_output", meta_data::meta_data_gen::meta_data};
        sym_result_data.add_buffer("node", state.sym_node.size(), state.sym_node.size(), nullptr,
                                   state.sym_node.data());
        sym_result_data.add_buffer("line", state.sym_line.size(), state.sym_line.size(), nullptr,
                                   state.sym_line.data());
        sym_result_data.add_buffer("link", state.sym_link.size(), state.sym_link.size(), nullptr,
                                   state.sym_link.data());
        sym_result_data.add_buffer("source", state.sym_source.size(), state.sym_source.size(), nullptr,
                                   state.sym_source.data());
        sym_result_data.add_buffer("sym_load", state.sym_load_sym.size(), state.sym_load_sym.size(), nullptr,
                                   state.sym_load_sym.data());
        sym_result_data.add_buffer("asym_load", state.sym_load_asym.size(), state.sym_load_asym.size(), nullptr,
                                   state.sym_load_asym.data());
        sym_result_data.add_buffer("shunt", state.sym_shunt.size(), state.sym_shunt.size(), nullptr,
                                   state.sym_shunt.data());

        MutableDataset asym_result_data{true, 1, "asym_output", meta_data::meta_data_gen::meta_data};
        asym_result_data.add_buffer("node", state.asym_node.size(), state.asym_node.size(), nullptr,
                                    state.asym_node.data());

        MainModel model{50.0, input_data};
        auto const count = model.all_component_count();
        CHECK(count.at("node") == 3);
        CHECK(count.at("source") == 2);
        CHECK(count.find("sym_gen") == count.cend());

        // calculation
        model.calculate(get_default_options(symmetric, newton_raphson), sym_result_data);
        CHECK(state.sym_node[0].u_pu == doctest::Approx(1.05));
        CHECK(state.sym_node[1].u_pu == doctest::Approx(test::u1));
        CHECK(state.sym_node[2].u_pu == doctest::Approx(test::u1));
        CHECK(state.sym_line[0].i_from == doctest::Approx(test::i));
        CHECK(state.sym_link[0].i_from == doctest::Approx(test::i));
        CHECK(state.sym_source[0].i == doctest::Approx(test::i));
        CHECK(state.sym_source[1].i == doctest::Approx(0.0));
        CHECK(state.sym_load_sym[0].i == doctest::Approx(test::i_load));
        CHECK(state.sym_load_asym[0].i == doctest::Approx(test::i_load));
        CHECK(state.sym_shunt[0].i == doctest::Approx(test::i_shunt));
        model.calculate(get_default_options(asymmetric, newton_raphson), asym_result_data);
        CHECK(state.asym_node[0].u_pu(0) == doctest::Approx(1.05));
        CHECK(state.asym_node[1].u_pu(1) == doctest::Approx(test::u1));
        CHECK(state.asym_node[2].u_pu(2) == doctest::Approx(test::u1));

        // update and calculation
        model.update_component<permanent_update_t>(update_data);
        model.calculate(get_default_options(symmetric, newton_raphson), sym_result_data);
        CHECK(state.sym_node[0].u_pu == doctest::Approx(1.05));
        CHECK(state.sym_node[1].u_pu == doctest::Approx(1.05));
        CHECK(state.sym_node[2].u_pu == doctest::Approx(test::u1));
        model.calculate(get_default_options(asymmetric, newton_raphson), asym_result_data);
        CHECK(state.asym_node[0].u_pu(0) == doctest::Approx(1.05));
        CHECK(state.asym_node[1].u_pu(1) == doctest::Approx(1.05));
        CHECK(state.asym_node[2].u_pu(2) == doctest::Approx(test::u1));

        // test batch calculation
        model = MainModel{50.0, input_data};
        // symmetric sequential
        model.calculate(get_default_options(symmetric, newton_raphson), sym_result_data, update_data);
        CHECK(state.sym_node[0].u_pu == doctest::Approx(1.05));
        CHECK(state.sym_node[1].u_pu == doctest::Approx(1.05));
        CHECK(state.sym_node[2].u_pu == doctest::Approx(test::u1));
        // symmetric parallel
        model.calculate(get_default_options(symmetric, newton_raphson, 0), sym_result_data, update_data);
        CHECK(state.sym_node[0].u_pu == doctest::Approx(1.05));
        CHECK(state.sym_node[1].u_pu == doctest::Approx(1.05));
        CHECK(state.sym_node[2].u_pu == doctest::Approx(test::u1));
        // asymmetric sequential
        model.calculate(get_default_options(asymmetric, newton_raphson), asym_result_data, update_data);
        CHECK(state.asym_node[0].u_pu(0) == doctest::Approx(1.05));
        CHECK(state.asym_node[1].u_pu(1) == doctest::Approx(1.05));
        CHECK(state.asym_node[2].u_pu(2) == doctest::Approx(test::u1));
        // asymmetric parallel
        model.calculate(get_default_options(asymmetric, newton_raphson, 0), asym_result_data, update_data);
        CHECK(state.asym_node[0].u_pu(0) == doctest::Approx(1.05));
        CHECK(state.asym_node[1].u_pu(1) == doctest::Approx(1.05));
        CHECK(state.asym_node[2].u_pu(2) == doctest::Approx(test::u1));
    }

    SUBCASE("no dependent updates within batches") { // TODO(mgovers): validation cases capture this; whitebox test
                                                     // should be isolated
        MainModel model{50.0, input_data};
        std::vector<SymLoadGenUpdate> sym_load_update_2{{7, 1, nan, 1.0e7}, {7, 1, 1.0e3, nan}, {7, 1, 1.0e3, 1.0e7}};

        ConstDataset dependent_update_data{true, static_cast<Idx>(sym_load_update_2.size()), "update",
                                           meta_data::meta_data_gen::meta_data};
        MutableDataset dependent_result_data{true, static_cast<Idx>(sym_load_update_2.size()), "sym_output",
                                             meta_data::meta_data_gen::meta_data};

        dependent_update_data.add_buffer("sym_load", 1, sym_load_update_2.size(), nullptr, sym_load_update_2.data());

        std::vector<NodeOutput<symmetric_t>> sym_node_2(sym_load_update_2.size() * state.sym_node.size());
        dependent_result_data.add_buffer("node", state.sym_node.size(), sym_node_2.size(), nullptr, sym_node_2.data());

        model.calculate(get_default_options(symmetric, newton_raphson), dependent_result_data, dependent_update_data);
        CHECK(sym_node_2[0].u_pu == doctest::Approx(1.05));
        CHECK(sym_node_2[1].u_pu == doctest::Approx(0.66).epsilon(0.005));
        CHECK(sym_node_2[2].u_pu == doctest::Approx(0.66).epsilon(0.005));
        CHECK(sym_node_2[3].u_pu == doctest::Approx(1.05));
        CHECK(sym_node_2[4].u_pu == doctest::Approx(0.87).epsilon(0.005));
        CHECK(sym_node_2[5].u_pu == doctest::Approx(0.87).epsilon(0.005));
        CHECK(sym_node_2[6].u_pu == doctest::Approx(1.05));
        CHECK(sym_node_2[7].u_pu == doctest::Approx(0.67).epsilon(0.005));
        CHECK(sym_node_2[8].u_pu == doctest::Approx(0.67).epsilon(0.005));
    }

    SUBCASE("Columnar buffers in dataset") { // TODO(mgovers): API tests now capture this; validation cases do not test
                                             // columnar buffers
        auto const options = get_default_options(symmetric, CalculationMethod::newton_raphson);

        SUBCASE("Columnar buffers in input data") {
            std::vector<ID> node_ids;
            std::vector<double> node_u_rated;
            std::ranges::transform(state.node_input, std::back_inserter(node_ids),
                                   [](auto const& node) { return node.id; });
            std::ranges::transform(state.node_input, std::back_inserter(node_u_rated),
                                   [](auto const& node) { return node.u_rated; });
            REQUIRE(node_ids.size() == node_u_rated.size());

            ConstDataset input_data_with_columns{false, 1, "input", meta_data::meta_data_gen::meta_data};
            input_data_with_columns.add_buffer("node", state.node_input.size(), state.node_input.size(), nullptr,
                                               nullptr);
            input_data_with_columns.add_attribute_buffer("node", "id", node_ids.data());
            input_data_with_columns.add_attribute_buffer("node", "u_rated", node_u_rated.data());
            input_data_with_columns.add_buffer("line", state.line_input.size(), state.line_input.size(), nullptr,
                                               state.line_input.data());
            input_data_with_columns.add_buffer("link", state.link_input.size(), state.link_input.size(), nullptr,
                                               state.link_input.data());
            input_data_with_columns.add_buffer("source", state.source_input.size(), state.source_input.size(), nullptr,
                                               state.source_input.data());
            input_data_with_columns.add_buffer("sym_load", state.sym_load_input.size(), state.sym_load_input.size(),
                                               nullptr, state.sym_load_input.data());
            input_data_with_columns.add_buffer("asym_load", state.asym_load_input.size(), state.asym_load_input.size(),
                                               nullptr, state.asym_load_input.data());
            input_data_with_columns.add_buffer("shunt", state.shunt_input.size(), state.shunt_input.size(), nullptr,
                                               state.shunt_input.data());

            MainModel row_based_model{50.0, input_data};
            MainModel columnar_model{50.0, input_data_with_columns};

            std::vector<SymNodeOutput> node_output_from_row_based(state.node_input.size());
            std::vector<SymNodeOutput> node_output_from_columnar(node_ids.size());

            MutableDataset sym_output_from_row_based{true, 1, "sym_output", meta_data::meta_data_gen::meta_data};
            sym_output_from_row_based.add_buffer("node", node_output_from_row_based.size(),
                                                 node_output_from_row_based.size(), nullptr,
                                                 node_output_from_row_based.data());
            MutableDataset sym_output_from_columnar{true, 1, "sym_output", meta_data::meta_data_gen::meta_data};
            sym_output_from_columnar.add_buffer("node", node_output_from_columnar.size(),
                                                node_output_from_columnar.size(), nullptr,
                                                node_output_from_columnar.data());

            row_based_model.calculate(options, sym_output_from_row_based);
            columnar_model.calculate(options, sym_output_from_columnar);

            REQUIRE(node_output_from_columnar.size() == node_output_from_row_based.size());

            for (Idx idx = 0; idx < std::ssize(node_output_from_columnar); ++idx) {
                CHECK(node_output_from_columnar[idx].id == node_output_from_row_based[idx].id);
                CHECK(node_output_from_columnar[idx].u_pu == node_output_from_row_based[idx].u_pu);
            }
        }

        SUBCASE("Columnar buffers in output data") {
            MainModel model{50.0, input_data};

            std::vector<SymNodeOutput> row_based_node_output(state.node_input.size());
            std::vector<ID> columnar_node_output_id(state.node_input.size());
            std::vector<double> columnar_node_output_u_pu(state.node_input.size());

            MutableDataset row_based_sym_output{true, 1, "sym_output", meta_data::meta_data_gen::meta_data};
            row_based_sym_output.add_buffer("node", row_based_node_output.size(), row_based_node_output.size(), nullptr,
                                            row_based_node_output.data());
            MutableDataset columnar_sym_output{true, 1, "sym_output", meta_data::meta_data_gen::meta_data};
            columnar_sym_output.add_buffer("node", row_based_node_output.size(), row_based_node_output.size(), nullptr,
                                           nullptr);
            columnar_sym_output.add_attribute_buffer("node", "id", columnar_node_output_id.data());
            columnar_sym_output.add_attribute_buffer("node", "u_pu", columnar_node_output_u_pu.data());

            model.calculate(options, row_based_sym_output);
            model.calculate(options, columnar_sym_output);

            REQUIRE(columnar_node_output_id.size() == row_based_node_output.size());
            REQUIRE(columnar_node_output_u_pu.size() == row_based_node_output.size());

            for (Idx idx = 0; idx < std::ssize(columnar_node_output_id); ++idx) {
                CHECK(columnar_node_output_id[idx] == row_based_node_output[idx].id);
                CHECK(columnar_node_output_u_pu[idx] == doctest::Approx(row_based_node_output[idx].u_pu));
            }
        }

        SUBCASE("Columnar buffers in update data") {
            std::vector<ID> sym_load_ids;
            std::vector<double> sym_load_p_specified;
            std::ranges::transform(state.sym_load_update, std::back_inserter(sym_load_p_specified),
                                   [](auto const& sym_load) { return sym_load.p_specified; });

            std::ranges::transform(state.sym_load_update, std::back_inserter(sym_load_ids),
                                   [](auto const& sym_load) { return sym_load.id; });
            REQUIRE(sym_load_ids.size() == sym_load_p_specified.size());
            REQUIRE(sym_load_p_specified.size() == state.sym_load_update.size());

            auto const update_size = sym_load_ids.size();

            SUBCASE("With IDs") {
                ConstDataset update_data_with_rows{false, 1, "update", meta_data::meta_data_gen::meta_data};
                update_data_with_rows.add_buffer("sym_load", state.sym_load_update.size(), state.sym_load_update.size(),
                                                 nullptr, state.sym_load_update.data());

                ConstDataset update_data_with_columns{false, 1, "update", meta_data::meta_data_gen::meta_data};
                update_data_with_columns.add_buffer("sym_load", update_size, update_size, nullptr, nullptr);
                update_data_with_columns.add_attribute_buffer("sym_load", "id", sym_load_ids.data());
                update_data_with_columns.add_attribute_buffer("sym_load", "p_specified", sym_load_p_specified.data());

                MainModel base_model{50.0, input_data};
                MainModel row_based_model{base_model};
                MainModel columnar_model{base_model};
                row_based_model.update_component<permanent_update_t>(update_data_with_rows);
                columnar_model.update_component<permanent_update_t>(update_data_with_columns);

                std::vector<SymNodeOutput> node_output_from_base(state.node_input.size());
                std::vector<SymNodeOutput> node_output_from_row_based(state.node_input.size());
                std::vector<SymNodeOutput> node_output_from_columnar(state.node_input.size());

                MutableDataset sym_output_from_base{true, 1, "sym_output", meta_data::meta_data_gen::meta_data};
                sym_output_from_base.add_buffer("node", node_output_from_base.size(), node_output_from_base.size(),
                                                nullptr, node_output_from_base.data());
                MutableDataset sym_output_from_row_based{true, 1, "sym_output", meta_data::meta_data_gen::meta_data};
                sym_output_from_row_based.add_buffer("node", node_output_from_row_based.size(),
                                                     node_output_from_row_based.size(), nullptr,
                                                     node_output_from_row_based.data());
                MutableDataset sym_output_from_columnar{true, 1, "sym_output", meta_data::meta_data_gen::meta_data};
                sym_output_from_columnar.add_buffer("node", node_output_from_columnar.size(),
                                                    node_output_from_columnar.size(), nullptr,
                                                    node_output_from_columnar.data());

                base_model.calculate(options, sym_output_from_base);
                row_based_model.calculate(options, sym_output_from_row_based);
                columnar_model.calculate(options, sym_output_from_columnar);

                REQUIRE(node_output_from_columnar.size() == node_output_from_base.size());
                REQUIRE(node_output_from_columnar.size() == node_output_from_row_based.size());

                for (Idx idx = 0; idx < std::ssize(node_output_from_columnar); ++idx) {
                    // check columnar updates work same way as row-based updates
                    CHECK(node_output_from_columnar[idx].id == doctest::Approx(node_output_from_row_based[idx].id));
                    CHECK(node_output_from_columnar[idx].u_pu == doctest::Approx(node_output_from_row_based[idx].u_pu));
                    // check update actually changed something
                    CHECK(node_output_from_columnar[idx].id == doctest::Approx(node_output_from_base[idx].id));
                    if (idx == 0) { // sym_load node
                        CHECK(node_output_from_columnar[idx].u_pu == doctest::Approx(node_output_from_base[idx].u_pu));
                    } else {
                        CHECK(node_output_from_columnar[idx].u_pu != doctest::Approx(node_output_from_base[idx].u_pu));
                    }
                }
            }

            SUBCASE("Without IDs") { // TODO(mgovers): API tests already captured this; validation cases added
                ConstDataset update_data_with_ids{false, 1, "update", meta_data::meta_data_gen::meta_data};
                update_data_with_ids.add_buffer("sym_load", update_size, update_size, nullptr, nullptr);
                update_data_with_ids.add_attribute_buffer("sym_load", "id", sym_load_ids.data());
                update_data_with_ids.add_attribute_buffer("sym_load", "p_specified", sym_load_p_specified.data());

                ConstDataset update_data_without_ids{false, 1, "update", meta_data::meta_data_gen::meta_data};
                update_data_without_ids.add_buffer("sym_load", update_size, update_size, nullptr, nullptr);
                update_data_without_ids.add_attribute_buffer("sym_load", "p_specified", sym_load_p_specified.data());

                MainModel const base_model{50.0, input_data};
                MainModel columnar_model_w_id{base_model};
                MainModel columnar_model_wo_id{base_model};

                columnar_model_w_id.update_component<permanent_update_t>(update_data_with_ids);
                columnar_model_wo_id.update_component<permanent_update_t>(update_data_without_ids);

                std::vector<SymNodeOutput> node_output_columnar_w_id(state.node_input.size());
                std::vector<SymNodeOutput> node_output_columnar_wo_id(state.node_input.size());

                MutableDataset sym_output_columnar_w_id{true, 1, "sym_output", meta_data::meta_data_gen::meta_data};
                sym_output_columnar_w_id.add_buffer("node", node_output_columnar_w_id.size(),
                                                    node_output_columnar_w_id.size(), nullptr,
                                                    node_output_columnar_w_id.data());
                MutableDataset sym_output_columnar_wo_id{true, 1, "sym_output", meta_data::meta_data_gen::meta_data};
                sym_output_columnar_wo_id.add_buffer("node", node_output_columnar_wo_id.size(),
                                                     node_output_columnar_wo_id.size(), nullptr,
                                                     node_output_columnar_wo_id.data());

                columnar_model_w_id.calculate(options, sym_output_columnar_w_id);
                columnar_model_wo_id.calculate(options, sym_output_columnar_wo_id);

                REQUIRE(node_output_columnar_wo_id.size() == node_output_columnar_w_id.size());
                REQUIRE(node_output_columnar_wo_id.size() == node_output_columnar_w_id.size());

                for (Idx idx = 0; idx < std::ssize(node_output_columnar_w_id); ++idx) {
                    // check columnar updates without ids work same way as ones with ids
                    CHECK(node_output_columnar_wo_id[idx].id == doctest::Approx(node_output_columnar_w_id[idx].id));
                    CHECK(node_output_columnar_wo_id[idx].u_pu == doctest::Approx(node_output_columnar_w_id[idx].u_pu));
                }
            }
        }

        SUBCASE(
            "Empty columnar update data") { // TODO(mgovers): power_flow/dummy-test-batch-shunt already captures this
            std::vector<ID> sym_load_ids;
            std::vector<double> sym_load_p_specified;
            REQUIRE(sym_load_ids.size() == sym_load_p_specified.size());

            ConstDataset update_data_with_columns{false, 1, "update", meta_data::meta_data_gen::meta_data};
            update_data_with_columns.add_buffer("sym_load", sym_load_ids.size(), sym_load_ids.size(), nullptr, nullptr);
            update_data_with_columns.add_attribute_buffer("sym_load", "id", sym_load_ids.data());
            update_data_with_columns.add_attribute_buffer("sym_load", "p_specified", sym_load_p_specified.data());

            MainModel base_model{50.0, input_data};
            MainModel columnar_model{base_model};
            columnar_model.update_component<permanent_update_t>(update_data_with_columns);

            std::vector<SymNodeOutput> node_output_from_base(state.node_input.size());
            std::vector<SymNodeOutput> node_output_from_columnar(state.node_input.size());

            MutableDataset sym_output_from_base{true, 1, "sym_output", meta_data::meta_data_gen::meta_data};
            sym_output_from_base.add_buffer("node", node_output_from_base.size(), node_output_from_base.size(), nullptr,
                                            node_output_from_base.data());
            MutableDataset sym_output_from_columnar{true, 1, "sym_output", meta_data::meta_data_gen::meta_data};
            sym_output_from_columnar.add_buffer("node", node_output_from_columnar.size(),
                                                node_output_from_columnar.size(), nullptr,
                                                node_output_from_columnar.data());

            base_model.calculate(options, sym_output_from_base);
            columnar_model.calculate(options, sym_output_from_columnar);

            REQUIRE(node_output_from_columnar.size() == node_output_from_base.size());

            for (Idx idx = 0; idx < std::ssize(node_output_from_base); ++idx) {
                CHECK(node_output_from_columnar[idx].id == doctest::Approx(node_output_from_base[idx].id));
                CHECK(node_output_from_columnar[idx].u_pu == doctest::Approx(node_output_from_base[idx].u_pu));
            }
        }
    }
}

namespace {
auto incomplete_input_model(State const& state) -> MainModel {
    MainModel main_model{50.0, meta_data::meta_data_gen::meta_data};

    std::vector<SourceInput> const incomplete_source_input{{6, 1, 1, nan, nan, 1e12, nan, nan},
                                                           {10, 3, 1, nan, nan, 1e12, nan, nan}};
    std::vector<SymLoadGenInput> const incomplete_sym_load_input{{7, 3, 1, LoadGenType::const_y, nan, 0.0}};
    std::vector<AsymLoadGenInput> const incomplete_asym_load_input{
        {8, 3, 1, LoadGenType::const_y, RealValue<asymmetric_t>{nan}, RealValue<asymmetric_t>{0.0}}};

    main_model.add_component<Node>(state.node_input);
    main_model.add_component<Line>(state.line_input);
    main_model.add_component<Link>(state.link_input);
    main_model.add_component<Source>(incomplete_source_input);
    main_model.add_component<SymLoad>(incomplete_sym_load_input);
    main_model.add_component<AsymLoad>(incomplete_asym_load_input);
    main_model.add_component<Shunt>(state.shunt_input);
    main_model.set_construction_complete();

    return main_model;
}
} // namespace

TEST_CASE("Test main model - incomplete input") {
    using CalculationMethod::iterative_current;
    using CalculationMethod::linear;
    using CalculationMethod::linear_current;
    using CalculationMethod::newton_raphson;

    State const state;
    auto main_model = default_model(state);
    auto test_model = incomplete_input_model(state);

    std::vector<SourceUpdate> complete_source_update{{6, 1, 1.05, nan}, {10, 1, 1.05, 0}};
    std::vector<SymLoadGenUpdate> complete_sym_load_update{{7, 1, 0.5e6, nan}};
    std::vector<AsymLoadGenUpdate> complete_asym_load_update{
        {8, 1, RealValue<asymmetric_t>{0.5e6 / 3.0}, RealValue<asymmetric_t>{nan}}};

    ConstDataset update_data{false, 1, "update", meta_data::meta_data_gen::meta_data};
    update_data.add_buffer("source", complete_source_update.size(), complete_source_update.size(), nullptr,
                           complete_source_update.data());
    update_data.add_buffer("sym_load", complete_sym_load_update.size(), complete_sym_load_update.size(), nullptr,
                           complete_sym_load_update.data());
    update_data.add_buffer("asym_load", complete_asym_load_update.size(), complete_asym_load_update.size(), nullptr,
                           complete_asym_load_update.data());

    std::vector<SourceUpdate> incomplete_source_update{{6, na_IntS, nan, nan}, {10, na_IntS, nan, nan}};
    std::vector<SymLoadGenUpdate> incomplete_sym_load_update{{7, na_IntS, nan, nan}};
    std::vector<AsymLoadGenUpdate> incomplete_asym_load_update{
        {8, na_IntS, RealValue<asymmetric_t>{nan}, RealValue<asymmetric_t>{nan}}};

    ConstDataset incomplete_update_data{false, 1, "update", meta_data::meta_data_gen::meta_data};
    incomplete_update_data.add_buffer("source", incomplete_source_update.size(), incomplete_source_update.size(),
                                      nullptr, incomplete_source_update.data());
    incomplete_update_data.add_buffer("sym_load", incomplete_sym_load_update.size(), incomplete_sym_load_update.size(),
                                      nullptr, incomplete_sym_load_update.data());
    incomplete_update_data.add_buffer("asym_load", incomplete_asym_load_update.size(),
                                      incomplete_asym_load_update.size(), nullptr, incomplete_asym_load_update.data());

    MainModel const ref_model{main_model};

    SUBCASE("Symmetrical - Complete") { // TODO(mgovers): validation case with different values in
                                        // power_flow/dummy-test-batch-incomplete-input
        MutableDataset test_result_data{true, 1, "sym_output", meta_data::meta_data_gen::meta_data};
        MutableDataset ref_result_data{true, 1, "sym_output", meta_data::meta_data_gen::meta_data};

        std::vector<NodeOutput<symmetric_t>> test_sym_node(state.sym_node.size());
        std::vector<NodeOutput<symmetric_t>> ref_sym_node(state.sym_node.size());
        test_result_data.add_buffer("node", test_sym_node.size(), test_sym_node.size(), nullptr, test_sym_node.data());
        ref_result_data.add_buffer("node", ref_sym_node.size(), ref_sym_node.size(), nullptr, ref_sym_node.data());

        SUBCASE("Test linear calculation") {
            test_model.calculate(get_default_options(symmetric, linear), test_result_data, update_data);
            main_model.calculate(get_default_options(symmetric, linear), ref_result_data, update_data);
        }

        SUBCASE("Test linear current calculation") {
            test_model.calculate(get_default_options(symmetric, linear_current), test_result_data, update_data);
            main_model.calculate(get_default_options(symmetric, linear_current), ref_result_data, update_data);
        }

        SUBCASE("Test iterative current calculation") {
            test_model.calculate(get_default_options(symmetric, iterative_current), test_result_data, update_data);
            main_model.calculate(get_default_options(symmetric, iterative_current), ref_result_data, update_data);
        }

        SUBCASE("Test iterative Newton-Raphson calculation") {
            test_model.calculate(get_default_options(symmetric, newton_raphson), test_result_data, update_data);
            main_model.calculate(get_default_options(symmetric, newton_raphson), ref_result_data, update_data);
        }

        CHECK(test_sym_node[0].u_pu == doctest::Approx(ref_sym_node[0].u_pu));
        CHECK(test_sym_node[1].u_pu == doctest::Approx(ref_sym_node[1].u_pu));
        CHECK(test_sym_node[2].u_pu == doctest::Approx(ref_sym_node[2].u_pu));
    }

    SUBCASE("Asymmetrical - Complete") { // TODO(mgovers): no validation case for asym exists
        MutableDataset test_result_data{true, 1, "asym_output", meta_data::meta_data_gen::meta_data};
        MutableDataset ref_result_data{true, 1, "asym_output", meta_data::meta_data_gen::meta_data};

        std::vector<NodeOutput<asymmetric_t>> test_asym_node(state.asym_node.size());
        std::vector<NodeOutput<asymmetric_t>> ref_asym_node(state.asym_node.size());
        test_result_data.add_buffer("node", test_asym_node.size(), test_asym_node.size(), nullptr,
                                    test_asym_node.data());
        ref_result_data.add_buffer("node", ref_asym_node.size(), ref_asym_node.size(), nullptr, ref_asym_node.data());

        SUBCASE("Test linear calculation") {
            test_model.calculate(get_default_options(asymmetric, linear), test_result_data, update_data);
            main_model.calculate(get_default_options(asymmetric, linear), ref_result_data, update_data);
        }

        SUBCASE("Test linear current calculation") {
            test_model.calculate(get_default_options(asymmetric, linear_current), test_result_data, update_data);
            main_model.calculate(get_default_options(asymmetric, linear_current), ref_result_data, update_data);
        }

        SUBCASE("Test iterative current calculation") {
            test_model.calculate(get_default_options(asymmetric, iterative_current), test_result_data, update_data);
            main_model.calculate(get_default_options(asymmetric, iterative_current), ref_result_data, update_data);
        }

        SUBCASE("Test iterative Newton-Rhapson calculation") {
            test_model.calculate(get_default_options(asymmetric, newton_raphson), test_result_data, update_data);
            main_model.calculate(get_default_options(asymmetric, newton_raphson), ref_result_data, update_data);
        }

        for (auto component_idx : {0, 1, 2}) {
            CAPTURE(component_idx);

            for (auto phase_idx : {0, 1, 2}) {
                CAPTURE(phase_idx);

                CHECK(test_asym_node[component_idx].u_pu(phase_idx) ==
                      doctest::Approx(ref_asym_node[component_idx].u_pu(phase_idx)));
            }
        }
    }

    SUBCASE("Symmetrical - Incomplete") { // TODO(mgovers): not tested elsewhere; maybe test in API model?
        MutableDataset test_result_data{true, 1, "sym_output", meta_data::meta_data_gen::meta_data};
        MutableDataset const ref_result_data{true, 1, "sym_output", meta_data::meta_data_gen::meta_data};

        std::vector<NodeOutput<symmetric_t>> test_sym_node(state.sym_node.size());
        test_result_data.add_buffer("node", test_sym_node.size(), test_sym_node.size(), nullptr, test_sym_node.data());

        SUBCASE("Target dataset") {
            CHECK_THROWS_AS(test_model.calculate({.calculation_type = power_flow,
                                                  .calculation_symmetry = symmetric,
                                                  .calculation_method = linear,
                                                  .err_tol = 1e-8,
                                                  .max_iter = 1},
                                                 test_result_data),
                            SparseMatrixError);
        }
        SUBCASE("Empty update dataset") {
            ConstDataset const update_data{false, 1, "update", meta_data::meta_data_gen::meta_data};

            CHECK_THROWS_AS(test_model.calculate({.calculation_type = power_flow,
                                                  .calculation_symmetry = symmetric,
                                                  .calculation_method = linear,
                                                  .err_tol = 1e-8,
                                                  .max_iter = 1},
                                                 test_result_data, update_data),
                            SparseMatrixError);
        }
        SUBCASE("Update dataset") {
            CHECK_THROWS_AS(test_model.calculate({.calculation_type = power_flow,
                                                  .calculation_symmetry = symmetric,
                                                  .calculation_method = linear,
                                                  .err_tol = 1e-8,
                                                  .max_iter = 1},
                                                 test_result_data, incomplete_update_data),
                            BatchCalculationError);
        }
    }

    SUBCASE("Asymmetrical - Incomplete") { // TODO(mgovers): not tested elsewhere; maybe test in API model?
        MutableDataset test_result_data{true, 1, "asym_output", meta_data::meta_data_gen::meta_data};
        MutableDataset const ref_result_data{true, 1, "asym_output", meta_data::meta_data_gen::meta_data};

        std::vector<NodeOutput<asymmetric_t>> test_sym_node(state.sym_node.size());
        test_result_data.add_buffer("node", test_sym_node.size(), test_sym_node.size(), nullptr, test_sym_node.data());

        SUBCASE("Target dataset") {
            CHECK_THROWS_AS(test_model.calculate({.calculation_type = power_flow,
                                                  .calculation_symmetry = asymmetric,
                                                  .calculation_method = linear,
                                                  .err_tol = 1e-8,
                                                  .max_iter = 1},
                                                 test_result_data),
                            SparseMatrixError);
        }
        SUBCASE("Empty update dataset") {
            ConstDataset const update_data{false, 1, "update", meta_data::meta_data_gen::meta_data};

            CHECK_THROWS_AS(test_model.calculate({.calculation_type = power_flow,
                                                  .calculation_symmetry = asymmetric,
                                                  .calculation_method = linear,
                                                  .err_tol = 1e-8,
                                                  .max_iter = 1},
                                                 test_result_data, update_data),
                            SparseMatrixError);
        }
        SUBCASE("Update dataset") {
            CHECK_THROWS_AS(test_model.calculate({.calculation_type = power_flow,
                                                  .calculation_symmetry = asymmetric,
                                                  .calculation_method = linear,
                                                  .err_tol = 1e-8,
                                                  .max_iter = 1},
                                                 test_result_data, incomplete_update_data),
                            BatchCalculationError);
        }
    }
}

TEST_CASE("Test main model - Incomplete followed by complete") { // TODO(mgovers): This tests the reset of 2 consecutive
                                                                 // batch scenarios and definitely needs to be tested
    using CalculationMethod::linear;

    State const state;
    auto main_model = default_model(state);
    auto test_model = incomplete_input_model(state);

    constexpr Idx batch_size = 2;

    std::vector<SourceUpdate> mixed_source_update{
        {6, 1, nan, nan}, {10, 1, nan, nan}, {6, 1, 1.05, nan}, {10, 1, 1.05, 0}};
    std::vector<SymLoadGenUpdate> mixed_sym_load_update{{7, 1, nan, 1.0}, {7, 1, 0.5e6, nan}};
    std::vector<AsymLoadGenUpdate> mixed_asym_load_update{
        {8, 1, RealValue<asymmetric_t>{nan}, RealValue<asymmetric_t>{1.0}},
        {8, 1, RealValue<asymmetric_t>{0.5e6 / 3.0}, RealValue<asymmetric_t>{nan}}};

    auto const source_indptr = IdxVector{0, 0, static_cast<Idx>(mixed_source_update.size())};

    REQUIRE(source_indptr.size() == batch_size + 1);

    ConstDataset mixed_update_data{true, batch_size, "update", meta_data::meta_data_gen::meta_data};
    mixed_update_data.add_buffer("source", 2, 4, nullptr, mixed_source_update.data());
    mixed_update_data.add_buffer("sym_load", 1, 2, nullptr, mixed_sym_load_update.data());
    mixed_update_data.add_buffer("asym_load", 1, 2, nullptr, mixed_asym_load_update.data());

    ConstDataset second_scenario_update_data{false, 1, "update", meta_data::meta_data_gen::meta_data};
    second_scenario_update_data.add_buffer("source", 2, 2, nullptr, mixed_source_update.data() + 2);
    second_scenario_update_data.add_buffer("sym_load", 1, 1, nullptr, mixed_sym_load_update.data() + 1);
    second_scenario_update_data.add_buffer("asym_load", 1, 1, nullptr, mixed_asym_load_update.data() + 1);

    SUBCASE("Symmetrical") {
        MutableDataset test_result_data{true, batch_size, "sym_output", meta_data::meta_data_gen::meta_data};
        MutableDataset ref_result_data{false, 1, "sym_output", meta_data::meta_data_gen::meta_data};

        std::vector<NodeOutput<symmetric_t>> test_sym_node(batch_size * state.sym_node.size(),
                                                           {na_IntID, na_IntS, nan, nan, nan, nan, nan});
        std::vector<NodeOutput<symmetric_t>> ref_sym_node(state.sym_node.size(),
                                                          {na_IntID, na_IntS, nan, nan, nan, nan, nan});
        test_result_data.add_buffer("node", state.sym_node.size(), test_sym_node.size(), nullptr, test_sym_node.data());
        ref_result_data.add_buffer("node", ref_sym_node.size(), ref_sym_node.size(), nullptr, ref_sym_node.data());

        CHECK_THROWS_AS(test_model.calculate({.calculation_type = power_flow,
                                              .calculation_symmetry = symmetric,
                                              .calculation_method = linear,
                                              .err_tol = 1e-8,
                                              .max_iter = 1},
                                             test_result_data, mixed_update_data),
                        BatchCalculationError);
        main_model.calculate({.calculation_type = power_flow,
                              .calculation_symmetry = symmetric,
                              .calculation_method = linear,
                              .err_tol = 1e-8,
                              .max_iter = 1},
                             ref_result_data, second_scenario_update_data);

        CHECK(is_nan(test_sym_node[0].u_pu));
        CHECK(is_nan(test_sym_node[1].u_pu));
        CHECK(is_nan(test_sym_node[2].u_pu));
        CHECK(test_sym_node[state.sym_node.size() + 0].u_pu == doctest::Approx(ref_sym_node[0].u_pu));
        CHECK(test_sym_node[state.sym_node.size() + 1].u_pu == doctest::Approx(ref_sym_node[1].u_pu));
        CHECK(test_sym_node[state.sym_node.size() + 2].u_pu == doctest::Approx(ref_sym_node[2].u_pu));
    }

    SUBCASE("Asymmetrical") {
        MutableDataset test_result_data{true, batch_size, "asym_output", meta_data::meta_data_gen::meta_data};
        MutableDataset ref_result_data{false, 1, "asym_output", meta_data::meta_data_gen::meta_data};

        std::vector<NodeOutput<asymmetric_t>> test_asym_node(
            batch_size * state.sym_node.size(),
            {na_IntID, na_IntS, RealValue<asymmetric_t>{nan}, RealValue<asymmetric_t>{nan},
             RealValue<asymmetric_t>{nan}, RealValue<asymmetric_t>{nan}, RealValue<asymmetric_t>{nan}});
        std::vector<NodeOutput<asymmetric_t>> ref_asym_node(
            state.sym_node.size(),
            {na_IntID, na_IntS, RealValue<asymmetric_t>{nan}, RealValue<asymmetric_t>{nan},
             RealValue<asymmetric_t>{nan}, RealValue<asymmetric_t>{nan}, RealValue<asymmetric_t>{nan}});
        test_result_data.add_buffer("node", state.sym_node.size(), test_asym_node.size(), nullptr,
                                    test_asym_node.data());
        ref_result_data.add_buffer("node", ref_asym_node.size(), ref_asym_node.size(), nullptr, ref_asym_node.data());

        CHECK_THROWS_AS(test_model.calculate({.calculation_type = power_flow,
                                              .calculation_symmetry = asymmetric,
                                              .calculation_method = linear,
                                              .err_tol = 1e-8,
                                              .max_iter = 1},
                                             test_result_data, mixed_update_data),
                        BatchCalculationError);
        main_model.calculate({.calculation_type = power_flow,
                              .calculation_symmetry = asymmetric,
                              .calculation_method = linear,
                              .err_tol = 1e-8,
                              .max_iter = 1},
                             ref_result_data, second_scenario_update_data);

        for (auto component_idx : {0, 1, 2}) {
            CAPTURE(component_idx);

            CHECK(is_nan(test_asym_node[component_idx].u_pu));

            for (auto phase_idx : {0, 1, 2}) {
                CAPTURE(phase_idx);

                CHECK(test_asym_node[state.asym_node.size() + component_idx].u_pu(phase_idx) ==
                      doctest::Approx(ref_asym_node[component_idx].u_pu(phase_idx)));
            }
        }
    }
}

} // namespace power_grid_model
