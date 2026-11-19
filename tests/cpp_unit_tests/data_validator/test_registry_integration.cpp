// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/validation/registration.hpp>

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/auxiliary/meta_data_gen.hpp>
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/validation/context.hpp>
#include <power_grid_model/validation/issues.hpp>

#include <doctest/doctest.h>

#include <set>
#include <string_view>
#include <tuple>

TEST_CASE("Default validation registry") {
    using namespace power_grid_model;
    using namespace power_grid_model::validation;
    using meta_data::meta_data_gen::meta_data;

    SUBCASE("references valid fields") {
        auto const& input_meta = meta_data.get_dataset("input");
        auto const& registry = default_registry();
        std::set<std::string_view> registered_components;
        std::set<std::tuple<std::string_view, bool, RuleCode, std::string_view>> registered_fields;

        for (auto const& component_rules : registry) {
            registered_components.insert(component_rules.component);
            auto const& component = input_meta.get_component(component_rules.component);
            auto check_rules = [&component, &component_rules, &registered_fields](auto const& rules, bool is_batch) {
                for (auto const& rule : rules) {
                    CHECK(static_cast<bool>(rule.apply));
                    REQUIRE_FALSE(rule.fields.empty());
                    for (auto const& field : rule.fields) {
                        CHECK(component.has_attribute(field));
                        CHECK(registered_fields.emplace(component_rules.component, is_batch, rule.code, field).second);
                    }
                }
            };
            check_rules(component_rules.input_rules, false);
            check_rules(component_rules.batch_rules, true);
        }

        CHECK(registered_components == (std::set<std::string_view>{"line", "node"}));
    }

    SUBCASE("rule adapters accept empty component buffers") {
        // TODO(figueroa1395): this test probably doesn't make sense without proper ID buffers and/or
        // proper ID handling
        ConstDataset input{false, 1, "input", meta_data};
        input.add_buffer("node", 0, 0, nullptr, nullptr);
        input.add_buffer("line", 0, 0, nullptr, nullptr);
        ConstDataset update{true, 1, "update", meta_data};
        update.add_buffer("node", 0, 0, nullptr, nullptr);
        update.add_buffer("line", 0, 0, nullptr, nullptr);

        ValidationContext context{.input = input,
                                  .update = update,
                                  .options = {.calculation_type = CalculationType::power_flow,
                                              .calculation_symmetry = CalculationSymmetry::asymmetric}};
        Issues issues;

        for (auto const& component_rules : default_registry()) {
            std::string_view const component_name = component_rules.component;
            for (auto const& rule : component_rules.input_rules) {
                rule.apply(component_name, context, issues, na_Idx);
            }
            // TODO(figueroa1395): batch unary rules don't work without ID buffers in the update or input (fallback)
            // dataset for (auto const& rule : component_rules.batch_rules) {
            //     rule.apply(component_name, context, issues, Idx{1});
            // }
        }

        CHECK(issues.empty());
    }
}
