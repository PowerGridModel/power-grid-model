#pragma once

#include "input.hpp"

namespace power_grid_model::validation {

ValidationReport validate_batch_report(ConstDataset const& input_dataset, ConstDataset const& update_dataset,
                                       bool symmetric);

} // namespace power_grid_model::validation