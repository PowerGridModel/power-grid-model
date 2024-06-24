// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/sparse_ordering.hpp>

#include <doctest/doctest.h>

#include <chrono>
#include <iostream>
#include <sstream>

namespace {
using power_grid_model::Idx;
} // namespace

TEST_CASE("Test sparse ordering") {
    SUBCASE("minimum_degree_ordering") {
        std::map<Idx, std::vector<Idx>> graph{{0, {3, 5}}, {1, {4, 5, 8}}, {2, {4, 5, 6}}, {3, {6, 7}},
                                              {4, {6, 8}}, {6, {7, 8, 9}}, {7, {8, 9}},    {8, {9}}};

        auto const start = std::chrono::high_resolution_clock::now();
        auto const [alpha, fills] = power_grid_model::minimum_degree_ordering(std::move(graph));
        auto const stop = std::chrono::high_resolution_clock::now();

        auto const duration = duration_cast<std::chrono::microseconds>(stop - start);
        std::cout << "Time taken by function: " << duration.count() << " microseconds\n";

        CHECK(alpha == std::vector<Idx>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9});
        CHECK(fills == std::vector<std::pair<Idx, Idx>>{{3, 5}, {4, 5}, {5, 8}, {5, 6}, {5, 7}});
    }
}
