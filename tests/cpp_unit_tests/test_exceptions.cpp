// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/common/exception.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {
namespace {
enum TestCStyleEnum { // NOLINT(performance-enum-size) // NOSONAR // for testing C-style enum
    TestCStyleEnum_foo = 0,
    TestCStyleEnum_bar = 1,
    TestCStyleEnum_baz = -1,
    TestCStyleEnum_nan = na_IntS,
};

static_assert(static_cast<Idx>(TestCStyleEnum_foo) == 0);
static_assert(static_cast<Idx>(TestCStyleEnum_bar) == 1);
static_assert(static_cast<Idx>(TestCStyleEnum_baz) == -1);
static_assert(static_cast<Idx>(TestCStyleEnum_nan) == na_IntS);

enum class TestCppStyleEnumClass : IntS {
    foo = 0,
    bar = 1,
    baz = -1,
    nan = na_IntS,
};

static_assert(static_cast<Idx>(TestCppStyleEnumClass::foo) == 0);
static_assert(static_cast<Idx>(TestCppStyleEnumClass::bar) == 1);
static_assert(static_cast<Idx>(TestCppStyleEnumClass::baz) == -1);
static_assert(static_cast<Idx>(TestCppStyleEnumClass::nan) == na_IntS);

TEST_CASE("Exceptions") {
    SUBCASE("InvalidArguments") {
        CHECK(std::string{InvalidArguments{"foo"}.what()} == "foo");
        CHECK(std::string{InvalidArguments{"bar"}.what()} == "bar");
        CHECK(std::string{InvalidArguments{"foo", "bar"}.what()} == "foo is not implemented for bar!\n");

        InvalidArguments::TypeValuePair const foo{.name = "foo", .value = "baz"};
        InvalidArguments::TypeValuePair const bar{.name = "bar", .value = "bla"};
        CHECK(std::string{InvalidArguments{"Test method", foo}.what()} ==
              "Test method is not implemented for the following combination of options!\n"
              " foo: baz\n");
        CHECK(std::string{InvalidArguments{"Test method", foo, bar}.what()} ==
              "Test method is not implemented for the following combination of options!\n"
              " foo: baz\n"
              " bar: bla\n");
    }
    SUBCASE("MissingCaseForEnumError") {
        SUBCASE("C-style enum") {
            CHECK(MissingCaseForEnumError{"test_foo", TestCStyleEnum_foo}.what() ==
                  doctest::Contains("test_foo is not implemented for "));
            CHECK(MissingCaseForEnumError{"test_foo", TestCStyleEnum_foo}.what() ==
                  doctest::Contains{"TestCStyleEnum"});
            CHECK(MissingCaseForEnumError{"test_foo", TestCStyleEnum_foo}.what() == doctest::Contains{" #0"});
            CHECK(MissingCaseForEnumError{"test_bar", TestCStyleEnum_bar}.what() ==
                  doctest::Contains{"TestCStyleEnum"});
            CHECK(MissingCaseForEnumError{"test_bar", TestCStyleEnum_bar}.what() == doctest::Contains{" #1"});
            CHECK(MissingCaseForEnumError{"test_baz", TestCStyleEnum_baz}.what() ==
                  doctest::Contains{"TestCStyleEnum"});
            CHECK(MissingCaseForEnumError{"test_baz", TestCStyleEnum_baz}.what() == doctest::Contains{" #-1"});
            CHECK(MissingCaseForEnumError{"test_nan", TestCStyleEnum_nan}.what() ==
                  doctest::Contains{"TestCStyleEnum"});
            CHECK(MissingCaseForEnumError{"test_nan", TestCStyleEnum_nan}.what() == doctest::Contains{" #-128"});
        }
        SUBCASE("C++-style enum class") {
            CHECK(MissingCaseForEnumError{"test_foo", TestCppStyleEnumClass::foo}.what() ==
                  doctest::Contains{"test_foo is not implemented for "});
            CHECK(MissingCaseForEnumError{"test_foo", TestCppStyleEnumClass::foo}.what() ==
                  doctest::Contains{"TestCppStyleEnumClass"});
            CHECK(MissingCaseForEnumError{"test_foo", TestCppStyleEnumClass::foo}.what() == doctest::Contains{" #0"});
            CHECK(MissingCaseForEnumError{"test_bar", TestCppStyleEnumClass::bar}.what() ==
                  doctest::Contains{"TestCppStyleEnumClass"});
            CHECK(MissingCaseForEnumError{"test_bar", TestCppStyleEnumClass::bar}.what() == doctest::Contains{" #1"});
            CHECK(MissingCaseForEnumError{"test_baz", TestCppStyleEnumClass::baz}.what() ==
                  doctest::Contains{"TestCppStyleEnumClass"});
            CHECK(MissingCaseForEnumError{"test_baz", TestCppStyleEnumClass::baz}.what() == doctest::Contains{" #-1"});
            CHECK(MissingCaseForEnumError{"test_nan", TestCppStyleEnumClass::nan}.what() ==
                  doctest::Contains{"TestCppStyleEnumClass"});
            CHECK(MissingCaseForEnumError{"test_nan", TestCppStyleEnumClass::nan}.what() ==
                  doctest::Contains{" #-128"});
        }
    }
    SUBCASE("ConflictVoltage") {
        CHECK(std::string{ConflictVoltage{ID{0}, ID{1}, ID{2}, 1.0f, -1.0f}.what()} ==
              "Conflicting voltage for line 0\n voltage at from node 1 is 1\n voltage at to node 2 is -1\n");
        CHECK(std::string{ConflictVoltage{ID{0}, ID{1}, ID{2}, 1.5f, -1.5f}.what()} ==
              "Conflicting voltage for line 0\n voltage at from node 1 is 1.5\n voltage at to node 2 is -1.5\n");
        CHECK(std::string{ConflictVoltage{ID{0}, ID{1}, ID{2}, 1.0e5f, -1.0e5f}.what()} ==
              "Conflicting voltage for line 0\n voltage at from node 1 is 1e+05\n voltage at to node 2 is -1e+05\n");
        CHECK(std::string{ConflictVoltage{ID{0}, ID{1}, ID{2}, 1.0e3f, 1.0e8f}.what()} ==
              "Conflicting voltage for line 0\n voltage at from node 1 is 1000\n voltage at to node 2 is 1e+08\n");
        CHECK(std::string{ConflictVoltage{na_IntID, na_IntID, na_IntID, nan, -nan}.what()} ==
              "Conflicting voltage for line -2147483648\n voltage at from node -2147483648 is nan\n voltage at to node "
              "-2147483648 is -nan\n");
        CHECK(std::string{ConflictVoltage{ID{0}, ID{0}, ID{0}, std::numeric_limits<double>::infinity(),
                                          -std::numeric_limits<double>::infinity()}
                              .what()} ==
              "Conflicting voltage for line 0\n voltage at from node 0 is inf\n voltage at to node 0 is -inf\n");
    }
    SUBCASE("InvalidBranch") {
        CHECK(std::string{InvalidBranch{ID{0}, ID{1}}.what()} ==
              "Branch 0 has the same from- and to-node 1,\n This is not allowed!\n");
        CHECK(std::string{InvalidBranch{na_IntID, na_IntID}.what()} ==
              "Branch -2147483648 has the same from- and to-node -2147483648,\n This is not allowed!\n");
    }
    SUBCASE("InvalidBranch3") {
        CHECK(std::string{InvalidBranch3{ID{0}, ID{4}, ID{5}, ID{6}}.what()} ==
              "Branch3 0 is connected to the same node at least twice. Node 1/2/3: 4/5/6,\n This is not allowed!\n");
        CHECK(std::string{InvalidBranch3{na_IntID, na_IntID, na_IntID, na_IntID}.what()} ==
              "Branch3 -2147483648 is connected to the same node at least twice. Node 1/2/3: "
              "-2147483648/-2147483648/-2147483648,\n This is not allowed!\n");
    }
    SUBCASE("InvalidTransformerClock") {
        CHECK(std::string{InvalidTransformerClock{ID{0}, IntS{1}}.what()} ==
              "Invalid clock for transformer 0, clock 1\n");
        CHECK(std::string{InvalidTransformerClock{na_IntID, na_IntS}.what()} ==
              "Invalid clock for transformer -2147483648, clock -128\n");
    }
    SUBCASE("SparseMatrixError") {
        CHECK(std::string{SparseMatrixError{}.what()} ==
              "Sparse matrix error, possibly singular matrix!\n"
              "If you get this error from state estimation, it might mean the system is not fully observable, i.e. not "
              "enough measurements.\n"
              "It might also mean that you are running into a corner case where PGM cannot resolve yet.\n"
              "See https://github.com/PowerGridModel/power-grid-model/issues/864.");
        CHECK(std::string{SparseMatrixError{1}.what()} ==
              "Sparse matrix error with error code #1 (possibly singular)\n"
              "If you get this error from state estimation, it usually means the system is not fully observable, "
              "i.e. not enough measurements.");
        CHECK(std::string{SparseMatrixError{1, "Test error message"}.what()} ==
              "Sparse matrix error with error code #1 (possibly singular)\nTest error message\n"
              "If you get this error from state estimation, it usually means the system is not fully observable, "
              "i.e. not enough measurements.");
    }
    SUBCASE("NotObservableError") {
        CHECK(std::string{NotObservableError{}.what()} == "Not enough measurements available for state estimation.\n");
        CHECK(std::string{NotObservableError{"Test error message"}.what()} ==
              "Not enough measurements available for state estimation.\nTest error message\n");
    }
    SUBCASE("IterationDiverge") {
        CHECK(std::string{IterationDiverge{"Test error message"}.what()} == "Test error message");
        CHECK(std::string{IterationDiverge{20, 1.0e20, 1.0e-8}.what()} ==
              "Iteration failed to converge after 20 iterations! Max deviation: 1e+20, error tolerance: 1e-08.\n");
    }
    SUBCASE("MaxIterationReached") {
        CHECK(std::string{MaxIterationReached{}.what()} == "Maximum number of iterations reached! \n");
        CHECK(std::string{MaxIterationReached{"Test error message"}.what()} ==
              "Maximum number of iterations reached! Test error message\n");
    }
    SUBCASE("ConflictID") {
        CHECK(std::string{ConflictID{ID{}}.what()} == "Conflicting id detected: 0\n");
        CHECK(std::string{ConflictID{ID{1}}.what()} == "Conflicting id detected: 1\n");
        CHECK(std::string{ConflictID{ID{std::numeric_limits<ID>::max()}}.what()} ==
              "Conflicting id detected: 2147483647\n");
        CHECK(std::string{ConflictID{na_IntID}.what()} == "Conflicting id detected: -2147483648\n");
    }
    SUBCASE("IDNotFound") {
        CHECK(std::string{IDNotFound{ID{}}.what()} == "The id cannot be found: 0\n");
        CHECK(std::string{IDNotFound{ID{1}}.what()} == "The id cannot be found: 1\n");
        CHECK(std::string{IDNotFound{ID{std::numeric_limits<ID>::max()}}.what()} ==
              "The id cannot be found: 2147483647\n");
        CHECK(std::string{IDNotFound{na_IntID}.what()} == "The id cannot be found: -2147483648\n");
    }
    SUBCASE("Idx2DNotFound") {
        CHECK(std::string{Idx2DNotFound{Idx2D{.group = ID{}, .pos = ID{}}}.what()} ==
              "The idx 2d cannot be found: {0, 0}.\n");
        CHECK(std::string{Idx2DNotFound{Idx2D{.group = ID{1}, .pos = ID{2}}}.what()} ==
              "The idx 2d cannot be found: {1, 2}.\n");
        CHECK(std::string{Idx2DNotFound{
                  Idx2D{.group = ID{std::numeric_limits<ID>::max()}, .pos = ID{std::numeric_limits<ID>::max()}}}
                              .what()} == "The idx 2d cannot be found: {2147483647, 2147483647}.\n");
        CHECK(std::string{Idx2DNotFound{Idx2D{.group = na_IntID, .pos = na_IntID}}.what()} ==
              "The idx 2d cannot be found: {-2147483648, -2147483648}.\n");
    }
    SUBCASE("InvalidMeasuredObject") {
        CHECK(std::string{InvalidMeasuredObject{"foo", "bar"}.what()} ==
              "bar measurement is not supported for object of type foo");
    }
    SUBCASE("InvalidMeasuredTerminalType") {
        CHECK(std::string{InvalidMeasuredTerminalType{MeasuredTerminalType::branch_from, "foo"}.what()} ==
              "foo measurement is not supported for object of type 0");
        CHECK(std::string{InvalidMeasuredTerminalType{MeasuredTerminalType::branch_to, "foo"}.what()} ==
              "foo measurement is not supported for object of type 1");
        CHECK(std::string{InvalidMeasuredTerminalType{MeasuredTerminalType::source, "foo"}.what()} ==
              "foo measurement is not supported for object of type 2");
        CHECK(std::string{InvalidMeasuredTerminalType{MeasuredTerminalType::shunt, "foo"}.what()} ==
              "foo measurement is not supported for object of type 3");
        CHECK(std::string{InvalidMeasuredTerminalType{MeasuredTerminalType::load, "foo"}.what()} ==
              "foo measurement is not supported for object of type 4");
        CHECK(std::string{InvalidMeasuredTerminalType{MeasuredTerminalType::generator, "foo"}.what()} ==
              "foo measurement is not supported for object of type 5");
        CHECK(std::string{InvalidMeasuredTerminalType{MeasuredTerminalType::branch3_1, "foo"}.what()} ==
              "foo measurement is not supported for object of type 6");
        CHECK(std::string{InvalidMeasuredTerminalType{MeasuredTerminalType::branch3_2, "foo"}.what()} ==
              "foo measurement is not supported for object of type 7");
        CHECK(std::string{InvalidMeasuredTerminalType{MeasuredTerminalType::branch3_3, "foo"}.what()} ==
              "foo measurement is not supported for object of type 8");
        CHECK(std::string{InvalidMeasuredTerminalType{MeasuredTerminalType::node, "foo"}.what()} ==
              "foo measurement is not supported for object of type 9");
        CHECK(std::string{InvalidMeasuredTerminalType{static_cast<MeasuredTerminalType>(na_IntS), "foo"}.what()} ==
              "foo measurement is not supported for object of type -128");
    }
    SUBCASE("InvalidRegulatedObject") {
        CHECK(std::string{InvalidRegulatedObject{"foo", "bar"}.what()} ==
              "bar regulator is not supported for object of type foo");
        CHECK(std::string{InvalidRegulatedObject{ID{1}, "bar"}.what()} ==
              "bar regulator is not supported for object with ID 1");
    }
    SUBCASE("DuplicativelyRegulatedObject") {
        CHECK(std::string{DuplicativelyRegulatedObject{}.what()} ==
              "There are objects regulated by more than one regulator. Maximum one regulator is allowed.");
    }
    SUBCASE("AutomaticTapCalculationError") {
        CHECK(std::string{AutomaticTapCalculationError{ID{1}}.what()} ==
              "Automatic tap changing regulator with tap_side at LV side is not supported. Found at id 1");
        CHECK(std::string{AutomaticTapCalculationError{na_IntID}.what()} ==
              "Automatic tap changing regulator with tap_side at LV side is not supported. Found at id -2147483648");
    }
    SUBCASE("AutomaticTapInputError") {
        CHECK(std::string{AutomaticTapInputError{"foo"}.what()} ==
              "Automatic tap changer has invalid configuration. foo");
    }
    SUBCASE("IDWrongType") {
        CHECK(std::string{IDWrongType{ID{1}}.what()} == "Wrong type for object with id 1\n");
        CHECK(std::string{IDWrongType{na_IntID}.what()} == "Wrong type for object with id -2147483648\n");
    }
    SUBCASE("ConflictingAngleMeasurementType") {
        CHECK(std::string{ConflictingAngleMeasurementType{"foo"}.what()} == "Conflicting angle measurement type. foo");
    }
    SUBCASE("CalculationError") { CHECK(std::string{CalculationError{"foo"}.what()} == "foo"); }
    SUBCASE("BatchCalculationError") {
        IdxVector const failed_scenarios{1, 2, 3, na_Idx};
        std::vector<std::string> const err_msgs{"Error 1", "Error 2", "Error 3", "Error 4"};
        BatchCalculationError const error{"Batch error", failed_scenarios, err_msgs};
        CHECK(std::string{error.what()} == "Batch error");
        CHECK(error.failed_scenarios() == failed_scenarios);
        CHECK(error.err_msgs() == err_msgs);
    }
    SUBCASE("InvalidCalculationMethod") {
        CHECK(std::string{InvalidCalculationMethod{}.what()} ==
              "The calculation method is invalid for this calculation!");
    }
    SUBCASE("InvalidShortCircuitType") {
        CHECK(std::string{InvalidShortCircuitType{FaultType::three_phase}.what()} ==
              "The short circuit type (0) is invalid!\n");
        CHECK(std::string{InvalidShortCircuitType{FaultType::single_phase_to_ground}.what()} ==
              "The short circuit type (1) is invalid!\n");
        CHECK(std::string{InvalidShortCircuitType{FaultType::two_phase}.what()} ==
              "The short circuit type (2) is invalid!\n");
        CHECK(std::string{InvalidShortCircuitType{FaultType::two_phase_to_ground}.what()} ==
              "The short circuit type (3) is invalid!\n");
        CHECK(std::string{InvalidShortCircuitType{FaultType::nan}.what()} ==
              "The short circuit type (-128) is invalid!\n");
        CHECK(std::string{InvalidShortCircuitType{true, FaultType::three_phase}.what()} ==
              "The short circuit type (0) does not match the calculation type (symmetric=1)\n");
        CHECK(std::string{InvalidShortCircuitType{false, FaultType::three_phase}.what()} ==
              "The short circuit type (0) does not match the calculation type (symmetric=0)\n");
    }
    SUBCASE("InvalidShortCircuitPhases") {
        CHECK(std::string{InvalidShortCircuitPhases{FaultType::three_phase, FaultPhase::abc}.what()} ==
              "The short circuit phases (0) do not match the short circuit type (0)\n");
        CHECK(std::string{InvalidShortCircuitPhases{FaultType::three_phase, FaultPhase::a}.what()} ==
              "The short circuit phases (1) do not match the short circuit type (0)\n");
        CHECK(std::string{InvalidShortCircuitPhases{FaultType::three_phase, FaultPhase::b}.what()} ==
              "The short circuit phases (2) do not match the short circuit type (0)\n");
        CHECK(std::string{InvalidShortCircuitPhases{FaultType::three_phase, FaultPhase::c}.what()} ==
              "The short circuit phases (3) do not match the short circuit type (0)\n");
        CHECK(std::string{InvalidShortCircuitPhases{FaultType::three_phase, FaultPhase::ab}.what()} ==
              "The short circuit phases (4) do not match the short circuit type (0)\n");
        CHECK(std::string{InvalidShortCircuitPhases{FaultType::three_phase, FaultPhase::ac}.what()} ==
              "The short circuit phases (5) do not match the short circuit type (0)\n");
        CHECK(std::string{InvalidShortCircuitPhases{FaultType::three_phase, FaultPhase::bc}.what()} ==
              "The short circuit phases (6) do not match the short circuit type (0)\n");
        CHECK(std::string{InvalidShortCircuitPhases{FaultType::three_phase, FaultPhase::nan}.what()} ==
              "The short circuit phases (-128) do not match the short circuit type (0)\n");
        CHECK(std::string{InvalidShortCircuitPhases{FaultType::single_phase_to_ground, FaultPhase::abc}.what()} ==
              "The short circuit phases (0) do not match the short circuit type (1)\n");
        CHECK(std::string{InvalidShortCircuitPhases{FaultType::two_phase, FaultPhase::abc}.what()} ==
              "The short circuit phases (0) do not match the short circuit type (2)\n");
        CHECK(std::string{InvalidShortCircuitPhases{FaultType::two_phase_to_ground, FaultPhase::abc}.what()} ==
              "The short circuit phases (0) do not match the short circuit type (3)\n");
    }
    SUBCASE("InvalidShortCircuitPhaseOrType") {
        CHECK(std::string{InvalidShortCircuitPhaseOrType{}.what()} ==
              "During one calculation the short circuit types phases should be similar for all faults\n");
    }
    SUBCASE("SerializationError") {
        CHECK(std::string{SerializationError{"Test serialization error"}.what()} == "Test serialization error");
    }
    SUBCASE("DatasetError") {
        CHECK(std::string{DatasetError{"Test dataset error"}.what()} == "Dataset error: Test dataset error");
    }
    SUBCASE("ExperimentalFeature") {
        CHECK(std::string{ExperimentalFeature{"foo"}.what()} == "foo");
        CHECK(std::string{ExperimentalFeature{"bar"}.what()} == "bar");
        CHECK(std::string{ExperimentalFeature{"foo", "bar"}.what()} == "foo is not implemented for bar!\n");

        ExperimentalFeature::TypeValuePair const foo{.name = "foo", .value = "baz"};
        ExperimentalFeature::TypeValuePair const bar{.name = "bar", .value = "bla"};
        CHECK(std::string{ExperimentalFeature{"Test method", foo}.what()} ==
              "Test method is not implemented for the following combination of options!\n"
              " foo: baz\n");
        CHECK(std::string{ExperimentalFeature{"Test method", foo, bar}.what()} ==
              "Test method is not implemented for the following combination of options!\n"
              " foo: baz\n"
              " bar: bla\n");
    }
    SUBCASE("NotImplementedError") {
        CHECK(std::string{NotImplementedError{}.what()} == "Function not yet implemented");
    }
    SUBCASE("UnreachableHit") {
        CHECK(std::string{UnreachableHit{"foo", "bar"}.what()} ==
              "Unreachable code hit when executing foo.\n The following assumption for unreachability "
              "was not met: bar.\n This may be a bug in the library\n");
    }
    SUBCASE("TapSearchStrategyIncompatibleError") {
        auto const foo_error =
            TapSearchStrategyIncompatibleError{"foo_error", TestCppStyleEnumClass::foo, TestCppStyleEnumClass::foo};
        CHECK(foo_error.what() == doctest::Contains("foo_error is not implemented for "));
        CHECK(foo_error.what() == doctest::Contains("TestCppStyleEnumClass"));
        CHECK(foo_error.what() == doctest::Contains(" #0 and "));
        CHECK(foo_error.what() == doctest::Contains(" #0!\n"));

        auto const bar_error =
            TapSearchStrategyIncompatibleError{"bar_error", TestCppStyleEnumClass::bar, TestCStyleEnum_foo};
        CHECK(bar_error.what() == doctest::Contains("bar_error is not implemented for "));
        CHECK(bar_error.what() == doctest::Contains("TestCppStyleEnumClass"));
        CHECK(bar_error.what() == doctest::Contains(" #1 and "));
        CHECK(bar_error.what() == doctest::Contains("TestCStyleEnum"));
        CHECK(bar_error.what() == doctest::Contains(" #0!\n"));

        auto const baz_error = TapSearchStrategyIncompatibleError{"baz_error", TestCStyleEnum_bar, TestCStyleEnum_bar};
        CHECK(baz_error.what() == doctest::Contains("baz_error is not implemented for "));
        CHECK(baz_error.what() == doctest::Contains("TestCStyleEnum"));
        CHECK(baz_error.what() == doctest::Contains(" #1 and "));
        CHECK(baz_error.what() == doctest::Contains(" #1!\n"));
    }
}
} // namespace
} // namespace power_grid_model
