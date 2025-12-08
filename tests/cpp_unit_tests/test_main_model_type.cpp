// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/container.hpp>
#include <power_grid_model/main_core/core_utils.hpp>
#include <power_grid_model/main_core/main_model_type.hpp>
#include <power_grid_model/main_model_impl.hpp>
#include <power_grid_model/math_solver/math_solver_dispatch.hpp>

#include <doctest/doctest.h>

namespace power_grid_model::main_core {

namespace {

struct AType {};

class AComponent {
  public:
    // This dependency exists because of main_core/update.hpp
    using UpdateType = AType;
    static constexpr char const* name = "a_component";
};

} // namespace

static_assert(is_main_model_type_v<MainModelType<AllExtraRetrievableTypes, AllComponents>>);
static_assert(!is_main_model_type_v<MainModelType<AllComponents, AllComponents>>);
static_assert(!is_main_model_type_v<MainModelType<AType, AllComponents>>);

static_assert(std::constructible_from<MainModelImpl<MainModelType<AllExtraRetrievableTypes, AllComponents>>, double,
                                      meta_data::MetaData const&, MathSolverDispatcher const&>);

static_assert(detail::validate_component_types_c<AllComponents>);

static_assert(detail::validate_component_types_c<ComponentList<Node, Source>>);
static_assert(detail::validate_component_types_c<ComponentList<Node, Line>>);
static_assert(detail::validate_component_types_c<ComponentList<Node, Line, AComponent>>);
static_assert(detail::validate_component_types_c<ComponentList<Source, Node>>);

static_assert(!detail::validate_component_types_c<ComponentList<Line>>);
static_assert(!detail::validate_component_types_c<ComponentList<Source, Line>>);

TEST_CASE("MainModelType") {

    SUBCASE("Node Source") {
        using ModelType = MainModelType<ExtraRetrievableTypes<Base, Node, Appliance>, ComponentList<Node, Source>>;

        static_assert(std::is_same_v<typename ModelType::ComponentContainer,
                                     Container<ExtraRetrievableTypes<Base, Node, Appliance>, Node, Source>>);
        static_assert(std::is_same_v<typename ModelType::ComponentTypesTuple, std::tuple<Node, Source>>);
        static_assert(std::is_same_v<typename ModelType::TopologyTypesTuple, std::tuple<Node, Source>>);
        static_assert(std::is_same_v<typename ModelType::TopologyConnectionTypesTuple, std::tuple<Source>>);
        static_assert(ModelType::index_of_component<Node> == 0);
        static_assert(ModelType::index_of_component<Source> == 1);
        static_assert(ModelType::n_types == 2);
        static_assert(is_main_model_type_v<ModelType>);
        static_assert(std::constructible_from<MainModelImpl<ModelType>, double, meta_data::MetaData const&,
                                              MathSolverDispatcher const&>);

        CHECK(ModelType::run_functor_with_all_component_types_return_array([]<typename CompType>() {
                  return std::string_view(CompType::name);
              }) == std::array<std::string_view, 2>{"node", "source"});

        std::vector<std::string_view> calls;
        ModelType::run_functor_with_all_component_types_return_void(
            [&calls]<typename CompType>() { calls.push_back(std::string_view(CompType::name)); });
        CHECK(calls == std::vector<std::string_view>{"node", "source"});

        calls.clear();
        utils::run_functor_with_tuple_return_void<typename ModelType::TopologyTypesTuple>(
            [&calls]<typename CompType>() { calls.push_back(std::string_view(CompType::name)); });
        CHECK(calls == std::vector<std::string_view>{"node", "source"});
    }
    SUBCASE("Node Line Source") {
        using ModelType =
            MainModelType<ExtraRetrievableTypes<Base, Node, Branch, Appliance>, ComponentList<Node, Line, Source>>;

        static_assert(
            std::is_same_v<typename ModelType::ComponentContainer,
                           Container<ExtraRetrievableTypes<Base, Node, Branch, Appliance>, Node, Line, Source>>);
        static_assert(std::is_same_v<typename ModelType::ComponentTypesTuple, std::tuple<Node, Line, Source>>);
        static_assert(std::is_same_v<typename ModelType::TopologyTypesTuple, std::tuple<Node, Branch, Source>>);
        static_assert(std::is_same_v<typename ModelType::TopologyConnectionTypesTuple, std::tuple<Branch, Source>>);
        static_assert(ModelType::index_of_component<Node> == 0);
        static_assert(ModelType::index_of_component<Line> == 1);
        static_assert(ModelType::index_of_component<Source> == 2);
        static_assert(ModelType::n_types == 3);
        static_assert(is_main_model_type_v<ModelType>);
        static_assert(std::constructible_from<MainModelImpl<ModelType>, double, meta_data::MetaData const&,
                                              MathSolverDispatcher const&>);

        CHECK(ModelType::run_functor_with_all_component_types_return_array([]<typename CompType>() {
                  return std::string_view(CompType::name);
              }) == std::array<std::string_view, 3>{"node", "line", "source"});

        std::vector<std::string_view> calls;
        ModelType::run_functor_with_all_component_types_return_void(
            [&calls]<typename CompType>() { calls.push_back(std::string_view(CompType::name)); });
        CHECK(calls == std::vector<std::string_view>{"node", "line", "source"});
        calls.clear();

        utils::run_functor_with_tuple_return_void<typename ModelType::TopologyTypesTuple>(
            [&calls]<typename CompType>() { calls.push_back(std::string_view(CompType::name)); });
        CHECK(calls == std::vector<std::string_view>{"node", "branch", "source"});
    }
    SUBCASE("Different component order: Line Source Node") {
        using ModelType =
            MainModelType<ExtraRetrievableTypes<Base, Node, Branch, Appliance>, ComponentList<Line, Source, Node>>;

        static_assert(
            std::is_same_v<typename ModelType::ComponentContainer,
                           Container<ExtraRetrievableTypes<Base, Node, Branch, Appliance>, Line, Source, Node>>);
        static_assert(std::is_same_v<typename ModelType::ComponentTypesTuple, std::tuple<Line, Source, Node>>);
        static_assert(std::is_same_v<typename ModelType::TopologyTypesTuple, std::tuple<Node, Branch, Source>>);
        static_assert(std::is_same_v<typename ModelType::TopologyConnectionTypesTuple, std::tuple<Branch, Source>>);
        static_assert(ModelType::index_of_component<Line> == 0);
        static_assert(ModelType::index_of_component<Source> == 1);
        static_assert(ModelType::index_of_component<Node> == 2);
        static_assert(ModelType::n_types == 3);
        static_assert(is_main_model_type_v<ModelType>);

        static_assert(std::constructible_from<MainModelImpl<ModelType>, double, meta_data::MetaData const&,
                                              MathSolverDispatcher const&>);

        CHECK(ModelType::run_functor_with_all_component_types_return_array([]<typename CompType>() {
                  return std::string_view(CompType::name);
              }) == std::array<std::string_view, 3>{"line", "source", "node"});

        std::vector<std::string_view> calls;
        ModelType::run_functor_with_all_component_types_return_void(
            [&calls]<typename CompType>() { calls.push_back(std::string_view(CompType::name)); });
        CHECK(calls == std::vector<std::string_view>{"line", "source", "node"});
        calls.clear();

        utils::run_functor_with_tuple_return_void<typename ModelType::TopologyTypesTuple>(
            [&calls]<typename CompType>() { calls.push_back(std::string_view(CompType::name)); });
        CHECK(calls == std::vector<std::string_view>{"node", "branch", "source"});
    }

    SUBCASE("Node AComponent Source") {
        using ModelType =
            MainModelType<ExtraRetrievableTypes<Base, Node, Appliance>, ComponentList<Node, AComponent, Source>>;

        static_assert(
            std::is_same_v<typename ModelType::ComponentContainer,
                           Container<ExtraRetrievableTypes<Base, Node, Appliance>, Node, AComponent, Source>>);
        static_assert(std::is_same_v<typename ModelType::ComponentTypesTuple, std::tuple<Node, AComponent, Source>>);
        static_assert(std::is_same_v<typename ModelType::TopologyTypesTuple, std::tuple<Node, Source>>);
        static_assert(std::is_same_v<typename ModelType::TopologyConnectionTypesTuple, std::tuple<Source>>);
        static_assert(ModelType::index_of_component<Node> == 0);
        static_assert(ModelType::index_of_component<AComponent> == 1);
        static_assert(ModelType::index_of_component<Source> == 2);
        static_assert(ModelType::n_types == 3);
        static_assert(is_main_model_type_v<ModelType>);
        static_assert(std::constructible_from<MainModelImpl<ModelType>, double, meta_data::MetaData const&,
                                              MathSolverDispatcher const&>);

        CHECK(ModelType::run_functor_with_all_component_types_return_array([]<typename CompType>() {
                  return std::string_view(CompType::name);
              }) == std::array<std::string_view, 3>{"node", "a_component", "source"});

        std::vector<std::string_view> calls;
        ModelType::run_functor_with_all_component_types_return_void(
            [&calls]<typename CompType>() { calls.push_back(std::string_view(CompType::name)); });
        CHECK(calls == std::vector<std::string_view>{"node", "a_component", "source"});

        calls.clear();
        utils::run_functor_with_tuple_return_void<typename ModelType::TopologyTypesTuple>(
            [&calls]<typename CompType>() { calls.push_back(std::string_view(CompType::name)); });
        CHECK(calls == std::vector<std::string_view>{"node", "source"});
    }
}

} // namespace power_grid_model::main_core
