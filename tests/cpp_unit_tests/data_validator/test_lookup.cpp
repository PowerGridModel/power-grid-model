// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "power_grid_model/validation/context.hpp"
#include "validation_test_utils.hpp"

#include <power_grid_model/auxiliary/input.hpp>
#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/component/node.hpp>

#include <doctest/doctest.h>

#include <array>
#include <cstddef>
#include <string_view>

TEST_CASE("Input ID lookup deduplicates IDs and excludes the missing sentinel") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;

    std::array<ID, 4> const ids{41, 41, na_IntID, 42};
    InputValidationLookup lookup;
    lookup.add_component_ids(Node::name, ids);

    auto const& node_ids = lookup.component_ids(Node::name);
    CHECK(node_ids.size() == 2);
    CHECK(node_ids.contains(41));
    CHECK(node_ids.contains(42));
    CHECK_FALSE(lookup.contains_component_id(Node::name, na_IntID));
    CHECK_FALSE(lookup.contains_component_id(Node::name, 99));
}

TEST_CASE("Input ID lookup distinguishes absent IDs from unprepared components") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;

    std::array<ID, 0> const no_ids{};
    InputValidationLookup lookup;
    lookup.add_component_ids(Node::name, no_ids);

    CHECK_FALSE(lookup.contains_component_id(Node::name, 41));
    CHECK_THROWS_AS(static_cast<void>(lookup.component_ids("line")), InvalidArguments);
}

TEST_CASE("Node ID index builder reads row, columnar, and absent component data") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using test::make_empty_dataset;

    SUBCASE("row data") {
        std::array<NodeInput, 2> const nodes{{{.id = 41, .u_rated = 1.0}, {.id = 42, .u_rated = 2.0}}};
        auto dataset = make_empty_dataset(false, "input");
        dataset.add_buffer("node", nodes.size(), nodes.size(), nullptr, nodes.data());
        InputValidationLookup lookup;

        power_grid_model::validation::detail::index_input_component_ids<Node>(dataset, lookup);

        CHECK(lookup.contains_component_id(Node::name, 41));
        CHECK(lookup.contains_component_id(Node::name, 42));
    }

    SUBCASE("columnar data") {
        std::array<ID, 2> const ids{41, 42};
        auto dataset = make_empty_dataset(false, "input");
        dataset.add_buffer("node", ids.size(), ids.size(), nullptr, nullptr);
        dataset.add_attribute_buffer("node", "id", ids.data());
        InputValidationLookup lookup;

        power_grid_model::validation::detail::index_input_component_ids<Node>(dataset, lookup);

        CHECK(lookup.contains_component_id(Node::name, 41));
        CHECK(lookup.contains_component_id(Node::name, 42));
    }

    SUBCASE("absent component") {
        auto const dataset = make_empty_dataset(false, "input");
        InputValidationLookup lookup;

        power_grid_model::validation::detail::index_input_component_ids<Node>(dataset, lookup);

        CHECK(lookup.component_ids(Node::name).empty());
    }
}

TEST_CASE("Input ID preparation invokes every registered index builder once") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using test::make_empty_dataset;

    std::array<ID, 1> const node_ids{41};
    std::array<ID, 1> const line_ids{42};
    std::size_t node_builds{};
    std::size_t line_builds{};
    ComponentRules const node_rules{
        .component = Node::name,
        .input_rules = {},
        .batch_rules = {},
        .build_input_id_lookup = [&node_builds, &node_ids](ConstDataset const&, InputValidationLookup& lookup) {
            ++node_builds;
            lookup.add_component_ids(Node::name, node_ids);
        }};
    ComponentRules const line_rules{
        .component = "line",
        .input_rules = {},
        .batch_rules = {},
        .build_input_id_lookup = [&line_builds, &line_ids](ConstDataset const&, InputValidationLookup& lookup) {
            ++line_builds;
            lookup.add_component_ids("line", line_ids);
        }};
    auto const dataset = make_empty_dataset(false, "input");
    ValidationRegistry const registry{node_rules, line_rules};

    auto const lookup = power_grid_model::validation::detail::prepare_input_id_lookup(dataset, registry);

    CHECK(node_builds == 1);
    CHECK(line_builds == 1);
    CHECK(lookup.contains_component_id(Node::name, 41));
    CHECK(lookup.contains_component_id("line", 42));
}

TEST_CASE("Input ID preparation rejects registered components without an index builder") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using test::make_empty_dataset;

    auto const dataset = make_empty_dataset(false, "input");
    ValidationRegistry const registry{
        {.component = Node::name, .input_rules = {}, .batch_rules = {}, .build_input_id_lookup = {}}};

    CHECK_THROWS_AS(power_grid_model::validation::detail::prepare_input_id_lookup(dataset, registry), InvalidArguments);
}
