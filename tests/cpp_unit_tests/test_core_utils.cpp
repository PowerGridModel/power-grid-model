// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/container.hpp>
#include <power_grid_model/main_core/core_utils.hpp>

#include <doctest/doctest.h>

namespace power_grid_model::main_core::utils {

namespace {
// TODO test with a non used type
struct AComponent {};
} // namespace

TEST_CASE("MainModelType") {

    SUBCASE("Node Source") {
        using ModelType = MainModelType<ExtraRetrievableTypes<Base, Node, Appliance>, ComponentList<Node, Source>>;
        CHECK(true);
        static_assert(std::is_same_v<typename ModelType::ComponentContainer,
                                     Container<ExtraRetrievableTypes<Base, Node, Appliance>, Node, Source>>);
        static_assert(std::is_same_v<typename ModelType::ComponentTypesTuple, std::tuple<Node, Source>>);
        static_assert(std::is_same_v<typename ModelType::TopologyTypesTuple, std::tuple<Node, Source>>);
        static_assert(std::is_same_v<typename ModelType::TopologyConnectionTypesTuple, std::tuple<Source>>);
        static_assert(ModelType::n_component_types == 2);
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
        static_assert(ModelType::n_component_types == 3);
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
        static_assert(ModelType::n_component_types == 3);
    }
    SUBCASE("Bad case: Line Source") {
        // TODO rewrite for checking fail instead of pass
        using ModelType = MainModelType<ExtraRetrievableTypes<Base, Branch, Appliance>, ComponentList<Line, Source>>;

        static_assert(std::is_same_v<typename ModelType::ComponentContainer,
                                     Container<ExtraRetrievableTypes<Base, Branch, Appliance>, Line, Source>>);
        static_assert(std::is_same_v<typename ModelType::ComponentTypesTuple, std::tuple<Line, Source>>);
        static_assert(std::is_same_v<typename ModelType::TopologyTypesTuple, std::tuple<Branch, Source>>);
        static_assert(std::is_same_v<typename ModelType::TopologyConnectionTypesTuple, std::tuple<Branch, Source>>);
        static_assert(ModelType::n_component_types == 2);
    }

    // TODO add static_assert(std::constructible_from<ModelType, double, meta_data::MetaData const&,
    // MathSolverDispatcher const&>);
}

} // namespace power_grid_model::main_core::utils
