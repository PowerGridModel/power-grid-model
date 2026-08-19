// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/main_core/topological_node_output.hpp>

#include <power_grid_model/auxiliary/input.hpp>
#include <power_grid_model/calculation_parameters.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/component_list.hpp>
#include <power_grid_model/common/three_phase_tensor.hpp>
#include <power_grid_model/component/appliance.hpp>
#include <power_grid_model/component/base.hpp>
#include <power_grid_model/component/fault.hpp>
#include <power_grid_model/component/load_gen.hpp>
#include <power_grid_model/component/node.hpp>
#include <power_grid_model/component/shunt.hpp>
#include <power_grid_model/component/source.hpp>
#include <power_grid_model/container.hpp>
#include <power_grid_model/main_core/container_queries.hpp>
#include <power_grid_model/main_core/state.hpp>
#include <power_grid_model/supernodes.hpp>

#include <doctest/doctest.h>

#include <functional>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

namespace power_grid_model::main_core {
namespace {
template <symmetry_tag sym> struct InjectionAccumulator {
    void operator()(Idx2D const& math_id, ComplexValue<sym> const& injection) {
        if (auto [it, inserted] = net_node_injections.try_emplace(math_id, injection); !inserted) {
            it->second += injection;
        }
    }
    std::unordered_map<Idx2D, ComplexValue<sym>, Idx2DHash> net_node_injections{};
};
} // namespace

TEST_CASE("Test topological node output") {
    double const dummy_value = 0.0;
    ComplexValue<symmetric_t> const dummy_complex_value_sym{3.14, 2.71};
    ComplexValue<asymmetric_t> const dummy_complex_value_asym{{0.0, 1.0}, {-2.0, -3.0}, {4.0, -5.0}};

    SUBCASE("get_injection") {
        SUBCASE("ApplianceSolverOutput") {
            ApplianceSolverOutput<symmetric_t> appliance_output;
            appliance_output.s = dummy_complex_value_sym;
            CHECK(detail::get_injection(appliance_output) == dummy_complex_value_sym);

            // asym
            ApplianceSolverOutput<asymmetric_t> appliance_output_asym;
            appliance_output_asym.s = dummy_complex_value_asym;
            CHECK((detail::get_injection(appliance_output_asym)).isApprox(dummy_complex_value_asym));
        }
        SUBCASE("ApplianceShortCircuitSolverOutput") {
            ApplianceShortCircuitSolverOutput<symmetric_t> appliance_short_circuit_output;
            appliance_short_circuit_output.i = dummy_complex_value_sym;
            CHECK(detail::get_injection(appliance_short_circuit_output) == dummy_complex_value_sym);

            // asym
            ApplianceShortCircuitSolverOutput<asymmetric_t> appliance_short_circuit_output_asym;
            appliance_short_circuit_output_asym.i = dummy_complex_value_asym;
            CHECK((detail::get_injection(appliance_short_circuit_output_asym)).isApprox(dummy_complex_value_asym));
        }
    }
    SUBCASE("get_node_sequence_idx") {
        using ComponentContainer = Container<ExtraRetrievableTypes<Appliance, Base, GenericLoadGen>, AsymLoad, Fault,
                                             Node, SymLoad, Source, Shunt>;
        using State = MainModelState<ComponentContainer>;

        State state;
        auto comp_topo = std::make_shared<ComponentTopology>();
        // arbitrary node indices should not matter for this test
        comp_topo->source_node_idx = {Idx{5}, Idx{1000}};
        comp_topo->shunt_node_idx = {Idx{-123}};
        comp_topo->load_gen_node_idx = {Idx{0}, Idx{-1}};
        state.comp_topo = std::make_shared<ComponentTopology const>(std::move(*comp_topo));

        emplace_component<Source>(state.components, 0, SourceInput{}, dummy_value);
        emplace_component<Source>(state.components, 1, SourceInput{}, dummy_value);
        emplace_component<Shunt>(state.components, 2, ShuntInput{}, dummy_value);
        emplace_component<SymLoad>(state.components, 3, LoadGenInput<symmetric_t>{}, dummy_value);
        emplace_component<AsymLoad>(state.components, 4, LoadGenInput<asymmetric_t>{}, dummy_value);
        emplace_component<Node>(state.components, 666, NodeInput{.id = 666}); // .id needed for fault linking
        emplace_component<Fault>(state.components, 5, FaultInput{.fault_object = 666});
        state.components.set_construction_complete();

        SUBCASE("Source") {
            CHECK(detail::get_node_sequence_idx<Source>(state, 0) == Idx{5});
            CHECK(detail::get_node_sequence_idx<Source>(state, 1) == Idx{1000});
        }
        SUBCASE("LoadGen") {
            CHECK(detail::get_node_sequence_idx<AsymLoad>(state, 0) == Idx{0});
            CHECK(detail::get_node_sequence_idx<SymLoad>(state, 0) == Idx{-1});
        }
    }
    SUBCASE("add_appliance_injection") {
        using ComponentContainer =
            Container<ExtraRetrievableTypes<Appliance, Base, GenericLoadGen>, SymLoad, Fault, Node, Source, Shunt>;
        using State = MainModelState<ComponentContainer>;

        State state;
        auto comp_topo = std::make_shared<ComponentTopology>();
        comp_topo->n_node = 2;
        comp_topo->source_node_idx = {Idx{0}, Idx{1}};
        comp_topo->shunt_node_idx = {Idx{1}};
        comp_topo->load_gen_node_idx = {Idx{1}};

        ComponentConnections const comp_conn;
        // no links, so no supernodes, identity mapping
        // TODO(figueroa1395): this needs to be modified later when link output is added
        state.reduced_topology =
            std::make_shared<ReducedTopology const>(supernodes::reduce_topology(*comp_topo, comp_conn));
        state.comp_topo = std::make_shared<ComponentTopology const>(std::move(*comp_topo));

        state.topo_comp_coup = std::make_shared<TopologicalComponentToMathCoupling const>([] {
            TopologicalComponentToMathCoupling result;
            result.load_gen = {{.group = 0, .pos = 0}};
            result.source = {{.group = 0, .pos = 0}, {.group = disconnected, .pos = disconnected}};
            return result;
        }());

        state.comp_coup = ComponentToMathCoupling{
            .fault = {{.group = 0, .pos = 0}, {.group = 0, .pos = 1}},
        };

        emplace_component<Source>(state.components, 0, SourceInput{}, dummy_value);
        emplace_component<Shunt>(state.components, 1, ShuntInput{}, dummy_value);
        emplace_component<SymLoad>(state.components, 2, LoadGenInput<symmetric_t>{}, dummy_value);
        emplace_component<Node>(state.components, 101, NodeInput{.id = 101}); // .id needed for fault linking
        emplace_component<Node>(state.components, 102, NodeInput{.id = 102}); // .id needed for fault linking
        emplace_component<Fault>(state.components, 3, FaultInput{.fault_object = 101});
        emplace_component<Fault>(state.components, 4, FaultInput{.fault_object = 102});
        state.components.set_construction_complete();

        InjectionAccumulator<symmetric_t> accumulator;

        SUBCASE("Steady state output") {
            MathOutput<std::vector<SolverOutput<symmetric_t>>> math_output{};
            math_output.solver_output.emplace_back(
                SolverOutput<symmetric_t>{.u = {},
                                          .bus_injection = {},
                                          .bus = {},
                                          .branch = {},
                                          .source = {{.s = dummy_complex_value_sym, .i = dummy_complex_value_sym}},
                                          .shunt = {{.s = dummy_complex_value_sym, .i = dummy_complex_value_sym}},
                                          .load_gen = {{.s = dummy_complex_value_sym, .i = dummy_complex_value_sym}},
                                          .voltage_regulator = {}});

            detail::add_appliance_injection<Source>(state, math_output, std::ref(accumulator));
            CHECK(accumulator.net_node_injections.size() == 1);
            CHECK(accumulator.net_node_injections.at(Idx2D{.group = 0, .pos = 0}) == dummy_complex_value_sym);
            CHECK(!accumulator.net_node_injections.contains(Idx2D{.group = 1, .pos = 0}));

            detail::add_appliance_injection<SymLoad>(state, math_output, std::ref(accumulator));
            CHECK(accumulator.net_node_injections.size() == 2);
            CHECK(accumulator.net_node_injections.at(Idx2D{.group = 0, .pos = 0}) == dummy_complex_value_sym);
            CHECK(accumulator.net_node_injections.at(Idx2D{.group = 1, .pos = 0}) == dummy_complex_value_sym);
        }
        SUBCASE("Short circuit output") {
            MathOutput<std::vector<ShortCircuitSolverOutput<symmetric_t>>> math_output{};
            math_output.solver_output.emplace_back(ShortCircuitSolverOutput<symmetric_t>{
                .u_bus = {},
                .fault = {{.i_fault = dummy_complex_value_sym}, {.i_fault = dummy_complex_value_sym}},
                .branch = {},
                .source = {{.i = dummy_complex_value_sym}},
                .shunt = {{.i = dummy_complex_value_sym}},
            });

            detail::add_appliance_injection<Source>(state, math_output, std::ref(accumulator));
            CHECK(accumulator.net_node_injections.size() == 1);
            CHECK(accumulator.net_node_injections.at(Idx2D{.group = 0, .pos = 0}) == dummy_complex_value_sym);
            CHECK(!accumulator.net_node_injections.contains(Idx2D{.group = 1, .pos = 0}));
        }
    }
    SUBCASE("solve_topological_nodes") {
        // first creates super node output empty
        // then it accumulates injection from math output to super node output - only for nodes
        // then gets supernode output
        using ComponentContainer =
            Container<ExtraRetrievableTypes<Appliance, Base, GenericLoadGen>, SymLoad, Fault, Node, Source, Shunt>;
        using State = MainModelState<ComponentContainer>;

        State state;
        auto comp_topo = std::make_shared<ComponentTopology>();
        comp_topo->n_node = 2;
        comp_topo->source_node_idx = {Idx{0}, Idx{1}};
        comp_topo->shunt_node_idx = {Idx{1}};
        comp_topo->load_gen_node_idx = {Idx{1}};

        ComponentConnections const comp_conn;
        // no links, so no supernodes, identity mapping
        // TODO(figueroa1395): this needs to be modified later when link output is added
        state.reduced_topology =
            std::make_shared<ReducedTopology const>(supernodes::reduce_topology(*comp_topo, comp_conn));
        state.comp_topo = std::make_shared<ComponentTopology const>(std::move(*comp_topo));

        state.topo_comp_coup = std::make_shared<TopologicalComponentToMathCoupling const>([] {
            TopologicalComponentToMathCoupling result;
            result.load_gen = {{.group = 0, .pos = 0}};
            result.source = {{.group = 0, .pos = 0}, {.group = disconnected, .pos = disconnected}};
            return result;
        }());

        state.comp_coup = ComponentToMathCoupling{
            .fault = {{.group = 0, .pos = 0}, {.group = 0, .pos = 1}},
        };

        emplace_component<Source>(state.components, 0, SourceInput{}, dummy_value);
        emplace_component<Shunt>(state.components, 1, ShuntInput{}, dummy_value);
        emplace_component<SymLoad>(state.components, 2, LoadGenInput<symmetric_t>{}, dummy_value);
        emplace_component<Node>(state.components, 101, NodeInput{.id = 101}); // .id needed for fault linking
        emplace_component<Node>(state.components, 102, NodeInput{.id = 102}); // .id needed for fault linking
        emplace_component<Fault>(state.components, 3, FaultInput{.fault_object = 101});
        emplace_component<Fault>(state.components, 4, FaultInput{.fault_object = 102});
        state.components.set_construction_complete();

        SUBCASE("Steady state output") {
            MathOutput<std::vector<SolverOutput<symmetric_t>>> math_output{};
            math_output.solver_output.emplace_back(
                SolverOutput<symmetric_t>{.u = {},
                                          .bus_injection = {},
                                          .bus = {},
                                          .branch = {},
                                          .source = {{.s = dummy_complex_value_sym, .i = dummy_complex_value_sym}},
                                          .shunt = {{.s = dummy_complex_value_sym, .i = dummy_complex_value_sym}},
                                          .load_gen = {{.s = dummy_complex_value_sym, .i = dummy_complex_value_sym}},
                                          .voltage_regulator = {}});

            solve_topological_nodes(state, math_output);
            CHECK(math_output.supernode_output.size() == 2);
            CHECK(math_output.supernode_output[0].bus_injection[0] == dummy_complex_value_sym);
            CHECK(math_output.supernode_output[1].bus_injection[0] == dummy_complex_value_sym);
        }

        // TODO(figueroa1395): add short circuit output test when short circuit output is added
    }
}
} // namespace power_grid_model::main_core
