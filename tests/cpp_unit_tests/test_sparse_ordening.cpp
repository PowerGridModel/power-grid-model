// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/sparse_ordening.hpp>

#include <doctest/doctest.h>

#include <chrono>
#include <iostream>
#include <sstream>

namespace {
using power_grid_model::ID;
} // namespace

TEST_CASE("Test sparse ordening") {
    SUBCASE("minimum_degree_ordening") {
        std::map<ID, std::vector<ID>> graph{{0, {3, 5}}, {1, {4, 5, 8}}, {2, {4, 5, 6}}, {3, {6, 7}},
                                            {4, {6, 8}}, {6, {7, 8, 9}}, {7, {8, 9}},    {8, {9}}};

        auto const start = std::chrono::high_resolution_clock::now();
        std::vector<std::pair<std::vector<ID>, std::vector<std::pair<ID, ID>>>> const alpha_fills =
            power_grid_model::minimum_degree_ordering(graph);
        auto const stop = std::chrono::high_resolution_clock::now();

        auto const duration = duration_cast<std::chrono::microseconds>(stop - start);
        std::cout << "Time taken by function: " << duration.count() << " microseconds"
                  << "\n";

        CHECK(alpha_fills[0].first == std::vector<ID>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9});
        CHECK(alpha_fills[0].second == std::vector<std::pair<ID, ID>>{{3, 5}, {4, 5}, {8, 5}, {6, 5}, {7, 5}});
    }
}
