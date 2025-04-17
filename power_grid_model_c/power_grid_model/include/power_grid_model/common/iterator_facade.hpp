// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"

#include <concepts>
#include <ranges>

namespace power_grid_model {
template <class Impl, typename ValueType, std::integral DifferenceType> class IteratorFacade {
  public:
    using iterator = Impl; // CRTP
    using const_iterator = std::add_const_t<iterator>;
    using value_type = std::remove_cvref_t<ValueType>;
    using difference_type = DifferenceType;
    using iterator_category = std::random_access_iterator_tag;
    using pointer = std::add_pointer_t<ValueType>;
    using reference = std::add_lvalue_reference_t<ValueType>;

    constexpr auto operator*() const -> decltype(auto) { return static_cast<const_iterator*>(this)->dereference(); }
    constexpr auto operator*() -> reference
        requires requires(iterator it) {
            { it.dereference() } -> std::same_as<reference>;
        }
    {
        return static_cast<iterator*>(this)->dereference();
    }
    constexpr auto operator->() const -> decltype(auto) { return &(*(*this)); }
    constexpr auto operator->() -> decltype(auto) { return &(*(*this)); }

    friend constexpr bool operator==(IteratorFacade const& first, IteratorFacade const& second) {
        return (first <=> second) == std::strong_ordering::equivalent;
    }
    friend constexpr std::strong_ordering operator<=>(IteratorFacade const& first, IteratorFacade const& second) {
        return first.three_way_compare(second);
    }

    constexpr auto operator++() -> iterator& {
        if constexpr (requires(iterator it) { it.increment(); }) {
            static_cast<iterator*>(this)->increment();
        } else {
            static_cast<iterator*>(this)->advance(1);
        }
        return *static_cast<iterator*>(this);
    }
    constexpr auto operator--() -> iterator& {
        if constexpr (requires(iterator it) { it.decrement(); }) {
            static_cast<iterator*>(this)->decrement();
        } else {
            static_cast<iterator*>(this)->advance(-1);
        }
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

    friend constexpr auto operator+(iterator const& it, difference_type offset) -> iterator {
        iterator result{it};
        result += offset;
        return result;
    }
    friend constexpr auto operator+(difference_type offset, iterator it) -> iterator { return (it += offset); }
    friend constexpr auto operator-(iterator const& it, difference_type idx) -> iterator { return it + (-idx); }
    friend constexpr auto operator-(IteratorFacade const& first, IteratorFacade const& second) -> difference_type {
        return second.distance_to(first);
    }

    constexpr auto operator[](difference_type idx) const -> value_type const& { return *(*this + idx); }

  private:
    // overloads for public bidirectional exposure (difference between MSVC and ClangCL)
    constexpr std::strong_ordering three_way_compare(IteratorFacade const& other) const {
        return static_cast<std::add_lvalue_reference_t<const_iterator>>(*this).three_way_compare(
            static_cast<std::add_lvalue_reference_t<const_iterator>>(other));
    }
    constexpr auto distance_to(IteratorFacade const& other) const -> difference_type {
        return static_cast<std::add_lvalue_reference_t<const_iterator>>(*this).distance_to(
            static_cast<std::add_lvalue_reference_t<const_iterator>>(other));
    }
};

} // namespace power_grid_model
