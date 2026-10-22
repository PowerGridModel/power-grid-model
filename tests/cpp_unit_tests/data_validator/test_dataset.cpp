// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "validation_test_utils.hpp"

#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/validation/dataset.hpp>

#include <doctest/doctest.h>

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

    auto const batch_as_input = make_empty_dataset(true, "input");
    CHECK_THROWS_AS(validate_batch_dataset_type(batch_as_input, batch_update), power_grid_model::InvalidArguments);
}
