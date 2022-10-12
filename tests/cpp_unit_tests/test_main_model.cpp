// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include "doctest/doctest.h"
#include "power_grid_model/main_model.hpp"

namespace power_grid_model {

TEST_CASE("Test main model") {
    std::vector<NodeInput> node_input{{{1}, 10e3}, {{2}, 10e3}, {{3}, 10e3}};
    std::vector<LineInput> line_input{{{{4}, 1, 2, true, true}, 10.0, 0.0, 0.0, 0.0, 10.0, 0.0, 0.0, 0.0, 1e3}};
    std::vector<LinkInput> link_input{{{{5}, 2, 3, true, true}}};
    std::vector<SourceInput> source_input{{{{6}, 1, true}, 1.05, nan, 1e12, nan, nan},
                                          {{{10}, 3, false}, 1.05, 0.0, 1e12, nan, nan}};
    std::vector<SymLoadGenInput> sym_load_input{{{{{7}, 3, true}, LoadGenType::const_y}, 0.5e6, 0.0}};
    std::vector<AsymLoadGenInput> asym_load_input{
        {{{{8}, 3, true}, LoadGenType::const_y}, RealValue<false>{0.5e6 / 3.0}, RealValue<false>{0.0}}};
    std::vector<ShuntInput> shunt_input{{{{9}, 3, true}, 0.015, 0.0, 0.015, 0.0}};

    // {{{id}, measured_object}, measured_terminal_type, power_sigma, p_measured, q_measured}
    std::vector<SymPowerSensorInput> sym_power_sensor_input{
        {{{{11}, 4}, MeasuredTerminalType::branch_from, 0.02}, 1.1e6, 1.1e3},
        {{{{13}, 6}, MeasuredTerminalType::source, 0.02}, 1.3e6, 1.3e3},
        {{{{14}, 6}, MeasuredTerminalType::source, 0.02}, 1.4e6, 1.4e3},
        {{{{15}, 9}, MeasuredTerminalType::shunt, 0.02}, 1.5e6, 1.5e3},
        {{{{16}, 7}, MeasuredTerminalType::load, 0.02}, 1.6e6, 1.6e3},
        {{{{17}, 8}, MeasuredTerminalType::load, 0.02}, 1.7e6, 1.7e3}};

    // {{{id}, measured_object}, measured_terminal_type, power_sigma, p_measured, q_measured}
    std::vector<AsymPowerSensorInput> asym_power_sensor_input{
        {{{{18}, 4}, MeasuredTerminalType::branch_from, 0.02}, {2.11e6, 2.12e6, 2.13e6}, {2.11e3, 2.12e3, 2.13e3}},
        {{{{20}, 6}, MeasuredTerminalType::source, 0.02}, {2.31e6, 2.32e6, 2.33e6}, {2.31e3, 2.32e3, 2.33e3}},
        {{{{21}, 6}, MeasuredTerminalType::source, 0.02}, {2.41e6, 2.42e6, 2.43e6}, {2.41e3, 2.42e3, 2.43e3}},
        {{{{22}, 9}, MeasuredTerminalType::shunt, 0.02}, {2.51e6, 2.52e6, 2.53e6}, {2.51e3, 2.52e3, 2.53e3}},
        {{{{23}, 7}, MeasuredTerminalType::load, 0.02}, {2.61e6, 2.62e6, 2.63e6}, {2.61e3, 2.62e3, 2.63e3}},
        {{{{24}, 8}, MeasuredTerminalType::load, 0.02}, {2.71e6, 2.72e6, 2.73e6}, {2.71e3, 2.72e3, 2.73e3}}};

    // {{{id}, measured_object}, u_sigma, u_measured, u_angle_measured}
    std::vector<SymVoltageSensorInput> sym_voltage_sensor_input{{{{{25}, 1}, 105.0}, 10.1e3, 0.1},
                                                                {{{{26}, 2}, 105.0}, 10.2e3, 0.2}};

    // {{{id}, measured_object}, u_sigma, u_measured, u_angle_measured}
    std::vector<AsymVoltageSensorInput> asym_voltage_sensor_input{
        {{{{27}, 3}, 105.0}, {10.31e3 / sqrt3, 10.32e3 / sqrt3, 10.33e3 / sqrt3}, {0.0, -deg_120, -deg_240}}};

    double const z_bus_2 = 1.0 / (0.015 + 0.5e6 / 10e3 / 10e3 * 2);
    double const z_total = z_bus_2 + 10.0;
    double const u1 = 1.05 * z_bus_2 / (z_bus_2 + 10.0);
    double const i = 1.05 * 10e3 / z_total / sqrt3;
    double const i_shunt = 0.015 / 0.025 * i;
    double const i_load = 0.005 / 0.025 * i;

    // output vector
    std::vector<NodeOutput<true>> sym_node(3);
    std::vector<BranchOutput<true>> sym_branch(2);
    std::vector<ApplianceOutput<true>> sym_appliance(5);
    std::vector<NodeOutput<false>> asym_node(3);
    std::vector<BranchOutput<false>> asym_branch(2);
    std::vector<ApplianceOutput<false>> asym_appliance(5);

    // individual symmetric
    std::vector<BranchOutput<true>> sym_line(1);
    std::vector<BranchOutput<true>> sym_link(1);
    std::vector<ApplianceOutput<true>> sym_load_sym(1);
    std::vector<ApplianceOutput<true>> sym_load_asym(1);
    std::vector<ApplianceOutput<true>> sym_source(2);
    std::vector<ApplianceOutput<true>> sym_shunt(1);
    std::vector<VoltageSensorOutput<true>> sym_voltage_sensor(2);
    std::vector<VoltageSensorOutput<true>> asym_voltage_sensor_sym_output(1);
    std::vector<PowerSensorOutput<true>> sym_power_sensor(7);
    std::vector<PowerSensorOutput<true>> asym_power_sensor_sym_output(7);

    // individual asymmetric
    std::vector<BranchOutput<false>> asym_line(1);
    std::vector<BranchOutput<false>> asym_link(1);
    std::vector<ApplianceOutput<false>> asym_load_sym(1);
    std::vector<ApplianceOutput<false>> asym_load_asym(1);
    std::vector<ApplianceOutput<false>> asym_source(2);
    std::vector<ApplianceOutput<false>> asym_shunt(1);
    std::vector<VoltageSensorOutput<false>> asym_voltage_sensor(1);
    std::vector<VoltageSensorOutput<false>> sym_voltage_sensor_asym_output(2);
    std::vector<PowerSensorOutput<false>> asym_power_sensor(7);
    std::vector<PowerSensorOutput<false>> sym_power_sensor_asym_output(7);

    // update vector
    std::vector<SymLoadGenUpdate> sym_load_update{{{{7}, true}, 1.0e6, nan}};
    std::vector<AsymLoadGenUpdate> asym_load_update{{{{8}, false}, RealValue<false>{nan}, RealValue<false>{nan}}};
    std::vector<ApplianceUpdate> shunt_update{{{9}, false}};
    std::vector<SourceUpdate> source_update{{{{10}, true}, u1, nan}};
    std::vector<BranchUpdate> link_update{{{5}, true, false}};

    MainModel main_model{50.0};
    main_model.add_component<Node>(node_input);
    main_model.add_component<Line>(line_input);
    main_model.add_component<Link>(link_input);
    main_model.add_component<Source>(source_input);
    main_model.add_component<AsymLoad>(asym_load_input);
    main_model.add_component<SymLoad>(sym_load_input);
    main_model.add_component<Shunt>(shunt_input);
    main_model.add_component<SymPowerSensor>(sym_power_sensor_input);
    main_model.add_component<AsymPowerSensor>(asym_power_sensor_input);
    main_model.add_component<SymVoltageSensor>(sym_voltage_sensor_input);
    main_model.add_component<AsymVoltageSensor>(asym_voltage_sensor_input);
    main_model.set_construction_complete();

    SUBCASE("Test get indexer") {
        std::vector<ID> const node_id{2, 1, 3, 2};
        IdxVector const expected_indexer{1, 0, 2, 1};
        IdxVector indexer(4);
        main_model.get_indexer("node", node_id.data(), 4, indexer.data());
        CHECK(indexer == expected_indexer);
    }

    SUBCASE("Test duplicated id") {
        MainModel main_model2{50.0};
        node_input[1].id = 1;
        CHECK_THROWS_AS(main_model2.add_component<Node>(node_input), ConflictID);
    }

    SUBCASE("Test no existing id") {
        MainModel main_model2{50.0};
        line_input[0].from_node = 100;
        main_model2.add_component<Node>(node_input);
        CHECK_THROWS_AS(main_model2.add_component<Line>(line_input), IDNotFound);
    }

    SUBCASE("Test id for wrong type") {
        MainModel main_model2{50.0};

        link_input[0].from_node = 4;
        main_model2.add_component<Node>(node_input);  // 1 2 3
        main_model2.add_component<Line>(line_input);  // 4
        CHECK_THROWS_AS(main_model2.add_component<Link>(link_input), IDWrongType);

        // Fix link input, retry
        link_input[0].from_node = 2;
        main_model2.add_component<Link>(link_input);  // 5

        main_model2.add_component<Source>(source_input);       // 6 10
        main_model2.add_component<SymLoad>(sym_load_input);    // 7
        main_model2.add_component<AsymLoad>(asym_load_input);  // 8
        main_model2.add_component<Shunt>(shunt_input);         // 9

        // voltage sensor with a measured id which is not a node (link)
        sym_voltage_sensor_input[0].measured_object = 5;
        CHECK_THROWS_AS(main_model2.add_component<SymVoltageSensor>(sym_voltage_sensor_input), IDWrongType);

        // Test for all MeasuredTerminalType instances
        std::vector<MeasuredTerminalType> const mt_types{
            MeasuredTerminalType::branch_from, MeasuredTerminalType::branch_to, MeasuredTerminalType::generator,
            MeasuredTerminalType::load,        MeasuredTerminalType::shunt,     MeasuredTerminalType::source};

        // power sensor with terminal branch, with a measured id which is not a branch (node)
        for (auto const& mt_type : mt_types) {
            sym_power_sensor_input[0].measured_object = 1;
            sym_power_sensor_input[0].measured_terminal_type = mt_type;
            CHECK_THROWS_AS(main_model2.add_component<SymPowerSensor>(sym_power_sensor_input), IDWrongType);
        }
    }

    SUBCASE("Test calculate power flow") {
        auto const math_output = main_model.calculate_power_flow<true>(1e-8, 20, CalculationMethod::linear);
        main_model.output_result<true, Node>(math_output, sym_node.begin());
        main_model.output_result<true, Branch>(math_output, sym_branch.begin());
        main_model.output_result<true, Appliance>(math_output, sym_appliance.begin());
    }

    SUBCASE("Test copy main model") {
        MainModel model_2{main_model};
        SUBCASE("Copied - Symmetrical") {
            auto const math_output = model_2.calculate_power_flow<true>(1e-8, 20, CalculationMethod::linear);
            model_2.output_result<true, Node>(math_output, sym_node.begin());
            model_2.output_result<true, Branch>(math_output, sym_branch.begin());
            model_2.output_result<true, Appliance>(math_output, sym_appliance.begin());
            CHECK(sym_node[0].u_pu == doctest::Approx(1.05));
            CHECK(sym_node[1].u_pu == doctest::Approx(u1));
            CHECK(sym_node[2].u_pu == doctest::Approx(u1));
            CHECK(sym_branch[0].i_from == doctest::Approx(i));
            CHECK(sym_appliance[0].i == doctest::Approx(i));
            CHECK(sym_appliance[1].i == doctest::Approx(0.0));
            CHECK(sym_appliance[2].i == doctest::Approx(i_load));
            CHECK(sym_appliance[3].i == doctest::Approx(i_load));
            CHECK(sym_appliance[4].i == doctest::Approx(i_shunt));
        }
        SUBCASE("Copied - Asymmetrical") {
            auto const math_output = model_2.calculate_power_flow<false>(1e-8, 20, CalculationMethod::linear);
            model_2.output_result<false, Node>(math_output, asym_node.begin());
            model_2.output_result<false, Branch>(math_output, asym_branch.begin());
            model_2.output_result<false, Appliance>(math_output, asym_appliance.begin());
            CHECK(asym_node[0].u_pu(0) == doctest::Approx(1.05));
            CHECK(asym_node[1].u_pu(1) == doctest::Approx(u1));
            CHECK(asym_node[2].u_pu(2) == doctest::Approx(u1));
            CHECK(asym_branch[0].i_from(0) == doctest::Approx(i));
            CHECK(asym_appliance[0].i(1) == doctest::Approx(i));
            CHECK(asym_appliance[1].i(2) == doctest::Approx(0.0));
            CHECK(asym_appliance[2].i(0) == doctest::Approx(i_load));
            CHECK(asym_appliance[3].i(1) == doctest::Approx(i_load));
            CHECK(asym_appliance[4].i(2) == doctest::Approx(i_shunt));
        }
        model_2 = main_model;
        SUBCASE("Assigned - Symmetrical") {
            auto const math_output = model_2.calculate_power_flow<true>(1e-8, 20, CalculationMethod::linear);
            model_2.output_result<true, Node>(math_output, sym_node.begin());
            model_2.output_result<true, Branch>(math_output, sym_branch.begin());
            model_2.output_result<true, Appliance>(math_output, sym_appliance.begin());
            // TODO: check voltage angle
            CHECK(sym_node[0].u_pu == doctest::Approx(1.05));
            CHECK(sym_node[1].u_pu == doctest::Approx(u1));
            CHECK(sym_node[2].u_pu == doctest::Approx(u1));
            CHECK(sym_branch[0].i_from == doctest::Approx(i));
            CHECK(sym_appliance[0].i == doctest::Approx(i));
            CHECK(sym_appliance[1].i == doctest::Approx(0.0));
            CHECK(sym_appliance[2].i == doctest::Approx(i_load));
            CHECK(sym_appliance[3].i == doctest::Approx(i_load));
            CHECK(sym_appliance[4].i == doctest::Approx(i_shunt));
        }
        SUBCASE("Assigned - Asymmetrical") {
            auto const math_output = model_2.calculate_power_flow<false>(1e-8, 20, CalculationMethod::linear);
            model_2.output_result<false, Node>(math_output, asym_node.begin());
            model_2.output_result<false, Branch>(math_output, asym_branch.begin());
            model_2.output_result<false, Appliance>(math_output, asym_appliance.begin());
            CHECK(asym_node[0].u_pu(0) == doctest::Approx(1.05));
            CHECK(asym_node[1].u_pu(1) == doctest::Approx(u1));
            CHECK(asym_node[2].u_pu(2) == doctest::Approx(u1));
            CHECK(asym_branch[0].i_from(0) == doctest::Approx(i));
            CHECK(asym_appliance[0].i(1) == doctest::Approx(i));
            CHECK(asym_appliance[1].i(2) == doctest::Approx(0.0));
            CHECK(asym_appliance[2].i(0) == doctest::Approx(i_load));
            CHECK(asym_appliance[3].i(1) == doctest::Approx(i_load));
            CHECK(asym_appliance[4].i(2) == doctest::Approx(i_shunt));
        }
        SUBCASE("Original - Symmetrical") {
            auto const math_output = main_model.calculate_power_flow<true>(1e-8, 20, CalculationMethod::linear);
            main_model.output_result<true, Node>(math_output, sym_node.begin());
            main_model.output_result<true, Branch>(math_output, sym_branch.begin());
            main_model.output_result<true, Appliance>(math_output, sym_appliance.begin());
            CHECK(sym_node[0].u_pu == doctest::Approx(1.05));
            CHECK(sym_node[1].u_pu == doctest::Approx(u1));
            CHECK(sym_node[2].u_pu == doctest::Approx(u1));
            CHECK(sym_branch[0].i_from == doctest::Approx(i));
            CHECK(sym_appliance[0].i == doctest::Approx(i));
            CHECK(sym_appliance[1].i == doctest::Approx(0.0));
            CHECK(sym_appliance[2].i == doctest::Approx(i_load));
            CHECK(sym_appliance[3].i == doctest::Approx(i_load));
            CHECK(sym_appliance[4].i == doctest::Approx(i_shunt));
        }
        SUBCASE("Original - Asymmetrical") {
            auto const math_output = main_model.calculate_power_flow<false>(1e-8, 20, CalculationMethod::linear);
            main_model.output_result<false, Node>(math_output, asym_node.begin());
            main_model.output_result<false, Branch>(math_output, asym_branch.begin());
            main_model.output_result<false, Appliance>(math_output, asym_appliance.begin());
            CHECK(asym_node[0].u_pu(0) == doctest::Approx(1.05));
            CHECK(asym_node[1].u_pu(1) == doctest::Approx(u1));
            CHECK(asym_node[2].u_pu(2) == doctest::Approx(u1));
            CHECK(asym_branch[0].i_from(0) == doctest::Approx(i));
            CHECK(asym_appliance[0].i(1) == doctest::Approx(i));
            CHECK(asym_appliance[1].i(2) == doctest::Approx(0.0));
            CHECK(asym_appliance[2].i(0) == doctest::Approx(i_load));
            CHECK(asym_appliance[3].i(1) == doctest::Approx(i_load));
            CHECK(asym_appliance[4].i(2) == doctest::Approx(i_shunt));
        }
    }

    SUBCASE("Test iterative calculation") {
        SUBCASE("Symmetrical") {
            auto const math_output = main_model.calculate_power_flow<true>(1e-8, 20, CalculationMethod::newton_raphson);
            main_model.output_result<true, Node>(math_output, sym_node.begin());
            main_model.output_result<true, Branch>(math_output, sym_branch.begin());
            main_model.output_result<true, Appliance>(math_output, sym_appliance.begin());
            CHECK(sym_node[0].u_pu == doctest::Approx(1.05));
            CHECK(sym_node[1].u_pu == doctest::Approx(u1));
            CHECK(sym_node[2].u_pu == doctest::Approx(u1));
            CHECK(sym_branch[0].i_from == doctest::Approx(i));
            CHECK(sym_appliance[0].i == doctest::Approx(i));
            CHECK(sym_appliance[1].i == doctest::Approx(0.0));
            CHECK(sym_appliance[2].i == doctest::Approx(i_load));
            CHECK(sym_appliance[3].i == doctest::Approx(i_load));
            CHECK(sym_appliance[4].i == doctest::Approx(i_shunt));
        }
        SUBCASE("Asymmetrical") {
            auto const math_output =
                main_model.calculate_power_flow<false>(1e-8, 20, CalculationMethod::newton_raphson);
            main_model.output_result<false, Node>(math_output, asym_node.begin());
            main_model.output_result<false, Branch>(math_output, asym_branch.begin());
            main_model.output_result<false, Appliance>(math_output, asym_appliance.begin());
            CHECK(asym_node[0].u_pu(0) == doctest::Approx(1.05));
            CHECK(asym_node[1].u_pu(1) == doctest::Approx(u1));
            CHECK(asym_node[2].u_pu(2) == doctest::Approx(u1));
            CHECK(asym_branch[0].i_from(0) == doctest::Approx(i));
            CHECK(asym_appliance[0].i(1) == doctest::Approx(i));
            CHECK(asym_appliance[1].i(2) == doctest::Approx(0.0));
            CHECK(asym_appliance[2].i(0) == doctest::Approx(i_load));
            CHECK(asym_appliance[3].i(1) == doctest::Approx(i_load));
            CHECK(asym_appliance[4].i(2) == doctest::Approx(i_shunt));
        }
    }

    SUBCASE("Test individual output (symmetric)") {
        auto const res = main_model.calculate_power_flow<true>(1e-8, 20, CalculationMethod::newton_raphson);

        SUBCASE("Node, sym output") {
            main_model.output_result<true, Node>(res, sym_node.begin());

            CHECK(sym_node[0].u_pu == doctest::Approx(1.05));
            CHECK(sym_node[1].u_pu == doctest::Approx(u1));
            CHECK(sym_node[2].u_pu == doctest::Approx(u1));

            /*
            TODO
            - u
            - angle
            */
        }

        SUBCASE("Line, sym output") {
            main_model.output_result<true, Line>(res, sym_line.begin());

            CHECK(sym_line[0].i_from == doctest::Approx(i));
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
            main_model.output_result<true, Link>(res, sym_link.begin());

            CHECK(sym_link[0].i_from == doctest::Approx(i));
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
            main_model.output_result<true, Source>(res, sym_source.begin());

            CHECK(sym_source[0].i == doctest::Approx(i));
            CHECK(sym_source[1].i == doctest::Approx(0.0));
            /*
            TODO
            - p
            - q
            - s
            */
        }

        SUBCASE("SymLoad, sym output") {
            main_model.output_result<true, SymLoad>(res, sym_load_sym.begin());

            CHECK(sym_load_sym[0].i == doctest::Approx(i_load));
            /*
            TODO
            - p
            - q
            - s
            */
        }

        SUBCASE("AsymLoad, sym output") {
            main_model.output_result<true, AsymLoad>(res, sym_load_asym.begin());

            CHECK(sym_load_asym[0].i == doctest::Approx(i_load));
            /*
            TODO
            - p
            - q
            - s
            */
        }

        SUBCASE("Shunt, sym output") {
            main_model.output_result<true, Shunt>(res, sym_shunt.begin());

            CHECK(sym_shunt[0].i == doctest::Approx(i_shunt));
            /*
            TODO
            - p
            - q
            - s
            */
        }

        SUBCASE("SymVoltageSensor, sym output") {
            main_model.output_result<true, Node>(res, sym_node.begin());
            main_model.output_result<true, SymVoltageSensor>(res, sym_voltage_sensor.begin());

            CHECK(sym_voltage_sensor[0].u_residual == doctest::Approx(1.01 * 10.0e3 - sym_node[0].u));
            CHECK(sym_voltage_sensor[1].u_residual == doctest::Approx(1.02 * 10.0e3 - sym_node[1].u));
            CHECK(sym_voltage_sensor[0].u_angle_residual == doctest::Approx(0.1 - sym_node[0].u_angle));
            CHECK(sym_voltage_sensor[1].u_angle_residual == doctest::Approx(0.2 - sym_node[1].u_angle));
        }

        SUBCASE("SymPowerSensor, sym output") {
            main_model.output_result<true, Line>(res, sym_line.begin());
            main_model.output_result<true, Link>(res, sym_link.begin());
            main_model.output_result<true, Source>(res, sym_source.begin());
            main_model.output_result<true, SymLoad>(res, sym_load_sym.begin());
            main_model.output_result<true, AsymLoad>(res, sym_load_asym.begin());
            main_model.output_result<true, Shunt>(res, sym_shunt.begin());
            main_model.output_result<true, SymPowerSensor>(res, sym_power_sensor.begin());

            CHECK(sym_power_sensor[0].p_residual == doctest::Approx(1.1e6 - sym_line[0].p_from));
            CHECK(sym_power_sensor[0].q_residual == doctest::Approx(1.1e3 - sym_line[0].q_from));
            CHECK(sym_power_sensor[1].p_residual == doctest::Approx(1.3e6 - sym_source[0].p));
            CHECK(sym_power_sensor[1].q_residual == doctest::Approx(1.3e3 - sym_source[0].q));
            CHECK(sym_power_sensor[2].p_residual == doctest::Approx(1.4e6 - sym_source[0].p));
            CHECK(sym_power_sensor[2].q_residual == doctest::Approx(1.4e3 - sym_source[0].q));
            CHECK(sym_power_sensor[3].p_residual == doctest::Approx(1.5e6 - sym_shunt[0].p));
            CHECK(sym_power_sensor[3].q_residual == doctest::Approx(1.5e3 - sym_shunt[0].q));
            CHECK(sym_power_sensor[4].p_residual == doctest::Approx(1.6e6 - sym_load_sym[0].p));
            CHECK(sym_power_sensor[4].q_residual == doctest::Approx(1.6e3 - sym_load_sym[0].q));
            CHECK(sym_power_sensor[5].p_residual == doctest::Approx(1.7e6 - sym_load_asym[0].p));
            CHECK(sym_power_sensor[5].q_residual == doctest::Approx(1.7e3 - sym_load_asym[0].q));
        }

        SUBCASE("AsymVoltageSensor, sym output") {
            main_model.output_result<true, Node>(res, sym_node.begin());
            main_model.output_result<true, AsymVoltageSensor>(res, asym_voltage_sensor_sym_output.begin());

            CHECK(asym_voltage_sensor_sym_output[0].u_residual == doctest::Approx(10.32e3 - sym_node[2].u));
            CHECK(asym_voltage_sensor_sym_output[0].u_angle_residual == doctest::Approx(0.0 - sym_node[2].u_angle));
        }

        SUBCASE("AsymPowerSensor, sym output") {
            main_model.output_result<true, Line>(res, sym_line.begin());
            main_model.output_result<true, Link>(res, sym_link.begin());
            main_model.output_result<true, Source>(res, sym_source.begin());
            main_model.output_result<true, SymLoad>(res, sym_load_sym.begin());
            main_model.output_result<true, AsymLoad>(res, sym_load_asym.begin());
            main_model.output_result<true, Shunt>(res, sym_shunt.begin());
            main_model.output_result<true, AsymPowerSensor>(res, asym_power_sensor_sym_output.begin());

            CHECK(asym_power_sensor_sym_output[0].p_residual == doctest::Approx(3 * 2.12e6 - sym_line[0].p_from));
            CHECK(asym_power_sensor_sym_output[0].q_residual == doctest::Approx(3 * 2.12e3 - sym_line[0].q_from));
            CHECK(asym_power_sensor_sym_output[1].p_residual == doctest::Approx(3 * 2.32e6 - sym_source[0].p));
            CHECK(asym_power_sensor_sym_output[1].q_residual == doctest::Approx(3 * 2.32e3 - sym_source[0].q));
            CHECK(asym_power_sensor_sym_output[2].p_residual == doctest::Approx(3 * 2.42e6 - sym_source[0].p));
            CHECK(asym_power_sensor_sym_output[2].q_residual == doctest::Approx(3 * 2.42e3 - sym_source[0].q));
            CHECK(asym_power_sensor_sym_output[3].p_residual == doctest::Approx(3 * 2.52e6 - sym_shunt[0].p));
            CHECK(asym_power_sensor_sym_output[3].q_residual == doctest::Approx(3 * 2.52e3 - sym_shunt[0].q));
            CHECK(asym_power_sensor_sym_output[4].p_residual == doctest::Approx(3 * 2.62e6 - sym_load_sym[0].p));
            CHECK(asym_power_sensor_sym_output[4].q_residual == doctest::Approx(3 * 2.62e3 - sym_load_sym[0].q));
            CHECK(asym_power_sensor_sym_output[5].p_residual == doctest::Approx(3 * 2.72e6 - sym_load_asym[0].p));
            CHECK(asym_power_sensor_sym_output[5].q_residual == doctest::Approx(3 * 2.72e3 - sym_load_asym[0].q));
        }
    }

    SUBCASE("Test individual output (asymmetric)") {
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

        SUBCASE("AsymVoltageSensor, asym output") {
            main_model.output_result<false, Node>(res, asym_node.begin());
            main_model.output_result<false, AsymVoltageSensor>(res, asym_voltage_sensor.begin());

            CHECK(asym_voltage_sensor[0].u_residual[0] == doctest::Approx(1.031 / sqrt3 * 10.0e3 - asym_node[2].u[0]));
            CHECK(asym_voltage_sensor[0].u_residual[1] == doctest::Approx(1.032 / sqrt3 * 10.0e3 - asym_node[2].u[1]));
            CHECK(asym_voltage_sensor[0].u_residual[2] == doctest::Approx(1.033 / sqrt3 * 10.0e3 - asym_node[2].u[2]));
            CHECK(asym_voltage_sensor[0].u_angle_residual[0] == doctest::Approx(0.0 - asym_node[2].u_angle[0]));
            CHECK(asym_voltage_sensor[0].u_angle_residual[1] == doctest::Approx(-deg_120 - asym_node[2].u_angle[1]));
            CHECK(asym_voltage_sensor[0].u_angle_residual[2] == doctest::Approx(-deg_240 - asym_node[2].u_angle[2]));
        }

        SUBCASE("SymVoltageSensor, asym output") {
            main_model.output_result<false, Node>(res, asym_node.begin());
            main_model.output_result<false, SymVoltageSensor>(res, sym_voltage_sensor_asym_output.begin());

            CHECK(sym_voltage_sensor_asym_output[0].u_residual[0] ==
                  doctest::Approx(10.1e3 / sqrt3 - asym_node[0].u[0]));
            CHECK(sym_voltage_sensor_asym_output[0].u_residual[1] ==
                  doctest::Approx(10.1e3 / sqrt3 - asym_node[0].u[1]));
            CHECK(sym_voltage_sensor_asym_output[0].u_residual[2] ==
                  doctest::Approx(10.1e3 / sqrt3 - asym_node[0].u[2]));
            CHECK(sym_voltage_sensor_asym_output[0].u_angle_residual[0] ==
                  doctest::Approx(0.1 - asym_node[0].u_angle[0]));
            CHECK(sym_voltage_sensor_asym_output[0].u_angle_residual[1] ==
                  doctest::Approx(0.1 - asym_node[0].u_angle[1]));
            CHECK(sym_voltage_sensor_asym_output[0].u_angle_residual[2] ==
                  doctest::Approx(0.1 - asym_node[0].u_angle[2]));
            CHECK(sym_voltage_sensor_asym_output[1].u_residual[0] ==
                  doctest::Approx(10.2e3 / sqrt3 - asym_node[1].u[0]));
            CHECK(sym_voltage_sensor_asym_output[1].u_residual[1] ==
                  doctest::Approx(10.2e3 / sqrt3 - asym_node[1].u[1]));
            CHECK(sym_voltage_sensor_asym_output[1].u_residual[2] ==
                  doctest::Approx(10.2e3 / sqrt3 - asym_node[1].u[2]));
            CHECK(sym_voltage_sensor_asym_output[1].u_angle_residual[0] ==
                  doctest::Approx(0.2 - asym_node[1].u_angle[0]));
            CHECK(sym_voltage_sensor_asym_output[1].u_angle_residual[1] ==
                  doctest::Approx(0.2 - asym_node[1].u_angle[1]));
            CHECK(sym_voltage_sensor_asym_output[1].u_angle_residual[2] ==
                  doctest::Approx(0.2 - asym_node[1].u_angle[2]));
        }

        // Note that only 1/3 of the values is being checked
        SUBCASE("AsymPowerSensor, asym output") {
            main_model.output_result<false, Line>(res, asym_line.begin());
            main_model.output_result<false, Link>(res, asym_link.begin());
            main_model.output_result<false, Source>(res, asym_source.begin());
            main_model.output_result<false, SymLoad>(res, asym_load_sym.begin());
            main_model.output_result<false, AsymLoad>(res, asym_load_asym.begin());
            main_model.output_result<false, Shunt>(res, asym_shunt.begin());
            main_model.output_result<false, AsymPowerSensor>(res, asym_power_sensor.begin());

            CHECK(asym_power_sensor[0].p_residual[0] == doctest::Approx(2.11e6 - asym_line[0].p_from[0]));
            CHECK(asym_power_sensor[0].q_residual[1] == doctest::Approx(2.12e3 - asym_line[0].q_from[1]));
            CHECK(asym_power_sensor[1].p_residual[1] == doctest::Approx(2.32e6 - asym_source[0].p[1]));
            CHECK(asym_power_sensor[1].q_residual[2] == doctest::Approx(2.33e3 - asym_source[0].q[2]));
            CHECK(asym_power_sensor[2].p_residual[0] == doctest::Approx(2.41e6 - asym_source[0].p[0]));
            CHECK(asym_power_sensor[2].q_residual[1] == doctest::Approx(2.42e3 - asym_source[0].q[1]));
            CHECK(asym_power_sensor[3].p_residual[2] == doctest::Approx(2.53e6 - asym_shunt[0].p[2]));
            CHECK(asym_power_sensor[3].q_residual[0] == doctest::Approx(2.51e3 - asym_shunt[0].q[0]));
            CHECK(asym_power_sensor[4].p_residual[1] == doctest::Approx(2.62e6 - asym_load_sym[0].p[1]));
            CHECK(asym_power_sensor[4].q_residual[2] == doctest::Approx(2.63e3 - asym_load_sym[0].q[2]));
            CHECK(asym_power_sensor[5].p_residual[0] == doctest::Approx(2.71e6 - asym_load_asym[0].p[0]));
            CHECK(asym_power_sensor[5].q_residual[1] == doctest::Approx(2.72e3 - asym_load_asym[0].q[1]));
        }

        SUBCASE("SymPowerSensor, asym output") {
            main_model.output_result<false, Line>(res, asym_line.begin());
            main_model.output_result<false, Link>(res, asym_link.begin());
            main_model.output_result<false, Source>(res, asym_source.begin());
            main_model.output_result<false, SymLoad>(res, asym_load_sym.begin());
            main_model.output_result<false, AsymLoad>(res, asym_load_asym.begin());
            main_model.output_result<false, Shunt>(res, asym_shunt.begin());
            main_model.output_result<false, SymPowerSensor>(res, sym_power_sensor_asym_output.begin());

            CHECK(sym_power_sensor_asym_output[0].p_residual[0] == doctest::Approx(1.1e6 / 3 - asym_line[0].p_from[0]));
            CHECK(sym_power_sensor_asym_output[0].q_residual[1] == doctest::Approx(1.1e3 / 3 - asym_line[0].q_from[1]));
            CHECK(sym_power_sensor_asym_output[1].p_residual[1] == doctest::Approx(1.3e6 / 3 - asym_source[0].p[1]));
            CHECK(sym_power_sensor_asym_output[1].q_residual[2] == doctest::Approx(1.3e3 / 3 - asym_source[0].q[2]));
            CHECK(sym_power_sensor_asym_output[2].p_residual[0] == doctest::Approx(1.4e6 / 3 - asym_source[0].p[0]));
            CHECK(sym_power_sensor_asym_output[2].q_residual[1] == doctest::Approx(1.4e3 / 3 - asym_source[0].q[1]));
            CHECK(sym_power_sensor_asym_output[3].p_residual[2] == doctest::Approx(1.5e6 / 3 - asym_shunt[0].p[2]));
            CHECK(sym_power_sensor_asym_output[3].q_residual[0] == doctest::Approx(1.5e3 / 3 - asym_shunt[0].q[0]));
            CHECK(sym_power_sensor_asym_output[4].p_residual[1] == doctest::Approx(1.6e6 / 3 - asym_load_sym[0].p[1]));
            CHECK(sym_power_sensor_asym_output[4].q_residual[2] == doctest::Approx(1.6e3 / 3 - asym_load_sym[0].q[2]));
            CHECK(sym_power_sensor_asym_output[5].p_residual[0] == doctest::Approx(1.7e6 / 3 - asym_load_asym[0].p[0]));
            CHECK(sym_power_sensor_asym_output[5].q_residual[1] == doctest::Approx(1.7e3 / 3 - asym_load_asym[0].q[1]));
        }
    }

    SUBCASE("Test linear calculation") {
        SUBCASE("Symmetrical") {
            auto const math_output = main_model.calculate_power_flow<true>(1e-8, 20, CalculationMethod::linear);
            main_model.output_result<true, Node>(math_output, sym_node.begin());
            main_model.output_result<true, Branch>(math_output, sym_branch.begin());
            main_model.output_result<true, Appliance>(math_output, sym_appliance.begin());
            CHECK(sym_node[0].u_pu == doctest::Approx(1.05));
            CHECK(sym_node[1].u_pu == doctest::Approx(u1));
            CHECK(sym_node[2].u_pu == doctest::Approx(u1));
            CHECK(sym_branch[0].i_from == doctest::Approx(i));
            CHECK(sym_appliance[0].i == doctest::Approx(i));
            CHECK(sym_appliance[1].i == doctest::Approx(0.0));
            CHECK(sym_appliance[2].i == doctest::Approx(i_load));
            CHECK(sym_appliance[3].i == doctest::Approx(i_load));
            CHECK(sym_appliance[4].i == doctest::Approx(i_shunt));
        }
        SUBCASE("Asymmetrical") {
            auto const math_output = main_model.calculate_power_flow<false>(1e-8, 20, CalculationMethod::linear);
            main_model.output_result<false, Node>(math_output, asym_node.begin());
            main_model.output_result<false, Branch>(math_output, asym_branch.begin());
            main_model.output_result<false, Appliance>(math_output, asym_appliance.begin());
            CHECK(asym_node[0].u_pu(0) == doctest::Approx(1.05));
            CHECK(asym_node[1].u_pu(1) == doctest::Approx(u1));
            CHECK(asym_node[2].u_pu(2) == doctest::Approx(u1));
            CHECK(asym_branch[0].i_from(0) == doctest::Approx(i));
            CHECK(asym_appliance[0].i(1) == doctest::Approx(i));
            CHECK(asym_appliance[1].i(2) == doctest::Approx(0.0));
            CHECK(asym_appliance[2].i(0) == doctest::Approx(i_load));
            CHECK(asym_appliance[3].i(1) == doctest::Approx(i_load));
            CHECK(asym_appliance[4].i(2) == doctest::Approx(i_shunt));
        }
    }

    SUBCASE("Test update with unknown id") {
        std::vector<SourceUpdate> source_update2{SourceUpdate{{{100}, true}, nan, nan}};
        CHECK_THROWS_AS(main_model.update_component<Source>(source_update2), IDNotFound);
    }

    SUBCASE("Test update only load") {
        main_model.update_component<SymLoad>(sym_load_update);
        main_model.update_component<AsymLoad>(asym_load_update);
        SUBCASE("Symmetrical") {
            auto const math_output = main_model.calculate_power_flow<true>(1e-8, 20, CalculationMethod::linear);
            main_model.output_result<true, Node>(math_output, sym_node.begin());
            main_model.output_result<true, Branch>(math_output, sym_branch.begin());
            main_model.output_result<true, Appliance>(math_output, sym_appliance.begin());
            CHECK(sym_node[0].u_pu == doctest::Approx(1.05));
            CHECK(sym_node[1].u_pu == doctest::Approx(u1));
            CHECK(sym_node[2].u_pu == doctest::Approx(u1));
            CHECK(sym_branch[0].i_from == doctest::Approx(i));
            CHECK(sym_appliance[0].i == doctest::Approx(i));
            CHECK(sym_appliance[1].i == doctest::Approx(0.0));
            CHECK(sym_appliance[2].i == doctest::Approx(i_load * 2));
            CHECK(sym_appliance[3].i == doctest::Approx(0.0));
            CHECK(sym_appliance[4].i == doctest::Approx(i_shunt));
        }
        SUBCASE("Asymmetrical") {
            auto const math_output = main_model.calculate_power_flow<false>(1e-8, 20, CalculationMethod::linear);
            main_model.output_result<false, Node>(math_output, asym_node.begin());
            main_model.output_result<false, Branch>(math_output, asym_branch.begin());
            main_model.output_result<false, Appliance>(math_output, asym_appliance.begin());
            CHECK(asym_node[0].u_pu(0) == doctest::Approx(1.05));
            CHECK(asym_node[1].u_pu(1) == doctest::Approx(u1));
            CHECK(asym_node[2].u_pu(2) == doctest::Approx(u1));
            CHECK(asym_branch[0].i_from(0) == doctest::Approx(i));
            CHECK(asym_appliance[0].i(1) == doctest::Approx(i));
            CHECK(asym_appliance[1].i(2) == doctest::Approx(0.0));
            CHECK(asym_appliance[2].i(0) == doctest::Approx(i_load * 2));
            CHECK(asym_appliance[3].i(1) == doctest::Approx(0.0));
            CHECK(asym_appliance[4].i(2) == doctest::Approx(i_shunt));
        }
    }

    SUBCASE("Test update load and shunt param") {
        sym_load_update[0].p_specified = 2.5e6;
        main_model.update_component<SymLoad>(sym_load_update);
        main_model.update_component<AsymLoad>(asym_load_update);
        main_model.update_component<Shunt>(shunt_update);
        SUBCASE("Symmetrical") {
            auto const math_output = main_model.calculate_power_flow<true>(1e-8, 20, CalculationMethod::linear);
            main_model.output_result<true, Node>(math_output, sym_node.begin());
            main_model.output_result<true, Branch>(math_output, sym_branch.begin());
            main_model.output_result<true, Appliance>(math_output, sym_appliance.begin());
            CHECK(sym_node[0].u_pu == doctest::Approx(1.05));
            CHECK(sym_node[1].u_pu == doctest::Approx(u1));
            CHECK(sym_node[2].u_pu == doctest::Approx(u1));
            CHECK(sym_branch[0].i_from == doctest::Approx(i));
            CHECK(sym_appliance[0].i == doctest::Approx(i));
            CHECK(sym_appliance[1].i == doctest::Approx(0.0));
            CHECK(sym_appliance[2].i == doctest::Approx(i_load * 2 + i_shunt));
            CHECK(sym_appliance[3].i == doctest::Approx(0.0));
            CHECK(sym_appliance[4].i == doctest::Approx(0.0));
        }
        SUBCASE("Asymmetrical") {
            auto const math_output = main_model.calculate_power_flow<false>(1e-8, 20, CalculationMethod::linear);
            main_model.output_result<false, Node>(math_output, asym_node.begin());
            main_model.output_result<false, Branch>(math_output, asym_branch.begin());
            main_model.output_result<false, Appliance>(math_output, asym_appliance.begin());
            CHECK(asym_node[0].u_pu(0) == doctest::Approx(1.05));
            CHECK(asym_node[1].u_pu(1) == doctest::Approx(u1));
            CHECK(asym_node[2].u_pu(2) == doctest::Approx(u1));
            CHECK(asym_branch[0].i_from(0) == doctest::Approx(i));
            CHECK(asym_appliance[0].i(1) == doctest::Approx(i));
            CHECK(asym_appliance[1].i(2) == doctest::Approx(0.0));
            CHECK(asym_appliance[2].i(0) == doctest::Approx(i_load * 2 + i_shunt));
            CHECK(asym_appliance[3].i(1) == doctest::Approx(0.0));
            CHECK(asym_appliance[4].i(2) == doctest::Approx(0.0));
        }
    }
    SUBCASE("Test all updates") {
        sym_load_update[0].p_specified = 2.5e6;
        main_model.update_component<AsymLoad>(asym_load_update);
        main_model.update_component<SymLoad>(sym_load_update);
        main_model.update_component<Shunt>(shunt_update);
        main_model.update_component<Source>(source_update);
        main_model.update_component<Link>(link_update);
        SUBCASE("Symmetrical") {
            auto const math_output = main_model.calculate_power_flow<true>(1e-8, 20, CalculationMethod::linear);
            main_model.output_result<true, Node>(math_output, sym_node.begin());
            main_model.output_result<true, Branch>(math_output, sym_branch.begin());
            main_model.output_result<true, Appliance>(math_output, sym_appliance.begin());
            CHECK(sym_node[0].u_pu == doctest::Approx(1.05));
            CHECK(sym_node[1].u_pu == doctest::Approx(1.05));
            CHECK(sym_node[2].u_pu == doctest::Approx(u1));
            CHECK(sym_branch[0].i_from == doctest::Approx(0.0).epsilon(1e-6));
            CHECK(sym_appliance[0].i == doctest::Approx(0.0).epsilon(1e-6));
            CHECK(sym_appliance[1].i == doctest::Approx(i));
            CHECK(sym_appliance[2].i == doctest::Approx(i));
            CHECK(sym_appliance[3].i == doctest::Approx(0.0));
            CHECK(sym_appliance[4].i == doctest::Approx(0.0));
        }
        SUBCASE("Asymmetrical") {
            auto const math_output = main_model.calculate_power_flow<false>(1e-8, 20, CalculationMethod::linear);
            main_model.output_result<false, Node>(math_output, asym_node.begin());
            main_model.output_result<false, Branch>(math_output, asym_branch.begin());
            main_model.output_result<false, Appliance>(math_output, asym_appliance.begin());
            CHECK(asym_node[0].u_pu(0) == doctest::Approx(1.05));
            CHECK(asym_node[1].u_pu(1) == doctest::Approx(1.05));
            CHECK(asym_node[2].u_pu(2) == doctest::Approx(u1));
            CHECK(asym_branch[0].i_from(0) == doctest::Approx(0.0).epsilon(1e-6));
            CHECK(asym_appliance[0].i(1) == doctest::Approx(0.0).epsilon(1e-6));
            CHECK(asym_appliance[1].i(2) == doctest::Approx(i));
            CHECK(asym_appliance[2].i(0) == doctest::Approx(i));
            CHECK(asym_appliance[3].i(1) == doctest::Approx(0.0));
            CHECK(asym_appliance[4].i(2) == doctest::Approx(0.0));
        }
    }

    SUBCASE("Test runtime dispatch") {
        ConstDataset input_data;
        input_data["node"] = DataPointer<true>{node_input.data(), (Idx)node_input.size()};
        input_data["line"] = DataPointer<true>{line_input.data(), (Idx)line_input.size()};
        input_data["link"] = DataPointer<true>{link_input.data(), (Idx)link_input.size()};
        input_data["source"] = DataPointer<true>{source_input.data(), (Idx)source_input.size()};
        input_data["sym_load"] = DataPointer<true>{sym_load_input.data(), (Idx)sym_load_input.size()};
        input_data["asym_load"] = DataPointer<true>{asym_load_input.data(), (Idx)asym_load_input.size()};
        input_data["shunt"] = DataPointer<true>{shunt_input.data(), (Idx)shunt_input.size()};
        ConstDataset update_data;
        update_data["sym_load"] = DataPointer<true>{sym_load_update.data(), (Idx)sym_load_update.size()};
        update_data["asym_load"] = DataPointer<true>{asym_load_update.data(), (Idx)asym_load_update.size()};
        update_data["shunt"] = DataPointer<true>{shunt_update.data(), (Idx)shunt_update.size()};
        update_data["source"] = DataPointer<true>{source_update.data(), (Idx)source_update.size()};
        update_data["link"] = DataPointer<true>{link_update.data(), (Idx)link_update.size()};
        Dataset sym_result_data;
        sym_result_data["node"] = DataPointer<false>{sym_node.data(), (Idx)sym_node.size()};
        sym_result_data["line"] = DataPointer<false>{sym_line.data(), (Idx)sym_line.size()};
        sym_result_data["link"] = DataPointer<false>{sym_link.data(), (Idx)sym_link.size()};
        sym_result_data["source"] = DataPointer<false>{sym_source.data(), (Idx)sym_source.size()};
        sym_result_data["sym_load"] = DataPointer<false>{sym_load_sym.data(), (Idx)sym_load_sym.size()};
        sym_result_data["asym_load"] = DataPointer<false>{sym_load_asym.data(), (Idx)sym_load_asym.size()};
        sym_result_data["shunt"] = DataPointer<false>{sym_shunt.data(), (Idx)sym_shunt.size()};
        Dataset asym_result_data;
        asym_result_data["node"] = DataPointer<false>{asym_node.data(), (Idx)asym_node.size()};

        MainModel model{50.0, input_data};
        auto const count = model.all_component_count();
        CHECK(count.at("node") == 3);
        CHECK(count.at("source") == 2);
        CHECK(count.find("sym_gen") == count.cend());

        // calculation
        model.calculate_power_flow<true>(1e-8, 20, CalculationMethod::newton_raphson, sym_result_data);
        CHECK(sym_node[0].u_pu == doctest::Approx(1.05));
        CHECK(sym_node[1].u_pu == doctest::Approx(u1));
        CHECK(sym_node[2].u_pu == doctest::Approx(u1));
        CHECK(sym_line[0].i_from == doctest::Approx(i));
        CHECK(sym_link[0].i_from == doctest::Approx(i));
        CHECK(sym_source[0].i == doctest::Approx(i));
        CHECK(sym_source[1].i == doctest::Approx(0.0));
        CHECK(sym_load_sym[0].i == doctest::Approx(i_load));
        CHECK(sym_load_asym[0].i == doctest::Approx(i_load));
        CHECK(sym_shunt[0].i == doctest::Approx(i_shunt));
        model.calculate_power_flow<false>(1e-8, 20, CalculationMethod::newton_raphson, asym_result_data);
        CHECK(asym_node[0].u_pu(0) == doctest::Approx(1.05));
        CHECK(asym_node[1].u_pu(1) == doctest::Approx(u1));
        CHECK(asym_node[2].u_pu(2) == doctest::Approx(u1));

        // update and calculation
        model.update_component(update_data);
        model.calculate_power_flow<true>(1e-8, 20, CalculationMethod::newton_raphson, sym_result_data);
        CHECK(sym_node[0].u_pu == doctest::Approx(1.05));
        CHECK(sym_node[1].u_pu == doctest::Approx(1.05));
        CHECK(sym_node[2].u_pu == doctest::Approx(u1));
        model.calculate_power_flow<false>(1e-8, 20, CalculationMethod::newton_raphson, asym_result_data);
        CHECK(asym_node[0].u_pu(0) == doctest::Approx(1.05));
        CHECK(asym_node[1].u_pu(1) == doctest::Approx(1.05));
        CHECK(asym_node[2].u_pu(2) == doctest::Approx(u1));

        // test batch calculation
        model = MainModel{50.0, input_data};
        // symmetric sequential
        model.calculate_power_flow<true>(1e-8, 20, CalculationMethod::newton_raphson, sym_result_data, update_data, -1);
        CHECK(sym_node[0].u_pu == doctest::Approx(1.05));
        CHECK(sym_node[1].u_pu == doctest::Approx(1.05));
        CHECK(sym_node[2].u_pu == doctest::Approx(u1));
        // symmetric parallel
        model.calculate_power_flow<true>(1e-8, 20, CalculationMethod::newton_raphson, sym_result_data, update_data, 0);
        CHECK(sym_node[0].u_pu == doctest::Approx(1.05));
        CHECK(sym_node[1].u_pu == doctest::Approx(1.05));
        CHECK(sym_node[2].u_pu == doctest::Approx(u1));
        // asymmetric sequential
        model.calculate_power_flow<false>(1e-8, 20, CalculationMethod::newton_raphson, asym_result_data, update_data,
                                          -1);
        CHECK(asym_node[0].u_pu(0) == doctest::Approx(1.05));
        CHECK(asym_node[1].u_pu(1) == doctest::Approx(1.05));
        CHECK(asym_node[2].u_pu(2) == doctest::Approx(u1));
        // asymmetric parallel
        model.calculate_power_flow<false>(1e-8, 20, CalculationMethod::newton_raphson, asym_result_data, update_data,
                                          0);
        CHECK(asym_node[0].u_pu(0) == doctest::Approx(1.05));
        CHECK(asym_node[1].u_pu(1) == doctest::Approx(1.05));
        CHECK(asym_node[2].u_pu(2) == doctest::Approx(u1));
    }

    SUBCASE("Test calculate state estimation") {
        auto const math_output =
            main_model.calculate_state_estimation<true>(1e-8, 20, CalculationMethod::iterative_linear);
    }
}

}  // namespace power_grid_model
