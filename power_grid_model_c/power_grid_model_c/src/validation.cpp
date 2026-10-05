#define PGM_DLL_EXPORTS
#include "forward_declarations.hpp"
#include "handle.hpp"
#include "input_sanitization.hpp"
#include "safe_memory_handling.hpp"

#include "power_grid_model_c/validation.h"

#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/validation/input.hpp>

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

struct PGM_ValidationResult {
    power_grid_model::validation::Issues issues;
    struct IssueProjection {
        std::vector<PGM_ValidationField> fields;
        std::vector<PGM_ValidationObject> objects;
    };
    std::vector<IssueProjection> projections;

    explicit PGM_ValidationResult(power_grid_model::validation::Issues issue_list) : issues{std::move(issue_list)} {
        projections.resize(issues.size());
        for (std::size_t issue_idx{}; issue_idx < issues.size(); ++issue_idx) {
            auto const& issue = issues[issue_idx];
            auto& projection = projections[issue_idx];
            for (auto const& field : issue.fields) {
                projection.fields.push_back({.component = field.component.data(), .field = field.field.data()});
            }
            for (auto const& object : issue.objects) {
                projection.objects.push_back({.component = object.component.data(), .id = object.id});
            }
        }
    }
};

namespace {
using power_grid_model::InvalidArguments;
using power_grid_model::validation::Rule;
using power_grid_model_c::call_with_catch;
using power_grid_model_c::cast_to_cpp;
using power_grid_model_c::create;
using power_grid_model_c::destroy;
using power_grid_model_c::safe_ptr_get;

enum PGM_ValidationIssueKind to_c_issue_kind(Rule rule) {
    switch (rule) {
    case Rule::missing_value:
        return PGM_validation_issue_missing_value;
    case Rule::infinity:
        return PGM_validation_issue_infinity;
    case Rule::not_unique:
        return PGM_validation_issue_not_unique;
    case Rule::not_greater_than_zero:
        return PGM_validation_issue_not_greater_than_zero;
    case Rule::not_boolean:
        return PGM_validation_issue_not_boolean;
    case Rule::two_values_zero:
        return PGM_validation_issue_two_values_zero;
    case Rule::invalid_id_reference:
        return PGM_validation_issue_invalid_id_reference;
    case Rule::cross_component_not_unique:
        return PGM_validation_issue_cross_component_not_unique;
    }
    throw std::logic_error{"Unknown validation issue rule."};
}
} // namespace

PGM_ValidationResult* PGM_validate_input_data(PGM_Handle* handle, PGM_ConstDataset const* dataset,
                                              PGM_Idx symmetric) noexcept {
    return call_with_catch(handle, [dataset, symmetric] {
        if (symmetric != 0 && symmetric != 1) {
            throw InvalidArguments{"PGM_validate_input_data expects symmetric to be 0 or 1.\n"};
        }
        auto const& cpp_dataset = safe_ptr_get(cast_to_cpp(dataset));
        auto issues = power_grid_model::validation::validate_input_dataset(cpp_dataset, symmetric != 0);
        return create<PGM_ValidationResult>(std::move(issues));
    });
}

PGM_Idx PGM_validation_result_issue_count(PGM_Handle* handle, PGM_ValidationResult const* result) noexcept {
    return call_with_catch(handle, [result] { return static_cast<PGM_Idx>(safe_ptr_get(result).issues.size()); });
}

PGM_ValidationIssue PGM_validation_result_get_issue(PGM_Handle* handle, PGM_ValidationResult const* result,
                                                    PGM_Idx issue_idx) noexcept {
    return call_with_catch(handle, [result, issue_idx] {
        auto const& validation_result = safe_ptr_get(result);
        if (issue_idx < 0 || static_cast<std::size_t>(issue_idx) >= validation_result.issues.size()) {
            throw InvalidArguments{"PGM_validation_result_get_issue received an invalid issue index.\n"};
        }
        auto const& issue = validation_result.issues[static_cast<std::size_t>(issue_idx)];
        auto const& projection = validation_result.projections[static_cast<std::size_t>(issue_idx)];
        return PGM_ValidationIssue{.kind = to_c_issue_kind(issue.rule),
                                   .component = issue.component.data(),
                                   .field = issue.field.data(),
                                   .n_ids = static_cast<PGM_Idx>(issue.ids.size()),
                                   .ids = issue.ids.data(),
                                   .n_fields = static_cast<PGM_Idx>(projection.fields.size()),
                                   .fields = projection.fields.data(),
                                   .reference_component =
                                       issue.reference_component.empty() ? nullptr : issue.reference_component.data(),
                                   .n_objects = static_cast<PGM_Idx>(projection.objects.size()),
                                   .objects = projection.objects.data()};
    });
}

void PGM_destroy_validation_result(PGM_ValidationResult* result) noexcept { destroy(result); }
