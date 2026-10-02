#pragma once

#include "issues.hpp"

#include <power_grid_model/auxiliary/dataset.hpp>

namespace power_grid_model_c::validation {

Issues validate_input_dataset(power_grid_model::ConstDataset const& dataset);

} // namespace power_grid_model_c::validation
