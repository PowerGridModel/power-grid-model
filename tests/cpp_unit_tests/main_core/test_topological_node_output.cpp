// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/main_core/topological_node_output.hpp>

#include <power_grid_model/auxiliary/input.hpp>
#include <power_grid_model/calculation_parameters.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/component_list.hpp>
#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/common/three_phase_tensor.hpp>
#include <power_grid_model/component/appliance.hpp>
#include <power_grid_model/component/asym_line.hpp>
#include <power_grid_model/component/base.hpp>
#include <power_grid_model/component/branch.hpp>
#include <power_grid_model/component/edge.hpp>
#include <power_grid_model/component/fault.hpp>
#include <power_grid_model/component/generic_branch.hpp>
#include <power_grid_model/component/line.hpp>
#include <power_grid_model/component/load_gen.hpp>
#include <power_grid_model/component/node.hpp>
#include <power_grid_model/component/shunt.hpp>
#include <power_grid_model/component/source.hpp>
#include <power_grid_model/component/three_winding_transformer.hpp>
#include <power_grid_model/component/transformer.hpp>
#include <power_grid_model/container.hpp>
#include <power_grid_model/main_core/container_queries.hpp>
#include <power_grid_model/main_core/state.hpp>
#include <power_grid_model/supernodes.hpp>

#include <doctest/doctest.h>

#include <algorithm>
#include <concepts>
#include <functional>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

namespace power_grid_model::main_core {
namespace {
using ComponentContainer = Container<ExtraRetrievableTypes<Appliance, Base, Edge, Branch, GenericLoadGen>, AsymLoad,
                                     SymLoad, Fault, Line, Node, Source, Shunt>;
using State = MainModelState<ComponentContainer>;

static_assert(detail::ContributesToSteadyStateUserNodeInjection::template value<Source>);
static_assert(detail::ContributesToSteadyStateUserNodeInjection::template value<SymLoad>);
static_assert(detail::ContributesToSteadyStateUserNodeInjection::template value<SymGenerator>);
static_assert(detail::ContributesToSteadyStateUserNodeInjection::template value<AsymLoad>);
static_assert(detail::ContributesToSteadyStateUserNodeInjection::template value<AsymGenerator>);
static_assert(detail::ContributesToSteadyStateUserNodeInjection::template value<Line>);
static_assert(detail::ContributesToSteadyStateUserNodeInjection::template value<GenericBranch>);
static_assert(detail::ContributesToSteadyStateUserNodeInjection::template value<Transformer>);
static_assert(detail::ContributesToSteadyStateUserNodeInjection::template value<AsymLine>);
static_assert(detail::ContributesToSteadyStateUserNodeInjection::template value<Shunt>);
static_assert(detail::ContributesToSteadyStateUserNodeInjection::template value<ThreeWindingTransformer>);
static_assert(!detail::ContributesToSteadyStateUserNodeInjection::template value<Fault>);

static_assert(detail::ContributesToShortCircuitUserNodeInjection::template value<Source>);
static_assert(detail::ContributesToShortCircuitUserNodeInjection::template value<Line>);
static_assert(detail::ContributesToShortCircuitUserNodeInjection::template value<GenericBranch>);
static_assert(detail::ContributesToShortCircuitUserNodeInjection::template value<Transformer>);
static_assert(detail::ContributesToShortCircuitUserNodeInjection::template value<AsymLine>);
static_assert(detail::ContributesToShortCircuitUserNodeInjection::template value<Fault>);
static_assert(detail::ContributesToShortCircuitUserNodeInjection::template value<Shunt>);
static_assert(detail::ContributesToShortCircuitUserNodeInjection::template value<ThreeWindingTransformer>);
static_assert(!detail::ContributesToShortCircuitUserNodeInjection::template value<SymLoad>);
static_assert(!detail::ContributesToShortCircuitUserNodeInjection::template value<AsymLoad>);
static_assert(!detail::ContributesToShortCircuitUserNodeInjection::template value<SymGenerator>);
static_assert(!detail::ContributesToShortCircuitUserNodeInjection::template value<AsymGenerator>);

double constexpr dummy_value = 123.321;
constexpr ComplexValue<symmetric_t> dummy_complex_value_sym() { return {2.14, 3.71}; }
ComplexValue<asymmetric_t> dummy_complex_value_asym() { return {{1.0, 2.0}, {-3.0, -4.0}, {5.0, -6.0}}; }

void check_close(ComplexValue<symmetric_t> const& x, ComplexValue<symmetric_t> const& y) {
    CHECK(x.real() == doctest::Approx(y.real()));
    CHECK(x.imag() == doctest::Approx(y.imag()));
}

inline State make_state() {
    State state;
    state.comp_topo = std::make_shared<ComponentTopology const>([]() {
        ComponentTopology comp_topo;
        comp_topo.n_node = 4;
        comp_topo.source_node_idx = {Idx{0}, Idx{1}};
        comp_topo.shunt_node_idx = {Idx{1}};
        comp_topo.load_gen_node_idx = {Idx{1}, Idx{2}};
        comp_topo.branch_node_idx = {{Idx{0}, Idx{1}}, {Idx{1}, Idx{1}}, {Idx{1}, Idx{0}}, {Idx{2}, Idx{3}}};
        comp_topo.link_node_idx = {{Idx{0}, Idx{1}}, {Idx{1}, Idx{2}}};
        return comp_topo;
    }());

    ComponentConnections const comp_conn = [] {
        ComponentConnections conn;
        conn.link_connected = {{1, 1}, {1, 1}};
        return conn;
    }();
    state.reduced_topology =
        std::make_shared<ReducedTopology const>(supernodes::reduce_topology(*state.comp_topo, comp_conn));

    state.topo_comp_coup = std::make_shared<TopologicalComponentToMathCoupling const>([] {
        TopologicalComponentToMathCoupling topo_comp_coup;
        topo_comp_coup.node = {{.group = 0, .pos = 0},
                               {.group = 0, .pos = 1},
                               {.group = disconnected, .pos = 0},
                               {.group = disconnected, .pos = 1}};
        topo_comp_coup.shunt = {{.group = 0, .pos = 0}};
        topo_comp_coup.load_gen = {{.group = 0, .pos = 0}, {.group = 0, .pos = 1}};
        topo_comp_coup.source = {{.group = 0, .pos = 0}, {.group = disconnected, .pos = disconnected}};
        topo_comp_coup.branch = {{.group = 0, .pos = 0},
                                 {.group = 0, .pos = 1},
                                 {.group = 0, .pos = disconnected},
                                 {.group = disconnected, .pos = disconnected}};
        return topo_comp_coup;
    }());

    state.comp_coup = ComponentToMathCoupling{
        .fault = {{.group = 0, .pos = 0}, {.group = 0, .pos = 1}},
    };

    emplace_component<Source>(state.components, 0, SourceInput{}, dummy_value);
    emplace_component<Source>(state.components, 1, SourceInput{}, dummy_value);
    emplace_component<Shunt>(state.components, 2, ShuntInput{}, dummy_value);
    emplace_component<SymLoad>(state.components, 3, LoadGenInput<symmetric_t>{}, dummy_value);
    emplace_component<AsymLoad>(state.components, 4, LoadGenInput<asymmetric_t>{}, dummy_value);
    emplace_component<Node>(state.components, 101, NodeInput{.id = 101}); // .id needed for fault linking
    emplace_component<Node>(state.components, 102, NodeInput{.id = 102}); // .id needed for fault linking
    emplace_component<Node>(state.components, 103, NodeInput{});
    emplace_component<Node>(state.components, 104, NodeInput{});
    emplace_component<Fault>(state.components, 5, FaultInput{.fault_object = 101});
    emplace_component<Fault>(state.components, 6, FaultInput{.fault_object = 102});
    emplace_component<Line>(state.components, 7, LineInput{}, dummy_value, dummy_value, dummy_value);
    emplace_component<Line>(state.components, 8, LineInput{}, dummy_value, dummy_value, dummy_value);
    emplace_component<Line>(state.components, 9, LineInput{}, dummy_value, dummy_value, dummy_value);
    emplace_component<Line>(state.components, 10, LineInput{}, dummy_value, dummy_value, dummy_value);
    state.components.set_construction_complete();
    return state;
};

inline MathOutput<std::vector<SolverOutput<symmetric_t>>> make_steady_state_math_output_sym() {
    MathOutput<std::vector<SolverOutput<symmetric_t>>> math_output{};
    math_output.solver_output.emplace_back(
        SolverOutput<symmetric_t>{.u = {dummy_complex_value_sym(), dummy_complex_value_sym()},
                                  .bus_injection = {dummy_complex_value_sym(), dummy_complex_value_sym()},
                                  .bus = {},
                                  .branch = {{.s_f = dummy_complex_value_sym(),
                                              .s_t = dummy_complex_value_sym(),
                                              .i_f = dummy_complex_value_sym(),
                                              .i_t = dummy_complex_value_sym()},
                                             {.s_f = 0.5 * dummy_complex_value_sym(),
                                              .s_t = 0.5 * dummy_complex_value_sym(),
                                              .i_f = 0.5 * dummy_complex_value_sym(),
                                              .i_t = 0.5 * dummy_complex_value_sym()},
                                             {},
                                             {}},
                                  .source = {{.s = dummy_complex_value_sym(), .i = dummy_complex_value_sym()}, {}},
                                  .shunt = {{.s = dummy_complex_value_sym(), .i = dummy_complex_value_sym()}},
                                  .load_gen = {{.s = dummy_complex_value_sym(), .i = dummy_complex_value_sym()},
                                               {.s = dummy_complex_value_sym(), .i = dummy_complex_value_sym()}},
                                  .voltage_regulator = {}});
    return math_output;
}

inline MathOutput<std::vector<ShortCircuitSolverOutput<symmetric_t>>> make_short_circuit_math_output_sym() {
    MathOutput<std::vector<ShortCircuitSolverOutput<symmetric_t>>> math_output{};
    math_output.solver_output.emplace_back(ShortCircuitSolverOutput<symmetric_t>{
        .u_bus = {},
        .fault = {{.i_fault = dummy_complex_value_sym()}, {.i_fault = dummy_complex_value_sym()}},
        .branch = {{.i_f = dummy_complex_value_sym(), .i_t = dummy_complex_value_sym()},
                   {.i_f = 0.5 * dummy_complex_value_sym(), .i_t = 0.5 * dummy_complex_value_sym()},
                   {},
                   {}},
        .source = {{.i = dummy_complex_value_sym()}, {}},
        .shunt = {{.i = dummy_complex_value_sym()}}});
    return math_output;
}

template <symmetry_tag sym> struct InjectionAccumulator {
    auto accumulator() {
        return [this]<typename ComponentType>(Idx2D const& math_id, ComplexValue<sym> const& injection) {
            auto& target_map = std::derived_from<ComponentType, Branch> ? branch_flow_into_nodes : net_node_injections;

            if (auto [it, inserted] = target_map.try_emplace(math_id, injection); !inserted) {
                it->second += injection;
            }
        };
    }

    std::unordered_map<Idx2D, ComplexValue<sym>, Idx2DHash> net_node_injections{};
    std::unordered_map<Idx2D, ComplexValue<sym>, Idx2DHash> branch_flow_into_nodes{};
};

// records every invocation and returns a preconfigured result per call, so the surrounding workflow can be tested
// independently of the real link solver implementation
struct LinkSolverMock {
    std::vector<std::vector<BranchIdx>> recorded_edges{};
    std::vector<ComplexVector> recorded_loads{};
    std::vector<ComplexVector> return_values{};
    Idx call_count{0};

    ComplexVector operator()(std::vector<BranchIdx> edges, ComplexVector node_loads) {
        recorded_edges.push_back(std::move(edges));
        recorded_loads.push_back(std::move(node_loads));
        return return_values.at(call_count++);
    }
};
} // namespace

TEST_CASE("Test topological node output") {

    SUBCASE("get_injection") {
        SUBCASE("ApplianceSolverOutput") {
            ApplianceSolverOutput<symmetric_t> appliance_output;
            appliance_output.s = dummy_complex_value_sym();
            CHECK(detail::get_injection(appliance_output) == dummy_complex_value_sym());

            // asym
            ApplianceSolverOutput<asymmetric_t> appliance_output_asym;
            appliance_output_asym.s = dummy_complex_value_asym();
            CHECK((detail::get_injection(appliance_output_asym)).isApprox(dummy_complex_value_asym()));
        }
        SUBCASE("ApplianceShortCircuitSolverOutput") {
            ApplianceShortCircuitSolverOutput<symmetric_t> appliance_short_circuit_output;
            appliance_short_circuit_output.i = dummy_complex_value_sym();
            CHECK(detail::get_injection(appliance_short_circuit_output) == dummy_complex_value_sym());

            // asym
            ApplianceShortCircuitSolverOutput<asymmetric_t> appliance_short_circuit_output_asym;
            appliance_short_circuit_output_asym.i = dummy_complex_value_asym();
            CHECK((detail::get_injection(appliance_short_circuit_output_asym)).isApprox(dummy_complex_value_asym()));
        }
        SUBCASE("FaultShortCircuitSolverOutput") {
            FaultShortCircuitSolverOutput<symmetric_t> fault_short_circuit_output;
            fault_short_circuit_output.i_fault = dummy_complex_value_sym();
            CHECK(detail::get_injection(fault_short_circuit_output) == dummy_complex_value_sym());

            // asym
            FaultShortCircuitSolverOutput<asymmetric_t> fault_short_circuit_output_asym;
            fault_short_circuit_output_asym.i_fault = dummy_complex_value_asym();
            CHECK((detail::get_injection(fault_short_circuit_output_asym)).isApprox(dummy_complex_value_asym()));
        }
        SUBCASE("BranchSolverOutput") {
            BranchSolverOutput<symmetric_t> branch_output;
            branch_output.s_f = dummy_complex_value_sym();
            CHECK(detail::get_injection(branch_output, BranchSide::from) == -dummy_complex_value_sym());
            branch_output.s_t = dummy_complex_value_sym();
            CHECK(detail::get_injection(branch_output, BranchSide::to) == -dummy_complex_value_sym());

            // asym
            BranchSolverOutput<asymmetric_t> branch_output_asym;
            branch_output_asym.s_f = dummy_complex_value_asym();
            CHECK((detail::get_injection(branch_output_asym, BranchSide::from)).isApprox(-dummy_complex_value_asym()));
            branch_output_asym.s_t = dummy_complex_value_asym();
            CHECK((detail::get_injection(branch_output_asym, BranchSide::to)).isApprox(-dummy_complex_value_asym()));
        }
        SUBCASE("BranchShortCircuitSolverOutput") {
            BranchShortCircuitSolverOutput<symmetric_t> branch_short_circuit_output;
            branch_short_circuit_output.i_f = dummy_complex_value_sym();
            CHECK(detail::get_injection(branch_short_circuit_output, BranchSide::from) == -dummy_complex_value_sym());
            branch_short_circuit_output.i_t = dummy_complex_value_sym();
            CHECK(detail::get_injection(branch_short_circuit_output, BranchSide::to) == -dummy_complex_value_sym());

            // asym
            BranchShortCircuitSolverOutput<asymmetric_t> branch_short_circuit_output_asym;
            branch_short_circuit_output_asym.i_f = dummy_complex_value_asym();
            CHECK((detail::get_injection(branch_short_circuit_output_asym, BranchSide::from))
                      .isApprox(-dummy_complex_value_asym()));
            branch_short_circuit_output_asym.i_t = dummy_complex_value_asym();
            CHECK((detail::get_injection(branch_short_circuit_output_asym, BranchSide::to))
                      .isApprox(-dummy_complex_value_asym()));
        }
    }
    SUBCASE("get_node_sequence_idx") {
        auto const state = make_state();

        SUBCASE("Source") {
            CHECK(detail::get_node_sequence_idx<Source>(state, 0) == Idx{0});
            CHECK(detail::get_node_sequence_idx<Source>(state, 1) == Idx{1});
        }
        SUBCASE("LoadGen") {
            CHECK(detail::get_node_sequence_idx<AsymLoad>(state, 0) == Idx{1});
            CHECK(detail::get_node_sequence_idx<SymLoad>(state, 0) == Idx{2});
        }
        SUBCASE("Line") {
            CHECK(detail::get_branch_sequence_idx<Line>(state, 0)[std::to_underlying(BranchSide::from)] == Idx{0});
            CHECK(detail::get_branch_sequence_idx<Line>(state, 0)[std::to_underlying(BranchSide::to)] == Idx{1});
            CHECK(detail::get_branch_sequence_idx<Line>(state, 1)[std::to_underlying(BranchSide::from)] == Idx{1});
            CHECK(detail::get_branch_sequence_idx<Line>(state, 1)[std::to_underlying(BranchSide::to)] == Idx{1});
            CHECK(detail::get_branch_sequence_idx<Line>(state, 2)[std::to_underlying(BranchSide::from)] == Idx{1});
            CHECK(detail::get_branch_sequence_idx<Line>(state, 2)[std::to_underlying(BranchSide::to)] == Idx{0});
            CHECK(detail::get_branch_sequence_idx<Line>(state, 3)[std::to_underlying(BranchSide::from)] == Idx{2});
            CHECK(detail::get_branch_sequence_idx<Line>(state, 3)[std::to_underlying(BranchSide::to)] == Idx{3});
        }
        SUBCASE("Fault") {
            CHECK(detail::get_node_sequence_idx<Fault>(state, 0) == Idx{0});
            CHECK(detail::get_node_sequence_idx<Fault>(state, 1) == Idx{1});
        }
    }
    SUBCASE("add_appliance_injection") {
        auto const state = make_state();
        InjectionAccumulator<symmetric_t> accumulator;

        SUBCASE("Steady state output") {
            auto const math_output = make_steady_state_math_output_sym();

            detail::add_appliance_injection.template operator()<Source>(state, math_output, accumulator.accumulator());
            CHECK(accumulator.net_node_injections.size() == 1);
            CHECK(accumulator.net_node_injections.at(Idx2D{.group = 0, .pos = 0}) == dummy_complex_value_sym());
            CHECK(!accumulator.net_node_injections.contains(Idx2D{.group = 0, .pos = 1}));
            CHECK(!accumulator.net_node_injections.contains(Idx2D{.group = 1, .pos = 0}));

            detail::add_appliance_injection.template operator()<AsymLoad>(state, math_output,
                                                                          accumulator.accumulator());
            CHECK(accumulator.net_node_injections.size() == 2);
            CHECK(accumulator.net_node_injections.at(Idx2D{.group = 0, .pos = 0}) == dummy_complex_value_sym());
            CHECK(accumulator.net_node_injections.at(Idx2D{.group = 0, .pos = 1}) == dummy_complex_value_sym());
            CHECK(!accumulator.net_node_injections.contains(Idx2D{.group = 0, .pos = 2}));
            CHECK(!accumulator.net_node_injections.contains(Idx2D{.group = 1, .pos = 0}));
            CHECK(accumulator.branch_flow_into_nodes.empty());

            detail::add_appliance_injection.template operator()<SymLoad>(state, math_output, accumulator.accumulator());
            CHECK(accumulator.net_node_injections.size() == 3);
            CHECK(accumulator.net_node_injections.at(Idx2D{.group = 0, .pos = 0}) == dummy_complex_value_sym());
            CHECK(accumulator.net_node_injections.at(Idx2D{.group = 0, .pos = 1}) == dummy_complex_value_sym());
            CHECK(accumulator.net_node_injections.at(Idx2D{.group = 0, .pos = 2}) == dummy_complex_value_sym());
            CHECK(!accumulator.net_node_injections.contains(Idx2D{.group = 0, .pos = 3}));
            CHECK(!accumulator.net_node_injections.contains(Idx2D{.group = 1, .pos = 0}));
            CHECK(accumulator.branch_flow_into_nodes.empty());

            detail::add_appliance_injection.template operator()<Line>(state, math_output, accumulator.accumulator());
            CHECK(accumulator.net_node_injections.size() == 3);
            CHECK(accumulator.net_node_injections.at(Idx2D{.group = 0, .pos = 0}) == dummy_complex_value_sym());
            CHECK(accumulator.net_node_injections.at(Idx2D{.group = 0, .pos = 1}) == dummy_complex_value_sym());
            CHECK(accumulator.net_node_injections.at(Idx2D{.group = 0, .pos = 2}) == dummy_complex_value_sym());
            CHECK(!accumulator.net_node_injections.contains(Idx2D{.group = 0, .pos = 3}));
            CHECK(!accumulator.net_node_injections.contains(Idx2D{.group = 1, .pos = 0}));

            CHECK(accumulator.branch_flow_into_nodes.size() == 2);
            CHECK(accumulator.branch_flow_into_nodes.at(Idx2D{.group = 0, .pos = 0}) == -dummy_complex_value_sym());
            CHECK(accumulator.branch_flow_into_nodes.at(Idx2D{.group = 0, .pos = 1}) ==
                  -2.0 * dummy_complex_value_sym());
            CHECK(!accumulator.branch_flow_into_nodes.contains(Idx2D{.group = 0, .pos = 2}));
            CHECK(!accumulator.branch_flow_into_nodes.contains(Idx2D{.group = 1, .pos = 0}));
        }

        SUBCASE("Short circuit output") {
            auto const math_output = make_short_circuit_math_output_sym();

            detail::add_appliance_injection.template operator()<Source>(state, math_output, accumulator.accumulator());
            CHECK(accumulator.net_node_injections.size() == 1);
            CHECK(accumulator.net_node_injections.at(Idx2D{.group = 0, .pos = 0}) == dummy_complex_value_sym());
            CHECK(!accumulator.net_node_injections.contains(Idx2D{.group = 0, .pos = 1}));
            CHECK(!accumulator.net_node_injections.contains(Idx2D{.group = 1, .pos = 0}));
            CHECK(accumulator.branch_flow_into_nodes.empty());

            detail::add_appliance_injection.template operator()<Fault>(state, math_output, accumulator.accumulator());
            CHECK(accumulator.net_node_injections.size() == 2);
            CHECK(accumulator.net_node_injections.at(Idx2D{.group = 0, .pos = 0}) == 2.0 * dummy_complex_value_sym());
            CHECK(accumulator.net_node_injections.at(Idx2D{.group = 0, .pos = 1}) == dummy_complex_value_sym());
            CHECK(!accumulator.net_node_injections.contains(Idx2D{.group = 0, .pos = 2}));
            CHECK(!accumulator.net_node_injections.contains(Idx2D{.group = 1, .pos = 0}));
            CHECK(accumulator.branch_flow_into_nodes.empty());

            detail::add_appliance_injection.template operator()<Line>(state, math_output, accumulator.accumulator());
            CHECK(accumulator.net_node_injections.size() == 2);
            CHECK(accumulator.net_node_injections.at(Idx2D{.group = 0, .pos = 0}) == 2.0 * dummy_complex_value_sym());
            CHECK(accumulator.net_node_injections.at(Idx2D{.group = 0, .pos = 1}) == dummy_complex_value_sym());
            CHECK(!accumulator.net_node_injections.contains(Idx2D{.group = 0, .pos = 2}));
            CHECK(!accumulator.net_node_injections.contains(Idx2D{.group = 1, .pos = 0}));

            CHECK(accumulator.branch_flow_into_nodes.size() == 2);
            CHECK(accumulator.branch_flow_into_nodes.at(Idx2D{.group = 0, .pos = 0}) == -dummy_complex_value_sym());
            CHECK(accumulator.branch_flow_into_nodes.at(Idx2D{.group = 0, .pos = 1}) ==
                  -2.0 * dummy_complex_value_sym());
            CHECK(!accumulator.branch_flow_into_nodes.contains(Idx2D{.group = 0, .pos = 2}));
            CHECK(!accumulator.branch_flow_into_nodes.contains(Idx2D{.group = 1, .pos = 0}));
        }
    }
    SUBCASE("add_flows") {
        auto const state = make_state();
        InjectionAccumulator<symmetric_t> accumulator;

        SUBCASE("Steady state output") {
            auto const math_output = make_steady_state_math_output_sym();

            detail::add_flows<detail::ContributesToSteadyStateUserNodeInjection>(state, math_output,
                                                                                 accumulator.accumulator());

            CHECK(accumulator.net_node_injections.size() == 3);
            CHECK(accumulator.net_node_injections.at(Idx2D{.group = 0, .pos = 0}) == dummy_complex_value_sym());
            CHECK(accumulator.net_node_injections.at(Idx2D{.group = 0, .pos = 1}) == dummy_complex_value_sym());
            CHECK(accumulator.net_node_injections.at(Idx2D{.group = 0, .pos = 2}) == dummy_complex_value_sym());

            CHECK(accumulator.branch_flow_into_nodes.size() == 2);
            CHECK(accumulator.branch_flow_into_nodes.at(Idx2D{.group = 0, .pos = 0}) == -dummy_complex_value_sym());
            CHECK(accumulator.branch_flow_into_nodes.at(Idx2D{.group = 0, .pos = 1}) ==
                  -2.0 * dummy_complex_value_sym());
        }

        SUBCASE("Short circuit output") {
            auto const math_output = make_short_circuit_math_output_sym();

            detail::add_flows<detail::ContributesToShortCircuitUserNodeInjection>(state, math_output,
                                                                                  accumulator.accumulator());

            CHECK(accumulator.net_node_injections.size() == 2);
            CHECK(accumulator.net_node_injections.at(Idx2D{.group = 0, .pos = 0}) == 2.0 * dummy_complex_value_sym());
            CHECK(accumulator.net_node_injections.at(Idx2D{.group = 0, .pos = 1}) == dummy_complex_value_sym());

            CHECK(accumulator.branch_flow_into_nodes.size() == 2);
            CHECK(accumulator.branch_flow_into_nodes.at(Idx2D{.group = 0, .pos = 0}) == -dummy_complex_value_sym());
            CHECK(accumulator.branch_flow_into_nodes.at(Idx2D{.group = 0, .pos = 1}) ==
                  -2.0 * dummy_complex_value_sym());
        }
    }
    SUBCASE("SuperNodeSolverInput::get_total_injection_per_node") {
        auto const state = make_state();
        auto const& links = state.reduced_topology->topo_node_coup.topo_nodes[0].user_links;

        SUBCASE("symmetric") {
            detail::SuperNodeSolverInput<symmetric_t> const input{
                .links = links,
                .node_injection = {dummy_complex_value_sym(), dummy_complex_value_sym(), dummy_complex_value_sym()},
                .node_flow_from_branch = {dummy_complex_value_sym(), dummy_complex_value_sym(),
                                          dummy_complex_value_sym()}};

            auto const total = input.get_total_injection_per_node();
            REQUIRE(total.size() == 3);
            CHECK(
                std::ranges::all_of(total, [](auto const& value) { return value == 2.0 * dummy_complex_value_sym(); }));
        }
        SUBCASE("asymmetric") {
            detail::SuperNodeSolverInput<asymmetric_t> const input{
                .links = links,
                .node_injection = {dummy_complex_value_asym(), dummy_complex_value_asym(), dummy_complex_value_asym()},
                .node_flow_from_branch = {dummy_complex_value_asym(), dummy_complex_value_asym(),
                                          dummy_complex_value_asym()}};

            auto const total = input.get_total_injection_per_node();
            REQUIRE(total.size() == 3);
            CHECK(std::ranges::all_of(
                total, [](auto const& value) { return value.isApprox(2.0 * dummy_complex_value_asym()); }));
        }
    }
    SUBCASE("compute_link_solver") {
        auto const state = make_state();
        auto const& links = state.reduced_topology->topo_node_coup.topo_nodes[0].user_links;

        SUBCASE("symmetric") {
            detail::SuperNodeSolverInput<symmetric_t> const input{
                .links = links,
                .node_injection = {dummy_complex_value_sym(), DoubleComplex{}, 2.0 * dummy_complex_value_sym()},
                .node_flow_from_branch = {DoubleComplex{}, 3.0 * dummy_complex_value_sym(),
                                          -dummy_complex_value_sym()}};

            LinkSolverMock mock{.return_values = {
                                    {2.0 * dummy_complex_value_sym(), -dummy_complex_value_sym()},
                                }};

            auto const result = detail::compute_link_solver<symmetric_t>(std::ref(mock), input);

            REQUIRE(mock.call_count == 1);
            CHECK(mock.recorded_edges[0] == links);
            CHECK(mock.recorded_loads[0] ==
                  ComplexVector{dummy_complex_value_sym(), 3.0 * dummy_complex_value_sym(), dummy_complex_value_sym()});

            REQUIRE(result.size() == 2);
            CHECK(result[0] == 2.0 * dummy_complex_value_sym());
            CHECK(result[1] == -dummy_complex_value_sym());
        }
        SUBCASE("asymmetric") {
            detail::SuperNodeSolverInput<asymmetric_t> const input{
                .links = links,
                .node_injection = {dummy_complex_value_asym(), ComplexValue<asymmetric_t>{},
                                   2.0 * dummy_complex_value_asym()},
                .node_flow_from_branch = {ComplexValue<asymmetric_t>{}, 3.0 * dummy_complex_value_asym(),
                                          -dummy_complex_value_asym()}};

            LinkSolverMock mock{.return_values = {{dummy_complex_value_asym()(0), 2.0 * dummy_complex_value_asym()(0)},
                                                  {-dummy_complex_value_asym()(1), 3.0 * dummy_complex_value_asym()(1)},
                                                  {4.0 * dummy_complex_value_asym()(2), DoubleComplex{}}}};

            auto const result = detail::compute_link_solver<asymmetric_t>(std::ref(mock), input);

            REQUIRE(mock.call_count == 3);
            for (Idx phase = 0; phase < 3; ++phase) {
                CHECK(mock.recorded_edges[phase] == links);
                CHECK(mock.recorded_loads[phase] == ComplexVector{dummy_complex_value_asym()(phase),
                                                                  3.0 * dummy_complex_value_asym()(phase),
                                                                  dummy_complex_value_asym()(phase)});
            }

            REQUIRE(result.size() == 2);
            CHECK(result[0].isApprox(ComplexValue<asymmetric_t>{
                dummy_complex_value_asym()(0), -dummy_complex_value_asym()(1), 4.0 * dummy_complex_value_asym()(2)}));
            CHECK(result[1].isApprox(ComplexValue<asymmetric_t>{2.0 * dummy_complex_value_asym()(0),
                                                                3.0 * dummy_complex_value_asym()(1), DoubleComplex{}}));
        }
    }
    SUBCASE("get_link_output") {
        SUBCASE("BranchSolverOutput") {
            ComplexValueVector<symmetric_t> const link_result{dummy_complex_value_sym(), dummy_complex_value_sym()};

            auto const link_output = detail::get_link_output<symmetric_t, BranchSolverOutput<symmetric_t>>(link_result);
            REQUIRE(link_output.size() == 2);
            CHECK(link_output[0].s_f == dummy_complex_value_sym());
            CHECK(link_output[0].s_t == -dummy_complex_value_sym());
            CHECK(link_output[1].s_f == dummy_complex_value_sym());
            CHECK(link_output[1].s_t == -dummy_complex_value_sym());

            // asym
            ComplexValueVector<asymmetric_t> const link_result_asym{dummy_complex_value_asym()};
            auto const link_output_asym =
                detail::get_link_output<asymmetric_t, BranchSolverOutput<asymmetric_t>>(link_result_asym);
            REQUIRE(link_output_asym.size() == 1);
            CHECK(link_output_asym[0].s_f.isApprox(dummy_complex_value_asym()));
            CHECK(link_output_asym[0].s_t.isApprox(-dummy_complex_value_asym()));
        }
        SUBCASE("BranchShortCircuitSolverOutput") {
            ComplexValueVector<symmetric_t> const link_result{dummy_complex_value_sym()};

            auto const link_output =
                detail::get_link_output<symmetric_t, BranchShortCircuitSolverOutput<symmetric_t>>(link_result);
            REQUIRE(link_output.size() == 1);
            CHECK(link_output[0].i_f == dummy_complex_value_sym());
            CHECK(link_output[0].i_t == -dummy_complex_value_sym());

            // asym
            ComplexValueVector<asymmetric_t> const link_result_asym{dummy_complex_value_asym()};
            auto const link_output_asym =
                detail::get_link_output<asymmetric_t, BranchShortCircuitSolverOutput<asymmetric_t>>(link_result_asym);
            REQUIRE(link_output_asym.size() == 1);
            CHECK(link_output_asym[0].i_f.isApprox(dummy_complex_value_asym()));
            CHECK(link_output_asym[0].i_t.isApprox(-dummy_complex_value_asym()));
        }
    }
    SUBCASE("solve_topological_nodes") {
        auto const state = make_state();
        auto const& links = state.reduced_topology->topo_node_coup.topo_nodes[0].user_links;

        SUBCASE("Steady state output") {
            auto const math_output = make_steady_state_math_output_sym();
            LinkSolverMock mock{.return_values = {
                                    {2.0 * dummy_complex_value_sym(), 3.0 * dummy_complex_value_sym()},
                                    {},
                                }};

            auto const result = detail::solve_topological_nodes(std::ref(mock), state, math_output);

            REQUIRE(mock.call_count == 2);
            CHECK(mock.recorded_edges[0] == links);
            CHECK(mock.recorded_edges[1].empty());
            CHECK(mock.recorded_loads[0] ==
                  ComplexVector{DoubleComplex{}, -dummy_complex_value_sym(), dummy_complex_value_sym()});
            CHECK(mock.recorded_loads[1] == ComplexVector{DoubleComplex{}});

            REQUIRE(result.size() == 2);
            CHECK(result[0].bus_injection ==
                  ComplexVector{dummy_complex_value_sym(), dummy_complex_value_sym(), dummy_complex_value_sym()});
            CHECK(result[1].bus_injection == ComplexVector{DoubleComplex{}});
            REQUIRE(result[0].link.size() == 2);
            check_close(result[0].link[0].s_f, 2.0 * dummy_complex_value_sym());
            check_close(result[0].link[0].s_t, -2.0 * dummy_complex_value_sym());
            check_close(result[0].link[1].s_f, 3.0 * dummy_complex_value_sym());
            check_close(result[0].link[1].s_t, -3.0 * dummy_complex_value_sym());
            // i_f/i_t are derived from the power flow and the topological node voltage: i = conj(s / u)
            auto const topo_node_u = math_output.solver_output[0].u[0];
            check_close(result[0].link[0].i_f, conj(2.0 * dummy_complex_value_sym() / topo_node_u));
            check_close(result[0].link[0].i_t, conj(-2.0 * dummy_complex_value_sym() / topo_node_u));
            check_close(result[0].link[1].i_f, conj(3.0 * dummy_complex_value_sym() / topo_node_u));
            check_close(result[0].link[1].i_t, conj(-3.0 * dummy_complex_value_sym() / topo_node_u));
            CHECK(result[1].link.empty());
        }

        SUBCASE("Short circuit output") {
            auto const math_output = make_short_circuit_math_output_sym();
            LinkSolverMock mock{.return_values = {
                                    {dummy_complex_value_sym(), dummy_complex_value_sym()},
                                    {},
                                }};

            auto const result = detail::solve_topological_nodes(std::ref(mock), state, math_output);

            REQUIRE(mock.call_count == 2);
            CHECK(mock.recorded_edges[0] == links);
            CHECK(mock.recorded_edges[1].empty());
            CHECK(mock.recorded_loads[0] ==
                  ComplexVector{dummy_complex_value_sym(), -dummy_complex_value_sym(), DoubleComplex{}});
            CHECK(mock.recorded_loads[1] == ComplexVector{DoubleComplex{}});

            REQUIRE(result.size() == 2);
            REQUIRE(result[0].link.size() == 2);
            check_close(result[0].link[0].i_f, dummy_complex_value_sym());
            check_close(result[0].link[0].i_t, -dummy_complex_value_sym());
            check_close(result[0].link[1].i_f, dummy_complex_value_sym());
            check_close(result[0].link[1].i_t, -dummy_complex_value_sym());
            CHECK(result[1].link.empty());
        }
    }
}
} // namespace power_grid_model::main_core
