// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"

#include <concepts>
#include <ranges>
#include <utility>

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
    // constexpr auto operator*(this auto&& self) -> decltype(auto) {
    //     using Self = decltype(self);
    //     return std::forward_like<std::add_const_t<Self>>(self).dereference();
    // }
    // template <typename Self>
    // constexpr auto operator*(this Self&& self) -> decltype(auto)
    //     requires(!std::is_const_v<Self> &&
    //              requires {
    //                  { self.dereference() } -> std::same_as<typename Self::reference>;
    //              })
    // {
    //     return std::forward<Self>(self).dereference();
    // }

    // constexpr auto operator->() const -> decltype(auto) { return &(*(*this)); }
    // constexpr auto operator->() -> decltype(auto) { return &(*(*this)); }
    constexpr auto operator->(this auto&& self) { return &(*std::forward<decltype(self)>(self)); }

    // friend constexpr bool operator==(Impl const& first, Impl const& second) {
    //     return (first <=> second) == std::strong_ordering::equivalent;
    // }
    friend constexpr std::strong_ordering operator<=>(Impl const& first, Impl const& second) {
        return first.three_way_compare(second);
    }
    template <typename Self, typename Other>
    constexpr bool operator==(this Self&& self, Other&& other)
        requires requires {
            { std::forward<Self>(self) <=> std::forward<Other>(other) } -> std::same_as<std::strong_ordering>;
        }
    {
        return (std::forward<Self>(self) <=> std::forward<Other>(other)) == std::strong_ordering::equivalent;
    }
    // template <typename Self, typename Other>
    // constexpr auto operator<=>(this Self&& self, Other&& other)
    //     requires std::same_as<std::remove_cvref_t<Self>, std::remove_cvref_t<Other>> && requires {
    //         { self.three_way_compare(other) } -> std::same_as<std::strong_ordering>;
    //     }
    // {
    //     return (std::forward<Self>(self).three_way_compare(std::forward<Other>(other)));
    // }

    constexpr auto operator++(this auto& self) -> iterator& {
        if constexpr (requires { self.increment(); }) {
            self.increment();
        } else {
            return (self += 1);
        }
        return self;
    }
    constexpr auto operator--(this auto& self) -> iterator& {
        if constexpr (requires { self.decrement(); }) {
            self.decrement();
        } else {
            return (self += -1);
        }
        return self;
    }
    constexpr auto operator++(this auto& self, std::integral auto /*idx*/) -> iterator {
        iterator result{self};
        ++self;
        return result;
    }
    constexpr auto operator--(this auto& self, std::integral auto /*idx*/) -> iterator {
        iterator result{self};
        --self;
        return result;
    }
    constexpr auto operator+=(this auto& self, std::integral auto offset) -> iterator& {
        self.advance(offset);
        return self;
    }
    constexpr auto operator-=(this auto& self, std::integral auto idx) -> iterator& { return (self += (-idx)); }

    constexpr auto operator+(this auto&& self, difference_type offset) -> iterator {
        iterator result{std::forward<decltype(self)>(self)};
        result += offset;
        return result;
    }
    friend constexpr auto operator+(difference_type offset, auto&& self) -> iterator {
        return std::forward<decltype(self)>(self) + offset;
    }
    constexpr auto operator-(this auto&& self, difference_type idx) -> iterator {
        return std::forward<decltype(self)>(self) + (-idx);
    }
    template <typename Self, typename Other>
        requires std::same_as<std::remove_cvref_t<Self>, std::remove_cvref_t<Other>>
    constexpr auto operator-(this Self&& self, Other&& other) -> difference_type {
        return std::forward<Other>(other).distance_to(std::forward<Self>(self));
    }

    constexpr auto operator[](this auto const& self, difference_type idx) -> decltype(auto) { return *(self + idx); }

  private:
    IteratorFacade() = default; // default constructor is private to prevent non-CRTP instantiation
    friend Impl;                // allow Impl to access private members; this is necessary for CRTP

    // overloads for public bidirectional exposure (difference between MSVC and ClangCL)
    // template <typename Self, typename Other>
    //     requires std::same_as<std::remove_cvref_t<Self>, std::remove_cvref_t<Other>>
    // constexpr std::strong_ordering three_way_compare(this Self&& self, Other&& other) {
    //     using ConstRefSelf = std::add_lvalue_reference_t<std::add_const_t<Self>>;
    //     using ConstRefOther = std::add_lvalue_reference_t<std::add_const_t<Other>>;
    //     return std::forward_like<ConstRefSelf>(self).three_way_compare(std::forward_like<ConstRefOther>(other));
    // }
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
