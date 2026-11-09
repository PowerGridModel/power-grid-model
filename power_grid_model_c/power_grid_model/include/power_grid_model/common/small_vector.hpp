// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef PGM_CXX_MODULE_PURVIEW
#include "common.hpp"
#endif

#ifndef PGM_CXX_MODULE_PURVIEW
#include <boost/container/small_vector.hpp>
#endif

#ifndef PGM_CXX_MODULE_PURVIEW
#include <cstddef>
#endif

namespace power_grid_model {

// A vector that keeps room for the first N elements inside the object itself and only allocates
// beyond that. Use it where a container is almost always short but occasionally is not. N is a
// per-use-site decision: too large inflates every object for slots that go unused.
template <class T, std::size_t N> using SmallVector = boost::container::small_vector<T, N>;

} // namespace power_grid_model
