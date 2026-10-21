// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "validation_test_utils.hpp"

#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/validation/registry.hpp>

#include <doctest/doctest.h>

#include <set>
#include <tuple>
#include <utility>

TEST_CASE("Default validation registry references valid, correctly typed fields") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using meta_data::meta_data_gen::meta_data;

    auto const& input_meta = meta_data.get_dataset("input");
    auto const& registry = default_registry();
    std::set<std::string_view> registered_components;
    std::set<std::tuple<std::string_view, bool, RuleCode, std::string_view>> registered_fields;

    for (auto const& component_rules : registry) {
        registered_components.insert(component_rules.component);
        CHECK(static_cast<bool>(component_rules.build_input_id_lookup));
        auto const& component = input_meta.get_component(component_rules.component);
        auto check_rules = [&component, &component_rules, &registered_fields](auto const& rules, bool is_batch) {
            for (auto const& rule : rules) {
                CHECK(static_cast<bool>(rule.apply));
                REQUIRE_FALSE(rule.fields.empty());
                for (auto const& field : rule.fields) {
                    CHECK(component.has_attribute(field.name));
                    CHECK(component.get_attribute(field.name).ctype == field.type);
                    CHECK(registered_fields.emplace(component_rules.component, is_batch, rule.code, field.name).second);
                }
            }
        };
        check_rules(component_rules.input_rules, false);
        check_rules(component_rules.batch_rules, true);
    }

    CHECK(registered_components == (std::set<std::string_view>{"line", "node"}));
}

TEST_CASE("Default validation rule adapters accept empty component data") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using test::make_empty_dataset;

    auto const input = make_empty_dataset(false, "input");
    auto const update = make_empty_dataset(true, "update");
    auto input_id_lookup = power_grid_model::validation::detail::prepare_input_id_lookup(input, default_registry());
    ValidationContext const context{.input = input,
                                    .update = &update,
                                    .symmetric = false,
                                    .calculation_type = CalculationType::power_flow,
                                    .input_id_lookup = std::move(input_id_lookup)};
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
