// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/sparse_mapping.hpp>

#include <ctime>
#include <doctest/doctest.h>
#include <fstream>
#include <iostream>
#include <random>

namespace power_grid_model {

TEST_CASE("Test sparse mapping") {
    IdxVector const idx_B_in_A{3, 5, 2, 1, 1, 2};
    SparseMapping mapping{{0, 0, 2, 4, 5, 5, 6, 6}, {3, 4, 2, 5, 0, 1}};
    SparseMapping mapping_2 = build_sparse_mapping(idx_B_in_A, 7);

    CHECK(mapping.indptr == mapping_2.indptr);
    CHECK(mapping.reorder == mapping_2.reorder);
}

TEST_CASE("Test dense mapping") {
    IdxVector const idx_B_in_A{3, 5, 2, 1, 1, 2};
    DenseMapping mapping{{1, 1, 2, 2, 3, 5}, {3, 4, 2, 5, 0, 1}};
    DenseMapping mapping_2 = build_dense_mapping(idx_B_in_A, 7);

    CHECK(mapping.indvector == mapping_2.indvector);
    CHECK(mapping.reorder == mapping_2.reorder);
    CHECK(mapping_2.indvector.begin() == std::min_element(mapping_2.indvector.begin(), mapping_2.indvector.end()));
    CHECK(mapping_2.indvector.end() - 1 == std::max_element(mapping_2.indvector.begin(), mapping_2.indvector.end()));
}

IdxVector generateVector(int n_A, int n_B) {
    IdxVector vec(n_A);
    // so we are always guaranteed to have n_B = max-min
    vec[0] = Idx(0);
    vec[n_A - 1] = Idx(n_B);

    std::mt19937 eng(16);
    std::uniform_int_distribution<> distr(0, n_B);

    for (int i = 1; i < n_A - 1; i++) {
        vec[i] = Idx(distr(eng));
    }
    return vec;
}

float measurePerformance(IdxVector idx_B_in_A, Idx const n_B) {
    float t1 = clock();

    DenseMapping mapping_2 = build_dense_mapping(idx_B_in_A, n_B);

    float t2 = clock() - t1;

    float time_req = t2 - t1;

    return time_req;
}

void writeToCSV(const std::vector<float>& data) {
    std::ofstream file("C:\resultt.xlsx");

    for (const auto& value : data) {
        file << value << ",";
    }
    file.close();
}

TEST_CASE("Benchmark") {
    std::vector<int> A_values = {10, 100, 1000, 10000, 100000, 1000000};
    std::vector<int> B_values = {1, 10, 100, 1000, 10000, 100000, 1000000};

    for (const auto& n_A : A_values) {
        std::vector<float> vec(7);
        for (const auto& n_B : B_values) {
            IdxVector idx_B_in_A = generateVector(n_A, n_B);
            float time_taken = measurePerformance(idx_B_in_A, n_B);
            vec.push_back(time_taken);
        }
        writeToCSV(vec);
    }
}

TEST_CASE("Benchmark dense mapping N") {
    using std::chrono::milliseconds;

    IdxVector idx_B_in_A = generateVector(10, 1);

    auto t1 = std::chrono::high_resolution_clock::now();

    DenseMapping mapping_2 = build_dense_mapping(idx_B_in_A, 1);

    auto t2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> ms_double = t2 - t1;

    std::cout << ms_double.count() << "ms\n";
}

TEST_CASE("Benchmark dense mapping N=2") {
    using std::chrono::milliseconds;

    IdxVector idx_B_in_A{0, 999999};

    auto t1 = std::chrono::high_resolution_clock::now();

    DenseMapping mapping_2 = build_dense_mapping(idx_B_in_A, 1000000);

    auto t2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> ms_double = t2 - t1;

    std::cout << ms_double.count() << "ms\n";
}

TEST_CASE("Benchmark dense mapping N=10") {
    using std::chrono::milliseconds;

    IdxVector idx_B_in_A(10);
    std::mt19937 eng(16);
    std::uniform_int_distribution<> distr(0, 10);

    for (auto& val : idx_B_in_A) {
        val = distr(eng);
    }

    auto t1 = std::chrono::high_resolution_clock::now();

    DenseMapping mapping_2 = build_dense_mapping(idx_B_in_A, 11);

    auto t2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> ms_double = t2 - t1;

    std::cout << ms_double.count() << "ms\n";
}

TEST_CASE("Benchmark dense mapping N=100") {
    using std::chrono::milliseconds;

    IdxVector idx_B_in_A(100);
    std::mt19937 eng(16);
    std::uniform_int_distribution<> distr(0, 100);

    for (auto& val : idx_B_in_A) {
        val = distr(eng);
    }

    auto t1 = std::chrono::high_resolution_clock::now();

    DenseMapping mapping_2 = build_dense_mapping(idx_B_in_A, 101);

    auto t2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> ms_double = t2 - t1;

    std::cout << ms_double.count() << "ms\n";
}

TEST_CASE("Benchmark dense mapping N=1000") {
    using std::chrono::milliseconds;

    IdxVector idx_B_in_A(1000);
    std::mt19937 eng(16);
    std::uniform_int_distribution<> distr(0, 1000);

    for (auto& val : idx_B_in_A) {
        val = distr(eng);
    }

    auto t1 = std::chrono::high_resolution_clock::now();

    DenseMapping mapping_2 = build_dense_mapping(idx_B_in_A, 1001);

    auto t2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> ms_double = t2 - t1;

    std::cout << ms_double.count() << "ms\n";
}

TEST_CASE("Benchmark dense mapping N=10 000") {
    using std::chrono::milliseconds;

    IdxVector idx_B_in_A(10000);
    std::mt19937 eng(16);
    std::uniform_int_distribution<> distr(0, 10000);

    for (auto& val : idx_B_in_A) {
        val = distr(eng);
    }

    auto t1 = std::chrono::high_resolution_clock::now();

    DenseMapping mapping_2 = build_dense_mapping(idx_B_in_A, 10001);

    auto t2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> ms_double = t2 - t1;

    std::cout << ms_double.count() << "ms\n";
}

TEST_CASE("Benchmark dense mapping N=100 000") {
    using std::chrono::milliseconds;

    IdxVector idx_B_in_A(100000);
    std::mt19937 eng(16);
    std::uniform_int_distribution<> distr(0, 100000);

    for (auto& val : idx_B_in_A) {
        val = distr(eng);
    }

    auto t1 = std::chrono::high_resolution_clock::now();

    DenseMapping mapping_2 = build_dense_mapping(idx_B_in_A, 100001);

    auto t2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> ms_double = t2 - t1;

    std::cout << ms_double.count() << "ms\n";
}

TEST_CASE("Benchmark dense mapping N=1000 000") {
    using std::chrono::milliseconds;

    IdxVector idx_B_in_A(1000000);
    std::mt19937 eng(16);
    std::uniform_int_distribution<> distr(0, 1000000);

    for (auto& val : idx_B_in_A) {
        val = distr(eng);
    }

    auto t1 = std::chrono::high_resolution_clock::now();

    DenseMapping mapping_2 = build_dense_mapping(idx_B_in_A, 1000001);

    auto t2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> ms_double = t2 - t1;

    std::cout << ms_double.count() << "ms\n";
}

} // namespace power_grid_model
