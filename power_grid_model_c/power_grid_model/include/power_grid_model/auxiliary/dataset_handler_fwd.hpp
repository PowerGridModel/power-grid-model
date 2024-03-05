// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

// forward declarations for handling dataset and buffer related stuff

#include <concepts>

namespace power_grid_model::meta_data {

struct data_mutable_t {};
struct data_immutable_t {};

template <typename T>
concept data_mutable_tag = std::same_as<T, data_mutable_t> || std::same_as<T, data_immutable_t>;
template <data_mutable_tag T> constexpr bool is_data_mutable_v = std::same_as<T, data_mutable_t>;

static_assert(data_mutable_tag<data_mutable_t>);
static_assert(data_mutable_tag<data_immutable_t>);
static_assert(is_data_mutable_v<data_mutable_t>);
static_assert(!is_data_mutable_v<data_immutable_t>);

struct indptr_mutable_t {};
struct indptr_immutable_t {};

template <typename T>
concept indptr_mutable_tag = std::same_as<T, indptr_mutable_t> || std::same_as<T, indptr_immutable_t>;
template <indptr_mutable_tag T> constexpr bool is_indptr_mutable_v = std::same_as<T, indptr_mutable_t>;

static_assert(indptr_mutable_tag<indptr_mutable_t>);
static_assert(indptr_mutable_tag<indptr_immutable_t>);
static_assert(is_indptr_mutable_v<indptr_mutable_t>);
static_assert(!is_indptr_mutable_v<indptr_immutable_t>);

template <data_mutable_tag data_mutable, indptr_mutable_tag indptr_mutable>
    requires(is_data_mutable_v<data_mutable> || !is_indptr_mutable_v<indptr_mutable>)
class DatasetHandler;

} // namespace power_grid_model::meta_data
