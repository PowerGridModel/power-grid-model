// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "issues.hpp"
#include "rules.hpp"

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/common/exception.hpp>

#include <functional>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace power_grid_model::validation {

enum class ValidationDataset : std::uint8_t { input, update };

struct ValidationContext {
    ConstDataset const& input;
    ConstDataset const* update;
    bool symmetric;
    CalculationType calculation_type;

    ConstDataset const& dataset(ValidationDataset source) const {
        switch (source) {
        case ValidationDataset::input:
            return input;
        case ValidationDataset::update:
            if (update != nullptr) {
                return *update;
            }
            throw InvalidArguments{"Validation rule requires an update dataset.\n"};
        }
        throw MissingCaseForEnumError{"ValidationContext::dataset", source};
    }
};

struct RuleField {
    std::string_view name;
    CType type;
};

struct RuleDefinition {
    RuleCode code;
    std::vector<RuleField> fields;
    std::function<void(ValidationContext const&, Issues&)> apply;
};

struct ComponentRules {
    std::string_view component;
    std::vector<RuleDefinition> input_rules;
    std::vector<RuleDefinition> batch_rules;
};

using ValidationRegistry = std::vector<ComponentRules>;

template <class Component, auto FieldMember, class Rule>
RuleDefinition make_unary_rule(std::string_view field, Rule rule, ValidationDataset source = ValidationDataset::input) {
    using Input = typename Component::InputType;
    using Value = std::remove_cvref_t<decltype(std::declval<Input const&>().*FieldMember)>;

    return {.code = Rule::code,
            .fields = {{field, meta_data::ctype_v<Value>}},
            .apply = [field, source, rule = std::move(rule)](ValidationContext const& context, Issues& issues) {
                auto const& dataset = context.dataset(source);
                Input const default_values{};
                auto const ids = dataset.get_attribute_view<Component, ID>("id", default_values.id);
                auto const values = dataset.get_attribute_view<Component, Value>(field, default_values.*FieldMember);
                rules::apply(rule, values, ids, Component::name, field, issues, context);
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
