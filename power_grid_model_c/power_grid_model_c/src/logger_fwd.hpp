// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef PGM_DLL_EXPORTS
#define PGM_DLL_EXPORTS
#endif

#include "handle.hpp"

#include <memory>

namespace power_grid_model::common::logging {
class MultiThreadedLogger;
} // namespace power_grid_model::common::logging

namespace power_grid_model_c {
using power_grid_model::common::logging::MultiThreadedLogger;
} // namespace power_grid_model_c
