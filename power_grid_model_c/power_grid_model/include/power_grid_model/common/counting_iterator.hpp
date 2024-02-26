// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"

#include <boost/iterator/counting_iterator.hpp>

namespace power_grid_model {

// couting iterator
using IdxCount = boost::counting_iterator<Idx>;

} // namespace power_grid_model
