// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/common/exception.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {
namespace {
enum TestEnum {
    TestEnum_foo = 0,
    TestEnum_bar = 1,
    TestEnum_baz = -1,
    TestEnum_nan = na_IntS,
};

static_assert(static_cast<Idx>(TestEnum_foo) == 0);
static_assert(static_cast<Idx>(TestEnum_bar) == 1);
static_assert(static_cast<Idx>(TestEnum_baz) == -1);
static_assert(static_cast<Idx>(TestEnum_nan) == na_IntS);

enum class TestEnumClass : IntS {
    foo = 0,
    bar = 1,
    baz = -1,
    nan = na_IntS,
};

static_assert(static_cast<Idx>(TestEnumClass::foo) == 0);
static_assert(static_cast<Idx>(TestEnumClass::bar) == 1);
static_assert(static_cast<Idx>(TestEnumClass::baz) == -1);
static_assert(static_cast<Idx>(TestEnumClass::nan) == na_IntS);

TEST_CASE("Exceptions") {
    SUBCASE("MissingCaseForEnumError") {
        SUBCASE("C-style enum") {
            CHECK(MissingCaseForEnumError{"test_foo", TestEnum_foo}.what() ==
                  doctest::Contains("test_foo is not implemented for "));
            CHECK(MissingCaseForEnumError{"test_foo", TestEnum_foo}.what() == doctest::Contains("TestEnum #0"));
            CHECK(MissingCaseForEnumError{"test_bar", TestEnum_bar}.what() == doctest::Contains("TestEnum #1"));
            CHECK(MissingCaseForEnumError{"test_baz", TestEnum_baz}.what() == doctest::Contains("TestEnum #-1"));
            CHECK(MissingCaseForEnumError{"test_nan", TestEnum_nan}.what() == doctest::Contains("TestEnum #-128"));
        }
        SUBCASE("C++-style enum class") {
            CHECK(MissingCaseForEnumError{"test_foo", TestEnumClass::foo}.what() ==
                  doctest::Contains("test_foo is not implemented for "));
            CHECK(MissingCaseForEnumError{"test_foo", TestEnumClass::foo}.what() ==
                  doctest::Contains("TestEnumClass #0"));
            CHECK(MissingCaseForEnumError{"test_bar", TestEnumClass::bar}.what() ==
                  doctest::Contains("TestEnumClass #1"));
            CHECK(MissingCaseForEnumError{"test_baz", TestEnumClass::baz}.what() ==
                  doctest::Contains("TestEnumClass #-1"));
            CHECK(MissingCaseForEnumError{"test_nan", TestEnumClass::nan}.what() ==
                  doctest::Contains("TestEnumClass #-128"));
        }
    }
    SUBCASE("Iteration Diverge") {
        REQUIRE(std::string{IterationDiverge{20, 1.0e20, 1.0e-8}.what()} ==
                "Iteration failed to converge after 20 iterations! Max deviation: 1e+20, error tolerance: 1e-08.\n");
    }
}
} // namespace
} // namespace power_grid_model
