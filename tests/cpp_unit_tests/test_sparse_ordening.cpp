// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/sparse_ordening.hpp>

#include <doctest/doctest.h>

#include <chrono>
#include <iostream>
#include <sstream>

namespace {
using namespace power_grid_model;

auto to_string(std::vector<std::pair<ID, ID>> const& d) {
    std::stringstream sstr;
    sstr << "{";
    for (const auto& it : d) {
        sstr << it.first << ": " << it.second << ", ";
    }
    sstr << "}";
    return sstr.str();
}

auto to_string(std::vector<int> const& input) {
    std::stringstream sstr;
    for (auto const& i : input) {
        sstr << i << ", ";
    }
    return sstr.str();
}
} // namespace

TEST_CASE("Test sparse ordening") {
    SUBCASE("minimum_degree_ordening") {
        std::map<ID, std::vector<ID>> graph{{0, {3, 5}}, {1, {4, 5, 8}}, {2, {4, 5, 6}}, {3, {6, 7}},
                                            {4, {6, 8}}, {6, {7, 8, 9}}, {7, {8, 9}},    {8, {9}}};

        auto const start = std::chrono::high_resolution_clock::now();
        std::vector<std::pair<std::vector<ID>, std::vector<std::pair<ID, ID>>>> const alpha_fills =
            minimum_degree_ordering(graph);
        auto const stop = std::chrono::high_resolution_clock::now();

        auto const duration = duration_cast<std::chrono::microseconds>(stop - start);
        std::cout << "Time taken by function: " << duration.count() << " microseconds"
                  << "\n";

        std::cout << to_string(alpha_fills[0].first) << "\n";
        std::cout << to_string(alpha_fills[0].second) << "\n";
    }
}
