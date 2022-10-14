// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include "doctest/doctest.h"
#include "power_grid_model/container.hpp"

namespace power_grid_model {

struct C {
    C(Idx a1) : a{a1} {
    }

    Idx a;
};

struct C1 : C {
    C1(Idx a1, double b1) : C{a1}, b{b1} {
    }
    double b;
};

struct C2 : C {
    C2(Idx a1, uint16_t b1) : C{a1}, b{b1} {
    }
    uint16_t b;
};

TEST_CASE("Test component container") {
    using CompContainer = Container<C, C1, C2>;
    CompContainer container;
    container.reserve<C>(3);
    container.reserve<C1>(2);
    container.reserve<C2>(1);
    using CompContainer2 = Container<ExtraRetrievableTypes<C>, C1, C2>;
    CompContainer2 container2;

    container.emplace<C>(1, 5);
    container.emplace<C>(11, 55);
    container.emplace<C>(111, 555);
    container.emplace<C1>(2, 6, 60);
    container.emplace<C1>(22, 66, 660);
    container.emplace<C2>(3, 7, 70);
    container.set_construction_complete();
    container2.emplace<C1>(2, 6, 60);
    container2.emplace<C1>(22, 66, 660);
    container2.emplace<C2>(3, 7, 70);
    container2.set_construction_complete();

    auto const& const_container = container;
    auto const& const_container2 = container2;

    SUBCASE("Test start index") {
        CHECK(const_container.get_start_idx<C, C1>() == 3);
        CHECK(const_container.get_start_idx<C, C2>() == 5);
        CHECK(const_container.get_start_idx<C, C>() == 0);
    }

    SUBCASE("Test iteration") {
        Idx i = 0;
        for (C& c : container.iter<C>()) {
            c.a = i;
            i++;
        }
        i = 0;
        for (C const& c : container.citer<C>()) {
            CHECK(c.a == i);
            i++;
        }
        i = 0;
        for (C const& c : const_container.iter<C>()) {
            CHECK(c.a == i);
            i++;
        }
        auto it_begin = container.iter<C>().begin();
        auto it_end = container.iter<C>().end();
        auto const_it_begin = const_container.iter<C>().begin();
        auto const_it_end = const_container.iter<C>().end();
        CHECK(it_begin != const_it_end);
        CHECK(it_begin == const_it_begin);
        CHECK(it_begin < const_it_end);
        CHECK(it_begin <= const_it_end);
        CHECK(it_end > const_it_begin);
        CHECK(it_end >= const_it_begin);
        CHECK((it_end - it_begin) == 6);
        CHECK((it_end - const_it_begin) == 6);
        CHECK((it_end == (const_it_begin + 6)));
        CHECK(((const_it_end - 6) == it_begin));
    }

    SUBCASE("Test get item by idx_2d") {
        C const& c = const_container.get_item<C>({0, 0});
        C const& c1 = const_container.get_item<C>({1, 0});
        C const& c2 = const_container.get_item<C2>({2, 0});
        CHECK(c.a == 5);
        CHECK(c1.a == 6);
        CHECK(c2.a == 7);
    }

    SUBCASE("Test get item by id") {
        C const& c = const_container.get_item<C>(1);
        C const& c1 = const_container.get_item<C>(2);
        C const& c2 = const_container.get_item<C2>(3);
        CHECK(c.a == 5);
        CHECK(c1.a == 6);
        CHECK(c2.a == 7);
        CHECK_THROWS_AS(container.get_item<C2>(2), IDWrongType);
        CHECK_THROWS_AS(container.get_item<C>(8), IDNotFound);
    }

    SUBCASE("Test size of a component class collection") {
        CHECK(const_container.size<C>() == 6);
        CHECK(const_container.size<C1>() == 2);
        CHECK(const_container.size<C2>() == 1);
    }

    SUBCASE("Test get sequence based on id") {
        CHECK(const_container.get_seq<C>(1) == 0);
        CHECK(const_container.get_seq<C>(11) == 1);
        CHECK(const_container.get_seq<C>(111) == 2);
        CHECK(const_container.get_seq<C>(2) == 3);
        CHECK(const_container.get_seq<C>(22) == 4);
        CHECK(const_container.get_seq<C>(3) == 5);
        CHECK(const_container.get_seq<C1>(2) == 0);
        CHECK(const_container.get_seq<C1>(22) == 1);
        CHECK(const_container.get_seq<C2>(3) == 0);
    }

    SUBCASE("Test get idx_2d based on sequence") {
        CHECK(const_container.get_idx_2d_by_seq<C>(0) == Idx2D{0, 0});
        CHECK(const_container.get_idx_2d_by_seq<C>(1) == Idx2D{0, 1});
        CHECK(const_container.get_idx_2d_by_seq<C>(2) == Idx2D{0, 2});
        CHECK(const_container.get_idx_2d_by_seq<C>(3) == Idx2D{1, 0});
        CHECK(const_container.get_idx_2d_by_seq<C>(4) == Idx2D{1, 1});
        CHECK(const_container.get_idx_2d_by_seq<C>(5) == Idx2D{2, 0});
        CHECK(const_container.get_idx_2d_by_seq<C1>(0) == Idx2D{1, 0});
        CHECK(const_container.get_idx_2d_by_seq<C1>(1) == Idx2D{1, 1});
        CHECK(const_container.get_idx_2d_by_seq<C2>(0) == Idx2D{2, 0});
    }

    SUBCASE("Test get component based on sequence") {
        CHECK(const_container.get_item_by_seq<C>(0).a == 5);
        CHECK(const_container.get_item_by_seq<C>(1).a == 55);
        CHECK(const_container.get_item_by_seq<C>(2).a == 555);
        CHECK(const_container.get_item_by_seq<C>(3).a == 6);
        CHECK(const_container.get_item_by_seq<C>(4).a == 66);
        CHECK(const_container.get_item_by_seq<C>(5).a == 7);
        CHECK(const_container.get_item_by_seq<C1>(0).b == 60);
        CHECK(const_container.get_item_by_seq<C1>(1).b == 660);
        CHECK(const_container.get_item_by_seq<C2>(0).b == 70);
    }

    SUBCASE("Test only one retrievable type") {
        CHECK(const_container2.get_seq<C>(2) == 0);
        CHECK(const_container2.get_seq<C>(22) == 1);
        CHECK(const_container2.get_seq<C>(3) == 2);

        CHECK(const_container2.get_idx_2d_by_seq<C>(0) == Idx2D{0, 0});
        CHECK(const_container2.get_idx_2d_by_seq<C>(1) == Idx2D{0, 1});
        CHECK(const_container2.get_idx_2d_by_seq<C>(2) == Idx2D{1, 0});

        CHECK(const_container2.get_item_by_seq<C>(0).a == 6);
        CHECK(const_container2.get_item_by_seq<C>(1).a == 66);
        CHECK(const_container2.get_item_by_seq<C>(2).a == 7);
    }
}

}  // namespace power_grid_model