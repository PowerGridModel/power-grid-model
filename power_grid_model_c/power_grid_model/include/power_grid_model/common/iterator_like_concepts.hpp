// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

namespace power_grid_model {
// TODO(mgovers): replace the below relevant iterator concepts with the STD equivalent when we have index ranges.
// e.g.: boost::counting_iterator does not satisfy all requirements std::*_iterator concepts:
// we have to declare the relevant subset here ourselves.

template <typename T, typename ElementType>
concept iterator_like = requires(T const t) {
    { *t } -> std::convertible_to<std::remove_cvref_t<ElementType> const&>;
};

template <typename T>
concept forward_iterator_like = std::regular<T> && std::input_iterator<T> && requires(T t) {
    { t++ } -> std::same_as<T>;  // NOLINT(bugprone-inc-dec-in-conditions)
    { ++t } -> std::same_as<T&>; // NOLINT(bugprone-inc-dec-in-conditions)
};

template <typename T>
concept bidirectional_iterator_like = std::forward_iterator<T> && requires(T t) {
    { t-- } -> std::same_as<T>;  // NOLINT(bugprone-inc-dec-in-conditions)
    { --t } -> std::same_as<T&>; // NOLINT(bugprone-inc-dec-in-conditions)
};

template <typename T>
concept random_access_iterator_like =
    std::bidirectional_iterator<T> && std::totally_ordered<T> && requires(T t, Idx n) {
        { t + n } -> std::same_as<T>;
        { t - n } -> std::same_as<T>;
        { t += n } -> std::same_as<T&>;
        { t -= n } -> std::same_as<T&>;
    };

template <typename T, typename ElementType>
concept random_access_iterable_like = std::ranges::random_access_range<T> && requires(T const t) {
    { t.begin() } -> iterator_like<ElementType>;
    { t.end() } -> iterator_like<ElementType>;
};

template <typename T>
concept index_range_iterator = random_access_iterator_like<T> && requires(T const t) {
    typename T::iterator;
    { *t } -> random_access_iterable_like<Idx>;
};

} // namespace power_grid_model
