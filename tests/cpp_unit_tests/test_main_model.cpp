// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/main_model.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {
namespace {
struct regular_update {
    using update_type = MainModel::permanent_update_t;
};

struct cached_update {
    using update_type = MainModel::cached_update_t;
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
    std::vector<NodeInput> node_input{{1, 10e3}, {2, 10e3}, {3, 10e3}};
    std::vector<LineInput> line_input{{4, 1, 2, 1, 1, 10.0, 0.0, 0.0, 0.0, 10.0, 0.0, 0.0, 0.0, 1e3}};
    std::vector<LinkInput> link_input{{5, 2, 3, 1, 1}};
    std::vector<SourceInput> source_input{{6, 1, 1, 1.05, nan, 1e12, nan, nan}, {10, 3, 0, 1.05, 0.0, 1e12, nan, nan}};
    std::vector<SymLoadGenInput> sym_load_input{{7, 3, 1, LoadGenType::const_y, 0.5e6, 0.0}};
    std::vector<AsymLoadGenInput> asym_load_input{
        {8, 3, 1, LoadGenType::const_y, RealValue<false>{0.5e6 / 3.0}, RealValue<false>{0.0}}};
    std::vector<ShuntInput> shunt_input{{9, 3, 1, 0.015, 0.0, 0.015, 0.0}};

    // {{{id}, measured_object}, measured_terminal_type, power_sigma, p_measured, q_measured}
    std::vector<SymPowerSensorInput> sym_power_sensor_input{
        {11, 4, MeasuredTerminalType::branch_from, 0.02, 1.1e6, 1.1e3, nan, nan},
        {13, 6, MeasuredTerminalType::source, 0.02, 1.3e6, 1.3e3, nan, nan},
        {14, 6, MeasuredTerminalType::source, 0.02, 1.4e6, 1.4e3, nan, nan},
        {15, 9, MeasuredTerminalType::shunt, 0.02, 1.5e6, 1.5e3, nan, nan},
        {16, 7, MeasuredTerminalType::load, 0.02, 1.6e6, 1.6e3, nan, nan},
        {17, 8, MeasuredTerminalType::load, 0.02, 1.7e6, 1.7e3, nan, nan},
        {28, 3, MeasuredTerminalType::node, 0.02, 3.0e6, 3.0e3, nan, nan}};

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

    // {{{id}, measured_object}, u_sigma, u_measured, u_angle_measured}
    std::vector<SymVoltageSensorInput> sym_voltage_sensor_input{{25, 1, 105.0, 10.1e3, 0.1},
                                                                {26, 2, 105.0, 10.2e3, 0.2}};

    // {{{id}, measured_object}, u_sigma, u_measured, u_angle_measured}
    std::vector<AsymVoltageSensorInput> asym_voltage_sensor_input{
        {27, 3, 105.0, {10.31e3 / sqrt3, 10.32e3 / sqrt3, 10.33e3 / sqrt3}, {0.0, -deg_120, -deg_240}}};

    std::vector<FaultInput> fault_input{{30, 1, FaultType::single_phase_to_ground, FaultPhase::a, 3, 0.1, 0.1}};

    // output vector
    std::vector<NodeOutput<true>> sym_node = std::vector<NodeOutput<true>>(3);
    std::vector<BranchOutput<true>> sym_branch = std::vector<BranchOutput<true>>(2);
    std::vector<ApplianceOutput<true>> sym_appliance = std::vector<ApplianceOutput<true>>(5);
    std::vector<NodeOutput<false>> asym_node = std::vector<NodeOutput<false>>(3);
    std::vector<BranchOutput<false>> asym_branch = std::vector<BranchOutput<false>>(2);
    std::vector<ApplianceOutput<false>> asym_appliance = std::vector<ApplianceOutput<false>>(5);

    // individual symmetric
    std::vector<BranchOutput<true>> sym_line = std::vector<BranchOutput<true>>(1);
    std::vector<BranchOutput<true>> sym_link = std::vector<BranchOutput<true>>(1);
    std::vector<ApplianceOutput<true>> sym_load_sym = std::vector<ApplianceOutput<true>>(1);
    std::vector<ApplianceOutput<true>> sym_load_asym = std::vector<ApplianceOutput<true>>(1);
    std::vector<ApplianceOutput<true>> sym_source = std::vector<ApplianceOutput<true>>(2);
    std::vector<ApplianceOutput<true>> sym_shunt = std::vector<ApplianceOutput<true>>(1);
    std::vector<VoltageSensorOutput<true>> sym_voltage_sensor = std::vector<VoltageSensorOutput<true>>(2);
    std::vector<VoltageSensorOutput<true>> asym_voltage_sensor_sym_output = std::vector<VoltageSensorOutput<true>>(1);
    std::vector<PowerSensorOutput<true>> sym_power_sensor = std::vector<PowerSensorOutput<true>>(7);
    std::vector<PowerSensorOutput<true>> asym_power_sensor_sym_output = std::vector<PowerSensorOutput<true>>(7);

    // individual asymmetric
    std::vector<BranchOutput<false>> asym_line = std::vector<BranchOutput<false>>(1);
    std::vector<BranchOutput<false>> asym_link = std::vector<BranchOutput<false>>(1);
    std::vector<ApplianceOutput<false>> asym_load_sym = std::vector<ApplianceOutput<false>>(1);
    std::vector<ApplianceOutput<false>> asym_load_asym = std::vector<ApplianceOutput<false>>(1);
    std::vector<ApplianceOutput<false>> asym_source = std::vector<ApplianceOutput<false>>(2);
    std::vector<ApplianceOutput<false>> asym_shunt = std::vector<ApplianceOutput<false>>(1);
    std::vector<VoltageSensorOutput<false>> asym_voltage_sensor = std::vector<VoltageSensorOutput<false>>(1);
    std::vector<VoltageSensorOutput<false>> sym_voltage_sensor_asym_output = std::vector<VoltageSensorOutput<false>>(2);
    std::vector<PowerSensorOutput<false>> asym_power_sensor = std::vector<PowerSensorOutput<false>>(7);
    std::vector<PowerSensorOutput<false>> sym_power_sensor_asym_output = std::vector<PowerSensorOutput<false>>(7);

    // update vector
    std::vector<SymLoadGenUpdate> sym_load_update{{7, 1, 1.0e6, nan}};
    std::vector<AsymLoadGenUpdate> asym_load_update{{8, 0, RealValue<false>{nan}, RealValue<false>{nan}}};
    std::vector<ShuntUpdate> shunt_update{{9, 0, nan, 0.02, nan, 0.02}};
    std::vector<SourceUpdate> source_update{{10, 1, test::u1, nan}};
    std::vector<BranchUpdate> link_update{{5, 1, 0}};
    std::vector<FaultUpdate> fault_update{{30, 1, FaultType::three_phase, FaultPhase::abc, 1, nan, nan}};
};

auto default_model(State const& state) -> MainModel {
    MainModel main_model{50.0};
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

    SUBCASE("Test get indexer") {
        std::vector<ID> const node_id{2, 1, 3, 2};
        IdxVector const expected_indexer{1, 0, 2, 1};
        IdxVector indexer(4);
        main_model.get_indexer("node", node_id.data(), 4, indexer.data());
        CHECK(indexer == expected_indexer);
    }

    SUBCASE("Test duplicated id") {
        MainModel main_model2{50.0};
        state.node_input[1].id = 1;
        CHECK_THROWS_AS(main_model2.add_component<Node>(state.node_input), ConflictID);
    }

    SUBCASE("Test no existing id") {
        MainModel main_model2{50.0};
        state.line_input[0].from_node = 100;
        main_model2.add_component<Node>(state.node_input);
        CHECK_THROWS_AS(main_model2.add_component<Line>(state.line_input), IDNotFound);
    }

    SUBCASE("Test id for wrong type") {
        MainModel main_model2{50.0};

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

    SUBCASE("Test calculate power flow") {
        auto const math_output = main_model.calculate_power_flow<true>(1e-8, 20, CalculationMethod::linear);
        main_model.output_result<Node>(math_output, state.sym_node.begin());
        main_model.output_result<Branch>(math_output, state.sym_branch.begin());
        main_model.output_result<Appliance>(math_output, state.sym_appliance.begin());
    }
}

TEST_CASE("Test copy main model") {
    State state;
    auto main_model = default_model(state);
    MainModel model_2{main_model};

    SUBCASE("Copied - Symmetrical") {
        auto const math_output = model_2.calculate_power_flow<true>(1e-8, 20, CalculationMethod::linear);
        model_2.output_result<Node>(math_output, state.sym_node.begin());
        model_2.output_result<Branch>(math_output, state.sym_branch.begin());
        model_2.output_result<Appliance>(math_output, state.sym_appliance.begin());
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
    SUBCASE("Copied - Asymmetrical") {
        auto const math_output = model_2.calculate_power_flow<false>(1e-8, 20, CalculationMethod::linear);
        model_2.output_result<Node>(math_output, state.asym_node.begin());
        model_2.output_result<Branch>(math_output, state.asym_branch.begin());
        model_2.output_result<Appliance>(math_output, state.asym_appliance.begin());
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
    model_2 = main_model;
    SUBCASE("Assigned - Symmetrical") {
        auto const math_output = model_2.calculate_power_flow<true>(1e-8, 20, CalculationMethod::linear);
        model_2.output_result<Node>(math_output, state.sym_node.begin());
        model_2.output_result<Branch>(math_output, state.sym_branch.begin());
        model_2.output_result<Appliance>(math_output, state.sym_appliance.begin());
        // TODO: check voltage angle
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
    SUBCASE("Assigned - Asymmetrical") {
        auto const math_output = model_2.calculate_power_flow<false>(1e-8, 20, CalculationMethod::linear);
        model_2.output_result<Node>(math_output, state.asym_node.begin());
        model_2.output_result<Branch>(math_output, state.asym_branch.begin());
        model_2.output_result<Appliance>(math_output, state.asym_appliance.begin());
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
    SUBCASE("Original - Symmetrical") {
        auto const math_output = main_model.calculate_power_flow<true>(1e-8, 20, CalculationMethod::linear);
        main_model.output_result<Node>(math_output, state.sym_node.begin());
        main_model.output_result<Branch>(math_output, state.sym_branch.begin());
        main_model.output_result<Appliance>(math_output, state.sym_appliance.begin());
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
    SUBCASE("Original - Asymmetrical") {
        auto const math_output = main_model.calculate_power_flow<false>(1e-8, 20, CalculationMethod::linear);
        main_model.output_result<Node>(math_output, state.asym_node.begin());
        main_model.output_result<Branch>(math_output, state.asym_branch.begin());
        main_model.output_result<Appliance>(math_output, state.asym_appliance.begin());
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

TEST_CASE("Test main model - iterative calculation") {
    State state;
    auto main_model = default_model(state);

    SUBCASE("Symmetrical") {
        auto const math_output = main_model.calculate_power_flow<true>(1e-8, 20, CalculationMethod::newton_raphson);
        main_model.output_result<Node>(math_output, state.sym_node.begin());
        main_model.output_result<Branch>(math_output, state.sym_branch.begin());
        main_model.output_result<Appliance>(math_output, state.sym_appliance.begin());
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
    SUBCASE("Asymmetrical") {
        auto const math_output = main_model.calculate_power_flow<false>(1e-8, 20, CalculationMethod::newton_raphson);
        main_model.output_result<Node>(math_output, state.asym_node.begin());
        main_model.output_result<Branch>(math_output, state.asym_branch.begin());
        main_model.output_result<Appliance>(math_output, state.asym_appliance.begin());
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

TEST_CASE("Test main model - individual output (symmetric)") {
    State state;
    auto main_model = default_model(state);

    auto const res = main_model.calculate_power_flow<true>(1e-8, 20, CalculationMethod::newton_raphson);

    SUBCASE("Node, sym output") {
        main_model.output_result<Node>(res, state.sym_node.begin());
        main_model.output_result<Appliance>(res, state.sym_appliance.begin());

        CHECK(state.sym_node[0].u_pu == doctest::Approx(1.05));
        CHECK(state.sym_node[1].u_pu == doctest::Approx(test::u1));
        CHECK(state.sym_node[2].u_pu == doctest::Approx(test::u1));
        CHECK(state.sym_node[0].p == doctest::Approx(state.sym_appliance[0].p).scale(1e3));
        CHECK(state.sym_node[1].p == doctest::Approx(0.0).scale(1e3));
        CHECK(
            state.sym_node[2].p ==
            doctest::Approx(state.sym_appliance[1].p - state.sym_appliance[2].p - state.sym_appliance[3].p).scale(1e3));
        CHECK(state.sym_node[0].q == doctest::Approx(state.sym_appliance[0].q).scale(1e3));
        CHECK(state.sym_node[1].q == doctest::Approx(0.0).scale(1e3));
        CHECK(
            state.sym_node[2].q ==
            doctest::Approx(state.sym_appliance[1].q - state.sym_appliance[2].q - state.sym_appliance[3].q).scale(2e3));

        /*
        TODO
        - u
        - angle
        */
    }

    SUBCASE("Line, sym output") {
        main_model.output_result<Line>(res, state.sym_line.begin());

        CHECK(state.sym_line[0].i_from == doctest::Approx(test::i));
        /*
        TODO
        - i_to
        - p_from
        - p_to
        - q_from
        - q_to
        */
    }

    SUBCASE("Link, sym output") {
        main_model.output_result<Link>(res, state.sym_link.begin());

        CHECK(state.sym_link[0].i_from == doctest::Approx(test::i));
        /*
        TODO
        - i_to
        - p_from
        - p_to
        - q_from
        - q_to
        - s_from
        - s_to
        */
    }

    SUBCASE("Source, sym output") {
        main_model.output_result<Source>(res, state.sym_source.begin());
        main_model.output_result<Node>(res, state.sym_node.begin());

        CHECK(state.sym_source[0].i == doctest::Approx(test::i));
        CHECK(state.sym_source[1].i == doctest::Approx(0.0));
        /*
        TODO
        - p
        - q
        - s
        */
    }

    SUBCASE("SymLoad, sym output") {
        main_model.output_result<SymLoad>(res, state.sym_load_sym.begin());

        CHECK(state.sym_load_sym[0].i == doctest::Approx(test::i_load));
        /*
        TODO
        - p
        - q
        - s
        */
    }

    SUBCASE("AsymLoad, sym output") {
        main_model.output_result<AsymLoad>(res, state.sym_load_asym.begin());

        CHECK(state.sym_load_asym[0].i == doctest::Approx(test::i_load));
        /*
        TODO
        - p
        - q
        - s
        */
    }

    SUBCASE("Shunt, sym output") {
        main_model.output_result<Node>(res, state.sym_node.begin());
        main_model.output_result<Shunt>(res, state.sym_shunt.begin());
        auto const& output = state.sym_shunt[0];
        CHECK(output.i == doctest::Approx(test::i_shunt));
        CHECK(output.p == doctest::Approx(sqrt3 * test::i_shunt * state.sym_node[2].u));
        CHECK(output.q == doctest::Approx(0.0));
        CHECK(output.s == doctest::Approx(output.p));
        CHECK(output.pf == doctest::Approx(1.0));
    }

    SUBCASE("SymVoltageSensor, sym output") {
        main_model.output_result<Node>(res, state.sym_node.begin());
        main_model.output_result<SymVoltageSensor>(res, state.sym_voltage_sensor.begin());

        CHECK(state.sym_voltage_sensor[0].u_residual == doctest::Approx(1.01 * 10.0e3 - state.sym_node[0].u));
        CHECK(state.sym_voltage_sensor[1].u_residual == doctest::Approx(1.02 * 10.0e3 - state.sym_node[1].u));
        CHECK(state.sym_voltage_sensor[0].u_angle_residual == doctest::Approx(0.1 - state.sym_node[0].u_angle));
        CHECK(state.sym_voltage_sensor[1].u_angle_residual == doctest::Approx(0.2 - state.sym_node[1].u_angle));
    }

    SUBCASE("SymPowerSensor, sym output") {
        main_model.output_result<Line>(res, state.sym_line.begin());
        main_model.output_result<Link>(res, state.sym_link.begin());
        main_model.output_result<Source>(res, state.sym_source.begin());
        main_model.output_result<SymLoad>(res, state.sym_load_sym.begin());
        main_model.output_result<AsymLoad>(res, state.sym_load_asym.begin());
        main_model.output_result<Shunt>(res, state.sym_shunt.begin());
        main_model.output_result<SymPowerSensor>(res, state.sym_power_sensor.begin());

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

    SUBCASE("AsymVoltageSensor, sym output") {
        main_model.output_result<Node>(res, state.sym_node.begin());
        main_model.output_result<AsymVoltageSensor>(res, state.asym_voltage_sensor_sym_output.begin());

        CHECK(state.asym_voltage_sensor_sym_output[0].u_residual == doctest::Approx(10.32e3 - state.sym_node[2].u));
        CHECK(state.asym_voltage_sensor_sym_output[0].u_angle_residual ==
              doctest::Approx(0.0 - state.sym_node[2].u_angle));
    }

    SUBCASE("AsymPowerSensor, sym output") {
        main_model.output_result<Line>(res, state.sym_line.begin());
        main_model.output_result<Link>(res, state.sym_link.begin());
        main_model.output_result<Source>(res, state.sym_source.begin());
        main_model.output_result<SymLoad>(res, state.sym_load_sym.begin());
        main_model.output_result<AsymLoad>(res, state.sym_load_asym.begin());
        main_model.output_result<Shunt>(res, state.sym_shunt.begin());
        main_model.output_result<AsymPowerSensor>(res, state.asym_power_sensor_sym_output.begin());

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

    auto const res = main_model.calculate_power_flow<false>(1e-8, 20, CalculationMethod::newton_raphson);

    /*
    TODO:
    - test node
    - test line
    - test link
    - test source
    - test sym load
    - test asym load
    - test shunt
    */

    SUBCASE("Node, asym output") {
        main_model.output_result<Node>(res, state.asym_node.begin());
        main_model.output_result<Appliance>(res, state.asym_appliance.begin());

        CHECK(state.asym_node[0].u_pu(0) == doctest::Approx(1.05));
        CHECK(state.asym_node[1].u_pu(1) == doctest::Approx(test::u1));
        CHECK(state.asym_node[2].u_pu(2) == doctest::Approx(test::u1));

        CHECK(state.asym_node[0].p(0) == doctest::Approx(state.asym_appliance[0].p(0)).scale(1e3));
        CHECK(state.asym_node[1].p(1) == doctest::Approx(0.0).scale(1e3));
        CHECK(state.asym_node[2].p(2) == doctest::Approx(state.asym_appliance[1].p(2) - state.asym_appliance[2].p(2) -
                                                         state.asym_appliance[3].p(2))
                                             .scale(1e3));
        CHECK(state.asym_node[0].q(2) == doctest::Approx(state.asym_appliance[0].q(2)).scale(1e3));
        CHECK(state.asym_node[1].q(1) == doctest::Approx(0.0).scale(1e3));
        CHECK(state.asym_node[2].q(0) == doctest::Approx(state.asym_appliance[1].q(0) - state.asym_appliance[2].q(0) -
                                                         state.asym_appliance[3].q(0))
                                             .scale(1e3));
    }

    SUBCASE("AsymVoltageSensor, asym output") {
        main_model.output_result<Node>(res, state.asym_node.begin());
        main_model.output_result<AsymVoltageSensor>(res, state.asym_voltage_sensor.begin());

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

    SUBCASE("SymVoltageSensor, asym output") {
        main_model.output_result<Node>(res, state.asym_node.begin());
        main_model.output_result<SymVoltageSensor>(res, state.sym_voltage_sensor_asym_output.begin());

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
    SUBCASE("AsymPowerSensor, asym output") {
        main_model.output_result<Line>(res, state.asym_line.begin());
        main_model.output_result<Link>(res, state.asym_link.begin());
        main_model.output_result<Source>(res, state.asym_source.begin());
        main_model.output_result<SymLoad>(res, state.asym_load_sym.begin());
        main_model.output_result<AsymLoad>(res, state.asym_load_asym.begin());
        main_model.output_result<Shunt>(res, state.asym_shunt.begin());
        main_model.output_result<AsymPowerSensor>(res, state.asym_power_sensor.begin());

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

    SUBCASE("SymPowerSensor, asym output") {
        main_model.output_result<Line>(res, state.asym_line.begin());
        main_model.output_result<Link>(res, state.asym_link.begin());
        main_model.output_result<Source>(res, state.asym_source.begin());
        main_model.output_result<SymLoad>(res, state.asym_load_sym.begin());
        main_model.output_result<AsymLoad>(res, state.asym_load_asym.begin());
        main_model.output_result<Shunt>(res, state.asym_shunt.begin());
        main_model.output_result<SymPowerSensor>(res, state.sym_power_sensor_asym_output.begin());

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

    SUBCASE("Symmetrical") {
        auto const math_output = main_model.calculate_power_flow<true>(1e-8, 20, CalculationMethod::linear);
        main_model.output_result<Node>(math_output, state.sym_node.begin());
        main_model.output_result<Branch>(math_output, state.sym_branch.begin());
        main_model.output_result<Appliance>(math_output, state.sym_appliance.begin());
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
    SUBCASE("Asymmetrical") {
        auto const math_output = main_model.calculate_power_flow<false>(1e-8, 20, CalculationMethod::linear);
        main_model.output_result<Node>(math_output, state.asym_node.begin());
        main_model.output_result<Branch>(math_output, state.asym_branch.begin());
        main_model.output_result<Appliance>(math_output, state.asym_appliance.begin());
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

TEST_CASE_TEMPLATE("Test main model - unknown id", settings, regular_update, cached_update) {
    State const state;
    auto main_model = default_model(state);

    std::vector<SourceUpdate> const source_update2{SourceUpdate{100, true, nan, nan}};
    ConstDataset const update_data{
        {"source", ConstDataPointer{source_update2.data(), static_cast<Idx>(source_update2.size())}}};
    CHECK_THROWS_AS((main_model.update_component<typename settings::update_type>(update_data)), IDNotFound);
}

TEST_CASE_TEMPLATE("Test main model - update only load", settings, regular_update, cached_update) {
    State state;
    auto main_model = default_model(state);

    ConstDataset const update_data{
        {"sym_load", ConstDataPointer{state.sym_load_update.data(), static_cast<Idx>(state.sym_load_update.size())}},
        {"asym_load",
         ConstDataPointer{state.asym_load_update.data(), static_cast<Idx>(state.asym_load_update.size())}}};
    main_model.update_component<typename settings::update_type>(update_data);

    SUBCASE("Symmetrical") {
        auto const math_output = main_model.calculate_power_flow<true>(1e-8, 20, CalculationMethod::linear);
        main_model.output_result<Node>(math_output, state.sym_node.begin());
        main_model.output_result<Branch>(math_output, state.sym_branch.begin());
        main_model.output_result<Appliance>(math_output, state.sym_appliance.begin());
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
        auto const math_output = main_model.calculate_power_flow<false>(1e-8, 20, CalculationMethod::linear);
        main_model.output_result<Node>(math_output, state.asym_node.begin());
        main_model.output_result<Branch>(math_output, state.asym_branch.begin());
        main_model.output_result<Appliance>(math_output, state.asym_appliance.begin());
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

TEST_CASE_TEMPLATE("Test main model - update load and shunt param", settings, regular_update, cached_update) {
    State state;
    auto main_model = default_model(state);

    state.sym_load_update[0].p_specified = 2.5e6;

    ConstDataset const update_data{
        {"sym_load", ConstDataPointer{state.sym_load_update.data(), static_cast<Idx>(state.sym_load_update.size())}},
        {"asym_load", ConstDataPointer{state.asym_load_update.data(), static_cast<Idx>(state.asym_load_update.size())}},
        {"shunt", ConstDataPointer{state.shunt_update.data(), static_cast<Idx>(state.shunt_update.size())}}};
    main_model.update_component<typename settings::update_type>(update_data);

    SUBCASE("Symmetrical") {
        auto const math_output = main_model.calculate_power_flow<true>(1e-8, 20, CalculationMethod::linear);
        main_model.output_result<Node>(math_output, state.sym_node.begin());
        main_model.output_result<Branch>(math_output, state.sym_branch.begin());
        main_model.output_result<Appliance>(math_output, state.sym_appliance.begin());
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
        auto const math_output = main_model.calculate_power_flow<false>(1e-8, 20, CalculationMethod::linear);
        main_model.output_result<Node>(math_output, state.asym_node.begin());
        main_model.output_result<Branch>(math_output, state.asym_branch.begin());
        main_model.output_result<Appliance>(math_output, state.asym_appliance.begin());
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

TEST_CASE_TEMPLATE("Test main model - all updates", settings, regular_update, cached_update) {
    State state;
    auto main_model = default_model(state);

    state.sym_load_update[0].p_specified = 2.5e6;

    ConstDataset const update_data{
        {"sym_load", ConstDataPointer{state.sym_load_update.data(), static_cast<Idx>(state.sym_load_update.size())}},
        {"asym_load", ConstDataPointer{state.asym_load_update.data(), static_cast<Idx>(state.asym_load_update.size())}},
        {"shunt", ConstDataPointer{state.shunt_update.data(), static_cast<Idx>(state.shunt_update.size())}},
        {"source", ConstDataPointer{state.source_update.data(), static_cast<Idx>(state.source_update.size())}},
        {"link", ConstDataPointer{state.link_update.data(), static_cast<Idx>(state.link_update.size())}},
        {"fault", ConstDataPointer{state.fault_update.data(), static_cast<Idx>(state.fault_update.size())}}};

    main_model.update_component<typename settings::update_type>(update_data);

    SUBCASE("Symmetrical") {
        auto const math_output = main_model.calculate_power_flow<true>(1e-8, 20, CalculationMethod::linear);
        main_model.output_result<Node>(math_output, state.sym_node.begin());
        main_model.output_result<Branch>(math_output, state.sym_branch.begin());
        main_model.output_result<Appliance>(math_output, state.sym_appliance.begin());
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
        auto const math_output = main_model.calculate_power_flow<false>(1e-8, 20, CalculationMethod::linear);
        main_model.output_result<Node>(math_output, state.asym_node.begin());
        main_model.output_result<Branch>(math_output, state.asym_branch.begin());
        main_model.output_result<Appliance>(math_output, state.asym_appliance.begin());
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

TEST_CASE_TEMPLATE("Test main model - restore components", settings, regular_update, cached_update) {
    State state;
    auto main_model = default_model(state);

    auto const math_output_orig = main_model.calculate_power_flow<true>(1e-8, 20, CalculationMethod::linear);

    ConstDataset const update_data{
        {"sym_load", ConstDataPointer{state.sym_load_update.data(), static_cast<Idx>(state.sym_load_update.size())}},
        {"asym_load",
         ConstDataPointer{state.asym_load_update.data(), static_cast<Idx>(state.asym_load_update.size())}}};

    main_model.update_component<typename settings::update_type>(update_data);
    main_model.restore_components(main_model.get_sequence_idx_map(update_data));

    SUBCASE("Symmetrical") {
        auto const math_output_result = main_model.calculate_power_flow<true>(1e-8, 20, CalculationMethod::linear);
        main_model.output_result<Node>(math_output_result, state.sym_node.begin());
        main_model.output_result<Branch>(math_output_result, state.sym_branch.begin());
        main_model.output_result<Appliance>(math_output_result, state.sym_appliance.begin());

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
        auto const math_output = main_model.calculate_power_flow<false>(1e-8, 20, CalculationMethod::linear);
        main_model.output_result<Node>(math_output, state.asym_node.begin());
        main_model.output_result<Branch>(math_output, state.asym_branch.begin());
        main_model.output_result<Appliance>(math_output, state.asym_appliance.begin());

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

TEST_CASE("Test main model - runtime dispatch") {
    State state;
    auto main_model = default_model(state);

    ConstDataset input_data;
    input_data["node"] = DataPointer<true>{state.node_input.data(), static_cast<Idx>(state.node_input.size())};
    input_data["line"] = DataPointer<true>{state.line_input.data(), static_cast<Idx>(state.line_input.size())};
    input_data["link"] = DataPointer<true>{state.link_input.data(), static_cast<Idx>(state.link_input.size())};
    input_data["source"] = DataPointer<true>{state.source_input.data(), static_cast<Idx>(state.source_input.size())};
    input_data["sym_load"] =
        DataPointer<true>{state.sym_load_input.data(), static_cast<Idx>(state.sym_load_input.size())};
    input_data["asym_load"] =
        DataPointer<true>{state.asym_load_input.data(), static_cast<Idx>(state.asym_load_input.size())};
    input_data["shunt"] = DataPointer<true>{state.shunt_input.data(), static_cast<Idx>(state.shunt_input.size())};

    SUBCASE("Single-size batches") {
        ConstDataset update_data;
        update_data["sym_load"] =
            DataPointer<true>{state.sym_load_update.data(), static_cast<Idx>(state.sym_load_update.size())};
        update_data["asym_load"] =
            DataPointer<true>{state.asym_load_update.data(), static_cast<Idx>(state.asym_load_update.size())};
        update_data["shunt"] =
            DataPointer<true>{state.shunt_update.data(), static_cast<Idx>(state.shunt_update.size())};
        update_data["source"] =
            DataPointer<true>{state.source_update.data(), static_cast<Idx>(state.source_update.size())};
        update_data["link"] = DataPointer<true>{state.link_update.data(), static_cast<Idx>(state.link_update.size())};

        Dataset sym_result_data;
        sym_result_data["node"] = DataPointer<false>{state.sym_node.data(), static_cast<Idx>(state.sym_node.size())};
        sym_result_data["line"] = DataPointer<false>{state.sym_line.data(), static_cast<Idx>(state.sym_line.size())};
        sym_result_data["link"] = DataPointer<false>{state.sym_link.data(), static_cast<Idx>(state.sym_link.size())};
        sym_result_data["source"] =
            DataPointer<false>{state.sym_source.data(), static_cast<Idx>(state.sym_source.size())};
        sym_result_data["sym_load"] =
            DataPointer<false>{state.sym_load_sym.data(), static_cast<Idx>(state.sym_load_sym.size())};
        sym_result_data["asym_load"] =
            DataPointer<false>{state.sym_load_asym.data(), static_cast<Idx>(state.sym_load_asym.size())};
        sym_result_data["shunt"] = DataPointer<false>{state.sym_shunt.data(), static_cast<Idx>(state.sym_shunt.size())};

        Dataset asym_result_data;
        asym_result_data["node"] = DataPointer<false>{state.asym_node.data(), static_cast<Idx>(state.asym_node.size())};

        MainModel model{50.0, input_data};
        auto const count = model.all_component_count();
        CHECK(count.at("node") == 3);
        CHECK(count.at("source") == 2);
        CHECK(count.find("sym_gen") == count.cend());

        // calculation
        model.calculate_power_flow<true>(1e-8, 20, CalculationMethod::newton_raphson, sym_result_data);
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
        model.calculate_power_flow<false>(1e-8, 20, CalculationMethod::newton_raphson, asym_result_data);
        CHECK(state.asym_node[0].u_pu(0) == doctest::Approx(1.05));
        CHECK(state.asym_node[1].u_pu(1) == doctest::Approx(test::u1));
        CHECK(state.asym_node[2].u_pu(2) == doctest::Approx(test::u1));

        // update and calculation
        model.update_component<MainModel::permanent_update_t>(update_data);
        model.calculate_power_flow<true>(1e-8, 20, CalculationMethod::newton_raphson, sym_result_data);
        CHECK(state.sym_node[0].u_pu == doctest::Approx(1.05));
        CHECK(state.sym_node[1].u_pu == doctest::Approx(1.05));
        CHECK(state.sym_node[2].u_pu == doctest::Approx(test::u1));
        model.calculate_power_flow<false>(1e-8, 20, CalculationMethod::newton_raphson, asym_result_data);
        CHECK(state.asym_node[0].u_pu(0) == doctest::Approx(1.05));
        CHECK(state.asym_node[1].u_pu(1) == doctest::Approx(1.05));
        CHECK(state.asym_node[2].u_pu(2) == doctest::Approx(test::u1));

        // test batch calculation
        model = MainModel{50.0, input_data};
        // symmetric sequential
        model.calculate_power_flow<true>(1e-8, 20, CalculationMethod::newton_raphson, sym_result_data, update_data, -1);
        CHECK(state.sym_node[0].u_pu == doctest::Approx(1.05));
        CHECK(state.sym_node[1].u_pu == doctest::Approx(1.05));
        CHECK(state.sym_node[2].u_pu == doctest::Approx(test::u1));
        // symmetric parallel
        model.calculate_power_flow<true>(1e-8, 20, CalculationMethod::newton_raphson, sym_result_data, update_data, 0);
        CHECK(state.sym_node[0].u_pu == doctest::Approx(1.05));
        CHECK(state.sym_node[1].u_pu == doctest::Approx(1.05));
        CHECK(state.sym_node[2].u_pu == doctest::Approx(test::u1));
        // asymmetric sequential
        model.calculate_power_flow<false>(1e-8, 20, CalculationMethod::newton_raphson, asym_result_data, update_data,
                                          -1);
        CHECK(state.asym_node[0].u_pu(0) == doctest::Approx(1.05));
        CHECK(state.asym_node[1].u_pu(1) == doctest::Approx(1.05));
        CHECK(state.asym_node[2].u_pu(2) == doctest::Approx(test::u1));
        // asymmetric parallel
        model.calculate_power_flow<false>(1e-8, 20, CalculationMethod::newton_raphson, asym_result_data, update_data,
                                          0);
        CHECK(state.asym_node[0].u_pu(0) == doctest::Approx(1.05));
        CHECK(state.asym_node[1].u_pu(1) == doctest::Approx(1.05));
        CHECK(state.asym_node[2].u_pu(2) == doctest::Approx(test::u1));
    }

    SUBCASE("no dependent updates within batches") {
        MainModel model{50.0, input_data};
        ConstDataset dependent_update_data;
        Dataset dependent_result_data;

        std::vector<SymLoadGenUpdate> sym_load_update_2{{7, 1, nan, 1.0e7}, {7, 1, 1.0e3, nan}, {7, 1, 1.0e3, 1.0e7}};
        dependent_update_data["sym_load"] =
            DataPointer<true>{sym_load_update_2.data(), static_cast<Idx>(sym_load_update_2.size()), 1};

        std::vector<NodeOutput<true>> sym_node_2(sym_load_update_2.size() * state.sym_node.size());
        dependent_result_data["node"] = DataPointer<false>{
            sym_node_2.data(), static_cast<Idx>(sym_load_update_2.size()), static_cast<Idx>(state.sym_node.size())};

        model.calculate_power_flow<true>(1e-8, 20, CalculationMethod::newton_raphson, dependent_result_data,
                                         dependent_update_data, -1);
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
}

TEST_CASE("Test main model - incomplete input") {
    using CalculationMethod::iterative_current;
    using CalculationMethod::linear;
    using CalculationMethod::linear_current;
    using CalculationMethod::newton_raphson;

    State state;
    auto main_model = default_model(state);

    std::vector<SourceInput> const incomplete_source_input{{6, 1, 1, nan, nan, 1e12, nan, nan},
                                                           {10, 3, 1, nan, nan, 1e12, nan, nan}};
    std::vector<SymLoadGenInput> incomplete_sym_load_input{{7, 3, 1, LoadGenType::const_y, nan, nan}};
    std::vector<AsymLoadGenInput> incomplete_asym_load_input{
        {8, 3, 1, LoadGenType::const_y, RealValue<false>{nan}, RealValue<false>{nan}}};

    ConstDataset input_data;
    input_data["node"] = DataPointer<true>{state.node_input.data(), static_cast<Idx>(state.node_input.size())};
    input_data["line"] = DataPointer<true>{state.line_input.data(), static_cast<Idx>(state.line_input.size())};
    input_data["link"] = DataPointer<true>{state.link_input.data(), static_cast<Idx>(state.link_input.size())};
    input_data["source"] =
        DataPointer<true>{incomplete_source_input.data(), static_cast<Idx>(incomplete_source_input.size())};
    input_data["sym_load"] =
        DataPointer<true>{incomplete_sym_load_input.data(), static_cast<Idx>(incomplete_sym_load_input.size())};
    input_data["asym_load"] =
        DataPointer<true>{incomplete_asym_load_input.data(), static_cast<Idx>(incomplete_asym_load_input.size())};
    input_data["shunt"] = DataPointer<true>{state.shunt_input.data(), static_cast<Idx>(state.shunt_input.size())};

    std::vector<SourceUpdate> complete_source_update{{6, 1, 1.05, nan}, {10, 1, 1.05, 0}};
    std::vector<SymLoadGenUpdate> complete_sym_load_update{{7, 1, 0.5e6, 0.0}};
    std::vector<AsymLoadGenUpdate> complete_asym_load_update{
        {8, 1, RealValue<false>{0.5e6 / 3.0}, RealValue<false>{0.0}}};

    ConstDataset update_data;
    update_data["source"] =
        DataPointer<true>{complete_source_update.data(), static_cast<Idx>(complete_source_update.size())};
    update_data["sym_load"] =
        DataPointer<true>{complete_sym_load_update.data(), static_cast<Idx>(complete_sym_load_update.size())};
    update_data["asym_load"] =
        DataPointer<true>{complete_asym_load_update.data(), static_cast<Idx>(complete_asym_load_update.size())};

    std::vector<SourceUpdate> incomplete_source_update{{6, na_IntS, nan, nan}, {10, na_IntS, nan, nan}};
    std::vector<SymLoadGenUpdate> incomplete_sym_load_update{{7, na_IntS, nan, nan}};
    std::vector<AsymLoadGenUpdate> incomplete_asym_load_update{
        {8, na_IntS, RealValue<false>{nan}, RealValue<false>{nan}}};

    ConstDataset incomplete_update_data;
    incomplete_update_data["source"] =
        DataPointer<true>{incomplete_source_update.data(), static_cast<Idx>(incomplete_source_update.size())};
    incomplete_update_data["sym_load"] =
        DataPointer<true>{incomplete_sym_load_update.data(), static_cast<Idx>(incomplete_sym_load_update.size())};
    incomplete_update_data["asym_load"] =
        DataPointer<true>{incomplete_asym_load_update.data(), static_cast<Idx>(incomplete_asym_load_update.size())};

    MainModel test_model{50.0, input_data};
    MainModel const ref_model{main_model};

    Dataset test_result_data;
    Dataset ref_result_data;

    SUBCASE("Symmetrical - Complete") {
        std::vector<NodeOutput<true>> test_sym_node(state.sym_node.size());
        std::vector<NodeOutput<true>> ref_sym_node(state.sym_node.size());
        test_result_data["node"] = DataPointer<false>{test_sym_node.data(), static_cast<Idx>(test_sym_node.size())};
        ref_result_data["node"] = DataPointer<false>{ref_sym_node.data(), static_cast<Idx>(ref_sym_node.size())};

        SUBCASE("Test linear calculation") {
            test_model.calculate_power_flow<true>(1e-8, 20, linear, test_result_data, update_data, -1);
            main_model.calculate_power_flow<true>(1e-8, 20, linear, ref_result_data, update_data, -1);
        }

        SUBCASE("Test linear current calculation") {
            test_model.calculate_power_flow<true>(1e-8, 20, linear_current, test_result_data, update_data, -1);
            main_model.calculate_power_flow<true>(1e-8, 20, linear_current, ref_result_data, update_data, -1);
        }

        SUBCASE("Test iterative current calculation") {
            test_model.calculate_power_flow<true>(1e-8, 20, iterative_current, test_result_data, update_data, -1);
            main_model.calculate_power_flow<true>(1e-8, 20, iterative_current, ref_result_data, update_data, -1);
        }

        SUBCASE("Test iterative Newton-Raphson calculation") {
            test_model.calculate_power_flow<true>(1e-8, 20, newton_raphson, test_result_data, update_data, -1);
            main_model.calculate_power_flow<true>(1e-8, 20, newton_raphson, ref_result_data, update_data, -1);
        }

        CHECK(test_sym_node[0].u_pu == doctest::Approx(ref_sym_node[0].u_pu));
        CHECK(test_sym_node[1].u_pu == doctest::Approx(ref_sym_node[1].u_pu));
        CHECK(test_sym_node[2].u_pu == doctest::Approx(ref_sym_node[2].u_pu));
    }

    SUBCASE("Asymmetrical - Complete") {
        std::vector<NodeOutput<false>> test_asym_node(state.asym_node.size());
        std::vector<NodeOutput<false>> ref_asym_node(state.asym_node.size());
        test_result_data["node"] = DataPointer<false>{test_asym_node.data(), static_cast<Idx>(test_asym_node.size())};
        ref_result_data["node"] = DataPointer<false>{ref_asym_node.data(), static_cast<Idx>(ref_asym_node.size())};

        SUBCASE("Test linear calculation") {
            test_model.calculate_power_flow<false>(1e-8, 20, linear, test_result_data, update_data, -1);
            main_model.calculate_power_flow<false>(1e-8, 20, linear, ref_result_data, update_data, -1);
        }

        SUBCASE("Test linear current calculation") {
            test_model.calculate_power_flow<false>(1e-8, 20, linear_current, test_result_data, update_data, -1);
            main_model.calculate_power_flow<false>(1e-8, 20, linear_current, ref_result_data, update_data, -1);
        }

        SUBCASE("Test iterative current calculation") {
            test_model.calculate_power_flow<false>(1e-8, 20, iterative_current, test_result_data, update_data, -1);
            main_model.calculate_power_flow<false>(1e-8, 20, iterative_current, ref_result_data, update_data, -1);
        }

        SUBCASE("Test iterative Newton-Rhapson calculation") {
            test_model.calculate_power_flow<false>(1e-8, 20, newton_raphson, test_result_data, update_data, -1);
            main_model.calculate_power_flow<false>(1e-8, 20, newton_raphson, ref_result_data, update_data, -1);
        }

        CHECK(test_asym_node[0].u_pu(0) == doctest::Approx(ref_asym_node[0].u_pu(0)));
        CHECK(test_asym_node[0].u_pu(1) == doctest::Approx(ref_asym_node[0].u_pu(1)));
        CHECK(test_asym_node[0].u_pu(2) == doctest::Approx(ref_asym_node[0].u_pu(2)));
        CHECK(test_asym_node[1].u_pu(0) == doctest::Approx(ref_asym_node[1].u_pu(0)));
        CHECK(test_asym_node[1].u_pu(1) == doctest::Approx(ref_asym_node[1].u_pu(1)));
        CHECK(test_asym_node[1].u_pu(2) == doctest::Approx(ref_asym_node[1].u_pu(2)));
        CHECK(test_asym_node[2].u_pu(0) == doctest::Approx(ref_asym_node[2].u_pu(0)));
        CHECK(test_asym_node[2].u_pu(1) == doctest::Approx(ref_asym_node[2].u_pu(1)));
        CHECK(test_asym_node[2].u_pu(2) == doctest::Approx(ref_asym_node[2].u_pu(2)));
    }

    SUBCASE("Symmetrical - Incomplete") {
        std::vector<NodeOutput<true>> test_sym_node(state.sym_node.size());
        std::vector<NodeOutput<true>> ref_sym_node(state.sym_node.size());
        test_result_data["node"] = DataPointer<false>{test_sym_node.data(), static_cast<Idx>(test_sym_node.size())};
        ref_result_data["node"] = DataPointer<false>{ref_sym_node.data(), static_cast<Idx>(ref_sym_node.size())};

        SUBCASE("Direct call") {
            CHECK_THROWS_AS(test_model.calculate_power_flow<true>(1e-8, 1, linear), SparseMatrixError);
        }
        SUBCASE("Target dataset") {
            CHECK_THROWS_AS(test_model.calculate_power_flow<true>(1e-8, 1, linear, test_result_data),
                            SparseMatrixError);
        }
        SUBCASE("Empty update dataset") {
            update_data = {};

            CHECK_THROWS_AS(test_model.calculate_power_flow<true>(1e-8, 1, linear, test_result_data, update_data),
                            SparseMatrixError);
        }
        SUBCASE("Update dataset") {
            CHECK_THROWS_AS(
                test_model.calculate_power_flow<true>(1e-8, 1, linear, test_result_data, incomplete_update_data),
                BatchCalculationError);
        }
    }

    SUBCASE("Asymmetrical - Incomplete") {
        std::vector<NodeOutput<false>> test_sym_node(state.sym_node.size());
        std::vector<NodeOutput<false>> ref_sym_node(state.sym_node.size());
        test_result_data["node"] = DataPointer<false>{test_sym_node.data(), static_cast<Idx>(test_sym_node.size())};
        ref_result_data["node"] = DataPointer<false>{ref_sym_node.data(), static_cast<Idx>(ref_sym_node.size())};

        SUBCASE("Direct call") {
            CHECK_THROWS_AS(test_model.calculate_power_flow<false>(1e-8, 1, linear), SparseMatrixError);
        }
        SUBCASE("Target dataset") {
            CHECK_THROWS_AS(test_model.calculate_power_flow<false>(1e-8, 1, linear, test_result_data),
                            SparseMatrixError);
        }
        SUBCASE("Empty update dataset") {
            update_data = {};

            CHECK_THROWS_AS(test_model.calculate_power_flow<false>(1e-8, 1, linear, test_result_data, update_data),
                            SparseMatrixError);
        }
        SUBCASE("Update dataset") {
            CHECK_THROWS_AS(
                test_model.calculate_power_flow<false>(1e-8, 1, linear, test_result_data, incomplete_update_data),
                BatchCalculationError);
        }
    }

    SUBCASE("Sparse followed by dense") {}
}

} // namespace power_grid_model
