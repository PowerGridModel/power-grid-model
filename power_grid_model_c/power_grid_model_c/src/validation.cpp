#define PGM_DLL_EXPORTS
#include "validation.hpp"
#include "forward_declarations.hpp"
#include "handle.hpp"
#include "input_sanitization.hpp"
#include "options.hpp"
#include "safe_memory_handling.hpp"

#include "power_grid_model_c/validation.h"

#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/validation/batch.hpp>
#include <power_grid_model/validation/input.hpp>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace {
using power_grid_model::InvalidArguments;
using power_grid_model::validation::Rule;
using power_grid_model::validation::ValidationReport;
using power_grid_model::validation::ValidationStatus;
using power_grid_model::validation::ValidationType;
using power_grid_model_c::call_with_catch;
using power_grid_model_c::cast_to_cpp;
using power_grid_model_c::create;
using power_grid_model_c::destroy;
using power_grid_model_c::safe_ptr;
using power_grid_model_c::safe_ptr_get;

bool validation_symmetric(PGM_Options const* options) {
    auto const& cpp_options = safe_ptr_get(options);
    if (cpp_options.symmetric != 0 && cpp_options.symmetric != 1) {
        throw InvalidArguments{"Validation options.symmetric must be 0 or 1.\n"};
    }
    return cpp_options.symmetric != 0;
}

char const* to_json_issue_kind(Rule rule) {
    switch (rule) {
    case Rule::missing_value:
        return "missing_value";
    case Rule::infinity:
        return "infinity";
    case Rule::not_unique:
        return "not_unique";
    case Rule::not_greater_than_zero:
        return "not_greater_than";
    case Rule::not_boolean:
        return "not_boolean";
    case Rule::two_values_zero:
        return "two_values_zero";
    case Rule::invalid_id_reference:
        return "invalid_id_reference";
    case Rule::id_not_in_dataset:
        return "id_not_in_dataset";
    case Rule::invalid_update_id:
        return "invalid_update_id";
    case Rule::cross_component_not_unique:
        return "cross_component_not_unique";
    }
    throw std::logic_error{"Unknown validation issue rule."};
}

nlohmann::json to_json(power_grid_model::validation::Issue const& issue) {
    auto details = nlohmann::json::object();
    if (issue.rule == Rule::not_greater_than_zero) {
        details["ref_value"] = 0;
    } else if (issue.rule == Rule::invalid_id_reference) {
        details["reference"] = {{"dataset", "input"}, {"component", issue.reference_component}, {"field", "id"}};
    } else if (issue.rule == Rule::id_not_in_dataset || issue.rule == Rule::invalid_update_id) {
        details["reference"] = {{"dataset", "input"}, {"component", issue.component}, {"field", "id"}};
    }

    auto issues = nlohmann::json::array();
    for (auto const& object : issue.objects) {
        nlohmann::json id_value = nullptr;
        if (object.id != power_grid_model::na_IntID) {
            id_value = object.id;
        }
        for (auto const& field : issue.fields) {
            if (field.component != object.component) {
                continue;
            }
            issues.push_back({{"kind", to_json_issue_kind(issue.rule)},
                              {"component", object.component},
                              {"field", field.field},
                              {"id", id_value},
                              {"row", object.row},
                              {"details", details}});
        }
    }
    if (issues.empty()) {
        throw std::logic_error{"Validation issue has no matching field and object references."};
    }
    return issues;
}

char const* to_json_validation_type(power_grid_model::validation::ValidationType type) {
    switch (type) {
    case power_grid_model::validation::ValidationType::input:
        return "input";
    case power_grid_model::validation::ValidationType::batch:
        return "batch";
    }
    throw std::logic_error{"Unknown validation report type."};
}

std::string to_json_report(power_grid_model::validation::ValidationReport const& validation_report) {
    auto results = nlohmann::json::array();
    bool valid = true;
    for (auto const& result : validation_report.results) {
        if (result.issues.empty()) {
            continue;
        }
        auto issues = nlohmann::json::array();
        for (auto const& issue : result.issues) {
            for (auto& flat_issue : to_json(issue)) {
                issues.push_back(std::move(flat_issue));
            }
        }
        nlohmann::json scenario = nullptr;
        if (result.scenario.has_value()) {
            scenario = *result.scenario;
        }
        results.push_back({{"scenario", std::move(scenario)}, {"issues", std::move(issues)}});
        valid = false;
    }
    auto report = nlohmann::json{{"schema_version", 3},
                                 {"validation_type", to_json_validation_type(validation_report.type)},
                                 {"valid", valid},
                                 {"results", std::move(results)}};
    return report.dump(-1, ' ', false, nlohmann::json::error_handler_t::strict);
}
} // namespace

PGM_ValidationResult::PGM_ValidationResult(Report report) : report_{std::move(report)} {}

std::string const& PGM_ValidationResult::get_json_report() {
    if (json_report_.empty()) {
        json_report_ = to_json_report(report_);
    }
    return json_report_;
}

PGM_ValidationResult* PGM_validate_input_data(PGM_Handle* handle, PGM_ConstDataset const* dataset,
                                              PGM_Options const* options) noexcept {
    return call_with_catch(handle, [dataset, options] {
        bool const symmetric = validation_symmetric(options);
        auto const& cpp_dataset = safe_ptr_get(cast_to_cpp(dataset));
        auto issues = power_grid_model::validation::validate_input_dataset(cpp_dataset, symmetric);
        auto report = ValidationReport{
            .type = ValidationType::input,
            .results = {{.scenario = std::nullopt,
                         .status = issues.empty() ? ValidationStatus::passed : ValidationStatus::failed,
                         .issues = std::move(issues)}},
        };
        return create<PGM_ValidationResult>(std::move(report));
    });
}

PGM_ValidationResult* PGM_validate_batch_data(PGM_Handle* handle, PGM_ConstDataset const* input_dataset,
                                              PGM_ConstDataset const* update_dataset,
                                              PGM_Options const* options) noexcept {
    return call_with_catch(handle, [input_dataset, update_dataset, options] {
        bool const symmetric = validation_symmetric(options);
        auto const& cpp_input_dataset = safe_ptr_get(cast_to_cpp(input_dataset));
        auto const& cpp_update_dataset = safe_ptr_get(cast_to_cpp(update_dataset));
        auto report = power_grid_model::validation::validate_batch_report(cpp_input_dataset, cpp_update_dataset,
                                                                          symmetric);
        return create<PGM_ValidationResult>(std::move(report));
    });
}

void PGM_validation_result_get_json(PGM_Handle* handle, PGM_ValidationResult* result, char const** data,
                                    PGM_Idx* size) noexcept {
    if (data != nullptr) {
        *data = nullptr;
    }
    if (size != nullptr) {
        *size = 0;
    }

    call_with_catch(handle, [result, data, size] {
        auto* output_data = safe_ptr(data);
        auto* output_size = safe_ptr(size);
        auto const& json_report = safe_ptr_get(result).get_json_report();
        auto const report_size = power_grid_model_c::safe_cast<PGM_Idx>(json_report.size());
        *output_data = json_report.data();
        *output_size = report_size;
    });
}

void PGM_destroy_validation_result(PGM_ValidationResult* result) noexcept { destroy(result); }
