// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_DLL_EXPORTS
#include "forward_declarations.hpp"
#include "handle.hpp"
#include "input_sanitization.hpp"
#include "options_utils.hpp"

#include "power_grid_model_c/basics.h"
#include "power_grid_model_c/validation.h"

#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/common/exception.hpp>
#include <power_grid_model/validation/batch.hpp>
#include <power_grid_model/validation/input.hpp>
#include <power_grid_model/validation/report.hpp>

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include <string>
#include <utility>

namespace {
using power_grid_model::CalculationType;
using power_grid_model::ExperimentalFeature;
using power_grid_model::InvalidArguments;
using power_grid_model::MissingCaseForEnumError;
using power_grid_model::na_Idx;
using power_grid_model::validation::ValidationIssue;
using power_grid_model::validation::ValidationReport;
using power_grid_model::validation::ValidationScope;
using power_grid_model_c::call_with_catch;
using power_grid_model_c::cast_to_cpp;
using power_grid_model_c::clear_validation_report;
using power_grid_model_c::get_calculation_symmetry;
using power_grid_model_c::safe_cast;
using power_grid_model_c::safe_enum;
using power_grid_model_c::safe_ptr;
using power_grid_model_c::safe_ptr_get;

void check_experimental_features(PGM_Options const& options) {
    switch (options.experimental_features) {
    case PGM_experimental_features_enabled:
        return;
    case PGM_experimental_features_disabled:
        throw ExperimentalFeature{"Data validation requires experimental features to be enabled.\n"};
    default:
        throw MissingCaseForEnumError{"validate_data", options.experimental_features};
    }
}

nlohmann::json to_json(ValidationIssue const& issue) {
    return {{"rule", issue.rule}, {"component", issue.component}, {"field", issue.field}, {"ids", issue.ids}};
}

nlohmann::json to_json(ValidationScope const& scope) {
    nlohmann::json scenario = nullptr;
    if (scope.scenario != na_Idx) {
        scenario = scope.scenario;
    }

    auto issues = nlohmann::json::array();
    for (ValidationIssue const& issue : scope.issues) {
        issues.push_back(to_json(issue));
    }

    return {{"scenario", std::move(scenario)}, {"issues", std::move(issues)}};
}

std::string to_json_report(ValidationReport const& report) {
    auto scopes = nlohmann::json::array();
    for (ValidationScope const& scope : report.scopes) {
        if (!scope.issues.empty()) {
            scopes.push_back(to_json(scope));
        }
    }

    auto const json_report = nlohmann::json{{"validation_type", power_grid_model::validation::to_string(report.type)},
                                            {"valid", report.valid()},
                                            {"results", std::move(scopes)}};
    return json_report.dump(-1, ' ', false, nlohmann::json::error_handler_t::strict);
}
} // namespace

PGM_Idx PGM_validate_input_data(PGM_Handle* handle, PGM_ConstDataset const* input_dataset,
                                PGM_Options const* options) noexcept {
    clear_validation_report(handle);
    return call_with_catch(handle, [handle, input_dataset, options] {
        auto& handle_ref = safe_ptr_get(handle);
        auto const& cpp_options = safe_ptr_get(options);
        check_experimental_features(cpp_options);
        auto const calculation_type = safe_enum<CalculationType>(cpp_options.calculation_type);
        bool const symmetric =
            get_calculation_symmetry(cpp_options) == power_grid_model::CalculationSymmetry::symmetric;
        auto const& cpp_dataset = safe_ptr_get(cast_to_cpp(input_dataset));
        handle_ref.validation_report =
            power_grid_model::validation::validate_input_dataset(cpp_dataset, symmetric, calculation_type);
        return handle_ref.validation_report->valid() ? PGM_Idx{0} : PGM_Idx{1};
    });
}

PGM_Idx PGM_validate_batch_data(PGM_Handle* handle, PGM_ConstDataset const* input_dataset,
                                PGM_ConstDataset const* update_dataset, PGM_Options const* options) noexcept {
    clear_validation_report(handle);
    return call_with_catch(handle, [handle, input_dataset, update_dataset, options] {
        auto& handle_ref = safe_ptr_get(handle);
        auto const& cpp_options = safe_ptr_get(options);
        check_experimental_features(cpp_options);
        auto const calculation_type = safe_enum<CalculationType>(cpp_options.calculation_type);
        bool const symmetric =
            get_calculation_symmetry(cpp_options) == power_grid_model::CalculationSymmetry::symmetric;
        auto const& cpp_input_dataset = safe_ptr_get(cast_to_cpp(input_dataset));
        auto const& cpp_update_dataset = safe_ptr_get(cast_to_cpp(update_dataset));
        handle_ref.validation_report = power_grid_model::validation::validate_batch_report(
            cpp_input_dataset, cpp_update_dataset, symmetric, calculation_type);
        return handle_ref.validation_report->valid() ? PGM_Idx{0} : PGM_Idx{1};
    });
}

void PGM_validation_report_get_json(PGM_Handle* handle, char const** data, PGM_Idx* size) noexcept {
    if (data != nullptr) {
        *data = nullptr;
    }
    if (size != nullptr) {
        *size = 0;
    }

    call_with_catch(handle, [handle, data, size] {
        auto& handle_ref = safe_ptr_get(handle);
        auto* output_data = safe_ptr(data);
        auto* output_size = safe_ptr(size);
        if (!handle_ref.validation_report.has_value()) {
            throw InvalidArguments{"No validation report is available in the handle.\n"};
        }
        if (handle_ref.validation_report_json.empty()) {
            handle_ref.validation_report_json = to_json_report(*handle_ref.validation_report);
        }
        std::string const& json_report = handle_ref.validation_report_json;
        *output_data = json_report.data();
        *output_size = safe_cast<PGM_Idx>(json_report.size());
    });
}
