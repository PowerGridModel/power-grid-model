// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once
#ifndef POWER_GRID_MODEL_COUNTING_ITERATOR_HPP
#define POWER_GRID_MODEL_COUNTING_ITERATOR_HPP

#include "power_grid_model.hpp"

#include <boost/iterator/counting_iterator.hpp>

namespace power_grid_model {

// couting iterator
using IdxCount = boost::counting_iterator<Idx>;

} // namespace power_grid_model

#endif
