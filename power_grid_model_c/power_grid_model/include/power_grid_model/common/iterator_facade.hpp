// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"

#include <concepts>
#include <ranges>

namespace power_grid_model {
template <typename T, typename ElementType>
concept iterator_like = requires(T const t) {
    { *t } -> std::convertible_to<std::remove_cvref_t<ElementType> const&>;
};

template <typename T, typename ElementType>
concept random_access_iterable_like = std::ranges::random_access_range<T> && requires(T const t) {
    { t.begin() } -> iterator_like<ElementType>;
    { t.end() } -> iterator_like<ElementType>;
};

template <typename T>
concept index_range_iterator = std::random_access_iterator<T> && requires(T const t) {
    typename T::iterator;
    { *t } -> random_access_iterable_like<Idx>;
};

template <class Impl, typename ValueType, typename DifferenceType> class IteratorFacade {
  public:
    using iterator = Impl;
    using const_iterator = std::add_const_t<iterator>;
    using value_type = ValueType;
    using difference_type = DifferenceType;
    using iterator_category = std::random_access_iterator_tag;
    using reference = value_type const&;

    constexpr auto operator*() const -> std::add_lvalue_reference_t<std::add_const_t<value_type>> {
        return static_cast<const_iterator*>(this)->dereference();
    }
    constexpr auto operator*() -> std::add_lvalue_reference_t<value_type&>
        requires requires(iterator it) {
            { it.dereference() } -> std::same_as<std::add_lvalue_reference_t<value_type>>;
        }
    {
        return static_cast<iterator*>(this)->dereference();
    }
    constexpr auto operator->() const -> value_type const* {
        return &static_cast<const_iterator*>(this)->dereference();
    }
    constexpr auto operator->() -> value_type*
        requires requires(iterator it) {
            { it.dereference() } -> std::convertible_to<std::add_pointer_t<std::remove_cvref_t<value_type>>>;
        }
    {
        return &static_cast<iterator*>(this)->dereference();
    }
    friend constexpr bool operator==(IteratorFacade const& first, IteratorFacade const& second) {
        return first.equal(second);
    }
    friend constexpr std::strong_ordering operator<=>(IteratorFacade const& first, IteratorFacade const& second)
        requires requires(const_iterator it) {
            { it.cmp(it) } -> std::convertible_to<std::strong_ordering>;
        }
    {
        return first.cmp(second);
    }
    constexpr auto operator++() -> iterator& {
        static_cast<iterator*>(this)->increment();
        return *static_cast<iterator*>(this);
    }
    constexpr auto operator--() -> iterator& {
        static_cast<iterator*>(this)->decrement();
        return *static_cast<iterator*>(this);
    }
    constexpr auto operator++(std::integral auto /*idx*/) -> iterator {
        iterator result{*static_cast<iterator*>(this)};
        ++(*this);
        return result;
    }
    constexpr auto operator--(std::integral auto /*idx*/) -> iterator {
        iterator result{*static_cast<iterator*>(this)};
        --(*this);
        return result;
    }
    constexpr auto operator+=(std::integral auto offset) -> iterator& {
        static_cast<iterator*>(this)->advance(offset);
        return *static_cast<iterator*>(this);
    }
    constexpr auto operator-=(std::integral auto idx) -> iterator& { return ((*this) += (-idx)); }
    constexpr auto operator+(difference_type offset) const -> iterator {
        iterator result{*static_cast<const_iterator*>(this)};
        result += offset;
        return result;
    }
    friend constexpr auto operator+(const difference_type offset, iterator it) -> iterator { return (it += offset); }
    constexpr auto operator-(difference_type idx) const -> iterator { return (*this) + (-idx); }
    constexpr auto operator-(iterator const& other) const -> difference_type {
        return other.distance_to(*static_cast<const_iterator*>(this));
    }
    constexpr auto operator[](difference_type idx) const -> value_type const& { return *(*this + idx); }

  private:
    // overloads for public bidirectional exposure (difference between MSVC and ClangCL)
    constexpr bool equal(IteratorFacade const& other) const {
        return static_cast<std::add_lvalue_reference_t<const_iterator>>(*this).equal(
            static_cast<std::add_lvalue_reference_t<const_iterator>>(other));
    }
    constexpr std::strong_ordering cmp(IteratorFacade const& other) const {
        return static_cast<std::add_lvalue_reference_t<const_iterator>>(*this).cmp(
            static_cast<std::add_lvalue_reference_t<const_iterator>>(other));
    }
};

} // namespace power_grid_model
