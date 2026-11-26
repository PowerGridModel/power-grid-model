// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/validation/registration.hpp>

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/auxiliary/meta_data_gen.hpp>
#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/validation/context.hpp>
#include <power_grid_model/validation/issues.hpp>

#include <doctest/doctest.h>

#include <set>
#include <string_view>
#include <tuple>
#include <vector>

TEST_CASE("Default validation registry") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using meta_data::meta_data_gen::meta_data;

    SUBCASE("Full registry checks") {
        auto const& input_meta = meta_data.get_dataset("input");
        auto const& registry = default_registry();
        std::set<std::string_view> registered_components;
        std::set<std::tuple<std::string_view, bool, RuleCode, std::vector<std::string_view>>> registered_rules;

        for (auto const& component_rules : registry) {
            registered_components.insert(component_rules.component);
            auto const& component = input_meta.get_component(component_rules.component);
            auto check_rules = [&component, &component_rules, &registered_rules](auto const& rules, bool is_batch) {
                for (auto const& rule : rules) {
                    CHECK(static_cast<bool>(rule.apply));
                    REQUIRE_FALSE(rule.fields.empty());
                    for (auto const& field : rule.fields) {
                        // Check if the field exists in the component
                        CHECK(component.has_attribute(field));
                    }
                    // Check for duplicate rules based on component, fields, input/batch, code
                    CHECK(registered_rules.emplace(component_rules.component, is_batch, rule.code, rule.fields).second);
                }
            };
            check_rules(component_rules.input_rules, false);
            check_rules(component_rules.batch_rules, true);
        }

        // Check if all components are registered in default registry
        CHECK(registered_components == (std::set<std::string_view>{"line", "node", "sym_power_sensor"}));
    }

    SUBCASE("rule adapters accept empty component buffers") {
        ConstDataset input{false, 1, "input", meta_data};
        input.add_buffer("node", 0, 0, nullptr, nullptr);
        input.add_buffer("line", 0, 0, nullptr, nullptr);
        ValidationContext const context{.input = input,
                                        .options = {.calculation_type = CalculationType::power_flow,
                                                    .calculation_symmetry = CalculationSymmetry::asymmetric}};
        Issues issues;

        for (auto const& component_rules : default_registry()) {
            std::string_view const component_name = component_rules.component;
            for (auto const& rule : component_rules.input_rules) {
                rule.apply(component_name, context, issues);
            }
            for (auto const& rule : component_rules.batch_rules) {
                rule.apply(component_name, context, issues);
            }
        }

        CHECK(issues.empty());
    }
}
