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

template <typename T, typename ElementType>
concept forward_iterator_like = std::regular<T> && iterator_like<T, ElementType> && requires(T t) {
    { t++ } -> std::same_as<T>;
    { ++t } -> std::same_as<T&>;
};

template <typename T, typename ElementType>
concept bidirectional_iterator_like = forward_iterator_like<T, ElementType> && requires(T t) {
    { t-- } -> std::same_as<T>;
    { --t } -> std::same_as<T&>;
};

template <typename T, typename ElementType>
concept random_access_iterator_like =
    bidirectional_iterator_like<T, ElementType> && std::totally_ordered<T> && requires(T t, Idx n) {
        { t + n } -> std::same_as<T>;
        { t - n } -> std::same_as<T>;
        { t += n } -> std::same_as<T&>;
        { t -= n } -> std::same_as<T&>;
    };

template <typename T, typename ElementType>
concept random_access_iterable_like = requires(T const t) {
    { t.begin() } -> random_access_iterator_like<ElementType>;
    { t.end() } -> random_access_iterator_like<ElementType>;
};

template <typename T>
concept index_range_iterator = random_access_iterator_like<T, typename T::iterator> && requires(T const t) {
    typename T::iterator;
    { *t } -> random_access_iterable_like<Idx>;
};

} // namespace power_grid_model
