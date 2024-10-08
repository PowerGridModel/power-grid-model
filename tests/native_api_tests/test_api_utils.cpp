// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model_cpp/utils.hpp>

#include <doctest/doctest.h>

namespace power_grid_model_cpp {
TEST_CASE("API Utils") {
    SUBCASE("NaN values and checks") {
        CHECK_FALSE(is_nan(ID{}));
        CHECK_FALSE(is_nan(IntS{}));
        CHECK_FALSE(is_nan(std::complex<double>{}));
        CHECK_FALSE(is_nan(std::array<double, 3>{}));
        CHECK_FALSE(is_nan(std::array<std::complex<double>, 3>{}));

        CHECK(is_nan(nan));
        CHECK(is_nan(na_IntS));
        CHECK(is_nan(na_IntID));

        CHECK(is_nan(nan_value<double>()));
        CHECK(is_nan(nan_value<std::array<double, 3>>()));
        CHECK(is_nan(nan_value<ID>()));
        CHECK(is_nan(nan_value<IntS>()));
    }
}
} // namespace power_grid_model_cpp
