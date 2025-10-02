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

    constexpr auto operator->(this auto&& self) -> decltype(auto) { return &(*std::forward<decltype(self)>(self)); }

    template <typename Self, typename Other>
        requires std::same_as<std::remove_cvref_t<Self>, std::remove_cvref_t<Other>>
    constexpr std::strong_ordering operator<=>(this Self&& self, Other&& other) {
        using CRefSelf = std::add_lvalue_reference_t<std::add_const_t<Self>>;
        using CRefOther = std::add_lvalue_reference_t<std::add_const_t<Other>>;
        return std::forward_like<CRefSelf>(self).three_way_compare(std::forward_like<CRefOther>(other));
    }
    template <typename Self, typename Other>
    constexpr bool operator==(this Self&& self, Other&& other)
        requires requires {
            { std::forward<Self>(self) <=> std::forward<Other>(other) } -> std::same_as<std::strong_ordering>;
        }
    {
        return (std::forward<Self>(self) <=> std::forward<Other>(other)) == std::strong_ordering::equivalent;
    }

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
};

} // namespace power_grid_model
