// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <power_grid_model_cpp/serialization.hpp>

namespace power_grid_model_cpp_test {
inline power_grid_model_cpp::OwningDataset load_dataset(std::string const& json_string) {
    power_grid_model_cpp::Deserializer deserializer{json_string, PGM_json};
    auto& writable_dataset = deserializer.get_dataset();
    auto owning_dataset = power_grid_model_cpp::create_owning_dataset(writable_dataset);
    deserializer.parse_to_buffer();
    return owning_dataset;
}
} // namespace power_grid_model_cpp_test
