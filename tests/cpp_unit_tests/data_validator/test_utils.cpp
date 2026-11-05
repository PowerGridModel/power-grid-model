// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "validation_test_utils.hpp"

#include <power_grid_model/validation/utils.hpp>

#include <power_grid_model/common/common.hpp>

#include <doctest/doctest.h>

#include <array>
#include <format>

TEST_CASE("Test get_total_input_elements") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using test::make_empty_dataset;
    using test::TestComponent;
    using test::TestInput;
    using validation::detail::get_total_input_elements;

    SUBCASE("empty dataset") {
        auto const dataset = make_empty_dataset(false, "input");

        CHECK(get_total_input_elements(dataset) == 0);
    }

    SUBCASE("component without elements") {
        std::array<TestInput, 0> const empty_component{};
        auto dataset = make_empty_dataset(false, "input");
        dataset.add_buffer(TestComponent::name, 0, 0, nullptr, empty_component.data());

        CHECK(get_total_input_elements(dataset) == 0);
    }

    SUBCASE("complete dataset") {
        std::array<TestInput, 2> const comp_1{{{.id = 1}, {.id = 2}}};
        std::array<TestInput, 3> const comp_2{{{.id = 3}, {.id = 4}, {.id = 5}}};
        std::array<ID, 1> const comp_3_ids{6};
        auto dataset = make_empty_dataset(false, "input");

        dataset.add_buffer(TestComponent::name, comp_1.size(), comp_1.size(), nullptr, comp_1.data());
        dataset.add_buffer(std::format("{}_2", TestComponent::name), comp_2.size(), comp_2.size(), nullptr,
                           comp_2.data());
        dataset.add_buffer(std::format("{}_3", TestComponent::name), comp_3_ids.size(), comp_3_ids.size(), nullptr,
                           nullptr);
        dataset.add_attribute_buffer(std::format("{}_3", TestComponent::name), "id", comp_3_ids.data());

        CHECK(get_total_input_elements(dataset) == 6);
    }
}
