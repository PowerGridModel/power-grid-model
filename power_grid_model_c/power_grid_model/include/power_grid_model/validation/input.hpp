#pragma once

#include "report.hpp"

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/common/exception.hpp>

namespace power_grid_model::validation {

inline ValidationReport validate_input_dataset(ConstDataset const&, bool, CalculationType) {
    throw NotImplementedError{};
}

} // namespace power_grid_model::validation
