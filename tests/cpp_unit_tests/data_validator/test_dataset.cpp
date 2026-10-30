// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "validation_test_utils.hpp"

#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/validation/dataset.hpp>

#include <doctest/doctest.h>

#include <array>

TEST_CASE("Input dataset validation accepts only a single input dataset") {
    using namespace power_grid_model::validation;
    using power_grid_model::validation::test::make_empty_dataset;

    auto const input = make_empty_dataset(false, "input");
    CHECK_NOTHROW(validate_dataset_type(input));

    auto const update = make_empty_dataset(false, "update");
    CHECK_THROWS_AS(validate_dataset_type(update), power_grid_model::InvalidArguments);

    auto const batch_input = make_empty_dataset(true, "input");
    CHECK_THROWS_AS(validate_dataset_type(batch_input), power_grid_model::InvalidArguments);
}

TEST_CASE("Component validation requires IDs for columnar data") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using test::make_empty_dataset;
    using test::TestComponent;
    using test::TestInput;

    std::array<TestInput, 1> const rows{{{.id = 7, .value = 1.0}}};
    auto const row_based = [&] {
        auto dataset = make_empty_dataset(false, "input");
        dataset.add_buffer(TestComponent::name, rows.size(), rows.size(), nullptr, rows.data());
        return dataset;
    }();
    CHECK(should_validate_component(row_based, TestComponent::name));

    std::array<double, 1> const values{1.0};
    auto const columnar_without_ids = [&] {
        auto dataset = make_empty_dataset(false, "input");
        dataset.add_buffer(TestComponent::name, values.size(), values.size(), nullptr, nullptr);
        dataset.add_attribute_buffer(TestComponent::name, TestInput::value_name, values.data());
        return dataset;
    }();
    CHECK_FALSE(should_validate_component(columnar_without_ids, TestComponent::name));

    std::array<ID, 1> const ids{7};
    auto const columnar_with_ids = [&] {
        auto dataset = make_empty_dataset(false, "input");
        dataset.add_buffer(TestComponent::name, ids.size(), ids.size(), nullptr, nullptr);
        dataset.add_attribute_buffer(TestComponent::name, TestInput::id_name, ids.data());
        dataset.add_attribute_buffer(TestComponent::name, TestInput::value_name, values.data());
        return dataset;
    }();
    CHECK(should_validate_component(columnar_with_ids, TestComponent::name));
    CHECK_FALSE(should_validate_component(columnar_with_ids, "missing"));
}

TEST_CASE("Batch dataset validation requires input and batch update datasets") {
    using namespace power_grid_model::validation;
    using power_grid_model::validation::test::make_empty_dataset;

    auto const input = make_empty_dataset(false, "input");
    auto const batch_update = make_empty_dataset(true, "update");
    CHECK_NOTHROW(validate_batch_dataset_type(input, batch_update));

    auto const wrong_update_type = make_empty_dataset(true, "sym_output");
    CHECK_THROWS_AS(validate_batch_dataset_type(input, wrong_update_type), power_grid_model::InvalidArguments);

    auto const single_update = make_empty_dataset(false, "update");
    CHECK_THROWS_AS(validate_batch_dataset_type(input, single_update), power_grid_model::InvalidArguments);

    auto chained_update = make_empty_dataset(true, "update");
    auto const second_dimension = make_empty_dataset(true, "update");
    chained_update.set_next_cartesian_product_dimension(&second_dimension);
    CHECK_THROWS_AS(validate_batch_dataset_type(input, chained_update), power_grid_model::InvalidArguments);

    auto const batch_as_input = make_empty_dataset(true, "input");
    CHECK_THROWS_AS(validate_batch_dataset_type(batch_as_input, batch_update), power_grid_model::InvalidArguments);
}
