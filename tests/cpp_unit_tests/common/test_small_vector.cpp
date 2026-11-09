// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#ifdef PGM_USE_CXX_MODULES
// Workaround: declarations from the global module fragments of imported modules are not visible to ADL and
// operators in importers. Making them textually available before any import avoids this.
#include <Eigen/Dense>
#include <boost/container/small_vector.hpp>

#include <algorithm>
#include <cmath>
#include <complex>
#include <new>
#include <ranges>
#include <variant>

import power_grid_model.common.common;
import power_grid_model.common.small_vector;
#else
#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/small_vector.hpp>
#endif

#include <doctest/doctest.h>

namespace power_grid_model {
TEST_CASE("SmallVector") {
    SmallVector<Idx, 2> vec{1, 2};
    CHECK(vec.size() == 2);
    CHECK(vec.capacity() == 2);

    vec.push_back(3);
    CHECK(vec.size() == 3);
    CHECK(vec[0] == 1);
    CHECK(vec[1] == 2);
    CHECK(vec[2] == 3);
}
} // namespace power_grid_model
