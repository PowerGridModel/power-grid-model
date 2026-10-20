// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <power_grid_model/auxiliary/dataset.hpp>
#include <power_grid_model/auxiliary/meta_data_gen.hpp>

namespace power_grid_model::validation::test {

inline ConstDataset make_empty_dataset(bool is_batch, char const* dataset_type) {
    return {is_batch, 1, dataset_type, meta_data::meta_data_gen::meta_data};
}

} // namespace power_grid_model::validation::test
