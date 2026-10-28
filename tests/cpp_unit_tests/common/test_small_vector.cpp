// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#ifdef PGM_USE_CXX_MODULES
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
