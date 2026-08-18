// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/main_core/container_queries.hpp>
#include <power_grid_model/main_core/output.hpp>
#include <power_grid_model/main_core/state.hpp>

#include <power_grid_model/auxiliary/input.hpp>
#include <power_grid_model/auxiliary/output.hpp>
#include <power_grid_model/calculation_parameters.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/component_list.hpp>
#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/component/appliance.hpp>
#include <power_grid_model/component/base.hpp>
#include <power_grid_model/component/branch.hpp>
#include <power_grid_model/component/current_sensor.hpp>
#include <power_grid_model/component/generic_branch.hpp>
#include <power_grid_model/component/load_gen.hpp>
#include <power_grid_model/component/node.hpp>
#include <power_grid_model/component/power_sensor.hpp>
#include <power_grid_model/component/regulator.hpp>
#include <power_grid_model/component/shunt.hpp>
#include <power_grid_model/component/source.hpp>
#include <power_grid_model/component/transformer_tap_regulator.hpp>
#include <power_grid_model/container.hpp>

#include <doctest/doctest.h>

#include <limits>
#include <memory>
#include <utility>
#include <vector>

namespace power_grid_model::main_core {
namespace {

using PowerSensorOutputComponents =
    Container<ExtraRetrievableTypes<Base, Branch, Appliance, GenericLoadGen, GenericPowerSensor>, GenericBranch, Source,
              Shunt, SymGenerator, SymLoad, SymPowerSensor>;
using PowerSensorOutputState = MainModelState<PowerSensorOutputComponents>;

auto make_power_sensor_output_state() -> PowerSensorOutputState {
    constexpr double u_rated = 10e3;
    PowerSensorOutputState state;

    // This branch is connected on only one side. Its topology group, rather than the open terminal, determines
    // whether output from a sensor on either terminal is energized.
    emplace_component<GenericBranch>(state.components, 9,
                                     GenericBranchInput{.id = 9,
                                                        .from_node = 100,
                                                        .to_node = 101,
                                                        .from_status = 0,
                                                        .to_status = 1,
                                                        .r1 = 1.0,
                                                        .x1 = 1.0,
                                                        .g1 = 0.0,
                                                        .b1 = 0.0},
                                     u_rated, u_rated);

    emplace_component<Source>(state.components, 10, SourceInput{.id = 10, .node = 100, .status = 1, .u_ref = 1.0},
                              u_rated);
    emplace_component<Shunt>(state.components, 11,
                             ShuntInput{.id = 11, .node = 100, .status = 1, .g1 = 0.0, .b1 = 0.0, .g0 = 0.0, .b0 = 0.0},
                             u_rated);
    emplace_component<SymGenerator>(
        state.components, 12,
        SymLoadGenInput{
            .id = 12, .node = 100, .status = 1, .type = LoadGenType::const_pq, .p_specified = 0.0, .q_specified = 0.0},
        u_rated);
    emplace_component<SymLoad>(
        state.components, 13,
        SymLoadGenInput{
            .id = 13, .node = 100, .status = 1, .type = LoadGenType::const_pq, .p_specified = 0.0, .q_specified = 0.0},
        u_rated);

    state.components.set_construction_complete();

    auto coupling = std::make_shared<TopologicalComponentToMathCoupling>();
    coupling->branch = {{.group = 0, .pos = 0}};
    coupling->branch3 = {{.group = 0, .pos = {0, 1, 2}}};
    coupling->source = {{.group = 0, .pos = 0}};
    coupling->shunt = {{.group = 0, .pos = 0}};
    coupling->load_gen = {{.group = 0, .pos = 0}, {.group = 0, .pos = 1}};
    coupling->node = {{.group = 0, .pos = 0}};
    state.topo_comp_coup = std::move(coupling);

    return state;
}

template <symmetry_tag sym> auto make_power_sensor_solver_output() {
    std::vector<SolverOutput<sym>> output(1);
    output[0].u.resize(1);
    output[0].bus_injection.resize(1);
    output[0].branch.resize(3);
    output[0].source.resize(1);
    output[0].shunt.resize(1);
    output[0].load_gen.resize(2);
    return output;
}

template <symmetry_tag sym> void check_zero(RealValue<sym> const& value) {
    if constexpr (is_symmetric_v<sym>) {
        CHECK(value == 0.0);
    } else {
        for (Idx phase = 0; phase != 3; ++phase) {
            CAPTURE(phase);
            CHECK(value[phase] == 0.0);
        }
    }
}

template <symmetry_tag sym> void check_null_power_sensor_output(PowerSensorOutput<sym> const& output) {
    CHECK(output.energized == 0);
    check_zero<sym>(output.p_residual);
    check_zero<sym>(output.q_residual);
}

using CurrentSensorOutputComponents =
    Container<ExtraRetrievableTypes<Base, Node, Branch, GenericCurrentSensor>, GenericBranch, Node, SymCurrentSensor>;
using CurrentSensorOutputState = MainModelState<CurrentSensorOutputComponents>;

auto make_current_sensor_output_state() -> CurrentSensorOutputState {
    constexpr double u_rated = 10e3;
    CurrentSensorOutputState state;

    emplace_component<GenericBranch>(state.components, 9,
                                     GenericBranchInput{.id = 9,
                                                        .from_node = 100,
                                                        .to_node = 101,
                                                        .from_status = 0,
                                                        .to_status = 1,
                                                        .r1 = 1.0,
                                                        .x1 = 1.0,
                                                        .g1 = 0.0,
                                                        .b1 = 0.0},
                                     u_rated, u_rated);
    emplace_component<Node>(state.components, 100, NodeInput{.id = 100, .u_rated = u_rated});
    emplace_component<Node>(state.components, 101, NodeInput{.id = 101, .u_rated = u_rated});
    state.components.set_construction_complete();

    auto topology = std::make_shared<ComponentTopology>();
    topology->branch_node_idx = {{0, 1}};
    state.comp_topo = std::move(topology);

    auto coupling = std::make_shared<TopologicalComponentToMathCoupling>();
    coupling->branch = {{.group = 0, .pos = 0}};
    coupling->node = {{.group = 0, .pos = 0}, {.group = 0, .pos = 1}};
    state.topo_comp_coup = std::move(coupling);

    return state;
}

template <symmetry_tag sym> auto make_current_sensor_solver_output() {
    std::vector<SolverOutput<sym>> output(1);
    output[0].u = {ComplexValue<sym>{1.0}, ComplexValue<sym>{1.0}};
    output[0].branch.resize(1);
    output[0].branch[0].i_f = ComplexValue<sym>{1.0};
    output[0].branch[0].i_t = ComplexValue<sym>{1.0};
    return output;
}

template <symmetry_tag sym> void check_null_current_sensor_output(CurrentSensorOutput<sym> const& output) {
    CHECK(output.energized == 0);
    check_zero<sym>(output.i_residual);
    check_zero<sym>(output.i_angle_residual);
}

} // namespace

TEST_CASE("Test main core output") {
    SUBCASE("TransformerTapRegulator") {
        using ComponentContainer = Container<ExtraRetrievableTypes<Base, Regulator>, TransformerTapRegulator>;
        using State = MainModelState<ComponentContainer>;
        using SymOutput = MathOutput<std::vector<SolverOutput<symmetric_t>>>;
        using AsymOutput = MathOutput<std::vector<SolverOutput<asymmetric_t>>>;

        State state;
        emplace_component<TransformerTapRegulator>(state.components, 0,
                                                   TransformerTapRegulatorInput{.id = 0, .regulated_object = 2},
                                                   ComponentType::test, 10e3);
        emplace_component<TransformerTapRegulator>(state.components, 1,
                                                   TransformerTapRegulatorInput{.id = 1, .regulated_object = 3},
                                                   ComponentType::test, 10e3);
        state.components.set_construction_complete();

        auto comp_topo = std::make_shared<ComponentTopology>();
        comp_topo->regulated_object_idx = {2, 3};
        state.comp_topo = std::make_shared<ComponentTopology const>(std::move(*comp_topo));

        std::vector<TransformerTapRegulatorOutput> output(state.components.template size<TransformerTapRegulator>());

        SUBCASE("No regulation") {
            SUBCASE("Symmetric") {
                output_result<TransformerTapRegulator, ComponentContainer>(state, SymOutput{}, output);
            }
            SUBCASE("Asymmetric") {
                output_result<TransformerTapRegulator, ComponentContainer>(state, AsymOutput{}, output);
            }
            CHECK(output[0].id == 0);
            CHECK(output[0].energized == 0);
            CHECK(output[0].tap_pos == na_IntS);
            CHECK(output[1].id == 1);
            CHECK(output[1].energized == 0);
            CHECK(output[1].tap_pos == na_IntS);
        }
        SUBCASE("One regulated") {
            OptimizerOutput const optimizer_output{
                .transformer_tap_positions = {{.transformer_id = 3, .tap_position = 1}}};
            SUBCASE("Symmetric") {
                output_result<TransformerTapRegulator, ComponentContainer>(
                    state, SymOutput{.solver_output = {}, .optimizer_output = optimizer_output, .supernode_output = {}},
                    output);
            }
            SUBCASE("Asymmetric") {
                output_result<TransformerTapRegulator, ComponentContainer>(
                    state,
                    AsymOutput{.solver_output = {}, .optimizer_output = optimizer_output, .supernode_output = {}},
                    output);
            }
            CHECK(output[0].id == 0);
            CHECK(output[0].energized == 0);
            CHECK(output[0].tap_pos == na_IntS);
            CHECK(output[1].id == 1);
            CHECK(output[1].energized == 1);
            CHECK(output[1].tap_pos == 1);
        }
        SUBCASE("Two regulated") {
            OptimizerOutput const optimizer_output{
                .transformer_tap_positions = {{.transformer_id = 3, .tap_position = 1},
                                              {.transformer_id = 4, .tap_position = 2},
                                              {.transformer_id = 2, .tap_position = 3}}};
            SUBCASE("Symmetric") {
                output_result<TransformerTapRegulator, ComponentContainer>(
                    state, SymOutput{.solver_output = {}, .optimizer_output = optimizer_output, .supernode_output = {}},
                    output);
            }
            SUBCASE("Asymmetric") {
                output_result<TransformerTapRegulator, ComponentContainer>(
                    state,
                    AsymOutput{.solver_output = {}, .optimizer_output = optimizer_output, .supernode_output = {}},
                    output);
            }
            CHECK(output[0].id == 0);
            CHECK(output[0].energized == 1);
            CHECK(output[0].tap_pos == 3);
            CHECK(output[1].id == 1);
            CHECK(output[1].energized == 1);
            CHECK(output[1].tap_pos == 1);
        }
    }
}

TEST_CASE_TEMPLATE("Test main core power sensor output energized state", sym, symmetric_t, asymmetric_t) {
    auto state = make_power_sensor_output_state();
    auto const solver_output = make_power_sensor_solver_output<sym>();

    auto check_appliance = [&](MeasuredTerminalType terminal_type, ID object_id, Idx object_sequence,
                               Appliance& appliance) {
        SymPowerSensor const sensor{{.id = 20,
                                     .measured_object = object_id,
                                     .measured_terminal_type = terminal_type,
                                     .power_sigma = std::numeric_limits<double>::infinity(),
                                     .p_measured = 1.0,
                                     .q_measured = 2.0}};

        // An excluded measurement remains energized while its measured object is active.
        CHECK(output_result(sensor, state, solver_output, object_sequence).energized == 1);

        appliance.set_status(0);

        check_null_power_sensor_output(output_result(sensor, state, solver_output, object_sequence));
    };

    check_appliance(MeasuredTerminalType::source, 10, 0, get_component<Source>(state.components, ID{10}));
    check_appliance(MeasuredTerminalType::shunt, 11, 0, get_component<Shunt>(state.components, ID{11}));
    check_appliance(MeasuredTerminalType::generator, 12,
                    get_component_sequence_idx<GenericLoadGen>(state.components, ID{12}),
                    get_component<SymGenerator>(state.components, ID{12}));
    check_appliance(MeasuredTerminalType::load, 13,
                    get_component_sequence_idx<GenericLoadGen>(state.components, ID{13}),
                    get_component<SymLoad>(state.components, ID{13}));

    // At this overload, topology construction has already resolved the measured object to obj_seq. Its precomputed
    // coupling is the unit input; topology construction and object-ID resolution are tested separately.
    // A connected topology group keeps every terminal of a partially connected branch or branch3 energized.
    for (auto const terminal_type :
         {MeasuredTerminalType::branch_from, MeasuredTerminalType::branch_to, MeasuredTerminalType::branch3_1,
          MeasuredTerminalType::branch3_2, MeasuredTerminalType::branch3_3}) {
        CAPTURE(terminal_type);

        auto const measured_object =
            terminal_type == MeasuredTerminalType::branch_from || terminal_type == MeasuredTerminalType::branch_to
                ? ID{9}
                : ID{14};

        SymPowerSensor const sensor{{.id = 21,
                                     .measured_object = measured_object,
                                     .measured_terminal_type = terminal_type,
                                     .power_sigma = 1.0,
                                     .p_measured = 1.0,
                                     .q_measured = 2.0}};

        CHECK(output_result(sensor, state, solver_output, 0).energized == 1);
    }

    auto disconnected_coupling = std::make_shared<TopologicalComponentToMathCoupling>(*state.topo_comp_coup);
    disconnected_coupling->branch = {{.group = disconnected, .pos = disconnected}};
    disconnected_coupling->branch3 = {{.group = disconnected, .pos = {disconnected, disconnected, disconnected}}};
    state.topo_comp_coup = std::move(disconnected_coupling);

    for (auto const terminal_type :
         {MeasuredTerminalType::branch_from, MeasuredTerminalType::branch_to, MeasuredTerminalType::branch3_1,
          MeasuredTerminalType::branch3_2, MeasuredTerminalType::branch3_3}) {
        CAPTURE(terminal_type);

        auto const measured_object =
            terminal_type == MeasuredTerminalType::branch_from || terminal_type == MeasuredTerminalType::branch_to
                ? ID{9}
                : ID{14};

        SymPowerSensor const sensor{{.id = 22,
                                     .measured_object = measured_object,
                                     .measured_terminal_type = terminal_type,
                                     .power_sigma = 1.0,
                                     .p_measured = 1.0,
                                     .q_measured = 2.0}};

        check_null_power_sensor_output(output_result(sensor, state, solver_output, 0));
    }
}

TEST_CASE_TEMPLATE("Test main core power sensor output with reduced component container", sym, symmetric_t,
                   asymmetric_t) {
    // A reduced main-core state must not instantiate lookups for appliance types that its container omits.
    using ComponentContainer =
        Container<ExtraRetrievableTypes<Branch, GenericPowerSensor>, GenericBranch, SymPowerSensor>;
    static_assert(!common::component_container_c<ComponentContainer, Source>);
    static_assert(!common::component_container_c<ComponentContainer, Shunt>);
    static_assert(!common::component_container_c<ComponentContainer, GenericLoadGen>);

    MainModelState<ComponentContainer> state;
    auto coupling = std::make_shared<TopologicalComponentToMathCoupling>();
    coupling->branch = {{.group = 0, .pos = 0}};
    state.topo_comp_coup = std::move(coupling);

    std::vector<SolverOutput<sym>> solver_output(1);
    solver_output[0].branch.resize(1);

    SymPowerSensor const sensor{{.id = 23,
                                 .measured_object = 9,
                                 .measured_terminal_type = MeasuredTerminalType::branch_from,
                                 .power_sigma = 1.0,
                                 .p_measured = 1.0,
                                 .q_measured = 2.0}};

    CHECK(output_result(sensor, state, solver_output, 0).energized == 1);
}

TEST_CASE_TEMPLATE("Test main core current sensor output uses branch topology group", sym, symmetric_t, asymmetric_t) {
    constexpr double u_rated = 10e3;

    auto state = make_current_sensor_output_state();
    auto const solver_output = make_current_sensor_solver_output<sym>();

    // The branch is open at from and closed at to, but it still belongs to a mathematical group; both terminal
    // sensors therefore remain energized.
    for (auto const terminal_type : {MeasuredTerminalType::branch_from, MeasuredTerminalType::branch_to}) {
        CAPTURE(terminal_type);

        SymCurrentSensor const sensor{{.id = 30,
                                       .measured_object = 9,
                                       .measured_terminal_type = terminal_type,
                                       .angle_measurement_type = AngleMeasurementType::local_angle,
                                       .i_sigma = 1.0,
                                       .i_angle_sigma = 1.0,
                                       .i_measured = 1.0,
                                       .i_angle_measured = 0.0},
                                      u_rated};

        CHECK(output_result(sensor, state, solver_output, 0).energized == 1);
    }

    auto disconnected_coupling = std::make_shared<TopologicalComponentToMathCoupling>(*state.topo_comp_coup);
    disconnected_coupling->branch = {{.group = disconnected, .pos = disconnected}};
    state.topo_comp_coup = std::move(disconnected_coupling);

    for (auto const terminal_type : {MeasuredTerminalType::branch_from, MeasuredTerminalType::branch_to}) {
        CAPTURE(terminal_type);

        SymCurrentSensor const sensor{{.id = 31,
                                       .measured_object = 9,
                                       .measured_terminal_type = terminal_type,
                                       .angle_measurement_type = AngleMeasurementType::local_angle,
                                       .i_sigma = 1.0,
                                       .i_angle_sigma = 1.0,
                                       .i_measured = 1.0,
                                       .i_angle_measured = 0.0},
                                      u_rated};

        check_null_current_sensor_output(output_result(sensor, state, solver_output, 0));
    }
}
} // namespace power_grid_model::main_core
