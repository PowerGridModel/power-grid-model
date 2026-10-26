// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "issues.hpp"
#include "power_grid_model/auxiliary/meta_data.hpp"
#include "power_grid_model/common/common.hpp"
#include "rules.hpp"

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/common/exception.hpp>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string_view>
#include <utility>
#include <vector>

namespace power_grid_model::validation {

enum class ValidationDatasetType : std::uint8_t { input, input_with_update };

struct ValidationContext {
    std::reference_wrapper<ConstDataset const> input;
    bool symmetric;
    CalculationType calculation_type;

    ConstDataset const& dataset(ValidationDatasetType source) const {
        switch (source) {
        case ValidationDatasetType::input:
            return input.get();
        case ValidationDatasetType::input_with_update:
            throw NotImplementedError{};
        }
        throw MissingCaseForEnumError{"ValidationContext::dataset", source};
    }
};

struct RuleDefinition {
    RuleCode code;
    std::vector<std::string_view> fields;
    std::function<void(ValidationContext const&, Issues&)> apply;
};

struct ComponentRules {
    std::string_view component;
    std::vector<RuleDefinition> input_rules;
    std::vector<RuleDefinition> batch_rules;
};

using ValidationRegistry = std::vector<ComponentRules>;

template <class Component, auto FieldMember, class Rule>
RuleDefinition make_unary_rule(std::string_view field, Rule rule,
                               ValidationDatasetType source = ValidationDatasetType::input) {
    using Input = Component::InputType;

    return {.code = Rule::code,
            .fields = {field},
            .apply = [field, source, rule = std::move(rule)](ValidationContext const& context, Issues& issues) {
                auto const& dataset = context.dataset(source);
                Input const default_values{};
                auto const apply_to_component = [&](auto const& component_span) {
                    auto const ids = meta_data::get_attribute_span(component_span, &Input::id, "id");
                    // Mismatched id and compoennt size should validated or skipped at earlier stage
                    assert(ids.size() == static_cast<std::size_t>(component_span.size()));
                    auto const values = meta_data::get_attribute_span(component_span, FieldMember, field);
                    // Missing attribute is allowed. Ids should be available corresponding to all values otherwise.
                    assert(values.size() == 0 || values.size() == ids.size());
                    if (values.size() == 0 && ids.size() > 0) {
                        // All values are default. This path signifies missing attribute in columnar data
                        if (std::invoke(rule, default_values.*FieldMember)) {
                            std::vector<ID> invalid_ids(ids.begin(), ids.end());
                            add_issue(issues, Rule::code, Component::name, field, std::move(invalid_ids));
                        }
                        return;
                    }
                    rules::apply(rule, values, ids, Component::name, field, issues);
                };
                // TODO (nitbharambe): Dont use input here but make this available via context
                dataset.for_each_component<meta_data::input_getter_s, Component>(apply_to_component);
            }};
}

namespace detail {

inline void apply_rules(std::vector<RuleDefinition> const& rules, ValidationContext const& context, Issues& issues) {
    for (auto const& rule : rules) {
        rule.apply(context, issues);
    }
}

} // namespace detail

} // namespace power_grid_model::validation
