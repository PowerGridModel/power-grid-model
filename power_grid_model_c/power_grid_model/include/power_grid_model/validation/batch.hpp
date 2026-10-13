#pragma once

#include "input.hpp"

#include <power_grid_model/common/exception.hpp>

namespace power_grid_model::validation {

inline ValidationReport validate_batch_report(ConstDataset const&, ConstDataset const&, bool) {
    throw NotImplementedError{};
}

} // namespace power_grid_model::validation
