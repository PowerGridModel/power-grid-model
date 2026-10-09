#pragma once

#include "power_grid_model_c/validation.h"

#include <power_grid_model/validation/issues.hpp>

#include <mutex>
#include <string>

struct PGM_ValidationResult {
    explicit PGM_ValidationResult(power_grid_model::validation::Issues source_issues);
    explicit PGM_ValidationResult(power_grid_model::validation::BatchIssues source_scenarios);

    bool passed() const noexcept { return passed_; }
    bool is_batch() const noexcept { return is_batch_; }
    std::string const& get_json_report() const;

  private:
    bool passed_;
    bool is_batch_{};
    mutable std::once_flag json_report_once_;
    mutable power_grid_model::validation::Issues issues_;
    mutable power_grid_model::validation::BatchIssues batch_issues_;
    mutable std::string json_report_;
};
