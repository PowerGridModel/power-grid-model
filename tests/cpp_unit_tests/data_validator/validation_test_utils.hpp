// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/auxiliary/meta_data.hpp>
#include <power_grid_model/common/common.hpp>

#include <array>
#include <cstddef>
#include <span>

namespace power_grid_model::validation::test {

struct TestInput {
    static constexpr char const* id_name = "id";
    static constexpr char const* value_name = "value";

    ID id{na_IntID};
    double value{nan};
};

struct TestComponent {
    using InputType = TestInput;

    static constexpr char const* name = "test";
};

namespace detail {

inline constexpr std::array<meta_data::MetaAttribute, 2> test_attributes{
    meta_data::MetaAttribute{.name = TestInput::id_name,
                             .ctype = meta_data::ctype_v<ID>,
                             .offset = offsetof(TestInput, id),
                             .size = sizeof(ID),
                             .component_size = sizeof(TestInput)},
    meta_data::MetaAttribute{.name = TestInput::value_name,
                             .ctype = meta_data::ctype_v<double>,
                             .offset = offsetof(TestInput, value),
                             .size = sizeof(double),
                             .component_size = sizeof(TestInput)},
};

inline constexpr std::array<meta_data::MetaComponent, 1> test_components{
    meta_data::MetaComponent{.name = TestComponent::name,
                             .size = sizeof(TestInput),
                             .alignment = alignof(TestInput),
                             .attributes = std::span<meta_data::MetaAttribute const>{test_attributes}},
};

inline constexpr std::array<meta_data::MetaDataset, 3> test_datasets{
    meta_data::MetaDataset{.name = "input", .components = std::span<meta_data::MetaComponent const>{test_components}},
    meta_data::MetaDataset{.name = "update", .components = std::span<meta_data::MetaComponent const>{test_components}},
    meta_data::MetaDataset{.name = "sym_output", .components = std::span<meta_data::MetaComponent const>{}},
};

inline constexpr meta_data::MetaData test_meta_data{.datasets = std::span<meta_data::MetaDataset const>{test_datasets}};

} // namespace detail

inline ConstDataset make_empty_dataset(bool is_batch, char const* dataset_type) {
    return {is_batch, 1, dataset_type, detail::test_meta_data};
}

} // namespace power_grid_model::validation::test
