// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include "doctest/doctest.h"
#include "power_grid_model/main_model.hpp"

namespace power_grid_model {

TEST_CASE("Test main model static") {
    // Cacheable and independent base update data
    std::vector<BranchUpdate> link{{{0}, na_IntS, na_IntS}, {{1}, na_IntS, na_IntS}, {{0}, na_IntS, na_IntS},
                                   {{1}, na_IntS, na_IntS}, {{0}, na_IntS, na_IntS}, {{3}, na_IntS, na_IntS}};

    std::vector<SourceUpdate> source{
        {{{0}, na_IntS}, 1.0, nan},
        {{{0}, na_IntS}, 1.0, nan},
        {{{0}, na_IntS}, 1.0, nan},
    };

    Idx batches = 3;
    std::array<Idx, 4> const link_indptr = {0, 2, 4, 6};
    std::array<Idx, 4> const source_indptr = {0, 1, 2, 3};
    // dependent dataset
    ConstDataset update_data_dependent;
    update_data_dependent["link"] = ConstDataPointer{link.data(), link_indptr.data(), batches};
    update_data_dependent["source"] = ConstDataPointer{source.data(), source_indptr.data(), batches};
    // independent dataset
    ConstDataset update_data_independent;
    update_data_independent["link"] = ConstDataPointer{link.data(), link_indptr.data(), batches - 1};
    update_data_independent["source"] = ConstDataPointer{source.data(), source_indptr.data(), batches - 1};

    SUBCASE("Independent update data") {
        CHECK(MainModel::is_update_independent(update_data_independent) == true);
    }

    SUBCASE("Dependent update data") {
        CHECK(MainModel::is_update_independent(update_data_dependent) == false);
    }

    SUBCASE("Cacheable topology") {
        REQUIRE(is_nan(na_IntS));
        CHECK(MainModel::is_topology_cacheable(update_data_dependent) == true);
        CHECK(MainModel::is_topology_cacheable(update_data_independent) == true);
    }

    SUBCASE("Non-cacheable topology") {
        link[1].from_status = true;
        CHECK(MainModel::is_topology_cacheable(update_data_dependent) == false);
        CHECK(MainModel::is_topology_cacheable(update_data_independent) == false);
    }

    SUBCASE("Non-cacheable topology source") {
        source[1].status = true;
        CHECK(MainModel::is_topology_cacheable(update_data_dependent) == false);
        CHECK(MainModel::is_topology_cacheable(update_data_independent) == false);
    }
}

}  // namespace power_grid_model
