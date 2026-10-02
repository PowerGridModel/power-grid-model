#pragma once

#include "issues.hpp"

#include <power_grid_model/auxiliary/dataset.hpp>

namespace power_grid_model_c::validation {

void validate_node(power_grid_model::ConstDataset const& dataset, Issues& issues);

} // namespace power_grid_model_c::validation
