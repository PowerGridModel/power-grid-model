// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "logger_fwd.hpp"

#include <memory>

namespace power_grid_model_c {
std::unique_ptr<MultiThreadedLogger> make_handle_logger();
} // namespace power_grid_model_c
