// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/common/common.hpp>
#include <power_grid_model/common/maybe_owning_view.hpp>

#include <doctest/doctest.h>

#include <algorithm>
#include <array>
#include <unordered_map>

namespace power_grid_model {
namespace {
static_assert(std::ranges::viewable_range<
              maybe_owning_view<std::unordered_map<Idx, Idx>, std::views::all_t<std::unordered_map<Idx, Idx>>>>);
static_assert(std::ranges::viewable_range<
              maybe_owning_view<std::unordered_map<Idx, Idx>, std::views::all_t<std::unordered_map<Idx, Idx> const&>>>);
static_assert(detail::is_owning_view<MaybeOwningVector<Idx>>::value);
static_assert(detail::is_owning_view<MaybeOwningConstVector<Idx>>::value);
static_assert(!non_owning_view_c<MaybeOwningVector<Idx>>);
static_assert(!non_owning_view_c<MaybeOwningConstVector<Idx>>);
static_assert(owning_container_c<MaybeOwningVector<Idx>>);
static_assert(owning_container_c<MaybeOwningConstVector<Idx>>);

static_assert(std::convertible_to<MaybeOwningVector<Idx>, std::span<Idx>>);
static_assert(std::convertible_to<MaybeOwningConstVector<Idx>, std::span<Idx const>>);

template <typename T>
concept convertible_to_span_c = requires(T t) {
    { std::span{t} };
};

static_assert(convertible_to_span_c<MaybeOwningVector<Idx>>);
static_assert(convertible_to_span_c<MaybeOwningConstVector<Idx>>);
// can't use convertible_to here because the type is ill-defined
static_assert(!convertible_to_span_c<
              maybe_owning_view<std::unordered_map<Idx, Idx>, std::views::all_t<std::unordered_map<Idx, Idx>>>>);
} // namespace

TEST_CASE("maybe_owning_view") {
    SUBCASE("MaybeOwningVector") {
        SUBCASE("By view") {
            std::vector<Idx> vec{1, 2, 3};
            for (auto&& view : std::array{MaybeOwningVector<Idx>{vec}, MaybeOwningVector<Idx>{std::span{vec}},
                                          MaybeOwningVector<Idx>{std::span{vec.data(), vec.size()}}}) {
                CHECK(view.size() == 3);
                CHECK(std::ranges::equal(view, vec));

                SUBCASE("It is taken by view") {
                    vec[0] = 4;
                    CHECK(std::ranges::equal(view, vec));
                }
                SUBCASE("The view is random access and mutable") {
                    view[1] = 5;
                    CHECK(view[1] == 5);
                    CHECK(std::ranges::equal(view, vec));
                }
                SUBCASE("The view can be passed to all") {
                    CHECK(std::ranges::equal(std::views::all(by_ref(view)), vec));
                    CHECK(std::ranges::equal(std::views::all(by_const_ref(view)), vec));
                }
                SUBCASE("Conversion to span") { CHECK(std::ranges::equal(std::span{view}, vec)); }
            }
        }

        SUBCASE("By move") {
            // Test move semantics
            std::vector<Idx> vec{1, 2, 3};
            MaybeOwningVector<Idx> view{vec | std::ranges::to<IdxVector>()};
            CHECK(view.size() == 3);
            CHECK(std::ranges::equal(view, vec));

            SUBCASE("The view is taken by value") {
                vec[0] = 4;
                CHECK(!std::ranges::equal(view, vec));
            }
            SUBCASE("The view is random access and mutable") {
                view[1] = 5;
                CHECK(view[1] == 5);
                CHECK(!std::ranges::equal(view, vec));
            }
            SUBCASE("The view can be passed to all") {
                CHECK(std::ranges::equal(std::views::all(by_ref(view)), vec));
                CHECK(std::ranges::equal(std::views::all(by_const_ref(view)), vec));
                CHECK(std::ranges::equal(std::views::all(by_const_ref(std::as_const(view))), vec));
            }
            SUBCASE("Conversion to span") { CHECK(std::ranges::equal(std::span{view}, vec)); }
        }
    }
    SUBCASE("MaybeOwningConstVector") {
        std::vector<Idx> vec{1, 2, 3};
        SUBCASE("By view") {
            std::vector<Idx> const& vec_cref{vec};
            for (auto&& view :
                 std::array{MaybeOwningConstVector<Idx>{vec_cref}, MaybeOwningConstVector<Idx>{std::span{vec_cref}},
                            MaybeOwningConstVector<Idx>{std::span{vec_cref.data(), vec_cref.size()}}}) {
                CHECK(view.size() == 3);
                CHECK(std::ranges::equal(view, vec_cref));

                SUBCASE("It is taken by view") {
                    vec.at(0) = 4;
                    CHECK(std::ranges::equal(view, vec_cref));
                }
                SUBCASE("The view is random access but const") {
                    static_assert(std::is_const_v<std::remove_reference_t<decltype(view[1])>>);
                    CHECK(view[1] == vec_cref[1]);
                }
                SUBCASE("The view can be passed to all") {
                    CHECK(std::ranges::equal(std::views::all(by_ref(view)), vec));
                    CHECK(std::ranges::equal(std::views::all(by_const_ref(view)), vec));
                }
                SUBCASE("Conversion to span") { CHECK(std::ranges::equal(std::span{view}, vec)); }
            }
        }

        SUBCASE("By value") {
            MaybeOwningConstVector<Idx> view{vec | std::ranges::to<IdxVector>()};
            CHECK(view.size() == 3);
            SUBCASE("The view is taken by value") {
                vec[0] = 4;
                CHECK(!std::ranges::equal(view, vec));
            }
            SUBCASE("The view is random access but const") {
                static_assert(std::is_const_v<std::remove_reference_t<decltype(view[1])>>);
                CHECK(view[1] == 2);
            }
            SUBCASE("The view can be passed to all") {
                CHECK(std::ranges::equal(std::views::all(by_ref(view)), vec));
                CHECK(std::ranges::equal(std::views::all(by_const_ref(view)), vec));
            }
            SUBCASE("Conversion to span") { CHECK(std::ranges::equal(std::span{view}, vec)); }
        }
    }
}
} // namespace power_grid_model
