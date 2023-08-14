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
        // check dataset
        CHECK(PGM_meta_n_datasets(hl) == meta.n_datasets());
        for (Idx idx_dataset = 0; idx_dataset != meta.n_datasets(); ++idx_dataset) {
            MetaDataset const* const dataset = PGM_meta_get_dataset_by_idx(hl, idx_dataset);
            char const* const dataset_name = PGM_meta_dataset_name(hl, dataset);
            CHECK(PGM_meta_get_dataset_by_name(hl, dataset_name) == dataset);
            CHECK(dataset_name == meta.datasets[idx_dataset].name);

            // check component
            CHECK(PGM_meta_n_components(hl, dataset) == dataset->n_components());
            for (Idx idx_component = 0; idx_component != dataset->n_components(); ++idx_component) {
                MetaComponent const* const component = PGM_meta_get_component_by_idx(hl, dataset, idx_component);
                char const* const component_name = PGM_meta_component_name(hl, component);
                CHECK(PGM_meta_get_component_by_name(hl, dataset_name, component_name) == component);
                CHECK(component_name == dataset->components[idx_component].name);
                CHECK(PGM_meta_component_size(hl, component) == component->size);
                CHECK(PGM_meta_component_alignment(hl, component) == component->alignment);

                // check attribute
                CHECK(PGM_meta_n_attributes(hl, component) == component->n_attributes());
                for (Idx idx_attribute = 0; idx_attribute != component->n_attributes(); ++idx_attribute) {
                    MetaAttribute const* const attribute = PGM_meta_get_attribute_by_idx(hl, component, idx_attribute);
                    char const* const attribute_name = PGM_meta_attribute_name(hl, attribute);
                    CHECK(PGM_meta_get_attribute_by_name(hl, dataset_name, component_name, attribute_name) ==
                          attribute);
                    CHECK(attribute_name == component->attributes[idx_attribute].name);
                    CHECK(PGM_meta_attribute_ctype(hl, attribute) == static_cast<Idx>(attribute->ctype));
                    CHECK(PGM_meta_attribute_offset(hl, attribute) == attribute->offset);
                }
            }
        }

        SUBCASE("Endian") { CHECK(static_cast<bool>(PGM_is_little_endian(hl)) == is_little_endian()); }

        SUBCASE("Check error handling for unknown name") {
            CHECK(PGM_meta_get_attribute_by_name(hl, "No_dataset", "no_name", "no attribute") == nullptr);
            CHECK(PGM_error_code(hl) == PGM_regular_error);
            std::string const err_msg{PGM_error_message(hl)};
            CHECK(err_msg.find("You supplied wrong name and/or index!") != std::string::npos);
            // clear error
            PGM_clear_error(hl);
            CHECK(PGM_error_code(hl) == PGM_no_error);
        }
    }
}

} // namespace power_grid_model::meta_data