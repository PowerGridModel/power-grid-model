// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/common/exception.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {
namespace {
enum class TestEnum : IntS {
    foo = 0,
    bar = 1,
    baz = -1,
    nan = na_IntS,
};

static_assert(static_cast<Idx>(TestEnum::foo) == 0);
static_assert(static_cast<Idx>(TestEnum::bar) == 1);
static_assert(static_cast<Idx>(TestEnum::baz) == -1);
static_assert(static_cast<Idx>(TestEnum::nan) == na_IntS);

TEST_CASE("Exceptions") {
    SUBCASE("MissingCaseForEnumError") {
        CHECK(MissingCaseForEnumError{"test_foo", TestEnum::foo}.what() ==
              doctest::Contains("test_foo is not implemented for "));
        CHECK(MissingCaseForEnumError{"test_foo", TestEnum::foo}.what() == doctest::Contains("TestEnum"));
        CHECK(MissingCaseForEnumError{"test_foo", TestEnum::foo}.what() == doctest::Contains("#0"));
        CHECK(MissingCaseForEnumError{"test_bar", TestEnum::bar}.what() == doctest::Contains("#1"));
        CHECK(MissingCaseForEnumError{"test_baz", TestEnum::baz}.what() == doctest::Contains("#-1"));
        CHECK(MissingCaseForEnumError{"test_nan", TestEnum::nan}.what() == doctest::Contains("#-128"));
    }
    SUBCASE("Iteration Diverge") {
        REQUIRE(std::string{IterationDiverge{20, 1.0e20, 1.0e-8}.what()} ==
                "Iteration failed to converge after 20 iterations! Max deviation: 1e+20, error tolerance: 1e-08.\n");
    }
}
} // namespace
} // namespace power_grid_model
