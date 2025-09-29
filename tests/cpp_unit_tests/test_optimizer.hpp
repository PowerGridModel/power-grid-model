// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <power_grid_model/auxiliary/meta_data.hpp>
#include <power_grid_model/auxiliary/meta_gen/gen_getters.hpp>
#include <power_grid_model/container.hpp>
#include <power_grid_model/optimizer/optimizer.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {
namespace optimizer::test {
struct StubComponent {};

struct StubTransformerInput {};
struct StubTransformerUpdate {
    ID id{na_IntID};
    IntS tap_pos{na_IntS};
};
enum class StubTransformerSideType : IntS {};
struct StubTransformerMathIdType {};

struct StubTransformer {
    using InputType = StubTransformerInput;
    using UpdateType = StubTransformerUpdate;
    using SideType = StubTransformerSideType;

    static constexpr auto name = "StubTransformer";

    // NOLINTBEGIN(readability-convert-member-functions-to-static) // because it stubs non-static member functions
    constexpr auto math_model_type() const { return ComponentType::test; }

    constexpr auto id() const { return ID{}; }
    constexpr auto node(SideType /* side */) const { return ID{}; }
    constexpr auto status(SideType /* side */) const { return bool{}; }

    constexpr auto tap_side() const { return SideType{}; }
    constexpr auto tap_pos() const { return IntS{}; }
    constexpr auto tap_min() const { return IntS{}; }
    constexpr auto tap_max() const { return IntS{}; }
    constexpr auto tap_nom() const { return IntS{}; }

    constexpr auto update(UpdateType const& /* update */) const { return UpdateChange{}; }
    constexpr auto inverse(UpdateType /* update */) const { return UpdateType{}; }
    // NOLINTEND(readability-convert-member-functions-to-static)
};
struct StubTransformerA : public StubTransformer {};
struct StubTransformerB : public StubTransformer {};
static_assert(transformer_c<StubTransformer>);
static_assert(transformer_c<StubTransformerA>);
static_assert(transformer_c<StubTransformerB>);

template <std::derived_from<StubTransformer> ComponentType, typename ComponentContainer>
    requires common::component_container_c<ComponentContainer, ComponentType>
constexpr auto get_topology_index(ComponentContainer const& /* Components */, auto const& /* id_or_index */) {
    return Idx{};
}

template <std::derived_from<StubTransformer> ComponentType, typename State>
    requires common::component_container_c<typename State::ComponentContainer, ComponentType>
constexpr auto get_math_id(State const& /* state */, Idx /* topology_sequence_idx */) {
    return StubTransformerMathIdType{};
}

template <std::derived_from<StubTransformer> ComponentType, steady_state_solver_output_type SolverOutputType>
inline auto i_pu(std::vector<SolverOutputType> const& /* solver_output */,
                 StubTransformerMathIdType const& /* math_id */, ControlSide /* side */) {
    return ComplexValue<typename SolverOutputType::sym>{};
}

template <std::derived_from<StubTransformer> ComponentType, typename State,
          steady_state_solver_output_type SolverOutputType>
    requires common::component_container_c<typename State::ComponentContainer, ComponentType>
inline auto u_pu(State const& /* state */, std::vector<SolverOutputType> const& /* solver_output */,
                 Idx /* topology_index */, ControlSide /* control_side */) {
    return ComplexValue<typename SolverOutputType::sym>{};
}

// TODO(mgovers) revert
// using StubComponentContainer = Container<ExtraRetrievableTypes<Regulator>, StubComponent, StubTransformerA,
//                                          TransformerTapRegulator, StubTransformerB>;
using StubComponentContainer =
    Container<ExtraRetrievableTypes<Base, Node, Branch, Branch3, Appliance, Regulator>, Line, Link, Node, Transformer,
              ThreeWindingTransformer, TransformerTapRegulator, Source>;

using StubState = main_core::MainModelState<StubComponentContainer>;
static_assert(main_core::main_model_state_c<StubState>);

struct StubStateCalculatorResultType {
    Idx x{};
};

struct StubUpdateType {};

using StubStateCalculator = StubStateCalculatorResultType (*)(StubState const& /* state */,
                                                              CalculationMethod /* method */);
using SymStubSteadyStateCalculator = std::vector<SolverOutput<symmetric_t>> (*)(StubState const& /* state */,
                                                                                CalculationMethod /* method */);
using AsymStubSteadyStateCalculator = std::vector<SolverOutput<asymmetric_t>> (*)(StubState const& /* state */,
                                                                                  CalculationMethod /* method */);
using StubUpdate = void (*)(StubUpdateType const& /* update_data */);
using ConstDatasetUpdate = void (*)(ConstDataset const& /* update_data */);

static_assert(std::invocable<StubStateCalculator, StubState const&, CalculationMethod>);
static_assert(std::same_as<std::invoke_result_t<StubStateCalculator, StubState const&, CalculationMethod>,
                           StubStateCalculatorResultType>);
static_assert(std::invocable<SymStubSteadyStateCalculator, StubState const&, CalculationMethod>);
static_assert(std::same_as<std::invoke_result_t<SymStubSteadyStateCalculator, StubState const&, CalculationMethod>,
                           std::vector<SolverOutput<symmetric_t>>>);
static_assert(std::invocable<SymStubSteadyStateCalculator, StubState const&, CalculationMethod>);
static_assert(std::same_as<std::invoke_result_t<AsymStubSteadyStateCalculator, StubState const&, CalculationMethod>,
                           std::vector<SolverOutput<asymmetric_t>>>);
static_assert(std::invocable<StubUpdate, StubUpdateType const&>);
static_assert(std::invocable<ConstDatasetUpdate, ConstDataset const&>);

static_assert(optimizer_c<NoOptimizer<StubStateCalculator, StubState>>);
static_assert(optimizer_c<TapPositionOptimizer<SymStubSteadyStateCalculator, ConstDatasetUpdate, StubState>>);
static_assert(optimizer_c<TapPositionOptimizer<AsymStubSteadyStateCalculator, ConstDatasetUpdate, StubState>>);

constexpr auto mock_state_calculator(StubState const& /* state */, CalculationMethod /* method */) {
    return StubStateCalculatorResultType{.x = 1};
}
static_assert(std::convertible_to<decltype(mock_state_calculator), StubStateCalculator>);

template <symmetry_tag sym>
constexpr auto stub_steady_state_state_calculator(StubState const& /* state */, CalculationMethod /* method */) {
    return std::vector<SolverOutput<sym>>{};
}
static_assert(
    std::convertible_to<decltype(stub_steady_state_state_calculator<symmetric_t>), SymStubSteadyStateCalculator>);
static_assert(
    std::convertible_to<decltype(stub_steady_state_state_calculator<asymmetric_t>), AsymStubSteadyStateCalculator>);

constexpr void stub_update(StubUpdateType const& /* update_data */) {
    // stub
};
static_assert(std::convertible_to<decltype(stub_update), StubUpdate>);

constexpr void stub_const_dataset_update(ConstDataset const& /* update_data */) {
    // stub
};
static_assert(std::convertible_to<decltype(stub_const_dataset_update), ConstDatasetUpdate>);

constexpr auto strategies = [] {
    using enum OptimizerStrategy;
    return std::array{any, local_maximum, local_minimum, global_maximum, global_minimum};
}();

constexpr auto calculation_methods = [] {
    using enum CalculationMethod;
    return std::array{default_method,    linear,         linear_current, iterative_linear,
                      iterative_current, newton_raphson, iec60909};
}();

constexpr auto tap_sides = [] { return std::array{ControlSide::side_1, ControlSide::side_2, ControlSide::side_3}; }();

struct OptimizerStrategyMethod {
    OptimizerStrategy strategy{};
    CalculationMethod method{};
};

constexpr auto strategies_and_methods = [] {
    std::array<OptimizerStrategyMethod, strategies.size() * calculation_methods.size()> result;
    size_t idx{};
    for (auto strategy : strategies) {
        for (auto method : calculation_methods) {
            result[idx] = {.strategy = strategy, .method = method};
            ++idx;
        }
    }
    return result;
}();

struct OptimizerStrategySide {
    OptimizerStrategy strategy{};
    ControlSide side{};
};

constexpr auto strategies_and_sides = [] {
    std::array<OptimizerStrategySide, strategies.size() * tap_sides.size()> result;
    size_t idx{};
    for (auto strategy : strategies) {
        for (auto side : tap_sides) {
            result[idx] = {.strategy = strategy, .side = side};
            ++idx;
        }
    }
    return result;
}();

struct OptimizerStrategySearchSide {
    OptimizerStrategy strategy{};
    SearchMethod search{};
    ControlSide side{};
};

constexpr auto search_methods = [] { return std::array{SearchMethod::linear_search, SearchMethod::binary_search}; }();

constexpr auto strategy_search_and_sides = [] {
    // regular any strategy is only used in combination with linear_search search
    size_t const options_size = strategies.size() * tap_sides.size() * search_methods.size() - search_methods.size();
    std::array<OptimizerStrategySearchSide, options_size> result;
    size_t idx{};
    for (auto strategy : strategies) {
        for (auto search : search_methods) {
            if (strategy == OptimizerStrategy::any && search == SearchMethod::binary_search) {
                continue;
            }
            for (auto side : tap_sides) {
                result[idx] = {.strategy = strategy, .search = search, .side = side};
                ++idx;
            }
        }
    }
    return result;
}();

struct OptStrategyMethodSearch {
    OptimizerStrategy strategy{};
    CalculationMethod method{};
    SearchMethod search{};
};

constexpr auto strategy_method_and_searches = [] {
    // regular any strategy is only used in combination with linear_search search
    size_t const options_size =
        strategies.size() * calculation_methods.size() * search_methods.size() - search_methods.size();
    std::array<OptStrategyMethodSearch, options_size> result;
    size_t idx{};
    for (auto strategy : strategies) {
        for (auto search : search_methods) {
            if (strategy == OptimizerStrategy::any && search == SearchMethod::binary_search) {
                continue;
            }
            for (auto method : calculation_methods) {
                result[idx] = {.strategy = strategy, .method = method, .search = search};
                ++idx;
            }
        }
    }
    return result;
}();

} // namespace optimizer::test

namespace meta_data {
template <> struct get_attributes_list<optimizer::test::StubTransformerUpdate> {
    static constexpr std::array<MetaAttribute, 0> value{};
};
} // namespace meta_data
} // namespace power_grid_model
