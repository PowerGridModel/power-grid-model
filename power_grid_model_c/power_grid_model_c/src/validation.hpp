#pragma once

#include "power_grid_model_c/validation.h"

#include <string>

struct PGM_ValidationResult {
    explicit PGM_ValidationResult(std::string json_report);

    std::string const& get_json_report() const noexcept { return json_report_; }

  private:
    std::string json_report_;
};
