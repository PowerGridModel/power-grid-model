// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/container.hpp>
#include <power_grid_model/optimizer/optimizer.hpp>

#include <doctest/doctest.h>

namespace power_grid_model::optimizer::test {
struct StubComponent {};

struct StubTransformerInput {};
struct StubTransformerUpdate {
    ID id{};
    IntS tap_pos{};
};
enum class StubTransformerSideType : IntS {};
struct StubTransformerMathIdType {};

struct StubTransformer {
    using InputType = StubTransformerInput;
    using UpdateType = StubTransformerUpdate;
    using SideType = StubTransformerSideType;

    static constexpr auto name = "StubTransformer";
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
};
struct StubTransformerA : public StubTransformer {};
struct StubTransformerB : public StubTransformer {};
static_assert(transformer_c<StubTransformer>);
static_assert(transformer_c<StubTransformerA>);
static_assert(transformer_c<StubTransformerB>);

template <std::derived_from<StubTransformer> ComponentType, typename State>
    requires main_core::component_container_c<typename State::ComponentContainer, ComponentType>
constexpr auto get_topology_index(State const& /* state */, auto const& /* id_or_index */) {
    return Idx{};
}

template <std::derived_from<StubTransformer> ComponentType, typename State>
    requires main_core::component_container_c<typename State::ComponentContainer, ComponentType>
constexpr auto get_math_id(State const& /* state */, Idx /* topology_sequence_idx */) {
    return StubTransformerMathIdType{};
}

template <std::derived_from<StubTransformer> ComponentType, typename State>
    requires main_core::component_container_c<typename State::ComponentContainer, ComponentType>
constexpr auto get_topo_node(State const& /* state */, Idx /* topology_index */, ControlSide /* side */) {
    return Idx{};
}

template <std::derived_from<StubTransformer> ComponentType, steady_state_math_output_type MathOutputType>
inline auto i_pu(std::vector<MathOutputType> const& /* math_output */, StubTransformerMathIdType const& /* math_id */,
                 ControlSide /* side */) {
    return ComplexValue<typename MathOutputType::sym>{};
}

using StubComponentContainer = Container<StubComponent, StubTransformerA, StubTransformerB>;

using StubState = main_core::MainModelState<StubComponentContainer>;
static_assert(main_core::main_model_state_c<StubState>);

struct StubStateCalculatorResultType {
    Idx x{};
};

struct StubUpdateType {};

using StubStateCalculator = StubStateCalculatorResultType (*)(StubState const& /* state */,
                                                              CalculationMethod /* method */);
using SymStubSteadyStateCalculator = std::vector<MathOutput<symmetric_t>> (*)(StubState const& /* state */,
                                                                              CalculationMethod /* method */);
using AsymStubSteadyStateCalculator = std::vector<MathOutput<asymmetric_t>> (*)(StubState const& /* state */,
                                                                                CalculationMethod /* method */);
using StubUpdate = void (*)(StubUpdateType const& /* update_data */);
using ConstDatasetUpdate = void (*)(ConstDataset const& /* update_data */);

static_assert(std::invocable<StubStateCalculator, StubState const&, CalculationMethod>);
static_assert(std::same_as<std::invoke_result_t<StubStateCalculator, StubState const&, CalculationMethod>,
                           StubStateCalculatorResultType>);
static_assert(std::invocable<SymStubSteadyStateCalculator, StubState const&, CalculationMethod>);
static_assert(std::same_as<std::invoke_result_t<SymStubSteadyStateCalculator, StubState const&, CalculationMethod>,
                           std::vector<MathOutput<symmetric_t>>>);
static_assert(std::invocable<SymStubSteadyStateCalculator, StubState const&, CalculationMethod>);
static_assert(std::same_as<std::invoke_result_t<AsymStubSteadyStateCalculator, StubState const&, CalculationMethod>,
                           std::vector<MathOutput<asymmetric_t>>>);
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
    return std::vector<MathOutput<sym>>{};
}
static_assert(
    std::convertible_to<decltype(stub_steady_state_state_calculator<symmetric_t>), SymStubSteadyStateCalculator>);
static_assert(
    std::convertible_to<decltype(stub_steady_state_state_calculator<asymmetric_t>), AsymStubSteadyStateCalculator>);

constexpr void stub_update(StubUpdateType const& /* update_data */){
    // stub
};
static_assert(std::convertible_to<decltype(stub_update), StubUpdate>);

constexpr void stub_const_dataset_update(ConstDataset const& /* update_data */){
    // stub
};
static_assert(std::convertible_to<decltype(stub_const_dataset_update), ConstDatasetUpdate>);

constexpr auto strategies = [] {
    using enum OptimizerStrategy;
    return std::array{any, global_minimum, global_maximum, local_minimum, local_maximum};
}();

constexpr auto calculation_methods = [] {
    using enum CalculationMethod;
    return std::array{default_method,    linear,         linear_current, iterative_linear,
                      iterative_current, newton_raphson, iec60909};
}();

constexpr auto strategies_and_methods = [] {
    std::array<std::pair<OptimizerStrategy, CalculationMethod>, strategies.size() * calculation_methods.size()> result;
    size_t idx{};
    for (auto strategy : strategies) {
        for (auto method : calculation_methods) {
            result[idx++] = std::make_pair(strategy, method);
        }
    }
    return result;
}();
} // namespace power_grid_model::optimizer::test

namespace power_grid_model::meta_data {
template <> struct get_component_nan<optimizer::test::StubTransformerUpdate> {
    constexpr optimizer::test::StubTransformerUpdate operator()() const { return {.id = na_IntID, .tap_pos = na_IntS}; }
};
} // namespace power_grid_model::meta_data
