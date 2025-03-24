// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#include "power_grid_model_cpp.hpp"

#include <doctest/doctest.h>

#include <memory>

namespace power_grid_model_cpp {

TEST_CASE("API Meta Data") {
    SUBCASE("Datasets") {
        // check dataset
        for (Idx idx_dataset = 0; idx_dataset != MetaData::n_datasets(); ++idx_dataset) {
            MetaDataset const* const dataset = MetaData::get_dataset_by_idx(idx_dataset);
            std::string const dataset_name = MetaData::dataset_name(dataset);
            CHECK(MetaData::get_dataset_by_name(dataset_name) == dataset);
            CAPTURE(dataset_name);
            // check a few dataset types
            if (idx_dataset == 0) {
                CHECK(dataset_name == "input");
            } else if (idx_dataset == 1) {
                CHECK(dataset_name == "update");
            }

            // check component
            for (Idx idx_component = 0; idx_component != MetaData::n_components(dataset); ++idx_component) {
                MetaComponent const* const component = MetaData::get_component_by_idx(dataset, idx_component);
                std::string const component_name = MetaData::component_name(component);
                CHECK(MetaData::get_component_by_name(dataset_name, component_name) == component);

                // check a few known components
                if (idx_component == 0 && dataset_name == "input") {
                    CHECK(component_name == "node");
                    CHECK(MetaData::component_size(component) == 16);
                    CHECK(MetaData::component_alignment(component) == 8);
                } else if (idx_component == 1 && dataset_name == "input") {
                    CHECK(component_name == "line");
                    CHECK(MetaData::component_size(component) == 88);
                    CHECK(MetaData::component_alignment(component) == 8);
                }

                // check attribute
                for (Idx idx_attribute = 0; idx_attribute != MetaData::n_attributes(component); ++idx_attribute) {
                    MetaAttribute const* const attribute = MetaData::get_attribute_by_idx(component, idx_attribute);
                    std::string const attribute_name = MetaData::attribute_name(attribute);
                    CHECK(MetaData::get_attribute_by_name(dataset_name, component_name, attribute_name) == attribute);

                    // check a few known attributes
                    if (idx_attribute == 0 && dataset_name == "input" && component_name == "node") {
                        CHECK(attribute_name == "id");
                        CHECK(MetaData::attribute_ctype(attribute) == 0);
                        CHECK(MetaData::attribute_offset(attribute) == 0);
                    } else if (idx_attribute == 1 && dataset_name == "input" && component_name == "node") {
                        CHECK(attribute_name == "u_rated");
                        CHECK(MetaData::attribute_ctype(attribute) == 2);
                        CHECK(MetaData::attribute_offset(attribute) == 8);
                    }
                }
            }
        }

        SUBCASE("Check error handling for unknown name") {
            using namespace std::string_literals;

            CHECK_THROWS_WITH_AS(MetaData::get_attribute_by_name("No_dataset", "no_name", "no attribute"),
                                 doctest::Contains("You supplied wrong name and/or index!"), PowerGridRegularError);
        }
    }
}

} // namespace power_grid_model_cpp
