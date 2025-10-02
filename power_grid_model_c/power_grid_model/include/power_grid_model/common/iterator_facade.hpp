// SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
//
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "common.hpp"

#include <concepts>
#include <ranges>
#include <utility>

namespace power_grid_model {
namespace detail {
template <typename T>
concept iterator_facadeable_c = requires(std::remove_cvref_t<T> t) {
    { *t };
    { t <=> t } -> std::same_as<std::strong_ordering>;
    { t.advance(0) };
    { t.distance_to(t) } -> std::integral;
};
} // namespace detail

template <typename ValueType, std::integral DifferenceType> class IteratorFacade {
  public:
    using value_type = std::remove_cvref_t<ValueType>;
    using difference_type = DifferenceType;
    using iterator_category = std::random_access_iterator_tag;
    using pointer = std::add_pointer_t<ValueType>;
    using reference = std::add_lvalue_reference_t<ValueType>;

    constexpr auto operator->(this auto&& self) -> decltype(auto) { return &(*std::forward<decltype(self)>(self)); }

    template <typename Self, typename Other>
        requires std::same_as<std::remove_cvref_t<Self>, std::remove_cvref_t<Other>>
    constexpr bool operator==(this Self const& self, Other const& other) {
        return (self <=> other) == std::strong_ordering::equivalent;
    }

    constexpr auto operator++(this auto& self) -> std::add_lvalue_reference_t<decltype(self)> {
        if constexpr (requires { self.increment(); }) {
            self.increment();
        } else {
            return (self += 1);
        }
        return self;
    }
    constexpr auto operator--(this auto& self) -> std::add_lvalue_reference_t<decltype(self)> {
        if constexpr (requires { self.decrement(); }) {
            self.decrement();
        } else {
            return (self += -1);
        }
        return self;
    }
    template <typename Self>
    constexpr auto operator++(this Self& self, std::integral auto /*idx*/) -> std::remove_cvref_t<Self> {
        using Result = std::remove_cvref_t<Self>;
        Result result{self};
        ++self;
        return result;
    }
    template <typename Self>
    constexpr auto operator--(this Self& self, std::integral auto /*idx*/) -> std::remove_cvref_t<Self> {
        using Result = std::remove_cvref_t<Self>;
        Result result{self};
        --self;
        return result;
    }
    constexpr auto operator+=(this auto& self,
                              std::integral auto offset) -> std::add_lvalue_reference_t<decltype(self)> {
        self.advance(offset);
        return self;
    }
    constexpr auto operator-=(this auto& self, std::integral auto idx) -> std::add_lvalue_reference_t<decltype(self)> {
        return (self += (-idx));
    }

    template <typename Self>
    constexpr auto operator+(this Self&& self, difference_type offset) -> std::remove_cvref_t<Self> {
        using Result = std::remove_cvref_t<Self>;
        Result result{std::forward<decltype(self)>(self)};
        result += offset;
        return result;
    }
    template <typename Self>
        requires std::derived_from<std::remove_cvref_t<Self>, IteratorFacade> &&
                     detail::iterator_facadeable_c<std::remove_cvref_t<Self>>
    friend constexpr auto operator+(difference_type offset, Self&& self) -> std::remove_cvref_t<decltype(self)> {
        return std::forward<decltype(self)>(self) + offset;
    }
    constexpr auto operator-(this auto&& self, difference_type idx) -> std::remove_cvref_t<decltype(self)> {
        return (std::forward<decltype(self)>(self)) + (-idx);
    }
    template <typename Self, typename Other>
        requires std::same_as<std::remove_cvref_t<Self>, std::remove_cvref_t<Other>>
    constexpr auto operator-(this Self&& self, Other&& other) -> difference_type {
        return std::forward<Other>(other).distance_to(std::forward<Self>(self));
    }

    constexpr auto operator[](this auto const& self, difference_type idx) -> decltype(auto) { return *(self + idx); }

    // delete default, constructors of non-derived types and non-iterator-facadeable types to prevent instantiation from
    // non-derived classes
    IteratorFacade() = delete;
    // template <typename Self>
    //     requires((!std::derived_from<std::remove_cvref_t<Self>, IteratorFacade>) ||
    //              (!detail::iterator_facadeable_c<Self>))
    // IteratorFacade(Self&& self) = delete;
};

} // namespace power_grid_model
