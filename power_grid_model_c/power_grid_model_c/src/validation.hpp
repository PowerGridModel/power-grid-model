#pragma once

#include "power_grid_model_c/validation.h"

#include <power_grid_model/validation/report.hpp>

#include <string>

struct PGM_ValidationResult {
    using Report = power_grid_model::validation::ValidationReport;

    explicit PGM_ValidationResult(Report report);
    std::string const& get_json_report();

  private:
    Report report_;
    std::string json_report_;
};
