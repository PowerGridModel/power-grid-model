// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/common/counting_iterator.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {
TEST_CASE("Counting Iterator") {
    CHECK(IdxRange{}.size() == 0);
    CHECK(IdxRange{}.empty());
    CHECK(IdxRange{3}.size() == 3);
    CHECK(*IdxRange{3}.begin() == 0);
    CHECK(*(IdxRange{3}.end() - 1) == 2);
    CHECK(IdxRange{1, 3}.size() == 2);
    CHECK(*IdxRange{1, 3}.begin() == 1);
    CHECK(*(IdxRange{1, 3}.end() - 1) == 2);
    CHECK(IdxRange{3}[0] == 0);
    CHECK(IdxRange{3}[1] == 1);
    CHECK(IdxRange{3}[2] == 2);
    CHECK(IdxRange{2}[0] == 0);
    CHECK(IdxRange{2}[1] == 1);
    CHECK(*IdxCount{0} == 0);
    CHECK(*IdxCount{2} == 2);
    CHECK(*(++IdxCount{0}) == 1);
    CHECK(*(IdxCount { 0 } ++) == 0);
    CHECK(*(IdxCount{0} + 1) == 1);
    CHECK(IdxRange{IdxRange{1, 3}.begin(), IdxRange{1, 3}.end()}.size() == 2);
    CHECK(*IdxRange{IdxRange{1, 3}.begin(), IdxRange{1, 3}.end()}.begin() == 1);
    CHECK(*(IdxRange{IdxRange{1, 3}.begin(), IdxRange{1, 3}.end()}.end() - 1) == 2);
}
} // namespace power_grid_model
