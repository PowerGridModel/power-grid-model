// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include <memory>

#include "c_api_cpp_handle.hpp"
#include "doctest/doctest.h"
#include "power_grid_model/auxiliary/meta_data_gen.hpp"
#include "power_grid_model_c.h"

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

    SUBCASE("Data classes") {
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

    SUBCASE("Attributes") {
        for (auto const& [dataset_name, data_classes] : meta) {
            for (auto const& [class_name, class_meta] : data_classes) {
                auto const& attributes = class_meta.attributes;
                CHECK(PGM_meta_n_attributes(hl, dataset_name.c_str(), class_name.c_str()) == (Idx)attributes.size());

                for (Idx i = 0; i != (Idx)attributes.size(); ++i) {
                    auto const& attr = attributes[i];
                    CHECK(PGM_meta_attribute_name(hl, dataset_name.c_str(), class_name.c_str(), i) == attr.name);
                    CHECK(PGM_meta_attribute_ctype(hl, dataset_name.c_str(), class_name.c_str(), attr.name.c_str()) ==
                          attributes[i].ctype);
                    CHECK(PGM_meta_attribute_offset(hl, dataset_name.c_str(), class_name.c_str(), attr.name.c_str()) ==
                          attributes[i].offset);
                }
            }
        }
    }

    SUBCASE("Endian") {
        CHECK((bool)PGM_is_little_endian(hl) == is_little_endian());
    }

    SUBCASE("Check error handling for unknown name") {
        CHECK(PGM_meta_attribute_name(hl, "No_dataset", "no_name", 0) == nullptr);
        CHECK(PGM_err_code(hl) == 1);
        std::string const err_msg{PGM_err_msg(hl)};
        CHECK(err_msg.find("You supplied wrong name and/or index!") != std::string::npos);
        // clear error
        PGM_clear_error(hl);
        CHECK(PGM_err_code(hl) == 0);
    }
}

}  // namespace power_grid_model::meta_data