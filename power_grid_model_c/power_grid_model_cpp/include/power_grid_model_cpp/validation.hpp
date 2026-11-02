// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_CPP_VALIDATION_HPP
#define POWER_GRID_MODEL_CPP_VALIDATION_HPP

#include "basics.hpp"
#include "dataset.hpp"
#include "handle.hpp"
#include "options.hpp"

#include "power_grid_model_c/validation.h"

#include <string>

namespace power_grid_model_cpp {
// The result of validating an input dataset, optionally together with batch update scenarios.
// Construction performs the validation and throws a PowerGridError if the validation itself fails.
// Issues found in the data do not throw; they are reported through passed() and the report string.
class ValidationReport {
  public:
    ValidationReport(DatasetConst const& input_dataset, Options const& options)
        : report_{handle_.call_with(PGM_validate_data, input_dataset.get(), nullptr, options.get())} {}

    ValidationReport(DatasetConst const& input_dataset, DatasetConst const& update_dataset, Options const& options)
        : report_{handle_.call_with(PGM_validate_data, input_dataset.get(), update_dataset.get(), options.get())} {}

    RawValidationReport* get() { return report_.get(); }
    RawValidationReport const* get() const { return report_.get(); }

    bool passed() { return handle_.call_with(PGM_validation_report_passed, get()) == PGM_validation_pass; }

    std::string get_to_zero_terminated_string() {
        return std::string{handle_.call_with(PGM_validation_report_get_to_zero_terminated_string, get())};
    }

  private:
    Handle handle_{};
    detail::UniquePtr<RawValidationReport, &PGM_destroy_validation_report> report_;
};

inline ValidationReport validate_data(DatasetConst const& input_dataset, Options const& options) {
    return ValidationReport{input_dataset, options};
}

inline ValidationReport validate_data(DatasetConst const& input_dataset, DatasetConst const& update_dataset,
                                      Options const& options) {
    return ValidationReport{input_dataset, update_dataset, options};
}
} // namespace power_grid_model_cpp

#endif // POWER_GRID_MODEL_CPP_VALIDATION_HPP
