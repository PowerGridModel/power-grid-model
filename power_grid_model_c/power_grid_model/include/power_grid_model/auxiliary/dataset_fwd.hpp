// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

// forward declarations for dataset (handler) and buffer related stuff

#include <concepts>

namespace power_grid_model {

struct const_dataset_t {};
struct mutable_dataset_t {};
struct writable_dataset_t {};

template <typename T>
concept dataset_type_tag =
    std::same_as<T, const_dataset_t> || std::same_as<T, mutable_dataset_t> || std::same_as<T, writable_dataset_t>;

} // namespace power_grid_model
