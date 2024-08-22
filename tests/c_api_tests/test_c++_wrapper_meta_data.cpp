// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#define PGM_ENABLE_EXPERIMENTAL

#include "power_grid_model_cpp.hpp"

#include <power_grid_model/auxiliary/meta_data_gen.hpp>

#include <doctest/doctest.h>

#include <memory>

namespace power_grid_model_cpp {

TEST_CASE("C API Meta Data") {
    SUBCASE("Datasets") {
        // check dataset
        CHECK(MetaData::n_datasets() == meta_data_gen::meta_data.n_datasets());
        for (Idx idx_dataset = 0; idx_dataset != meta_data_gen::meta_data.n_datasets(); ++idx_dataset) {
            MetaDataset const* const dataset = MetaData::get_dataset_by_idx(idx_dataset);
            std::string const dataset_name = MetaData::dataset_name(dataset);
            CHECK(MetaData::get_dataset_by_name(dataset_name) == dataset);
            CHECK(dataset_name == meta_data_gen::meta_data.datasets[idx_dataset].name);

            // check component
            power_grid_model::meta_data::MetaDataset const& cpp_dataset = meta_data_gen::meta_data.get_dataset(dataset_name);
            CHECK(MetaData::n_components(dataset) == cpp_dataset.n_components());
            for (Idx idx_component = 0; idx_component != cpp_dataset.n_components(); ++idx_component) {
                MetaComponent const* const component = MetaData::get_component_by_idx(dataset, idx_component);
                std::string const component_name = MetaData::component_name(component);
                CHECK(MetaData::get_component_by_name(dataset_name, component_name) == component);
                power_grid_model::meta_data::MetaComponent const& cpp_component = cpp_dataset.components[idx_component];
                CHECK(component_name == cpp_component.name);
                CHECK(MetaData::component_size(component) == cpp_component.size);
                CHECK(MetaData::component_alignment(component) == cpp_component.alignment);

                // check attribute
                CHECK(MetaData::n_attributes(component) == cpp_component.n_attributes());
                for (Idx idx_attribute = 0; idx_attribute != cpp_component.n_attributes(); ++idx_attribute) {
                    MetaAttribute const* const attribute =
                        MetaData::get_attribute_by_idx(component, idx_attribute);
                    std::string const attribute_name = MetaData::attribute_name(attribute);
                    CHECK(MetaData::get_attribute_by_name(dataset_name, component_name,
                                                         attribute_name) == attribute);
                    power_grid_model::meta_data::MetaAttribute const& cpp_attribute = cpp_component.attributes[idx_attribute];
                    CHECK(attribute_name == cpp_attribute.name);
                    CHECK(MetaData::attribute_ctype(attribute) == static_cast<Idx>(cpp_attribute.ctype));
                    CHECK(MetaData::attribute_offset(attribute) == cpp_attribute.offset);
                }
            }
        }

        SUBCASE("Endian") { CHECK(static_cast<bool>(MetaData::is_little_endian()) == power_grid_model::meta_data::is_little_endian()); }

        SUBCASE("Check error handling for unknown name") { // This string check remains todo
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

} // namespace power_grid_model_cpp