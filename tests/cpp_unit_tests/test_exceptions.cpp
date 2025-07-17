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
            CHECK(MissingCaseForEnumError{"test_foo", TestEnum_foo}.what() == doctest::Contains{"TestEnum #0"});
            CHECK(MissingCaseForEnumError{"test_bar", TestEnum_bar}.what() == doctest::Contains{"TestEnum #1"});
            CHECK(MissingCaseForEnumError{"test_baz", TestEnum_baz}.what() == doctest::Contains{"TestEnum #-1"});
            CHECK(MissingCaseForEnumError{"test_nan", TestEnum_nan}.what() == doctest::Contains{"TestEnum #-128"});
        }
        SUBCASE("C++-style enum class") {
            CHECK(MissingCaseForEnumError{"test_foo", TestEnumClass::foo}.what() ==
                  doctest::Contains{"test_foo is not implemented for "});
            CHECK(MissingCaseForEnumError{"test_foo", TestEnumClass::foo}.what() ==
                  doctest::Contains{"TestEnumClass #0"});
            CHECK(MissingCaseForEnumError{"test_bar", TestEnumClass::bar}.what() ==
                  doctest::Contains{"TestEnumClass #1"});
            CHECK(MissingCaseForEnumError{"test_baz", TestEnumClass::baz}.what() ==
                  doctest::Contains{"TestEnumClass #-1"});
            CHECK(MissingCaseForEnumError{"test_nan", TestEnumClass::nan}.what() ==
                  doctest::Contains{"TestEnumClass #-128"});
        }
    }
    SUBCASE("ConflictVoltage") {
        CHECK(std::string{ConflictVoltage{ID{0}, ID{1}, ID{2}, 1.0f, -1.0f}.what()} ==
              "Conflicting voltage for line 0\n voltage at from node 1 is 1\n voltage at to node 2 is -1\n");
        CHECK(std::string{ConflictVoltage{ID{0}, ID{1}, ID{2}, 1.5f, -1.5f}.what()} ==
              "Conflicting voltage for line 0\n voltage at from node 1 is 1.5\n voltage at to node 2 is -1.5\n");
        CHECK(std::string{ConflictVoltage{ID{0}, ID{1}, ID{2}, 1.0e5f, -1.0e5f}.what()} ==
              "Conflicting voltage for line 0\n voltage at from node 1 is 1e+05\n voltage at to node 2 is -1e+05\n");
        CHECK(std::string{ConflictVoltage{ID{na_IntID}, ID{na_IntID}, ID{na_IntID}, nan, -nan}.what()} ==
              "Conflicting voltage for line -2147483648\n voltage at from node -2147483648 is nan\n voltage at to node "
              "-2147483648 is -nan\n");
        CHECK(std::string{ConflictVoltage{ID{0}, ID{0}, ID{0}, std::numeric_limits<double>::infinity(),
                                          -std::numeric_limits<double>::infinity()}
                              .what()} ==
              "Conflicting voltage for line 0\n voltage at from node 0 is inf\n voltage at to node 0 is -inf\n");
    }
    SUBCASE("IterationDiverge") {
        REQUIRE(std::string{IterationDiverge{20, 1.0e20, 1.0e-8}.what()} ==
                "Iteration failed to converge after 20 iterations! Max deviation: 1e+20, error tolerance: 1e-08.\n");
    }
}
} // namespace
} // namespace power_grid_model
