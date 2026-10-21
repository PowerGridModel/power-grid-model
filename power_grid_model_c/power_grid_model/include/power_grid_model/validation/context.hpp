// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "issues.hpp"
#include "rules.hpp"

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/common/exception.hpp>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace power_grid_model::validation {

enum class ValidationDataset : std::uint8_t { input, update };

class InputValidationLookup {
  public:
    template <class Ids> void add_component_ids(std::string_view component, Ids const& ids) {
        auto& indexed_ids = ids_by_component_[component];
        for (std::size_t idx{}; idx != ids.size(); ++idx) {
            ID const id = ids[idx];
            if (id != na_IntID) {
                indexed_ids.insert(id);
            }
        }
    }

    [[nodiscard]] std::unordered_set<ID> const& component_ids(std::string_view component) const {
        auto const iter = ids_by_component_.find(component);
        if (iter == ids_by_component_.end()) {
            throw InvalidArguments{"Input validation ID lookup was not prepared for the requested component.\n"};
        }
        return iter->second;
    }

    [[nodiscard]] bool contains_component_id(std::string_view component, ID id) const {
        return component_ids(component).contains(id);
    }

    [[nodiscard]] auto const& ids_by_component() const noexcept { return ids_by_component_; }

  private:
    std::unordered_map<std::string_view, std::unordered_set<ID>> ids_by_component_;
};

struct ValidationContext {
    ConstDataset const& input;
    ConstDataset const* update;
    bool symmetric;
    CalculationType calculation_type;
    InputValidationLookup input_id_lookup{};

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
    std::function<void(ConstDataset const&, InputValidationLookup&)> build_input_id_lookup{};
};

using ValidationRegistry = std::vector<ComponentRules>;

namespace detail {

template <class Component> void index_input_component_ids(ConstDataset const& dataset, InputValidationLookup& lookup) {
    typename Component::InputType const default_values{};
    auto const ids = dataset.get_attribute_view<Component, ID>("id", default_values.id);
    lookup.add_component_ids(Component::name, ids);
}

inline InputValidationLookup prepare_input_id_lookup(ConstDataset const& dataset, ValidationRegistry const& registry) {
    InputValidationLookup lookup;
    for (auto const& component_rules : registry) {
        if (!component_rules.build_input_id_lookup) {
            throw InvalidArguments{"Input validation requires an ID index builder for component '" +
                                   std::string{component_rules.component} + "'.\n"};
        }
        component_rules.build_input_id_lookup(dataset, lookup);
    }
    return lookup;
}

} // namespace detail

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
                rules::apply(rule, values, ids, Component::name, field, issues);
            }};
}

template <class Component, auto FieldMember, class Rule>
RuleDefinition make_range_rule(std::string_view field, Rule rule, ValidationDataset source = ValidationDataset::input) {
    using Input = typename Component::InputType;
    using Value = std::remove_cvref_t<decltype(std::declval<Input const&>().*FieldMember)>;

    return {.code = Rule::code,
            .fields = {{field, meta_data::ctype_v<Value>}},
            .apply = [field, source, rule = std::move(rule)](ValidationContext const& context, Issues& issues) {
                auto const& dataset = context.dataset(source);
                Input const default_values{};
                auto const ids = dataset.get_attribute_view<Component, ID>("id", default_values.id);
                auto const values = dataset.get_attribute_view<Component, Value>(field, default_values.*FieldMember);
                std::invoke(rule, values, ids, Component::name, field, issues);
            }};
}

template <class Component, auto FieldMember>
RuleDefinition make_id_reference_rule(std::string_view field, std::string_view reference_component) {
    using Input = typename Component::InputType;
    using Value = std::remove_cvref_t<decltype(std::declval<Input const&>().*FieldMember)>;
    static_assert(std::same_as<Value, ID>);

    return {.code = rules::ValidIdReference::code,
            .fields = {{field, meta_data::ctype_v<Value>}},
            .apply = [field, reference_component](ValidationContext const& context, Issues& issues) {
                Input const default_values{};
                auto const ids = context.input.get_attribute_view<Component, ID>("id", default_values.id);
                auto const values =
                    context.input.get_attribute_view<Component, Value>(field, default_values.*FieldMember);
                auto const& valid_ids = context.input_id_lookup.component_ids(reference_component);
                rules::ValidIdReference{}(values, ids, Component::name, field, reference_component, valid_ids, issues);
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
