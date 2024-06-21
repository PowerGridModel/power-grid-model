// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/common/exception.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {
namespace {
TEST_CASE("Exceptions") {
    SUBCASE("Iteration Diverge") {
        REQUIRE(std::string{IterationDiverge{20, 1.0e20, 1.0e-8}.what()} ==
                "Iteration failed to converge after 20 iterations! Max deviation: 1e+20, error tolerance: 1e-08.\n");
    }
}
} // namespace
} // namespace power_grid_model
