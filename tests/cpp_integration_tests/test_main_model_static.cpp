// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include <power_grid_model/auxiliary/meta_data_gen.hpp>
#include <power_grid_model_static/main_model_wrapper.hpp>

#include <doctest/doctest.h>

namespace power_grid_model {
namespace {
using pgm_static::MainModelWrapper;
}

TEST_CASE("Test main model static") {
    // Cacheable and independent base update data
    std::vector<BranchUpdate> link{{0, na_IntS, na_IntS}, {1, na_IntS, na_IntS}, {0, na_IntS, na_IntS},
                                   {1, na_IntS, na_IntS}, {0, na_IntS, na_IntS}, {3, na_IntS, na_IntS}};

    std::vector<SourceUpdate> source{{0, na_IntS, 1.0, nan}, {0, na_IntS, 1.0, nan}, {0, na_IntS, 1.0, nan}};

    Idx const batches = 3;
    std::array<Idx, 4> const link_indptr = {0, 2, 4, 6};
    std::array<Idx, 4> const source_indptr = {0, 1, 2, 3};
    // dependent dataset
    ConstDataset update_data_dependent{true, batches, "update", meta_data::meta_data_gen::meta_data};
    update_data_dependent.add_buffer("link", -1, link_indptr.back(), link_indptr.data(), link.data());
    update_data_dependent.add_buffer("source", -1, source_indptr.back(), source_indptr.data(), source.data());

    // independent dataset
    ConstDataset update_data_independent{true, batches - 1, "update", meta_data::meta_data_gen::meta_data};
    update_data_independent.add_buffer("link", -1, link_indptr.crbegin()[1], link_indptr.data(), source.data());
    update_data_independent.add_buffer("source", -1, source_indptr.crbegin()[1], source_indptr.data(), source.data());

    SUBCASE("Independent update data") {
        CHECK(MainModelWrapper::is_update_independent(update_data_independent) == true);
    }

    SUBCASE("Dependent update data") { CHECK(MainModelWrapper::is_update_independent(update_data_dependent) == false); }
}

} // namespace power_grid_model
