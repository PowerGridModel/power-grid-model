#pragma once

#include "options.hpp"

#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/common/exception.hpp>

namespace power_grid_model_c {
constexpr auto get_calculation_symmetry(PGM_Options const& options) {
    switch (options.symmetric) {
    case PGM_asymmetric:
        return power_grid_model::CalculationSymmetry::asymmetric;
    case PGM_symmetric:
        return power_grid_model::CalculationSymmetry::symmetric;
    default:
        throw power_grid_model::MissingCaseForEnumError{"get_calculation_symmetry", options.symmetric};
    }
}
} // namespace power_grid_model_c
