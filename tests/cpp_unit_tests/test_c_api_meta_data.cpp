// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include "doctest/doctest.h"
#include "power_grid_model_c.h"
#include "power_grid_model/auxiliary/meta_data_gen.hpp"
#include <memory>

struct HandleDestructor{
    void operator() (PGM_Handle* handle){
        PGM_destroy_handle(handle);
    }
};

using HandlePtr = std::unique_ptr<PGM_Handle, HandleDestructor>;


namespace power_grid_model::meta_data {

TEST_CASE("C API Meta Data") {
    SUBCASE("Datasets") {
        HandlePtr const unique_handle{PGM_create_handle()};
        PGM_Handle* hl = unique_handle.get();
        auto const& meta = meta_data();
        CHECK(PGM_meta_n_datasets(hl) == (Idx)meta.size());
        for (Idx i = 0; i != (Idx)meta.size(); ++i){
            auto const found = meta.find(PGM_meta_dataset_name(hl, i));
            CHECK(found != meta.end());
        } 
    }

}


} // namespace power_grid_model::meta_data