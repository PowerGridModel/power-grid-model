// SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
//
// SPDX-License-Identifier: MPL-2.0

#include "c_api_cpp_handle.hpp"
#include "power_grid_model_c.h"

#include <power_grid_model/auxiliary/meta_data_gen.hpp>

#include <doctest/doctest.h>

#include <memory>

namespace power_grid_model::meta_data {

TEST_CASE("C API Meta Data") {
    // get handle
    HandlePtr const unique_handle{PGM_create_handle()};
    PGM_Handle* hl = unique_handle.get();
    auto const& meta = meta_data();

    SUBCASE("Datasets") {
        CHECK(PGM_meta_n_datasets(hl) == (Idx)meta.n_datasets());
        for (Idx i = 0; i != (Idx)meta.n_datasets(); ++i) {
            auto const found = meta.find(PGM_meta_dataset_name(hl, i));
            CHECK(found != meta.end());
        }
    }

    SUBCASE("Component meta data") {
        for (auto const& [dataset_name, dataset] : meta) {
            CHECK(PGM_meta_n_components(hl, dataset_name.c_str()) == (Idx)dataset.size());
            for (Idx i = 0; i != (Idx)dataset.size(); ++i) {
                auto const found = dataset.find(PGM_meta_component_name(hl, dataset_name.c_str(), i));
                CHECK(found != dataset.end());
                auto const& component_name = found->first;
                auto const& component_meta = found->second;
                CHECK(PGM_meta_component_size(hl, dataset_name.c_str(), component_name.c_str()) == component_meta.size);
                CHECK(PGM_meta_component_alignment(hl, dataset_name.c_str(), component_name.c_str()) ==
                      component_meta.alignment);
            }
        }
    }

    SUBCASE("Attributes") {
        for (auto const& [dataset_name, dataset] : meta) {
            for (auto const& [component_name, component_meta] : dataset) {
                auto const& attributes = component_meta.attributes;
                CHECK(PGM_meta_n_attributes(hl, dataset_name.c_str(), component_name.c_str()) ==
                      (Idx)attributes.size());
                for (Idx i = 0; i != (Idx)attributes.size(); ++i) {
                    auto const& attr = attributes[i];
                    CHECK(PGM_meta_attribute_name(hl, dataset_name.c_str(), component_name.c_str(), i) == attr.name);
                    CHECK(PGM_meta_attribute_ctype(hl, dataset_name.c_str(), component_name.c_str(),
                                                   attr.name.c_str()) == attributes[i].ctype);
                    CHECK(PGM_meta_attribute_offset(hl, dataset_name.c_str(), component_name.c_str(),
                                                    attr.name.c_str()) == attributes[i].offset);
                }
            }
        }
    }

    SUBCASE("Endian") {
        CHECK(static_cast<bool>(PGM_is_little_endian(hl)) == is_little_endian());
    }

    SUBCASE("Check error handling for unknown name") {
        CHECK(PGM_meta_attribute_name(hl, "No_dataset", "no_name", 0) == nullptr);
        CHECK(PGM_error_code(hl) == PGM_regular_error);
        std::string const err_msg{PGM_error_message(hl)};
        CHECK(err_msg.find("You supplied wrong name and/or index!") != std::string::npos);
        // clear error
        PGM_clear_error(hl);
        CHECK(PGM_error_code(hl) == PGM_no_error);
    }
}

}  // namespace power_grid_model::meta_data