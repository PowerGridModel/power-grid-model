#pragma once

#include "report.hpp"

#include <power_grid_model/auxiliary/dataset.hpp>

namespace power_grid_model::validation {

ValidationReport validate_input_dataset(ConstDataset const& dataset, bool symmetric);

} // namespace power_grid_model::validation