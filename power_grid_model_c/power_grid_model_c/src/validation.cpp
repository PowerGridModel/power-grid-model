#define PGM_DLL_EXPORTS
#include "validation.hpp"
#include "forward_declarations.hpp"
#include "handle.hpp"
#include "input_sanitization.hpp"
#include "options.hpp"
#include "safe_memory_handling.hpp"

#include "power_grid_model_c/validation.h"

#include <power_grid_model/common/exception.hpp>
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
    } else if (issue.rule == Rule::id_not_in_dataset) {
        details["ref_dataset"] = "update";
    }

    auto fields = nlohmann::json::array();
    for (auto const& field : issue.fields) {
        fields.push_back({{"component", field.component}, {"field", field.field}});
    }

    auto objects = nlohmann::json::array();
    for (auto const& object : issue.objects) {
        objects.push_back({{"component", object.component}, {"id", object.id}});
    }

    nlohmann::json ids = nullptr;
    if (!issue.ids.empty()) {
        ids = issue.ids;
    }
    nlohmann::json reference_component = nullptr;
    if (!issue.reference_component.empty()) {
        reference_component = std::string{issue.reference_component};
    }

    return {{"kind", to_json_issue_kind(issue.rule)},
            {"component", issue.component},
            {"field", issue.field},
            {"ids", std::move(ids)},
            {"fields", std::move(fields)},
            {"objects", std::move(objects)},
            {"reference_component", std::move(reference_component)},
            {"details", std::move(details)}};
}

std::string to_json_report(power_grid_model::validation::Issues const& issues) {
    auto report = nlohmann::json{{"schema_version", 1}, {"issues", nlohmann::json::array()}};
    for (auto const& issue : issues) {
        report["issues"].push_back(to_json(issue));
    }
    return report.dump(-1, ' ', false, nlohmann::json::error_handler_t::strict);
}

std::string to_json_report(power_grid_model::validation::BatchIssues const& scenarios) {
    auto data = nlohmann::json::array();
    for (auto const& issues : scenarios) {
        auto scenario = nlohmann::json::object();
        for (auto const& issue : issues) {
            scenario[std::string{issue.component}].push_back(to_json(issue));
        }
        data.push_back(std::move(scenario));
    }
    auto report = nlohmann::json{{"schema_version", 1}, {"is_batch", true}, {"data", std::move(data)}};
    return report.dump(-1, ' ', false, nlohmann::json::error_handler_t::strict);
}

bool all_scenarios_passed(power_grid_model::validation::BatchIssues const& scenarios) {
    return std::ranges::all_of(scenarios,
                               [](power_grid_model::validation::Issues const& issues) { return issues.empty(); });
}
} // namespace

PGM_ValidationResult::PGM_ValidationResult(power_grid_model::validation::Issues source_issues)
    : passed_{source_issues.empty()}, issues_{std::move(source_issues)} {}

PGM_ValidationResult::PGM_ValidationResult(power_grid_model::validation::BatchIssues source_scenarios)
    : passed_{all_scenarios_passed(source_scenarios)}, is_batch_{true}, batch_issues_{std::move(source_scenarios)} {}

std::string const& PGM_ValidationResult::get_json_report() const {
    std::call_once(json_report_once_, [this] {
        if (is_batch_) {
            json_report_ = to_json_report(batch_issues_);
            power_grid_model::validation::BatchIssues{}.swap(batch_issues_);
        } else {
            json_report_ = to_json_report(issues_);
            power_grid_model::validation::Issues{}.swap(issues_);
        }
    });
    return json_report_;
}

PGM_ValidationResult* PGM_validate_input_data(PGM_Handle* handle, PGM_ConstDataset const* dataset,
                                              PGM_Options const* options) noexcept {
    return call_with_catch(handle, [dataset, options] {
        bool const symmetric = validation_symmetric(options);
        auto const& cpp_dataset = safe_ptr_get(cast_to_cpp(dataset));
        auto issues = power_grid_model::validation::validate_input_dataset(cpp_dataset, symmetric);
        return create<PGM_ValidationResult>(std::move(issues));
    });
}

PGM_ValidationResult* PGM_validate_batch_data(PGM_Handle* handle, PGM_ConstDataset const* input_dataset,
                                              PGM_ConstDataset const* update_dataset,
                                              PGM_Options const* options) noexcept {
    return call_with_catch(handle, [input_dataset, update_dataset, options] {
        bool const symmetric = validation_symmetric(options);
        auto const& cpp_input_dataset = safe_ptr_get(cast_to_cpp(input_dataset));
        auto const& cpp_update_dataset = safe_ptr_get(cast_to_cpp(update_dataset));
        auto scenarios =
            power_grid_model::validation::validate_batch_dataset(cpp_input_dataset, cpp_update_dataset, symmetric);
        return create<PGM_ValidationResult>(std::move(scenarios));
    });
}

PGM_Idx PGM_validation_result_passed(PGM_Handle* handle, PGM_ValidationResult const* result) noexcept {
    return call_with_catch(handle,
                           [result] { return power_grid_model_c::to_c_bool<PGM_Idx>(safe_ptr_get(result).passed()); });
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
