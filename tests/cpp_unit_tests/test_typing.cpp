// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/typing.hpp>

#include <doctest/doctest.h>

#include <algorithm>

namespace {
using namespace power_grid_model;

// Force static_cast without any Sonar Cloud warnings for redundant casts
template <typename T> constexpr auto force_static_cast(auto value) {
    return static_cast<T>(value); // NOSONAR
}

template <typename T, typename U> void check_narrow_cast() {
    static_assert(std::same_as<decltype(narrow_cast<T>(U{})), T>);

    using common_t = std::common_type_t<T, U>;

    auto constexpr neutral_common = common_t{};
    auto constexpr lowest_common = std::max(force_static_cast<common_t>(std::numeric_limits<T>::min()),
                                            force_static_cast<common_t>(std::numeric_limits<U>::min()));
    auto constexpr highest_common = std::min(force_static_cast<common_t>(std::numeric_limits<T>::max()),
                                             force_static_cast<common_t>(std::numeric_limits<U>::max()));

    for (auto value : {neutral_common, lowest_common, highest_common}) {
        CAPTURE(value);
        CHECK(narrow_cast<T>(force_static_cast<U>(value)) == force_static_cast<T>(value));
    }
}

template <typename T> void check_narrow_cast() {
    static_assert(std::same_as<decltype(narrow_cast<T>(T{})), T>);
    for (auto value : {T{}, std::numeric_limits<T>::min(), std::numeric_limits<T>::max()}) {
        CAPTURE(value);
        CHECK(narrow_cast<T>(value) == value);
    }
}

TEST_CASE("narrow_cast") {
    SUBCASE("Identical types") {
        check_narrow_cast<Idx>();
        check_narrow_cast<ID>();
        check_narrow_cast<uint8_t>();
        check_narrow_cast<int32_t>();
        check_narrow_cast<int64_t>();
    }

    SUBCASE("Different types (in range)") {
        check_narrow_cast<Idx, ID>();
        check_narrow_cast<ID, Idx>();
        check_narrow_cast<uint8_t, Idx>();
        check_narrow_cast<Idx, uint8_t>();
        check_narrow_cast<uint8_t, ID>();
        check_narrow_cast<ID, uint8_t>();

        check_narrow_cast<int32_t, int64_t>();
        check_narrow_cast<int64_t, int32_t>();
    }
}
} // namespace
