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

    auto fields = nlohmann::json::array();
    for (auto const& field : issue.fields) {
        fields.push_back({{"component", field.component}, {"field", field.field}});
    }

    auto objects = nlohmann::json::array();
    for (auto const& object : issue.objects) {
        nlohmann::json id_value = nullptr;
        if (object.id != power_grid_model::na_IntID) {
            id_value = object.id;
        }
        nlohmann::json reference{{"component", object.component}, {"id", std::move(id_value)}};
        if (object.row.has_value()) {
            reference["row"] = *object.row;
        }
        objects.push_back(std::move(reference));
    }

    return {{"kind", to_json_issue_kind(issue.rule)},
            {"fields", std::move(fields)},
            {"objects", std::move(objects)},
            {"details", std::move(details)}};
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

char const* to_json_validation_status(power_grid_model::validation::ValidationStatus status) {
    switch (status) {
    case power_grid_model::validation::ValidationStatus::passed:
        return "passed";
    case power_grid_model::validation::ValidationStatus::failed:
        return "failed";
    case power_grid_model::validation::ValidationStatus::blocked:
        return "blocked";
    }
    throw std::logic_error{"Unknown validation report status."};
}

std::string to_json_report(power_grid_model::validation::ValidationReport const& validation_report) {
    using power_grid_model::validation::ValidationStatus;
    auto results = nlohmann::json::array();
    bool valid = true;
    for (auto const& result : validation_report.results) {
        auto issues = nlohmann::json::array();
        for (auto const& issue : result.issues) {
            issues.push_back(to_json(issue));
        }
        nlohmann::json scenario = nullptr;
        if (result.scenario.has_value()) {
            scenario = *result.scenario;
        }
        results.push_back({{"scenario", std::move(scenario)},
                           {"status", to_json_validation_status(result.status)},
                           {"issues", std::move(issues)}});
        valid = valid && result.status == ValidationStatus::passed;
    }
    auto report = nlohmann::json{{"schema_version", 2},
                                 {"validation_type", to_json_validation_type(validation_report.type)},
                                 {"valid", valid},
                                 {"results", std::move(results)}};
    return report.dump(-1, ' ', false, nlohmann::json::error_handler_t::strict);
}
} // namespace

PGM_ValidationResult::PGM_ValidationResult(std::string json_report) : json_report_{std::move(json_report)} {}

PGM_ValidationResult* PGM_validate_input_data(PGM_Handle* handle, PGM_ConstDataset const* dataset,
                                              PGM_Options const* options) noexcept {
    return call_with_catch(handle, [dataset, options] {
        bool const symmetric = validation_symmetric(options);
        auto const& cpp_dataset = safe_ptr_get(cast_to_cpp(dataset));
        auto issues = power_grid_model::validation::validate_input_dataset(cpp_dataset, symmetric);
        using namespace power_grid_model::validation;
        ValidationReport report{.type = ValidationType::input, .results = {}};
        auto const status = issues.empty() ? ValidationStatus::passed : ValidationStatus::failed;
        report.results.push_back({.scenario = std::nullopt, .status = status, .issues = std::move(issues)});
        return create<PGM_ValidationResult>(to_json_report(report));
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
        return create<PGM_ValidationResult>(to_json_report(report));
    });
}

void PGM_validation_result_get_json(PGM_Handle* handle, PGM_ValidationResult const* result, char const** data,
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
        auto const& report = safe_ptr_get(result).get_json_report();
        auto const report_size = power_grid_model_c::safe_cast<PGM_Idx>(report.size());
        *output_data = report.data();
        *output_size = report_size;
    });
}

void PGM_destroy_validation_result(PGM_ValidationResult* result) noexcept { destroy(result); }
