// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "power_grid_model/validation/context.hpp"
#include "power_grid_model/validation/issues.hpp"
#include "validation_test_utils.hpp"

#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/validation/registry.hpp>

#include <doctest/doctest.h>

TEST_CASE("Default validation rule adapters accept empty component data") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using test::make_empty_dataset;

    auto const input = make_empty_dataset(false, "input");
    ValidationContext const context{
        .input = input, .symmetric = false, .calculation_type = CalculationType::power_flow};
    Issues issues;

    for (auto const& component_rules : default_registry()) {
        for (auto const& rule : component_rules.input_rules) {
            rule.apply(context, issues);
        }
        for (auto const& rule : component_rules.batch_rules) {
            rule.apply(context, issues);
        }
    }

    CHECK(issues.empty());
}
