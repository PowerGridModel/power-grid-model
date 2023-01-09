// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <memory>

#include "doctest/doctest.h"
#include "power_grid_model/auxiliary/meta_data_gen.hpp"
#include "power_grid_model_c.h"

struct HandleDestructor {
    void operator()(PGM_Handle* handle) {
        PGM_destroy_handle(handle);
    }
};

using HandlePtr = std::unique_ptr<PGM_Handle, HandleDestructor>;

namespace power_grid_model::meta_data {

TEST_CASE("C API Meta Data") {
    // get handle
    HandlePtr const unique_handle{PGM_create_handle()};
    PGM_Handle* hl = unique_handle.get();
    auto const& meta = meta_data();

    SUBCASE("Datasets") {
        CHECK(PGM_meta_n_datasets(hl) == (Idx)meta.size());
        for (Idx i = 0; i != (Idx)meta.size(); ++i) {
            auto const found = meta.find(PGM_meta_dataset_name(hl, i));
            CHECK(found != meta.end());
        }
    }

    SUBCASE("Data class") {
        for (auto const& [dataset_name, data_classes] : meta) {
            CHECK(PGM_meta_n_classes(hl, dataset_name.c_str()) == (Idx)data_classes.size());
            for (Idx i = 0; i != (Idx)data_classes.size(); ++i) {
                auto const found = data_classes.find(PGM_meta_class_name(hl, dataset_name.c_str(), i));
                CHECK(found != data_classes.end());
                auto const& class_name = found->first;
                auto const& class_meta = found->second;
                CHECK(PGM_meta_class_size(hl, dataset_name.c_str(), class_name.c_str()) == class_meta.size);
                CHECK(PGM_meta_class_alignment(hl, dataset_name.c_str(), class_name.c_str()) == class_meta.alignment);
            }
        }
    }
}

}  // namespace power_grid_model::meta_data