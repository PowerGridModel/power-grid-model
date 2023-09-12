// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <doctest/doctest.h>
#include <power_grid_model/main_core/sparse_idx_vector.hpp>

namespace power_grid_model {

TEST_CASE("Sparse idx data strucuture for topology") {
    IdxVector const sample_indptr{0, 3, 6, 7};
    IdxVector const data{10, 11, 12, 13, 14, 15, 16};
    Idx const location{1};
    IdxVector const expected_elements_at_1{13, 14, 15};

    main_core::SparseIdxVector sparseidxvector{sample_indptr};
    main_core::SparseVectorData<Idx> sparsevectordata{data};

    IdxVector actual_elements_at_1{};
    auto location_size = sparseidxvector.subset(location);
    auto sparsevectordata_range = sparsevectordata.subset_data(location_size);
    for (auto i : sparsevectordata_range) {
        actual_elements_at_1.push_back(i);
    }

    CHECK(actual_elements_at_1 == expected_elements_at_1);
    CHECK(sparsevectordata_range[1] == 14);
    CHECK(sparsevectordata_range.size() == 3);
    CHECK(sparsevectordata[4] == 14);
    CHECK(sparsevectordata.at(4) == 14);
    auto x = data[4];
}

} // namespace power_grid_model
