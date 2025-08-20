// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

namespace power_grid_model {

// batch parameter
struct BatchParameter {
    bool operator==(const BatchParameter&) const = default;
};

} // namespace power_grid_model
