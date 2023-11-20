// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/math_solver/sparse_iteration.hpp>

#include <doctest/doctest.h>

#include <chrono>

namespace {
namespace solver = power_grid_model::math_solver;
using namespace power_grid_model;

void printVectPair(std::vector<std::pair<ID, ID>>& d) {
    for (const auto& it : d) {
        std::cout << it.first << ": " << it.second << std::endl;
    }
}

void printMap(std::map<int, std::vector<int>>& d) {
    for (const auto& it : d) {
        std::cout << it.first << ": ";
        for (const ID& el : it.second) {
            std::cout << el << " ";
        }
        std::cout << std::endl;
    }
}

void show(std::vector<int> const& input) {
    for (auto const& i : input)
        std::cout << i << ", ";
    std::cout << std::endl;
}
} // namespace

TEST_CASE("Test sparse iteration") {
    std::map<ID, std::vector<ID>> graph{{0, {3, 5}}, {1, {4, 5, 8}}, {2, {4, 5, 6}}, {3, {6, 7}},
                                        {4, {6, 8}}, {6, {7, 8, 9}}, {7, {8, 9}},    {8, {9}}};

    auto start = std::chrono::high_resolution_clock::now();
    std::vector<std::pair<std::vector<ID>, std::vector<std::pair<ID, ID>>>> alpha_fills =
        solver::minimumDegreeAlgorithm(graph);
    auto stop = std::chrono::high_resolution_clock::now();

    auto duration = duration_cast<std::chrono::microseconds>(stop - start);
    float seconds = duration.count() / 1000000.0;
    /*
    std::cout << "Time taken by function: "
         << duration.count() << " microseconds" << std::endl;
    */
    std::cout << "Time taken by function: " << seconds << " seconds" << std::endl;

    show(alpha_fills[0].first);
    printVectPair(alpha_fills[0].second);
}
