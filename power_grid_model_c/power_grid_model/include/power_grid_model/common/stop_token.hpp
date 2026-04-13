// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "exception.hpp"

#include <stop_token>

namespace power_grid_model {
inline void stop_if_requested(std::stop_token const& stop_token) {
    if (stop_token.stop_requested()) {
        throw OperationCanceled{};
    }
}
} // namespace power_grid_model
