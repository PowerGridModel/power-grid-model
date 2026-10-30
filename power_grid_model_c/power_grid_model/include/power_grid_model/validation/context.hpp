// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/common/enum.hpp>
#include <power_grid_model/common/exception.hpp>

#include <cstdint>
#include <functional>

namespace power_grid_model::validation {

enum class ValidationDatasetType : std::uint8_t { input, input_with_update };

struct ValidationContext {
    std::reference_wrapper<ConstDataset const> input;
    bool symmetric;
    CalculationType calculation_type;

    ConstDataset const& dataset(ValidationDatasetType source) const {
        switch (source) {
        case ValidationDatasetType::input:
            return input.get();
        case ValidationDatasetType::input_with_update:
            throw NotImplementedError{};
        }
        throw MissingCaseForEnumError{"ValidationContext::dataset", source};
    }
};

} // namespace power_grid_model::validation
